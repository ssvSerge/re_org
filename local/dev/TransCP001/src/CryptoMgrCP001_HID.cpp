#include <assert.h>
#include <logging.h>
#include <CryptoMgrCP001.h>
#include <AutoHeapBuffer.h>

/////////////////////////////////////////////////////////////////////////////
// Lock state of virtual storage.
//
static std::string FKL_LOCK_PROP_STRING_CP001 = "FKL_LOCK_STATUS_CP001";

/////////////////////////////////////////////////////////////////////////////
// Session key.
//
static std::string DSK_PROP_STRING_CP001 = "DSK_CRYPTOGRAM_CP001";


/// /////////////////////////////////////////////////////////////////////////////
// Constructor
//
CryptoMgrCP001::CryptoMgrCP001() {

    m_ActiveSlot = KT_EXTKEY_VEND;
    m_MgrBusy    = false;
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
//
CryptoMgrCP001::~CryptoMgrCP001() {

}

/////////////////////////////////////////////////////////////////////////////
// Init
// Check the connection to SECURE_ELEMENT
// Remove KT_EXT_DSK key and Remove_Property (DSK_PROP_STRING_CP001)
// Remove KT_EXT_BSK key
//
bool CryptoMgrCP001::Init() {

    if (nullptr == m_SecureClient) {
        m_SecureClient = new (HBSEClient);
    }

    if (nullptr == m_SecureClient) {
        return false;
    }

    // set ANBIO to a random number - do not set to all zeros - guessable...
    CryptExecStatus status;

    status = m_SecureClient->Execute_GetRandomBuffer(m_ANBIO, sizeof(m_ANBIO));
    if (status != CryptExecStatus::Successful) {
        err("Init:Get random number for ANBIO returned error.");
        return false;
    } else {
        m_nActiveKeyType = KT_EXT_LAST;
        InvalidateDSK();
        InvalidateBSK();
        return true;
    }

}


/////////////////////////////////////////////////////////////////////////////
// Key material must be encrypted with KT_EXTKEY_CTK (Transportation Key)
// KT_EXTKEY_CTK must be configured before.
//
CryptoMgrRes CryptoMgrCP001::GetKeyInfo ( _V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32 ) {

    CryptoMgrRes retVal;

    if (m_MgrBusy) {
        return CryptoMgrRes::CM_ERROR_BUSY;
    }

    m_MgrBusy = true;

    try {
        retVal = GetKeyInfoInt(nKeyType, nKeyVer, nKeyMode, pKCV32);
    } catch (...) {
        retVal = CryptoMgrRes::CM_ERROR_GENERAL;
    }

    m_MgrBusy = false;

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
CryptoMgrRes CryptoMgrCP001::GetKeyInfoInt ( _V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32 ) {

    CryptoMgrRes  ioRes;

    nKeyVer  = 0;
    nKeyMode = KM_MODE_NONE;

    assert(m_SecureClient != nullptr);

    switch (nKeyType) {

        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_SP:
            nKeyVer     = 0;
            nKeyMode    = KM_MODE_NONE;
            return CryptoMgrRes::CM_ERROR_NONE;

        case KT_EXTKEY_VEND:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXT_DSK:
        case KT_EXT_BSK:
            break;

        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_LAST:
        case KT_MSK_MKD:
        case KT_MSK_SK:
        default:
            nKeyVer     = 0;
            nKeyMode    = KM_MODE_NONE;
            return CryptoMgrRes::CM_ERROR_BAD_PARAM;
    }

    u16     nActiveSlot     = 0;
    u16     nTempKeyMode    = 0;

    ioRes = GetActiveSlot ( nKeyType, nActiveSlot );
    if ( ioRes != CryptoMgrRes::CM_ERROR_NONE) {
        return ioRes;
    }

    ioRes = PullKeyInfo ( nActiveSlot, nKeyVer, nTempKeyMode, pKCV32 );
    if ( ioRes != CryptoMgrRes::CM_ERROR_NONE ) {
        return ioRes;
    }

    nKeyMode = (_V100_ENC_KEY_MODE) nTempKeyMode;
    return CryptoMgrRes::CM_ERROR_NONE;
}

/////////////////////////////////////////////////////////////////////////////
// Get Random Number with u256 length.
// Generated value also sets as ANBIO.
//
CryptoMgrRes CryptoMgrCP001::GetRandomNumber(u256* pRnd) {

    CryptoMgrRes retVal;

    if (m_MgrBusy) {
        return CryptoMgrRes::CM_ERROR_BUSY;
    }

    if (!pRnd) {
        return CryptoMgrRes::CM_ERROR_BAD_PARAM;
    }

    m_MgrBusy = true;

    try {
        retVal = GetRandomNumberInt(pRnd);
    } catch (...) {
        retVal = CryptoMgrRes::CM_ERROR_GENERAL;
    }

    m_MgrBusy = false;

    return retVal;

}

CryptoMgrRes CryptoMgrCP001::GetRandomNumberInt(u256* pRnd) {

    CryptoMgrRes retVal;

    CryptExecStatus status;

    assert ( pRnd != nullptr );
    assert ( m_SecureClient != nullptr );

    retVal = CryptoMgrRes::CM_ERROR_GENERAL;

    status = m_SecureClient->Execute_GetRandomBuffer( (u8*)pRnd, sizeof(u256) );
    if ( status == CryptExecStatus::Successful ) {
        memcpy(m_ANBIO, pRnd, 32);
        retVal = CryptoMgrRes::CM_ERROR_NONE;
    }

    return retVal;
}



/////////////////////////////////////////////////////////////////////////////
//
CryptoMgrRes CryptoMgrCP001::GetActiveSlot ( u16 nKeyType, u16& nKeySlot ) {

    nKeySlot = m_ActiveSlot;

    if (KT_EXTKEY_VEND == m_ActiveSlot) {
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }
    return CryptoMgrRes::CM_ERROR_NONE;
}

/////////////////////////////////////////////////////////////////////////////
//
CryptoMgrRes CryptoMgrCP001::PullKeyInfo ( u16 nKeySlot, u16& nKeyVer, u16& nKeyMode, u8* pKCV32 ) {

    CryptExecStatus  status;
    KeyInfoStructure keyInfo;

    if (m_SecureClient == nullptr) {
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    status = m_SecureClient->Execute_Get_Key_Info(nKeySlot, keyInfo);
    if (CryptExecStatus::Not_Exist != status) {
        info("PullKeyInfo: Key not exists.");
        return CryptoMgrRes::CM_ERROR_KEY_NOT_EXISTS;
    }

    if (CryptExecStatus::Successful != status) {
        err("PullKeyInfo: Failed Execute_Get_Key_Info.");
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    nKeyVer = keyInfo.key_ver;
    nKeyMode = (_V100_ENC_KEY_MODE)keyInfo.key_mode;

    memcpy(pKCV32, keyInfo.key_check_value, sizeof(keyInfo.key_check_value));

    return CryptoMgrRes::CM_ERROR_NONE;


}

/////////////////////////////////////////////////////////////////////////////
// Helper method to invalidate DSK
//
CryptoMgrRes CryptoMgrCP001::InvalidateDSK() {

    CryptoMgrRes    mgrRes = CryptoMgrRes::CM_ERROR_GENERAL;
    CryptExecStatus exeRes;
    u16             nSlot = 0;

    if (nullptr == m_SecureClient) {
        err("InvalidateDSK: SecureClient not initialized");
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    mgrRes = GetActiveSlot(KT_EXT_DSK, nSlot);
    if (CryptoMgrRes::CM_ERROR_NONE != mgrRes) {
        err("InvalidateDSK: GetActiveSlot failed");
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    mgrRes = CryptoMgrRes::CM_ERROR_NONE;

    exeRes = m_SecureClient->Execute_Erase_Key(nSlot);
    if (CryptExecStatus::Successful != exeRes) {
        err("InvalidateDSK: Execute_Erase_Key failed");
        mgrRes = CryptoMgrRes::CM_ERROR_GENERAL;
    }

    exeRes = m_SecureClient->Execute_Remove_Property((u8*)DSK_PROP_STRING_CP001.c_str(), DSK_PROP_STRING_CP001.length());
    if (CryptExecStatus::Successful != exeRes) {
        err("InvalidateDSK: Execute_Remove_Property failed");
        mgrRes = CryptoMgrRes::CM_ERROR_GENERAL;
    }

    return mgrRes;
}

/////////////////////////////////////////////////////////////////////////////
// Helper method to invalidate BSK
//
CryptoMgrRes CryptoMgrCP001::InvalidateBSK() {

    CryptoMgrRes    mgrRes = CryptoMgrRes::CM_ERROR_GENERAL;
    u16             nSlot = 0;

    if (nullptr == m_SecureClient) {
        err("InvalidateBSK: SecureClient not initialized");
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    mgrRes = GetActiveSlot(KT_EXT_BSK, nSlot);
    if (CryptoMgrRes::CM_ERROR_NONE != mgrRes) {
        err("InvalidateBSK: GetActiveSlot failed");
        return CryptoMgrRes::CM_ERROR_GENERAL;
    }

    mgrRes = CryptoMgrRes::CM_ERROR_NONE;

    if (CryptExecStatus::Successful != m_SecureClient->Execute_Erase_Key(nSlot)) {
        err("InvalidateBSK: Execute_Erase_Key failed");
        mgrRes = CryptoMgrRes::CM_ERROR_GENERAL;
    }

    return mgrRes;
}

/////////////////////////////////////////////////////////////////////////////
//
bool CryptoMgrCP001::IsZeroVecKey ( u8* pKey, u32 nKeySize ) {

    return ( !(pKey[0] || memcmp(&pKey[0], &pKey[1], nKeySize - 1)) );
}

/////////////////////////////////////////////////////////////////////////////
//
bool CryptoMgrCP001::DecryptBuffer ( u16 nKeySlot, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize ) {

    if ((NULL == pInCG) || (0 == nInCGSize)) {
        err("DecryptBuffer:Initial input arugment validation failed");
        return false;
    }

    if (nInCGSize % BLOCK_SIZE != 0) {
        err("DecryptBuffer:Invalid input size %d (should be multiple of %d).", nInCGSize, BLOCK_SIZE);
        return false;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(nKeySlot)) {
        err("DecryptBuffer:Selecting for KeySlot %d returned error", nKeySlot);
        return false;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Decrypt(pInCG, nInCGSize, pOut, nOutSize, nullptr, 0)) {
        err("DecryptBuffer:Decrypting with KeySlot %d returned error", nKeySlot);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
bool CryptoMgrCP001::IsKeyLoaded ( u16 nKeySlot ) {
    return  CryptExecStatus::Successful == SECURE_ELEMENT->Execute_Check_Key_Exist(nKeySlot);
}

#endif

