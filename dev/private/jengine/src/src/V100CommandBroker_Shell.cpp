#include "global.h"

TDBG_DEFINE_AREA(jengbrk);

#include <V100EncCmd.h>
#include <VcomBaseTypes.h>
#include <JEngineShell.h>
#include <JEngineExec.h>

_V100_ENC_KEY_MODE ConvertKeyModes(KeyModes mode) {

    _V100_ENC_KEY_MODE retVal = KM_MODE_NONE;

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "ConvertKeyModes()");

        switch (mode) {
            case KeyModes::MODE_NONE:      retVal = KM_MODE_NONE;      break;
            case KeyModes::AES_256_CBC:    retVal = KM_AES_256_CBC;    break;
            case KeyModes::AES_128_CBC:    retVal = KM_AES_128_CBC;    break;
            case KeyModes::TDES_ABA_ECB:   retVal = KM_TDES_ABA_ECB;   break;
            case KeyModes::TDES_ABA_CBC:   retVal = KM_TDES_ABA_CBC;   break;
            case KeyModes::TDES_ABC_ECB:   retVal = KM_TDES_ABC_ECB;   break;
            case KeyModes::TDES_ABC_CBC:   retVal = KM_TDES_ABC_CBC;   break;
            case KeyModes::RSA_2048_v15:   retVal = KM_RSA_2048_v15;   break;
            case KeyModes::RSA_2048_v21:   retVal = KM_RSA_2048_v21;   break;
            case KeyModes::DUKPT_IPEK_128: retVal = KM_DUKPT_IPEK_128; break;
            case KeyModes::DUKPT_KSN_64:   retVal = KM_DUKPT_KSN_64;   break;
        }

    TTRACE_LEAVE("ConvertKeyModes() => (_V100_ENC_KEY_MODE) %d", (int)retVal);
    return retVal;
}

void Atomic_Enc_Get_KeyVersion::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Get_KeyVersion::Exec()");

        CryptExecStatus         status;
        KeyInfoStructure        keyInfo;

        if ( CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy() ) {
            TTRACE(jengbrk, TDBG_ERROR, "%s: JEngine is BUSY", __FUNCTION__);
            m_nRC = GEN_ERROR_APP_BUSY;
            goto exit;
        }

        status = JEngineExec::GetInstance()->Execute_Get_Key_Info(m_nKeySlot, keyInfo);

        switch (status) {
            case CryptExecStatus::Not_Supported:
                // Wrong key.
                TTRACE(jengbrk, TDBG_ERROR, "Get_Key_Info failed with code Not_Supported");
                m_nRC = GEN_ERROR_PARAMETER;
                break;
            case CryptExecStatus::Not_Exist:
                // Key doesn't exists.
                TTRACE(jengbrk, TDBG_ERROR, "Get_Key_Info failed with code Not_Exist");
                m_nRC = GEN_ERROR_KEY_NOT_FOUND;
                break;
            case CryptExecStatus::Successful:
                // Key exists and data received.
                m_nKeyMode = ConvertKeyModes(keyInfo.key_mode);
                m_nKeyVersion = keyInfo.key_ver;
                m_nRC = GEN_OK;
                break;
            default:
                // Unexpected error(s).
                TTRACE(jengbrk, TDBG_ERROR, "Get_Key_Info failed with unknown status: %d", (int)status );
                m_nRC = GEN_ERROR_INTERNAL;
                break;
        }

exit:;
    TTRACE_LEAVE("Atomic_Enc_Get_KeyVersion::Exec() => (void)");
}

void Atomic_Enc_Get_Rnd_Number::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Get_Rnd_Number::Exec()");

        CryptExecStatus status;

        if (CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy()) {
            // Previous operation is active now.
            TTRACE(jengbrk, TDBG_ERROR, "%s: JEngine is BUSY", __FUNCTION__);
            m_nRC = GEN_ERROR_APP_BUSY;
            goto exit;
        }

        status = JEngineExec::GetInstance()->Execute_GetRandomBuffer( &m_Rand[0], ANBIO_LENGTH );

        switch (status) {
            case CryptExecStatus::Successful:
                // Processed.
                m_nRC = GEN_OK;
                break;
            case CryptExecStatus::Invalid_Argument:
            default:
                // Unexpected error(s).
                m_nRC = GEN_ERROR_INTERNAL;
                break;
        }

exit:;
    TTRACE_LEAVE("Atomic_Enc_Get_Rnd_Number::Exec() => (void)");
}

void Atomic_Enc_Get_Key::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Get_Key::Exec()");


    CryptExecStatus         status;
    KeyInfoStructure        keyInfo;

    m_nKeyLength = 0;
    memset ( &m_KCV[0],  0x00, sizeof(m_KCV) );
    memset ( &m_pKey[0], 0x00, sizeof(u2048) );

    if (CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy()) {
        // Previous operation is active now.
        m_nRC = GEN_ERROR_APP_BUSY;
        goto exit;
    }

    status = JEngineExec::GetInstance()->Execute_Get_Key_Info(m_nArg, keyInfo);

    switch (status) {
        case CryptExecStatus::Not_Supported:
            m_nRC = GEN_ERROR_PARAMETER;
            break;
        case CryptExecStatus::Not_Exist:
            m_nRC = GEN_ERROR_KEY_NOT_FOUND;
            break;
        case CryptExecStatus::Successful:
            m_nKeyVersion = keyInfo.key_ver;
            m_nKeyMode = ConvertKeyModes(keyInfo.key_mode);
            m_nRC = GEN_OK;
            break;
        default:
            m_nRC = GEN_ERROR_INTERNAL;
            break;
    }

    if (GEN_OK == m_nRC) {
        if (KT_EXTKEY_DEVICE_PUBLIC == m_nArg) {

            uint8_t* key_data = nullptr;
            uint32_t key_data_len = 0;

            status = JEngineExec::GetInstance()->Execute_Get_RSA_PublicKey(&key_data, key_data_len);
            if (CryptExecStatus::In_Progress == status) {
                m_nRC = GEN_ERROR_APP_BUSY;
            } else
            if (CryptExecStatus::Not_Exist == status) {
                m_nRC = GEN_ERROR_KEY_NOT_FOUND;
            } else
            if (CryptExecStatus::Successful == status) {
                if ( key_data_len > sizeof(m_pKey) ) {
                    m_nRC = GEN_ERROR_INTERNAL;
                } else {
                    m_nKeyLength = key_data_len;
                    memcpy(&m_pKey[0], key_data, key_data_len);
                }
            } else {
                m_nRC = GEN_ERROR_PROCESSING;
            }

            JEngineExec::GetInstance()->Delete_Buffer(&key_data);
        }
    }

exit:;
    TTRACE_LEAVE("Atomic_Enc_Get_Key::Exec() => (void)");
}

void Macro_Enc_Generate_RSA_Keys::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Macro_Enc_Generate_RSA_Keys::Exec()");


    CryptExecStatus     status;
    KeyInfoStructure    keyInfo;

    if (CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy()) {
        // Previous operation is active now.
        m_nRC = GEN_ERROR_APP_BUSY;
        goto exit;
    }

    status = JEngineExec::GetInstance()->Execute_Get_Key_Info(KT_EXTKEY_CTK, keyInfo);

    if ( CryptExecStatus::Successful == status ) {
        // Transport key exists.
        // It is expected CTK will be programmed after of RSA keys.
        m_nRC = GEN_ERROR_ACCESS_DENIED;
        goto exit;
    }

    status = JEngineExec::GetInstance()->Execute_Generate_RSA_Key();

    switch (status) {
        case CryptExecStatus::Successful:
            m_nRC = GEN_OK;
            break;
        case CryptExecStatus::Execute_Error:
        default:
            m_nRC = GEN_ERROR_INTERNAL;
            break;
    }

exit:;
    TTRACE_LEAVE("Macro_Enc_Generate_RSA_Keys::Exec() => (void)");
}

void Atomic_Get_OP_Status::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Get_OP_Status::Exec()");

    CryptExecStatus     status;
    _V100_OP_STATUS     res; // = { 0 };

    status = JEngineExec::GetInstance()->Get_Busy();

    switch (status) {
        case CryptExecStatus::Successful:
            res.nMode = OP_COMPLETE;
            break;
        case CryptExecStatus::In_Progress:
            res.nMode = OP_IN_PROGRESS;
            break;
        case CryptExecStatus::Execute_Error:
        default:
            res.nMode = OP_ERROR;
            break;
    }

    SetOPStatus( &res);

    TTRACE_LEAVE("Atomic_Get_OP_Status::Exec() => (void)");
}

void Atomic_Enc_Set_Key::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Set_Key::Exec()");

    CryptExecStatus     status;
    KeyInfoStructure    keyInfo;
    JEngineSetKeyCtx keyCtx;

    // Prevent the cross-command processing.
    if (CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy()) {
        // Previous operation is active now.
        m_nRC = GEN_ERROR_APP_BUSY;
        goto exit;
    }


    keyCtx.ctx          = GetBuffer();
    keyCtx.ctxLen       = GetBufferSize();
    keyCtx.keySlotCmd   = m_nKeyType;

    status = JEngineExec::GetInstance()->Execute_Set_Key ( keyCtx );

    m_nBufferSize = 0;

    if (m_pBuffer) {
        FREE(m_pBuffer);
        m_pBuffer = nullptr;
    }

    if (keyCtx.respLen > 0) {
        m_pBuffer = (u8*)MALLOC(keyCtx.respLen);
        if (m_pBuffer) {
            m_nBufferSize = keyCtx.respLen;
            memcpy(m_pBuffer, keyCtx.respVal, m_nBufferSize);
        }
    }

    switch (status) {

        case CryptExecStatus::Successful:
            m_nRC = GEN_OK;
            break;

        case CryptExecStatus::Cmd_Rejected:
            m_nRC = GEN_ERROR_ACCESS_DENIED;
            break;

        case CryptExecStatus::Execute_Error:
        default:
            m_nRC = GEN_ERROR_INTERNAL;
            break;
    }

exit:;
    TTRACE_LEAVE("Atomic_Enc_Set_Key::Exec() => void");
}

void Atomic_Enc_Factory_Set_Key::Exec() {
    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Factory_Set_Key::Exec()");
    TTRACE_LEAVE("Atomic_Enc_Factory_Set_Key::Exec() => (void)");
}

void Atomic_Enc_Unlock_Key::Exec() {

    TTRACE_ENTER(jengbrk, TDBG_DEBUG, "Atomic_Enc_Unlock_Key::Exec()");

    CryptExecStatus         status;

    uchar* pCGPacket;
    uint   nSize;

    GetCryptogram(&pCGPacket, nSize);

    if (CryptExecStatus::In_Progress == JEngineExec::GetInstance()->Get_Busy()) {
        // Previous operation is active now.
        m_nRC = GEN_ERROR_APP_BUSY;
        goto exit;
    }

    status = JEngineExec::GetInstance()->Execute_UnlockStorage(pCGPacket, nSize);

    switch (status) {

        case CryptExecStatus::Cmd_Rejected:
            m_nRC = GEN_ERROR_VERIFY;
            break;

        case CryptExecStatus::Successful:
            m_nRC = GEN_OK;
            break;

        default:
            m_nRC = GEN_ERROR_INTERNAL;
            break;
    }

exit:;
    TTRACE_LEAVE("Atomic_Enc_Unlock_Key::Exec() => (void)");
}

