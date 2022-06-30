#include "global.h"

TDBG_DEFINE_AREA(jengexec);

#include <cassert>
#include <sstream>
#include <iomanip>

#include <JEngineExec.h>
#include <HFApi.h>
#include <HFApiSimulation.h>
#include <HFTypesPrivate.h>
#include <V100_enc_types.h>

#include <application/stuff.h>

//-------------------------------------------------------------------------------------------//

#define  DETAILED_LOG_ENABLED       (1)
#define  KT_KEY_CMPK                (0x2000)
#define  JAGUAL_HOST_SIMULATION     (0)

//-------------------------------------------------------------------------------------------//

JEngineExec*    JEngineExec::m_instance_      = nullptr;
HBSEClient      JEngineExec::m_hbse_client_;
std::mutex      JEngineExec::m_race_mutex;

//-------------------------------------------------------------------------------------------//

static void LoadValue(const uint8_t* const src, int& offset, void* const dst, int len) {

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "LoadValue()");
        uint8_t* const ptr = (uint8_t*)dst;
        memcpy(ptr, src + offset, len);
        offset += len;
    TTRACE_LEAVE("LoadValue() => (void)");
}

static u32 GetKeySize(u16 slot, u16 nKeyMode) {

    u32 nKeySize = 0;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "GetKeySize()");

    switch (slot) {

        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
            nKeySize = 256;
            break;

        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
            nKeySize = 128;
            break;

        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
            nKeySize = 32;
            break;

        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            nKeySize = 24;
            break;

        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
            nKeySize = 16;
            break;

        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
            nKeySize = 8;
            break;

        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
            nKeySize = 4;
            break;

        case KT_EXT_BSK:
        case KT_EXTKEY_BTK:
        case KT_EXTKEY_CTK:
        case KT_MSK_MKD:
        case KT_MSK_SK:

            switch (nKeyMode) {
                case KM_AES_256_CBC:
                    nKeySize = 32;
                    break;
                case KM_TDES_ABC_ECB:
                case KM_TDES_ABC_CBC:
                    nKeySize = 24;
                    break;
                case KM_AES_128_CBC:
                case KM_TDES_ABA_ECB:
                case KM_TDES_ABA_CBC:
                    nKeySize = 16;
                    break;
                default:
                    break;
            }

            break;

        default:
            break;
    }

    TTRACE_LEAVE("GetKeySize() => void");

    return nKeySize;
}

static void logHfRes(HFData** const hfData) {

    UNUSED(hfData);

    #if 0

        TTRACE_ENTER(jengexec, TDBG_DEBUG, "logHfRes()");
        if (nullptr != hfData) {

            const HFResult* const hfImg =  static_cast<HFResult*> ( (*hfData)->data );

            info ( "HFRES begin" );
            info ( "    flags:         %ld ", hfImg->validFlags );
            info ( "    status:        %d  ", hfImg->contextStatus );
            info ( "    error:         %d  ", hfImg->errorCode );
            info ( "    encoding:      %d  ", hfImg->image.imageEncoding);
            info ( "    sequence:      %d  ", hfImg->sequenceNumber);
            info ( "    fdCnt:         %d  ", hfImg->facesDetectedCount);
            info ( "    quality:       %f  ", hfImg->quality);
            info ( "    bBox x1:       %d  ", hfImg->boundBox.upperLeft.x);
            info ( "    bBox y1:       %d  ", hfImg->boundBox.upperLeft.y);
            info ( "    bBox x2:       %d  ", hfImg->boundBox.bottomRight.x);
            info ( "    bBox y2:       %d  ", hfImg->boundBox.bottomRight.y);
            info ( "    spoof:         %f  ", hfImg->spoofProbability);
            info ( "    captured:      %d  ", hfImg->isCaptured);
            info ( "    lmLeftEye_x:   %d  ", hfImg->landmarks.leftEye.x);
            info ( "    lmLeftEye_y:   %d  ", hfImg->landmarks.leftEye.y);
            info ( "    lmRightEye_x:  %d  ", hfImg->landmarks.rightEye.x);
            info ( "    lmRightEye_y:  %d  ", hfImg->landmarks.rightEye.y);
            info ( "    lmLMouth_x:    %d  ", hfImg->landmarks.mouthLeft.x);
            info ( "    lmLMouth_y:    %d  ", hfImg->landmarks.mouthLeft.y);
            info ( "    lmRMouth_x:    %d  ", hfImg->landmarks.mouthRight.x);
            info ( "    lmRMouth_y:    %d  ", hfImg->landmarks.mouthRight.y);
            info ( "    nose_x:        %d  ", hfImg->landmarks.nose.x);
            info ( "    nose_y:        %d  ", hfImg->landmarks.nose.y);
            info ( "    img Len:       %d  ", hfImg->image.data.size);
            info ( "    tpl Len:       %d  ", hfImg->templ.size);
            info ( "HFRES end" );
        }

       TTRACE_LEAVE("logHfRes() => void");

    #endif
}

//-------------------------------------------------------------------------------------------//

JEngineSetKeyCtx::JEngineSetKeyCtx() {
    TTRACE_ENTER_NT(jengexec, TDBG_DEBUG, "JEngineSetKeyCtx()" );
    do_cleanup();
    TTRACE_LEAVE_NT("JEngineSetKeyCtx() => void");
}

JEngineSetKeyCtx::~JEngineSetKeyCtx() {
    TTRACE_ENTER_NT(jengexec, TDBG_DEBUG, "~JEngineSetKeyCtx()");
    do_cleanup();
    TTRACE_LEAVE_NT("~JEngineSetKeyCtx() => void");
}

bool JEngineSetKeyCtx::CtxValidate() {
    TTRACE_ENTER(jengexec, TDBG_DEBUG, "CtxValidate()" );
    TTRACE_LEAVE("CtxValidate() => void");
    return true;
}

void JEngineSetKeyCtx::do_cleanup() {

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "do_cleanup()" );

    ctx = nullptr;

    ctxLen       = 0;
    keySlotCmd   = 0;
    keySlotPack  = 0;
    keyVersion   = 0;
    keyCryptoLen = 0;
    keyMode      = 0;
    keyLen       = 0;
    respLen      = 0;

    memset(localAnbio,  0x00, sizeof(u128));
    memset(remoteAnbio, 0x00, sizeof(u128));
    memset(remoteAnsol, 0x00, sizeof(u128));
    memset(key_kcv,     0x00, sizeof(key_kcv));
    memset(keyVal,      0x00, sizeof(u2048));
    memset(respVal,     0x00, sizeof(u2048));

    TTRACE_LEAVE("do_cleanup() => (void)");
}

//-------------------------------------------------------------------------------------------//

JEngineExec::JEngineExec() {

    TTRACE_ENTER_NT(jengexec, TDBG_DEBUG, "JEngineExec()" );

    m_key_rsa_state_ = KeyState::KeyStateUnknow;
    m_key_ctx_state_ = KeyState::KeyStateUnknow;
    m_key_btk_state_ = KeyState::KeyStateUnknow;

    TTRACE_LEAVE_NT("JEngineExec() => void");
}

JEngineExec::~JEngineExec() {
    TTRACE_ENTER_NT(jengexec, TDBG_DEBUG, "~JEngineExec()" );

    TTRACE_LEAVE_NT("~JEngineExec() => void");
}

V100_ERROR_CODE JEngineExec::map_error(const char* const fName, int line, int jag_error_code) {

    V100_ERROR_CODE retVal = GEN_OK;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::map_error()" );

    switch (jag_error_code) {
        case HFERROR_OK:
            retVal = GEN_OK;
            break;
        case HFERROR_GENERAL:
            retVal = GEN_ERROR_JAG_GENERAL;
            break;
        case HFERROR_BUSY:
            retVal = GEN_ERROR_JAG_BUSY;
            break;
        case HFERROR_NOT_FOUND:
            retVal = GEN_ERROR_JAG_NOT_FOUND;
            break;
        case HFERROR_ALREADY_PRESENT:
            retVal = GEN_ERROR_JAG_ALREADY_PRESENT;
            break;
        case HFERROR_NOT_INITIALIZED:
            retVal = GEN_ERROR_JAG_NOT_INITIALIZED;
            break;
        case HFERROR_ALREADY_INITIALIZED:
            retVal = GEN_ERROR_JAG_ALREADY_INITIALIZED;
            break;
        case HFERROR_ARGUMENT_NULL:
            retVal = GEN_ERROR_JAG_ARGUMENT_NULL;
            break;
        case HFERROR_ARGUMENT_INVALID:
            retVal = GEN_ERROR_JAG_ARGUMENT_INVALID;
            break;
        case HFERROR_NOT_IMPLEMENTED:
            retVal = GEN_ERROR_JAG_NOT_IMPLEMENTED;
            break;
        case HFERROR_ACCESS_DENIED:
            retVal = GEN_ERROR_JAG_ACCESS_DENIED;
            break;
        case HFERROR_HW_ERROR:
            retVal = GEN_ERROR_JAG_HW_ERROR;
            break;
        case HFERROR_COMM_ERROR:
            retVal = GEN_ERROR_JAG_COMM_ERROR;
            break;
        case HFERROR_PREREQUISITES_NOT_MET:
            retVal = GEN_ERROR_JAG_PREREQUISITES_NOT_MET;
            break;
        case HFERROR_TIMEOUT:
            retVal = GEN_ERROR_JAG_TIMEOUT;
            break;
        case HFERROR_ABORTED:
            retVal = GEN_ERROR_JAG_ABORTED;
            break;
        case HFERROR_UNKNOWN_USER:
            retVal = GEN_ERROR_JAG_UNKNOWN_USER;
            break;
        case HFERROR_ALLOCATION_FAILED:
            retVal = GEN_ERROR_JAG_ALLOCATION_FAILED;
            break;
        default:
            TTRACE(jengexec, TDBG_ERROR, "Failed to map error JAGUAR err code %d. Called: (%s):(%d)", jag_error_code, fName, line);
            retVal = GEN_ERROR_PROCESSING;
            break;
    }

    TTRACE_LEAVE("JEngineExec::map_error() => V100_ERROR_CODE %d", retVal);
    return retVal;
}

JEngineExec* JEngineExec::GetInstance() {

    JEngineExec* retVal = nullptr;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::GetInstance()" );
        {   const std::lock_guard<std::mutex> lock(m_race_mutex);
            if (nullptr == m_instance_) {
                m_instance_ = new (JEngineExec);
            }
            retVal = m_instance_;
        }
    TTRACE_LEAVE("JEngineExec::GetInstance() => (JEngineExec*) %p", retVal);

    return retVal;
}

//-------------------------------------------------------------------------------------------//

bool JEngineExec::Start() {

    bool res = false;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Start()" );
        if ( ! m_hbse_client_.Connect() ) {
            TTRACE(jengexec, TDBG_ERROR, "%s: Failed to connect to hbse_client", __FUNCTION__ );
        } else {
            TTRACE(jengexec, TDBG_TRACE, "%s Connected to hbse_client", __FUNCTION__);
            res = true;
        }
    TTRACE_LEAVE("JEngineExec::Start() => (bool): %d", res);

    return res;
}

void JEngineExec::Stop() {

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Stop()" );
        m_hbse_client_.Disconnect();
    TTRACE_LEAVE("JEngineExec::Stop() => (void)");
}

void JEngineExec::Delete_Buffer(uint8_t** buffer) {

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Delete_Buffer()" );
        m_hbse_client_.Delete_Buffer(buffer);
    TTRACE_LEAVE("JEngineExec::Delete_Buffer() => (void)");
}

CryptExecStatus JEngineExec::Get_Busy() {

    CryptExecStatus retVal;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Get_Busy()" );

    {   const std::lock_guard<std::mutex> lock(m_race_mutex);

        if (m_pending_cmd_.size() == 0) {
            retVal = CryptExecStatus::Successful;
            goto exit;
        }

        std::future_status          cmd_status;
        std::chrono::milliseconds   dly(1);

        cmd_status = m_pending_cmd_.front().wait_for(dly);

        switch (cmd_status) {
            case std::future_status::deferred:
                // ASYNC command failed to start / created as DEFERRED.
                // Logic error. Not expected.
                TTRACE(jengexec, TDBG_ERROR, "Pending command: <Deferred>");
                assert(false);
                break;
            case std::future_status::ready:
                // Command successfully done.
                TTRACE(jengexec, TDBG_TRACE, "Pending command complete.");
                retVal = CryptExecStatus::Successful;
                m_pending_cmd_.pop_front();
                break;
            case std::future_status::timeout:
                // Command still running.
                TTRACE(jengexec, TDBG_TRACE, "Pending command still active.");
                retVal = CryptExecStatus::In_Progress;
                break;
        }
    }

exit:;
    TTRACE_LEAVE("JEngineExec::Get_Busy() => (CryptExecStatus): %d", (int)retVal );
    return retVal;
}

CryptExecStatus JEngineExec::Execute_GetRandomBuffer ( uint8_t* dst_buffer, uint32_t dst_buffer_len ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_GetRandomBuffer" );

        if (dst_buffer_len != ANBIO_LENGTH) {
            TTRACE(jengexec, TDBG_ERROR, "Invalid argument dst_buffer_len");
            retVal = CryptExecStatus::Invalid_Argument;
            goto exit;
        }

        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, "Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        retVal = m_hbse_client_.Execute_GetRandomBuffer(m_anbio_, ANBIO_LENGTH);
        if (CryptExecStatus::Successful != retVal ) {
            TTRACE(jengexec, TDBG_ERROR, "hbse_client failed. Code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        memcpy(dst_buffer, m_anbio_, ANBIO_LENGTH);
        retVal = CryptExecStatus::Successful;

exit:;
    TTRACE_LEAVE("JEngineExec::Execute_GetRandomBuffer() => (CryptExecStatus): %d", (int)retVal );
    return retVal;
}

CryptExecStatus JEngineExec::Execute_Get_Key_Info ( uint32_t key_slot, KeyInfoStructure& out_key_version ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_Get_Key_Info()" );

        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, "Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        retVal = m_hbse_client_.Execute_Get_Key_Info(key_slot, out_key_version);
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, "Execute_Get_Key_Info failed. Code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

exit:;
    TTRACE_LEAVE("JEngineExec::Execute_Get_Key_Info() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::Execute_Get_RSA_PublicKey ( uint8_t** out_buffer, uint32_t& out_buffer_len ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_Get_RSA_PublicKey()" );

        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, "Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        do_update_hbse_state();

        if ( KeyState::KeyStateValid != m_key_rsa_state_ ) {
            TTRACE(jengexec, TDBG_WARN, " RSA key not installed. Not personalized?");
            retVal = CryptExecStatus::Not_Exist;
        }

        retVal = m_hbse_client_.Execute_Get_RSA_PublicKey(out_buffer, out_buffer_len);
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, "Execute_Get_RSA_PublicKey failed. Code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

exit:;
    TTRACE_LEAVE("JEngineExec::Execute_Get_RSA_PublicKey() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::Execute_Generate_RSA_Key ( void ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_Generate_RSA_Key()" );

        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, "Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        do_update_hbse_state();

        if (KeyState::KeyStateValid == m_key_btk_state_) {
            TTRACE(jengexec, TDBG_ERROR, "BTK key already installed. ");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        if (KeyState::KeyStateValid == m_key_ctx_state_) {
            TTRACE(jengexec, TDBG_ERROR, "CTX key already installed. ");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        if (KeyState::KeyStateValid == m_key_rsa_state_) {
            // Not an error. It's allowed in the state NON-PROVISIONED.
            TTRACE(jengexec, TDBG_TRACE, "RSA key already installed. ");
        }

        // Start the generation of RSA key. Could take a lot of time.
        {   const std::lock_guard<std::mutex> lock(m_race_mutex);
            std::future<CryptExecStatus> request = std::async(std::launch::async, [this] { return do_gen_rsa_key(this); });
            m_pending_cmd_.push_back(std::move(request));
        }

        retVal = CryptExecStatus::Successful;

exit:;
    TTRACE_LEAVE("JEngineExec::Execute_Generate_RSA_Key() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::Execute_Set_Key ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_Set_Key()" );

        KeyInfoStructure keyInfo;
    
        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, "Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        switch (keyCtx.keySlotCmd) {

            // Keys aren't exposed. Not allowed to manipulate directly.
            case KT_EXTKEY_HOST_PUBLIC:
            case KT_EXTKEY_DEVICE_PUBLIC:
            case KT_EXTKEY_DEVICE_PRIVATE:
            case KT_EXTKEY_DEVICE_P:
            case KT_EXTKEY_DEVICE_Q:
            case KT_EXTKEY_PUBLIC_EXP:
            case KT_EXTKEY_PRIVATE_EXP:
                TTRACE(jengexec, TDBG_ERROR, " not allowed for slot: %d", keyCtx.keySlotCmd);
                retVal = CryptExecStatus::Cmd_Rejected;
                break;

                // Customer Provisioning Key.
            case KT_EXTKEY_CTK:
                TTRACE(jengexec, TDBG_TRACE, " Attempt to install CTK");
                retVal = do_set_key_ctk(keyCtx);
                if (retVal != CryptExecStatus::Successful) {
                    TTRACE(jengexec, TDBG_ERROR, " CTK failed. Code: %d", (int)retVal);
                    retVal = CryptExecStatus::Cmd_Rejected;
                }
                break;

                // Back-end transport key.
            case KT_EXTKEY_BTK:
                TTRACE(jengexec, TDBG_TRACE, " Attempt to install BTK");
                retVal = do_set_key_btk(keyCtx);
                if (retVal != CryptExecStatus::Successful) {
                    TTRACE(jengexec, TDBG_ERROR, " BTK failed. Code: %d", (int)retVal);
                    retVal = CryptExecStatus::Cmd_Rejected;
                }
                break;

                // Vendor keys.
            case KT_EXTKEY_AES0:
            case KT_EXTKEY_AES1:
            case KT_EXTKEY_AES2:
            case KT_EXTKEY_AES3:
            case KT_EXTKEY_AES_VEND:
            case KT_EXTKEY_TDES0:
            case KT_EXTKEY_TDES1:
            case KT_EXTKEY_TDES2:
            case KT_EXTKEY_TDES3:
                TTRACE(jengexec, TDBG_TRACE, " Attempt to install VENDOR key: %d", keyCtx.keySlotCmd);
                retVal = do_set_key_vendor(keyCtx);
                if (retVal != CryptExecStatus::Successful) {
                    TTRACE(jengexec, TDBG_ERROR, " VENDOR failed. Code: %d", (int)retVal);
                    retVal = CryptExecStatus::Cmd_Rejected;
                }
                break;

            // Session key (Device generated).
            // Expecting operation will be rejected.
            case KT_EXT_DSK:
                TTRACE(jengexec, TDBG_TRACE, " Attempt to install DSK key: %d", keyCtx.keySlotCmd);
                retVal = do_set_key_dsk(keyCtx);
                if (retVal != CryptExecStatus::Successful) {
                    TTRACE(jengexec, TDBG_ERROR, " DSK failed. Code: %d", (int)retVal);
                    retVal = CryptExecStatus::Cmd_Rejected;
                }
                break;

            // Session key (Host generated).
            case KT_EXT_BSK:
                TTRACE(jengexec, TDBG_TRACE, " Attempt to install BSK key: %d", keyCtx.keySlotCmd);
                retVal = do_set_key_bsk(keyCtx);
                if (retVal != CryptExecStatus::Successful) {
                    TTRACE(jengexec, TDBG_ERROR, " BSK failed. Code: %d", (int)retVal);
                    retVal = CryptExecStatus::Cmd_Rejected;
                }
                break;

            default:
                TTRACE(jengexec, TDBG_ERROR, " Rejected to install unknown key: %d", (int)keyCtx.keySlotCmd);
                retVal = CryptExecStatus::Cmd_Rejected;
                break;
        }

exit:;
    TTRACE_LEAVE("JEngineExec::Execute_Set_Key() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::Execute_UnlockStorage (uint8_t* pPacket, uint iLen ) {

    CryptExecStatus retVal  = CryptExecStatus::Execute_Error;

    uint8_t* plain_text     = nullptr;
    uint32_t plain_text_len = 0;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Execute_UnlockStorage()" );

        bool eraseAllowed = false;

        retVal = Get_Busy();
        if (retVal != CryptExecStatus::Successful) {
            TTRACE(jengexec, TDBG_ERROR, " Pending command");
            retVal = CryptExecStatus::In_Progress;
            goto exit;
        }

        retVal = m_hbse_client_.Execute_Select_Key(KT_EXTKEY_CTK);
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " CTK key not found");
            goto exit;
        }

        m_hbse_client_.Execute_Decrypt(pPacket, iLen, &plain_text, plain_text_len, nullptr, 0);

        if (nullptr == plain_text) {
            TTRACE(jengexec, TDBG_ERROR, " Cannot decrypt key");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        if (plain_text_len != ANBIO_LENGTH) {
            TTRACE(jengexec, TDBG_ERROR, " Wrong length of ANBIO.");
            m_hbse_client_.Delete_Buffer( &plain_text );
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        eraseAllowed = memcmp(plain_text, m_anbio_, ANBIO_LENGTH) == 0;
        if ( ! eraseAllowed ) {
            TTRACE(jengexec, TDBG_ERROR, " Invalid ANBIO.");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        m_key_rsa_state_ = KeyState::KeyStateUnknow;
        m_key_ctx_state_ = KeyState::KeyStateUnknow;
        m_key_btk_state_ = KeyState::KeyStateUnknow;

        retVal = m_hbse_client_.Execute_Reset_Secure_Element();
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Reset_Secure_Element failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        retVal = CryptExecStatus::Successful;

exit:;
        m_hbse_client_.Delete_Buffer( &plain_text );

    TTRACE_LEAVE("JEngineExec::Execute_UnlockStorage() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

//-------------------------------------------------------------------------------------------//

CryptExecStatus JEngineExec::do_gen_rsa_key ( JEngineExec* instance ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_gen_rsa_key()" );

        retVal = instance->m_hbse_client_.Execute_Generate_RSA_Key();

        if ( CryptExecStatus::Successful == retVal ) {
            TTRACE(jengexec, TDBG_TRACE, "New RSA key installed.");
            instance->m_key_rsa_state_ = KeyState::KeyStateValid;
        } else {
            TTRACE(jengexec, TDBG_ERROR, "Generate_RSA_Key failed with code: %d", (int)retVal );
            retVal = CryptExecStatus::Execute_Error;
        }

    TTRACE_LEAVE("JEngineExec::do_gen_rsa_key() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_ctk ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_ctk()" );

    do_update_hbse_state();

    if (KeyState::KeyStateValid == m_key_btk_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. BTK key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    if (KeyState::KeyStateValid == m_key_ctx_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. CTK key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    if (KeyState::KeyStateValid != m_key_rsa_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. RSA key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    retVal = do_set_key_common ( keyCtx, KT_EXTKEY_DEVICE_PRIVATE );

    if (CryptExecStatus::Successful != retVal ) {
        TTRACE(jengexec, TDBG_ERROR, " set_key failed with code: %d", (int)retVal);
        goto exit;
    }

    m_key_ctx_state_ = KeyState::KeyStateValid;
    retVal = CryptExecStatus::Successful;

exit:;
    TTRACE_LEAVE("JEngineExec::do_set_key_ctk() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_btk ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_btk()");

        do_update_hbse_state();

        if (KeyState::KeyStateValid != m_key_rsa_state_) {
            TTRACE(jengexec, TDBG_WARN, " Rejected. RSA key not found.");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        if (KeyState::KeyStateValid != m_key_ctx_state_) {
            TTRACE(jengexec, TDBG_WARN, " Rejected. CTK key not found.");
            retVal = CryptExecStatus::Cmd_Rejected;
            goto exit;
        }

        retVal = do_set_key_common(keyCtx, KT_EXTKEY_CTK);

        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " set_key failed with code: %d", (int)retVal);
            goto exit;
        }

        m_key_ctx_state_ = KeyState::KeyStateValid;
        retVal = CryptExecStatus::Successful;

exit:;
    TTRACE_LEAVE("JEngineExec::do_set_key_btk() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_vendor ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_vendor()" );

    UNUSED(keyCtx);

    do_update_hbse_state();

    if (KeyState::KeyStateValid != m_key_rsa_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. RSA key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    if (KeyState::KeyStateValid != m_key_ctx_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. CTK key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    if (KeyState::KeyStateValid != m_key_btk_state_) {
        TTRACE(jengexec, TDBG_WARN, " Rejected. BTK key not found.");
        retVal = CryptExecStatus::Cmd_Rejected;
        goto exit;
    }

    retVal = do_set_key_common(keyCtx, KT_EXTKEY_BTK);

    if (CryptExecStatus::Successful != retVal) {
        TTRACE(jengexec, TDBG_ERROR, " set_key failed with code: %d", (int)retVal);
        goto exit;
    }

    retVal = CryptExecStatus::Successful;

exit:;
    TTRACE_LEAVE("JEngineExec::do_set_key_vendor() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_dsk ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_dsk()" );
        UNUSED(keyCtx);
        TTRACE_LEAVE("JEngineExec::do_set_key_dsk() => (CryptExecStatus): %d", (int)retVal);

    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_bsk ( JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_bsk()" );
        UNUSED(keyCtx);
    TTRACE_LEAVE("JEngineExec::do_set_key_bsk() => (CryptExecStatus): %d", (int)retVal);

    return retVal;
}

CryptExecStatus JEngineExec::do_set_key_common(JEngineSetKeyCtx& keyCtx, int iParentKey) {

    CryptExecStatus retVal          = CryptExecStatus::Execute_Error;
    uint8_t*        out_buffer      = nullptr;
    uint32_t        out_buffer_len  = 0;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_set_key_common()" );

        KeyInfoStructure keyInfo;
        int offset;

        retVal = m_hbse_client_.Execute_Select_Key(iParentKey);
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Select_Key failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        retVal = m_hbse_client_.Execute_Decrypt ( keyCtx.ctx, keyCtx.ctxLen, &out_buffer, out_buffer_len, nullptr, 0 );
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Execute_Decrypt failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        offset = 0;
        LoadValue ( out_buffer, offset,  keyCtx.remoteAnbio,  sizeof(u128) ); // ANBIO(16 bytes)
        LoadValue ( out_buffer, offset,  keyCtx.remoteAnsol,  sizeof(u128) ); // ANSOL(16 bytes)
        LoadValue ( out_buffer, offset, &keyCtx.keySlotPack,  sizeof(u16)  ); // Slot(2 bytes)
        LoadValue ( out_buffer, offset, &keyCtx.keyVersion,   sizeof(u16)  ); // Version(2 bytes)
        LoadValue ( out_buffer, offset, &keyCtx.keyMode,      sizeof(u16)  ); // Mode(2 bytes)
        LoadValue ( out_buffer, offset,  keyCtx.key_kcv,      KCV_LENGTH   ); // KCV(4 bytes)
        LoadValue ( out_buffer, offset, &keyCtx.keyCryptoLen, sizeof(u16)  ); // KeyCryptoSize(2 bytes) + KeyVal(variable size)

        keyCtx.keyLen = GetKeySize ( keyCtx.keySlotPack, keyCtx.keyMode );
        if (keyCtx.keyLen > (int) (out_buffer_len - offset)) {
            TTRACE(jengexec, TDBG_ERROR, " GetKeySize failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        LoadValue ( out_buffer, offset, keyCtx.keyVal, keyCtx.keyLen );

        m_hbse_client_.Delete_Buffer(&out_buffer);
        out_buffer = nullptr;

        retVal = do_validate_ctx ( keyCtx );
        if ( CryptExecStatus::Successful != retVal ) {
            TTRACE(jengexec, TDBG_ERROR, " validate_ctx failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        keyInfo.slot     = keyCtx.keySlotPack;
        keyInfo.key_ver  = keyCtx.keyVersion;
        keyInfo.key_mode = (KeyModes) keyCtx.keyMode;
        keyInfo.key_size = keyCtx.keyLen;

        // Programming the CTK slot.
        retVal = m_hbse_client_.Execute_Set_Key(keyInfo, keyCtx.keyVal, keyCtx.keyLen);
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Set_Key failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        // Preparing the answer (Select the key for Encrypting the ANSOL).
        retVal = m_hbse_client_.Execute_Select_Key ( keyInfo.slot );
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Select_Key failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        // Preparing the answer (Encrypting the ANSOL).
        retVal = m_hbse_client_.Execute_Encrypt ( keyCtx.remoteAnsol, sizeof(u128), &out_buffer, out_buffer_len, nullptr, 0 );
        if (CryptExecStatus::Successful != retVal) {
            TTRACE(jengexec, TDBG_ERROR, " Execute_Encrypt failed with code: %d", (int)retVal);
            retVal = CryptExecStatus::Execute_Error;
            goto exit;
        }

        // Store the response to Key context.
        if ( out_buffer_len <= sizeof(u2048) ) {
            keyCtx.respLen = out_buffer_len;
            memcpy(keyCtx.respVal, out_buffer, out_buffer_len);
        }

        retVal = CryptExecStatus::Successful;

exit:;
    m_hbse_client_.Delete_Buffer(&out_buffer);
    TTRACE_LEAVE("JEngineExec::do_set_key_common() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_update_hbse_state ( void ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_update_hbse_state()" );

        KeyInfoStructure keyInfo;

        // Check RSA key.
        if (KeyState::KeyStateUnknow == m_key_rsa_state_) {

            TTRACE(jengexec, TDBG_TRACE, "RSA key state: UNKNOWN");

            m_key_rsa_state_ = KeyState::KeyStateInvalid;

            retVal = m_hbse_client_.Execute_Get_Key_Info(KT_EXTKEY_DEVICE_PRIVATE, keyInfo);
            if (CryptExecStatus::Successful != retVal) {
                TTRACE(jengexec, TDBG_WARN, " Cannot Get_Key_Info(KT_EXTKEY_DEVICE_PRIVATE) failed with code: %d", (int)retVal );
            } else {
                retVal = m_hbse_client_.Execute_Get_Key_Info(KT_EXTKEY_DEVICE_PUBLIC, keyInfo);
                if (CryptExecStatus::Successful != retVal) {
                    TTRACE(jengexec, TDBG_WARN, " Cannot Get_Key_Info(KT_EXTKEY_DEVICE_PUBLIC) failed with code: %d", (int)retVal);
                } else {
                    TTRACE(jengexec, TDBG_TRACE, "RSA key state: Key valid");
                    m_key_rsa_state_ = KeyState::KeyStateValid;
                }
            }
        }

        if (KeyState::KeyStateUnknow == m_key_ctx_state_) {

            TTRACE(jengexec, TDBG_TRACE, "CTK key state: UNKNOWN");

            m_key_ctx_state_ = KeyState::KeyStateInvalid;

            retVal = m_hbse_client_.Execute_Get_Key_Info(KT_EXTKEY_CTK, keyInfo);

            if (CryptExecStatus::Successful != retVal) {
                TTRACE(jengexec, TDBG_WARN, "CTK key state: invalid");
            } else {
                TTRACE(jengexec, TDBG_TRACE, "CTK key state: valid");
                m_key_ctx_state_ = KeyState::KeyStateValid;
            }
        }

        if (KeyState::KeyStateUnknow == m_key_btk_state_) {

            TTRACE(jengexec, TDBG_TRACE, "BTK key state: UNKNOWN");

            m_key_btk_state_ = KeyState::KeyStateInvalid;
            retVal = m_hbse_client_.Execute_Get_Key_Info(KT_EXTKEY_CTK, keyInfo);
            if (CryptExecStatus::Successful != retVal) {
                TTRACE(jengexec, TDBG_WARN, "BTK key state: invalid");
            } else {
                TTRACE(jengexec, TDBG_TRACE, "BTK key state: valid");
                m_key_btk_state_ = KeyState::KeyStateValid;
            }
        }

        retVal = CryptExecStatus::Successful;

    TTRACE_LEAVE("JEngineExec::do_update_hbse_state() => (CryptExecStatus): %d", (int)retVal);
    return retVal;
}

CryptExecStatus JEngineExec::do_validate_ctx( const JEngineSetKeyCtx& keyCtx ) {

    CryptExecStatus retVal = CryptExecStatus::Execute_Error;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::do_validate_ctx()" );
        UNUSED(keyCtx);
    TTRACE_LEAVE("JEngineExec::do_validate_ctx() => (CryptExecStatus): %d", (int)retVal);

    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFInit() {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFInit()" );
        int jag_status;
        jag_status = HFInit();
        retVal = map_error( __FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFInit() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFTerminate() {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFTerminate()" );
        int jag_status;
        jag_status = HFTerminate();
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFTerminate() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_Free(void* ptr) {

    V100_ERROR_CODE retVal = GEN_OK;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_Free()" );
        if (nullptr != ptr) {
            HFFree(ptr);
        }
    TTRACE_LEAVE("JEngineExec::Jag_Free() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFEnumerateCameras(HFStringArray** desc_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFEnumerateCameras()" );

        int jag_status;

        if (nullptr == desc_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        jag_status = HFEnumerateCameras (desc_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFEnumerateCameras() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFAsyncOpenContext(int32_t camId, HFAlgorithmType algoType, HFContext* ctx_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncOpenContext()" );

        int jag_status;

        if (nullptr == ctx_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        jag_status = HFOpenContext (camId, algoType, ctx_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncOpenContext() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFCloseContext(HFContext ctx) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFCloseContext()" );
        int jag_status;
        jag_status = HFCloseContext (ctx);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFCloseContext() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncStartCaptureImage(HFContext ctx, int32_t timeout, double minimalQuality, double minimalLivenessScore, uint64_t intermediateResultFlags, uint64_t finalResultFlags, HFOperation* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncStartCaptureImage()" );
        int jag_status;
        jag_status = HFAsyncStartCaptureImage (ctx, timeout, minimalQuality, minimalLivenessScore, intermediateResultFlags, finalResultFlags, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncStartCaptureImage() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncExtractTemplate(HFContext ctx, const HFImage* const img_ptr, uint64_t finalResultFlags, HFOperation* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncExtractTemplate()" );

        if (nullptr == img_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncExtractTemplate (ctx, img_ptr, finalResultFlags, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncExtractTemplate() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncVerifyWithCaptured(HFOperation op, const char* const gallery_ptr, const char* const id_ptr, double minScore) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncVerifyWithCaptured()" );

        if (nullptr == gallery_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == id_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncVerifyWithCaptured (op, gallery_ptr, id_ptr, minScore);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncVerifyWithCaptured() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncMatchWithTemplate(HFContext ctx, const HFData* const templA, const HFData* const templB, HFOperation* const op_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncMatchWithTemplate()" );

        if (nullptr == templA) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == templB) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == op_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncMatchWithTemplate (ctx, templA, templB, op_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncMatchWithTemplate() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncIdentifyWithCaptured(HFOperation op, const char* const gal_ptr, double minScore) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncIdentifyWithCaptured()" );

        if (nullptr == gal_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncIdentifyWithCaptured (op, gal_ptr, minScore);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncIdentifyWithCaptured() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncMatchWithCaptured(HFOperation op, const HFData* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncMatchWithCaptured()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncMatchWithCaptured (op, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncMatchWithCaptured() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncVerifyWithTemplate(HFContext ctx, const char* const gal_ptr, const char* const id_ptr, double minScore, const HFData* const templ_ptr, HFOperation* const op_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncVerifyWithTemplate()" );

        if (nullptr == gal_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == id_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == templ_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == op_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncVerifyWithTemplate (ctx, gal_ptr, id_ptr, minScore, templ_ptr, op_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncVerifyWithTemplate() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAsyncIdentifyWithTemplate(HFContext ctx, const char* const gal_ptr, double minScore, const HFData* const templ_ptr, HFOperation* const op_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAsyncIdentifyWithTemplate()" );

        if (nullptr == gal_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == templ_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == op_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFAsyncIdentifyWithTemplate (ctx, gal_ptr, minScore, templ_ptr, op_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFAsyncIdentifyWithTemplate() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFCloseOperation(HFOperation op) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFCloseOperation()" );
        int jag_status;
        jag_status = HFCloseOperation (op);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFCloseOperation() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFStopOperation(HFOperation op) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFStopOperation()" );
        int jag_status;
        jag_status = HFStopOperation (op);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFStopOperation() => (V100_ERROR_CODE): %d", (int)retVal);
        
    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFSetParamInt(int32_t ctx, uint32_t id, uint32_t val) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFSetParamInt()" );
        int jag_status;
        jag_status = HFSetParamInt (ctx, id, val);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFSetParamInt() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetParamInt(int32_t ctx, uint32_t id, int32_t* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetParamInt()");

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetParamInt (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetParamInt() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFSetParamStr(int32_t ctx, uint32_t id, const char* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFSetParamStr()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFSetParamString (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFSetParamStr() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetParamStr(int32_t ctx, uint32_t id, char** const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetParamStr()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        *val_ptr = nullptr;

        int jag_status;
        jag_status = HFGetParamString (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetParamStr() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFSetParamBin(int32_t ctx, uint32_t id, const HFData* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFSetParamBin()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFSetParamData (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFSetParamBin() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetParamBin(int32_t ctx, uint32_t id, HFData** const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetParamBin()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetParamData (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetParamBin() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFSetParamLng(int32_t ctx, uint32_t id, double val) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFSetParamLng()" );

        int jag_status;
        jag_status = HFSetParamDouble (ctx, id, val);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

    TTRACE_LEAVE("JEngineExec::Jag_HFSetParamLng() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetParamLng(int32_t ctx, uint32_t id, double* const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetParamLng()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetParamDouble (ctx, id, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetParamLng() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFGetIntermediateResult(HFOperation op, uint64_t resFlg, int32_t seqId, HFData** const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetIntermediateResult()");

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetIntermediateResult (op, resFlg, seqId, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

        if (0 == retVal ) {
            logHfRes(val_ptr);
        }
exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetIntermediateResult() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetFinalResult(HFOperation operation, uint64_t resultFlags, HFData** const val_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetFinalResult()" );

        if (nullptr == val_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetFinalResult (operation, resultFlags, val_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

        if (0 == retVal ) {
            logHfRes(val_ptr);
        }

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetFinalResult() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

//-------------------------------------------------------------------------------------------//

V100_ERROR_CODE JEngineExec::Jag_HFGetVideoFrame(HFContext ctx, int64_t lastSequenceNumber, HFImage** img_ptr, int64_t* seq_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetVideoFrame()" );

        if (nullptr == img_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == seq_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFGetVideoFrame (ctx, lastSequenceNumber, img_ptr, seq_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFGetVideoFrame() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAddRecordWithCaptured(HFOperation op, const HFDatabaseRecordHeader* header, bool replaceIfExists) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAddRecordWithCaptured()");
        int jag_status;
        jag_status = HFAddRecordWithCaptured (op, header, replaceIfExists);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFAddRecordWithCaptured() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFAddRecordWithTemplate(HFDatabaseRecord* const databaseRecord, bool replaceIfExists) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFAddRecordWithTemplate()");
        int jag_status;
        jag_status = HFAddRecordWithTemplate (databaseRecord, replaceIfExists);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFAddRecordWithTemplate() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFGetRecord(const char* const id_ptr, const char* const gal_ptr, HFDatabaseRecord** const rec_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFGetRecord()");
        int jag_status;
        jag_status = HFGetRecord (id_ptr, gal_ptr, rec_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);
    TTRACE_LEAVE("JEngineExec::Jag_HFGetRecord() => (V100_ERROR_CODE): %d", (int)retVal);

    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFListRecords(const char* galId, HFStringArray** ids_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFListRecords()");

        if (nullptr == galId) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == ids_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFListRecords (galId, ids_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFListRecords() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

V100_ERROR_CODE JEngineExec::Jag_HFDelRecords(const char* const id_ptr, const char* const gallery_ptr) {

    V100_ERROR_CODE retVal = GEN_ERROR_INTERNAL;

    TTRACE_ENTER(jengexec, TDBG_DEBUG, "JEngineExec::Jag_HFDelRecords()");

        if (nullptr == id_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        if (nullptr == gallery_ptr) {
            assert(false);
            retVal = GEN_INVALID_ARGUEMENT;
            goto exit;
        }

        int jag_status;
        jag_status = HFDeleteRecord (id_ptr, gallery_ptr);
        retVal = map_error(__FUNCTION__, __LINE__, jag_status);

exit:;
    TTRACE_LEAVE("JEngineExec::Jag_HFDelRecords() => (V100_ERROR_CODE): %d", (int)retVal);
    return retVal;
}

//-------------------------------------------------------------------------------------------//
