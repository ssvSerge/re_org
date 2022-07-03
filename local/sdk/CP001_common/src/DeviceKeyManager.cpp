#include <string.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <HostKeyManager.h>
#include <HostKeyMap.h>
#include <CryptoShared.h>

#define CTK_KEY_MODE                   (KM_AES_256_CBC)
#define CTK_KEY_VERSION                (1)

#define BTK_KEY_MODE                   (KM_AES_256_CBC)
#define BTK_KEY_VERSION                (1)

static const char* KEY_TYPE_STR[27] = {
    "KT_EXTKEY_VEND",
    "KT_EXTKEY_CTK",
    "KT_EXTKEY_BTK",
    "KT_EXTKEY_AES0",
    "KT_EXTKEY_AES1",
    "KT_EXTKEY_AES2",
    "KT_EXTKEY_AES3",
    "KT_EXTKEY_TDES0",
    "KT_EXTKEY_TDES1",
    "KT_EXTKEY_TDES2",
    "KT_EXTKEY_TDES3",
    "KT_EXTKEY_AES_VEND",
    "KT_EXTKEY_KSN_0",
    "KT_EXTKEY_KSN_1",
    "KT_EXTKEY_SPARE_2",
    "KT_EXTKEY_SPARE_3",
    "KT_EXTKEY_HOST_PUBLIC",
    "KT_EXTKEY_DEVICE_PUBLIC",
    "KT_EXTKEY_DEVICE_PRIVATE",
    "KT_EXTKEY_DEVICE_P",
    "KT_EXTKEY_DEVICE_Q",
    "KT_EXTKEY_PUBLIC_EXP",
    "KT_EXTKEY_PRIVATE_EXP",
    "KT_EXT_LAST",
    "KT_EXT_DSK",
    "KT_EXT_BSK",
    "KT_EXT_SP"
};

KeyMap_storage HostKeyMap::m_KeyMap;

static std::string _out_u128(u128 val) {

    std::stringstream ss;
    int max = 128 / 8;

    uint8_t* ptr = (uint8_t*) &val;

    for (int i = 0; i < max; i++) {
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)ptr[i];
        if (i < (max-1) ) {
            ss << " ";
        }
    }

    return ss.str();
}

static std::string _out_u2048(u2048 val) {

    std::stringstream ss;
    int max = 2048 / 8;

    uint8_t* ptr = (uint8_t*)&val;

    for (int i = 0; i < 2048/8; i++) {
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)ptr[i];
        if (i < (max-1) ) {
            ss << " ";
        }
    }

    return ss.str();
}

static std::string _out_ptr(const uint8_t* const val, int len) {

    std::stringstream ss;

    uint8_t* ptr = (uint8_t*)val;

    for (int i = 0; i < len; i++) {
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)ptr[i];
        if (i < (len - 1)) {
            ss << " ";
        }
    }

    return ss.str();

}

static std::string _out_kcv_type(int val) {

    std::string retVal;

    KCV_Algorithm_Type type = (KCV_Algorithm_Type)val;

    switch (type) {
        case KCV_NONE:
            retVal = "KCV_NONE";
            break;
        case KCV_AES_256_CBC:
            retVal = "KCV_AES_256_CBC";
            break;
        case KCV_SHA_256_NONE:
            retVal = "KCV_SHA_256_NONE";
            break;
        case KCV_AES_128_CBC:
            retVal = "KCV_AES_128_CBC";
            break;
        case KCV_AES_192_CBC:
            retVal = "KCV_TDES_128_CBC";
            break;
        case KCV_TDES_128_CBC:
            retVal = "KCV_TDES_128_CBC";
            break;
        case KCV_TDES_192_CBC:
            retVal = "KCV_TDES_192_CBC";
            break;
        case KCV_TDES_128_ECB:
            retVal = "KCV_TDES_128_ECB";
            break;
        case KCV_TDES_192_ECB:
            retVal = "KCV_TDES_192_ECB";
            break;
        default:
            retVal = "Unknown";
            break;
    }

    return retVal;
}

static void StoreValue ( uint8_t* const dst, int& offset, const uint8_t* const src, int len ) {
    memcpy ( dst + offset, src, len );
    offset += len;
}

static void StoreValue ( uint8_t* const dst, int& offset, uint64_t src, int len ) {
    memcpy(dst + offset, &src, len);
    offset += len;
}

HostKeyManager::HostKeyManager() {

    // m_nKeyModeCTK = KM_AES_256_CBC;
    // memset(m_strBuffer, 0, sizeof(m_strBuffer));
    // memset(m_strDebugString, 0, sizeof(m_strDebugString));
    // memset(m_strLogFile, 0, sizeof(m_strLogFile));

    memset ( m_strOutDir,  0,  sizeof(m_strOutDir)  );
    memset ( m_pSID,       0,  sizeof(m_pSID)       );

    // memset(m_pNullKey, 0, sizeof(m_pNullKey));
    // m_bUseSimpleKeys = false;

    m_pCryptoMgr = HostCryptoMgr::GetInstance();
    m_pCryptoMgr->Init();
}

HostKeyManager::~HostKeyManager() {

}

u32 HostKeyManager::GetKeySizeForVariableMode ( u16 nKeyMode ) {

    u32 nKeySize = 0;

    switch (nKeyMode) {

        //Only modes supported for variable mode slots
        case KM_AES_256_CBC:
            nKeySize = 32;
            break;

        case KM_AES_128_CBC:
            nKeySize = 16;
            break;

        case KM_TDES_ABA_ECB:
            nKeySize = 16;
            break;

        case KM_TDES_ABA_CBC:
            nKeySize = 16;
            break;

        case KM_TDES_ABC_ECB:
            nKeySize = 24;
            break;

        case KM_TDES_ABC_CBC:
            nKeySize = 24;
            break;

        default:
            break;
    }

    return nKeySize;
}

u16 HostKeyManager::GetKeySizeFromSlot ( u16 nSlot, u16 nKeyMode ) {

    u16 nKeySize = 0;

    switch ( nSlot ) {

        // 2048 bit keys
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
            nKeySize = 256;
            break;

        // 256 bit keys
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
            nKeySize = 32;
            break;

        // 192 bit keys
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            nKeySize = 24;
            break;

        // 128 bit keys
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
            nKeySize = 16;
            break;

        // 32 bit keys
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
            nKeySize = 4;
            break;

        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
            nKeySize = 8;
            break;

        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_EXT_BSK:
        case KT_KEY_CMPK:
            nKeySize = GetKeySizeForVariableMode(nKeyMode);
            break;

        // No Crypto
        case KT_EXT_SP:
        case KT_EXT_LAST:
        default:
            nKeySize = 0;
            break;

    }

    return nKeySize;
}

void HostKeyManager::SetDeviceInfo ( u16 nVendorID, u16 nProductID, u64 nSerialNum ) {

    memset( &m_pSID, 0, sizeof(m_pSID) );

    u8* pTmp = (u8*)&nVendorID;

    m_pSID[0] = pTmp[1];
    m_pSID[1] = pTmp[0];

    pTmp = (u8*)&nProductID;
    m_pSID[2] = pTmp[1];
    m_pSID[3] = pTmp[0];

    pTmp = (u8*)&nSerialNum;
    m_pSID[4] = 0xee;
    m_pSID[5] = pTmp[5];
    m_pSID[6] = pTmp[4];
    m_pSID[7] = pTmp[3];
    m_pSID[8] = pTmp[2];
    m_pSID[9] = pTmp[1];
    m_pSID[10] = pTmp[0];
    m_pSID[11] = 0x23;
    m_pSID[12] = 0x01;

    m_pSID[13] = '\0';
    m_pSID[14] = '\0';
    m_pSID[15] = '\0';
}

void HostKeyManager::InitializeKey(_V100_ENC_KEY_TYPE type, _V100_ENC_KEY_MODE mode, bool nUseSimpleKeys /* = false */) {

    // make a copy of the key and change to the selected mode
    HostKeySlot slot(*HostKeyMap::GetKey(type));
    slot.info.mode = mode;

    // figure out the properties of the key
    slot.info.size = m_pCryptoMgr->GetKeySizeFromSlot(type, mode);
    u16 nKCVType   = m_pCryptoMgr->GetKCVTypeFromSlot(type, mode);

    // fill in the key value based on those properties
    // hidres_t result = DeriveKey(slot.info.type, slot.info.mode, slot.info.size, slot.p_key, nUseSimpleKeys);
    // if (RES_API_OK != result) {
    //     return;
    // }
    //
    // // update the Key Check Value
    // bool bResult = m_pCryptoMgr->CalculateKCV(slot.p_key, slot.info.size, KCV_LENGTH, (u8*)&slot.info.kcv, nKCVType);
    // if (false == bResult) {
    // }

    // save the key
    HostKeyMap::SetKey(slot);
}

const char* HostKeyManager::GetKeySlotStr(u16 nKeySlot) {

    uint nIdx = nKeySlot;

    if (nKeySlot > KT_EXT_LAST) {
        if (nKeySlot == KT_KEY_CMPK) {
            return "KT_KEY_CPK";
        }

        nIdx = nKeySlot - 4072;
    }

    return KEY_TYPE_STR[nIdx];
}

hidres_t HostKeyManager::SetAnbio(u128 nANBIO) {

    memcpy( &m_ANBIO, &nANBIO, sizeof(u128) );
    return RES_API_OK;
}

hidres_t HostKeyManager::GetAnsol() {

    if (false == m_pCryptoMgr->GetRandomNumber(m_ANSOL, ANSOL_LENGTH)) {
        return RES_ERR_CRYPTO_FAIL;
    }

    return RES_API_OK;
}

hidres_t HostKeyManager::GenerateRsaKeyBlock(u256& ANBIO, u256& ANSOL, ClientKeyInfo& parentKey, ClientKeyInfo& childKey, u8* pOutBlock, uint& iOutBlockLen) {

    IRSA*   pRSA            = nullptr;          //
    u8      keyBlock[512]   = { 0 };            //
    uint    keyBlockLen     = sizeof(keyBlock); //
    uint    outLen          = 0;                //
    bool    bResult;                            //

    if (iOutBlockLen < sizeof(u2048)) {
        return RES_ERR_INTERNAL;
    }

    memset(pOutBlock, 0xBB, iOutBlockLen);
    do_build_key_block(ANBIO, ANSOL, childKey, keyBlock, keyBlockLen);

    pRSA = ICypher::GetInstance()->GetRSA();
    if (pRSA == nullptr) {
        return RES_ERR_CRYPTO_FAIL;
    }

    bResult = pRSA->Init(RSA_PKCS_V15, SIG_RSA_SHA256);
    if (false == bResult) {
        return RES_ERR_CRYPTO_FAIL;
    }

    bResult = pRSA->SetMode(RSA_PKCS_V15, SIG_RSA_SHA256);
    if (false == bResult) {
        return RES_ERR_CRYPTO_FAIL;
    }

    bResult = pRSA->SetContextPublic(parentKey.KeyVal, MAX_KEY_SIZE, EXPONENT);
    if (false == bResult) {
        return RES_ERR_CRYPTO_FAIL;
    }

    bResult = pRSA->Encrypt(NULL, RSA_PUBLIC, keyBlockLen, keyBlock, pOutBlock);
    if (false == bResult) {
        return RES_ERR_CRYPTO_FAIL;
    }

    iOutBlockLen = sizeof(u2048);

    return RES_API_OK;
}

hidres_t HostKeyManager::GenerateAesKeyBlock(u256& ANBIO, u256& ANSOL, ClientKeyInfo& parentKey, ClientKeyInfo& childKey, u8* pOutBlock, uint& iOutBlockLen) {

    IAES*   pAES            = nullptr;
    u8      keyBlock[512]   = {0};
    uint    keyBlockLen     = sizeof(keyBlock);
    uint    outLen          = 0;

    memset(pOutBlock, 0xBB, iOutBlockLen);
    do_build_key_block(ANBIO, ANSOL, childKey, keyBlock, keyBlockLen);

    keyBlockLen += 15;
    keyBlockLen -= (keyBlockLen % 16);

    if (keyBlockLen > iOutBlockLen) {
        return RES_ERR_INTERNAL;
    }

    pAES = ICypher::GetInstance()->GetAES();
    pAES->Encrypt( keyBlock, pOutBlock, keyBlockLen, parentKey.KeyVal, parentKey.KeyLen, nullptr, parentKey.nKeyMode);

    iOutBlockLen = keyBlockLen;

    return RES_API_OK;
}

hidres_t HostKeyManager::do_build_key_block (const u128 pANBIO, const u128 pANSOL, const ClientKeyInfo& keyInfo, u8 * const pOutBlock, uint& iOutBlockLen) {

    u16         kcvType  = { 0 };   //
    u8          kcv[4]   = { 0 };   //
    u16         keyCryptoSize = 0;  //
    uint        minLen;             //

    minLen = 48 + keyInfo.KeyLen;

    if (iOutBlockLen < minLen) {
        return RES_ERR_INTERNAL;
    }

    memset ( pOutBlock, 0xAA, iOutBlockLen );

    kcvType = m_pCryptoMgr->GetKCVTypeFromSlot(keyInfo.slot, CTK_KEY_MODE);
    m_pCryptoMgr->CalculateKCV(keyInfo.KeyVal, keyInfo.KeyLen, KCV_LENGTH, kcv, kcvType);

    int offset = 0;
    StoreValue ( pOutBlock, offset, pANBIO,               sizeof(u128)  ); // 16 bytes ANBIO
    StoreValue ( pOutBlock, offset, pANSOL,               sizeof(u128)  ); // 16 bytes ANSOL
    StoreValue ( pOutBlock, offset, keyInfo.slot,         sizeof(u16)   ); //  2 bytes SLOT
    StoreValue ( pOutBlock, offset, keyInfo.nkKeyVersion, sizeof(u16)   ); //  2 bytes KEY_VERSION
    StoreValue ( pOutBlock, offset, keyInfo.nKeyMode,     sizeof(u16)   ); //  2 bytes KEY_MODE
    StoreValue ( pOutBlock, offset, kcv,                  KCV_LENGTH    ); //  4 bytes KCV
    StoreValue ( pOutBlock, offset, keyCryptoSize,        sizeof(u16)   ); //  2 bytes KEY CRYPTO SIZE
    StoreValue ( pOutBlock, offset, keyInfo.KeyVal,       keyInfo.KeyLen ); // ?? bytes KEY MATERIAL

    iOutBlockLen = offset;

    return RES_API_OK;
}

hidres_t HostKeyManager::do_get_rand(u8* pDstBuff, u16 len) {

    bool ioRes;

    ioRes = m_pCryptoMgr->GetRandomNumber(pDstBuff, len);

    if (false == ioRes) {
        return RES_ERR_CRYPTO_FAIL;
    }

    return RES_API_OK;
}

hidres_t HostKeyManager::do_activate_cmpk (u16& nKeyMode) {

    hidres_t nResult  = RES_API_OK;  // Overall result of this function
    u16      nKeySize = 0;           // The size of the re-generated key
    u256     keyVal   = {};          // The re-generated raw key value

    nResult = do_read_key_file(KT_KEY_CMPK, nKeyMode, nKeySize, keyVal);
    if (RES_API_OK != nResult) {
        return RES_ERR_INVALID_DATA;
    }

    if (!m_pCryptoMgr->SetActiveKey(KT_KEY_CMPK, keyVal, nKeySize, nKeyMode)) {
        return  nResult;
    }

    return nResult;
}

hidres_t HostKeyManager::do_read_key_file(u16 nKeySlot, u16& nKeyMode, u16& nKeySize, u8* pKeyVal) {

    hidres_t nResult = RES_API_OK;

    char strFileName[256];
    memset(strFileName, 0, 256);

    if (nKeySlot == KT_KEY_CMPK) {
        sprintf(strFileName, "../dat/SampleMKPrime.bin");
    } else {
        sprintf(strFileName, "%s/%s.bin", m_strOutDir, GetKeySlotStr(nKeySlot));
    }

    FILE* pFile = fopen(strFileName, "rb");
    if (pFile) {
        u16 nFileSize = 0;
        fseek(pFile, 0, SEEK_END);
        nFileSize = (u16)ftell(pFile);
        rewind(pFile);

        fread(&nKeyMode, 1, sizeof(u16), pFile);
        fread(&nKeySize, 1, sizeof(u16), pFile);

        if (nFileSize != (sizeof(u16) + sizeof(u16) + nKeySize))
        {
            nResult = RES_ERR_READING_DATA;
        } else {
            fread(pKeyVal, 1, nKeySize, pFile);
        }

        fclose(pFile);
    } else {
        nResult = RES_ERR_READING_DATA;
    }

    return nResult;
}

