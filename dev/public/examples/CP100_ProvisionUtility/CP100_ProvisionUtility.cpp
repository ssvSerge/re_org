#include <iostream>
#include <VCOMBase.h>
#include <HostKeyManager.h>
#include <string>

#ifdef WIN32
    #include <windows.h>
    #define SLEEP(nMS)      Sleep(nMS)
#else
    #include <unistd.h>
    #define SLEEP(nMS)      usleep(nMS*1000)
#endif

enum class ResExpected {
    EXPECTED_SUCCESS  = 1,
    EXPECTED_REJECT   = 2
};

#define EMPTY_STORAGE   (0x0000)
#define RSA_EXPECTED    (0x0001)
#define CTK_EXPECTED    (0x0002)
#define BTK_EXPECTED    (0x0004)
#define AES0_EXPECTED   (0x0008)
#define AES1_EXPECTED   (0x0010)
#define AES2_EXPECTED   (0x0020)
#define AES3_EXPECTED   (0x0040)
#define DES0_EXPECTED   (0x0080)
#define DES1_EXPECTED   (0x0100)
#define DES2_EXPECTED   (0x0200)
#define DES3_EXPECTED   (0x0400)


#define CHECK_RES(x)
#define CHECK_RES_AND_CLOSE(x,y)

// Fixed parameters of Customer Provisioning Key  [KT_EXTKEY_CTK]
int   CTK_KEY_MODE      = KM_AES_256_CBC;
int   CTK_KEY_VERSION   = 1;
u8    CTK_VAL[32]       = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F };

// Back-end Transportation key [KT_EXTKEY_BTK]
int   BTK_KEY_MODE      = KM_AES_256_CBC;
int   BTK_KEY_VERSION   = 1;
u8    BTK_VAL[32]       = { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F };

// User key [KT_EXTKEY_AES0]
int   AES_KEY_MODE      = KM_AES_256_CBC;
int   AES_KEY_VERSION   = 1;
u8    AES_VAL[32]       = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };

u8    BIN_DATA[]        = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

u8    TEMPLATE_A[]      = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
u8    TEMPLATE_B[]      = { 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11 };



HostCryptoMgr*                  m_pCryptoMgr    = nullptr;
V100_DEVICE_TRANSPORT_INFO      hDev            = { 0 };
HostKeyManager                  m_pKeyMgr;

static V100_ERROR_CODE _get_ansol ( u8* dst, int len ) {
    m_pCryptoMgr->GetRandomNumber(dst, len);
    return GEN_OK;
}

static V100_ERROR_CODE _get_anbio ( u8* dst, int len ) {
    V100_ERROR_CODE rc;
    rc = V100_Enc_Get_Rnd_Number(&hDev, dst, len);
    CHECK_RES(rc);
    return rc;
}

static V100_ERROR_CODE _init ( void ) {

    V100_ERROR_CODE rc;

    _V100_INTERFACE_CONFIGURATION_TYPE devConfig;
    u32     nSN1;
    int     nDevices = 0;

    rc = V100_Get_Num_USB_Devices(&nDevices);
    CHECK_RES(rc);

    rc = V100_Open(&hDev);
    CHECK_RES(rc);

    rc = V100_Get_Config(&hDev, &devConfig);
    CHECK_RES(rc);
     
    rc = V100_Get_Serial(&hDev, &nSN1);
    CHECK_RES(rc);

    return GEN_OK;
}

static V100_ERROR_CODE _close ( void ) {
    V100_Close(&hDev);
    return GEN_OK;
}

static V100_ERROR_CODE _V100_Enc_Generate_RSA_Keys ( ResExpected expResult ) {

    V100_ERROR_CODE rc;
    V100_ERROR_CODE retVal;
    _V100_OP_STATUS opStatus;

    const char prefix[] = "\\|/-\\|/-\\";
    int out_idx_max = sizeof(prefix);
    int out_idx     = 0;

    std::cout << std::endl;
    std::cout << "Generating RSA key: Might take time" << std::endl;

    rc = V100_Enc_Generate_RSA_Keys(&hDev);

    if (ResExpected::EXPECTED_SUCCESS == expResult) {
        if (GEN_OK != rc) {
            std::cout << "    Command not stated. BAD." << std::endl << std::endl;
            return rc;
        }
    } else
    if (ResExpected::EXPECTED_REJECT == expResult) {
        if (GEN_ERROR_ACCESS_DENIED == rc) {
            std::cout << "    Command rejected. OK." << std::endl << std::endl;
            return GEN_OK;
        } else {
            std::cout << "    Command started while not expected. BAD." << std::endl;
        }
    }

    bool dly = true;

    while (dly) {

        std::cout << prefix[out_idx] << "\r";
        out_idx++;
        out_idx %= out_idx_max;

        SLEEP(1000);

        rc = V100_Get_OP_Status(&hDev, opStatus);

        switch (opStatus.nMode) {
            case OP_COMPLETE:
                dly    = false;
                retVal = GEN_OK;
                break;
            case OP_ERROR:
                dly    = false;
                retVal = GEN_ERROR_PROCESSING;
                break;
            default:
                break;
        }
    }

    std::cout << "\rKey generation: Done." << std::endl << std::endl;

    return retVal;
}

static V100_ERROR_CODE _V100_add_ctk_key ( ResExpected expResult ) {

    V100_ERROR_CODE rc;
    u2048           keyDevPublic;
    ClientKeyInfo   parentKey;
    ClientKeyInfo   childKey;
    u8              kcv[KCV_LENGTH];
    u256            ANBIO;
    u256            ANSOL;

    _get_ansol( ANSOL, ANSOL_LENGTH );
    _get_anbio( ANBIO, ANBIO_LENGTH );

    std::cout << "Configuring CTK key." << std::endl;

    parentKey.slot   = KT_EXTKEY_DEVICE_PUBLIC;
    parentKey.KeyVal = keyDevPublic;
    rc = V100_Enc_Get_Key(&hDev, KT_EXTKEY_DEVICE_PUBLIC, keyDevPublic, parentKey.nkKeyVersion, kcv, parentKey.KeyLen, parentKey.nKeyMode );
    if (GEN_OK != rc) {
        std::cout << "    Cannot get DEVICE_PUBLIC key." << std::endl;
        return rc;
    }

    childKey.slot           = KT_EXTKEY_CTK;
    childKey.nkKeyVersion   = CTK_KEY_VERSION;
    childKey.nKeyMode       = CTK_KEY_MODE;
    childKey.KeyLen         = sizeof(CTK_VAL);
    childKey.KeyVal         = CTK_VAL;

    u8   pOutBuffer[512];
    uint iOutBufferLen = sizeof(pOutBuffer);

    m_pKeyMgr.GenerateRsaKeyBlock ( ANBIO, ANSOL, parentKey, childKey, pOutBuffer, iOutBufferLen );

    u8* pDevResp    = nullptr;
    u32 iDevRespLen = 0;

    rc = V100_Enc_Set_Key ( &hDev, pOutBuffer, iOutBufferLen, KT_EXTKEY_CTK, &pDevResp, iDevRespLen );

    if ( ResExpected::EXPECTED_SUCCESS == expResult ) {
        if (GEN_OK == rc) {
            std::cout << "    KT_EXTKEY_CTK accepted. OK." << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_CTK rejected. BAD. " << std::endl;
        }
    } else
    if ( ResExpected::EXPECTED_REJECT == expResult ) {
        if (GEN_ERROR_ACCESS_DENIED == rc) {
            std::cout << "    KT_EXTKEY_CTK rejected. OK." << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_CTK accepted. BAD. " << std::endl;
        }
    }

    V100_Release_Memory( &hDev, pDevResp);

    std::cout << std::endl;

    return GEN_OK;
}

static V100_ERROR_CODE _V100_add_btk_key ( ResExpected expResult ) {

    V100_ERROR_CODE rc;

    u256    ANBIO;
    u256    ANSOL;

    _get_ansol(ANSOL, ANSOL_LENGTH);
    _get_anbio(ANBIO, ANBIO_LENGTH);

    std::cout << "Configuring BTK key. KT_EXTKEY_CTK key required." << std::endl;

    u8      pOutBuffer[512] = {};
    uint    iOutBufferLen   = sizeof(pOutBuffer);
    u8*     pDevResp        = nullptr;
    u32     iDevRespLen     = 0;

    ClientKeyInfo parentKey;
    ClientKeyInfo newKey;

    parentKey.slot          = KT_EXTKEY_CTK;
    parentKey.KeyLen        = sizeof(CTK_VAL);
    parentKey.KeyVal        = CTK_VAL;
    parentKey.nKeyMode      = CTK_KEY_MODE;
    parentKey.nkKeyVersion  = CTK_KEY_VERSION;

    newKey.slot             = KT_EXTKEY_BTK;
    newKey.KeyLen           = sizeof(BTK_VAL);
    newKey.KeyVal           = BTK_VAL;
    newKey.nKeyMode         = BTK_KEY_MODE;
    newKey.nkKeyVersion     = BTK_KEY_VERSION;

    // BTK key encrypted with CTK.
    m_pKeyMgr.GenerateAesKeyBlock ( ANBIO, ANSOL, parentKey, newKey, pOutBuffer, iOutBufferLen );

    rc = V100_Enc_Set_Key ( &hDev, pOutBuffer, iOutBufferLen, KT_EXTKEY_BTK, &pDevResp, iDevRespLen );

    if ( ResExpected::EXPECTED_SUCCESS == expResult ) {
        if (GEN_OK == rc) {
            std::cout << "    KT_EXTKEY_BTK accepted. OK."   << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_BTK rejected. BAD. " << std::endl;
        }
    } else
    if ( ResExpected::EXPECTED_REJECT == expResult ) {
        if (GEN_ERROR_ACCESS_DENIED == rc) {
            std::cout << "    KT_EXTKEY_BTK rejected. OK."   << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_BTK accepted. BAD. " << std::endl;
        }
    }

    V100_Release_Memory( &hDev, pDevResp);

    std::cout << std::endl;

    return GEN_OK;
}

static V100_ERROR_CODE _V100_add_user_key ( _V100_ENC_KEY_TYPE key, ResExpected expResult ) {

    V100_ERROR_CODE rc;

    u256    ANBIO;
    u256    ANSOL;

    _get_ansol(ANSOL, ANSOL_LENGTH);
    _get_anbio(ANBIO, ANBIO_LENGTH);

    std::cout << "Configuring AES key. " << std::endl;

    u8      pOutBuffer[512] = {};
    uint    iOutBufferLen   = sizeof(pOutBuffer);
    u8*     pDevResp        = nullptr;
    u32     iDevRespLen     = 0;

    ClientKeyInfo parentKey;
    ClientKeyInfo newKey;

    parentKey.slot             = KT_EXTKEY_BTK;
    parentKey.KeyLen           = sizeof(BTK_VAL);
    parentKey.KeyVal           = BTK_VAL;
    parentKey.nKeyMode         = BTK_KEY_MODE;
    parentKey.nkKeyVersion     = BTK_KEY_VERSION;

    newKey.slot                = key;
    newKey.KeyLen              = sizeof(AES_VAL);
    newKey.KeyVal              = AES_VAL;
    newKey.nKeyMode            = AES_KEY_MODE;
    newKey.nkKeyVersion        = AES_KEY_VERSION;

    // BTK key encrypted with CTK.
    m_pKeyMgr.GenerateAesKeyBlock ( ANBIO, ANSOL, parentKey, newKey, pOutBuffer, iOutBufferLen );

    rc = V100_Enc_Set_Key ( &hDev, pOutBuffer, iOutBufferLen, key, &pDevResp, iDevRespLen );

    if ( ResExpected::EXPECTED_SUCCESS == expResult ) {
        if (GEN_OK == rc) {
            std::cout << "    KT_EXTKEY_AES0 accepted. OK."   << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_AES0 rejected. BAD. " << std::endl;
        }
    } else
    if ( ResExpected::EXPECTED_REJECT == expResult ) {
        if (GEN_ERROR_ACCESS_DENIED == rc) {
            std::cout << "    KT_EXTKEY_AES0 rejected. OK."   << std::endl;
        } else {
            std::cout << "    KT_EXTKEY_AES0 accepted. BAD. " << std::endl;
        }
    }

    V100_Release_Memory( &hDev, pDevResp);

    std::cout << std::endl;

    return GEN_OK;
}

static V100_ERROR_CODE _remove_keys ( void ) {

    V100_ERROR_CODE rc;

    u256    device_anbio;
    u8*     cg_command;
    uint    len;

    // Configure the key.
    m_pCryptoMgr->SetActiveKey(KT_EXTKEY_CTK, CTK_VAL, sizeof(CTK_VAL), CTK_KEY_MODE);

    _get_anbio(device_anbio, ANBIO_LENGTH);

    len = ANBIO_LENGTH;
    m_pCryptoMgr->Encrypt(device_anbio, &cg_command, &len);

    rc = V100_Enc_Unlock_Key(&hDev, cg_command, len);

    m_pCryptoMgr->ReleaseMem(cg_command);

    return GEN_OK;
}

static void _log_key_state ( const char* const name, V100_ERROR_CODE rc, int expected_val ) {

    std::cout << "    " << name << " -> ";
    if ( expected_val ) {
        if (GEN_OK == rc) {
            std::cout << "OK.  Key Present.";
        } else
        if (GEN_ERROR_KEY_NOT_FOUND == rc) {
            std::cout << "Bad. Key not found.";
        } else {
            std::cout << "Bad. Unknown key state.";
        }
    } else {
        if (GEN_ERROR_KEY_NOT_FOUND == rc) {
            std::cout << "OK.  Key not found.";
        } else
        if (GEN_OK == rc) {
            std::cout << "Bad. Key present.";
        } else {
            std::cout << "Bad. Unknown key state.";
        }
    }
    std::cout << std::endl;
}

static void _log_storage_state ( int mask ) {

    V100_ERROR_CODE rc;
    u16             keyVersion = 0;
    u8              keyKCV[KCV_LENGTH] = { 0 };
    u16             keyMode = 0;

    std::cout << "Storage state:" << std::endl;

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_DEVICE_PUBLIC, keyVersion, keyKCV, keyMode);
    _log_key_state("DEVICE_PUBLIC  ", rc, mask & RSA_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_CTK, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_CTK  ",  rc, mask & CTK_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_BTK, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_BTK  ",  rc, mask & BTK_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_AES0, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_AES0 ", rc, mask & AES0_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_AES1, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_AES1 ", rc, mask & AES1_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_AES2, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_AES2 ", rc, mask & AES2_EXPECTED);

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_AES3, keyVersion, keyKCV, keyMode);
    _log_key_state("KT_EXTKEY_AES3 ", rc, mask & AES3_EXPECTED);

    std::cout << std::endl;
}

static void _reset_storage ( void ) {

    V100_ERROR_CODE rc;
    u16             keyVersion = 0;
    u8              keyKCV[KCV_LENGTH] = { 0 };
    u16             keyMode = 0;

    bool is_decommissioned;

    std::cout << "Check the state of CTK:   ";

    rc = V100_Enc_Get_KeyVersion(&hDev, KT_EXTKEY_CTK, keyVersion, keyKCV, keyMode);
    if (GEN_OK != rc) {
        is_decommissioned = true;
        std::cout << "Not installed.";
    } else {
        is_decommissioned = false;
        std::cout << "Present.";
    }
    std::cout << std::endl;

    std::cout << "Device state:             ";
    if (is_decommissioned) {
        std::cout << "Decommissioned.";
    } else {
        std::cout << "Going to delete keys.";
    }
    std::cout << std::endl;

    if (!is_decommissioned) {
        _remove_keys();
    }

    std::cout << std::endl;
}

static void _test_v100_key_management ( void ) {

    V100_ERROR_CODE rc;

    _reset_storage();
    _log_storage_state(EMPTY_STORAGE);

    // First attempt to generate RSA key.
    // Operation enabled because KT_EXTKEY_CTK not installed.
    rc = _V100_Enc_Generate_RSA_Keys(ResExpected::EXPECTED_SUCCESS);
    _log_storage_state(RSA_EXPECTED);

    // Second attempt to generate RSA key.
    // Operation enabled because KT_EXTKEY_CTK not installed.
    rc = _V100_Enc_Generate_RSA_Keys(ResExpected::EXPECTED_SUCCESS);
    _log_storage_state(RSA_EXPECTED);

    // First attempt to install CTK key.
    // Operation enabled because KT_EXTKEY_CTK not installed.
    _V100_add_ctk_key(ResExpected::EXPECTED_SUCCESS);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED);

    // Next attempt to install CTK key.
    // Operation must be rejected because KT_EXTKEY_CTK already installed.
    _V100_Enc_Generate_RSA_Keys(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED);

    // Attempt to create new RSA keys.
    // Operation must be rejected because of KT_EXTKEY_CTK key installed.
    _V100_add_ctk_key(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED);

    // First attempt to install BTK key.
    // Operation enabled because KT_EXTKEY_BTK not installed.
    _V100_add_btk_key(ResExpected::EXPECTED_SUCCESS);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED);

    // Next attempt to install BTK key.
    // Operation enabled because KT_EXTKEY_BTK not installed.
    _V100_add_btk_key(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED);

    {   // All commands are allowed.
        _V100_add_user_key(KT_EXTKEY_AES0, ResExpected::EXPECTED_SUCCESS);
        _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED);

        _V100_add_user_key(KT_EXTKEY_AES1, ResExpected::EXPECTED_SUCCESS);
        _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED | AES1_EXPECTED);

        _V100_add_user_key(KT_EXTKEY_AES2, ResExpected::EXPECTED_SUCCESS);
        _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED | AES1_EXPECTED | AES2_EXPECTED);

        _V100_add_user_key(KT_EXTKEY_AES3, ResExpected::EXPECTED_SUCCESS);
        _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED | AES1_EXPECTED | AES2_EXPECTED | AES3_EXPECTED);
    }

    // An attempt to manage BTK key.
    // Operation rejected because KT_EXTKEY_BTK key already installed.
    _V100_add_btk_key(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED | AES1_EXPECTED | AES2_EXPECTED | AES3_EXPECTED);

    // An attempt to manage CTK key.
    // Operation rejected because KT_EXTKEY_CTK key already installed.
    _V100_add_ctk_key(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED | BTK_EXPECTED | AES0_EXPECTED | AES1_EXPECTED | AES2_EXPECTED | AES3_EXPECTED);

    // An attempt to manage Device keys.
    // Operation must be rejected because KT_EXTKEY_CTK already installed.
    _V100_Enc_Generate_RSA_Keys(ResExpected::EXPECTED_REJECT);
    _log_storage_state(RSA_EXPECTED | CTK_EXPECTED);

    // Remove all keys.
    _reset_storage();
    _log_storage_state(EMPTY_STORAGE);
}

int main() {

    m_pCryptoMgr = HostCryptoMgr::GetInstance();
    m_pCryptoMgr->Init();

    _init();
    _test_v100_key_management();
    _close();

    return 0;
}
