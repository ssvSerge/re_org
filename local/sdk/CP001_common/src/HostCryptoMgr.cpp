#include <HostCryptoMgr.h>
#include <CryptoShared.h>

// #define KT_KEY_CMPK     0x2000  // Backend Key Slot for working with (Sample) Customer Master Prime Key


static u32 gCnt0 = 0;
static u32 gCnt1 = 0;

static HostCryptoMgr* g_pKeyManager = NULL;



u8 gDUKPT_MODE = DUKPT_RUN;

HostCryptoMgr::HostCryptoMgr() {

    m_pCypher            = ICypher::GetInstance();
    m_pCryptoAlgo       = NULL;
    m_nActiveKeySlot    = 0;
    m_nActiveKeySz      = 0;
    m_nMode             = 0;
    m_nKeyMode          = KM_MODE_NONE;
}

HostCryptoMgr::~HostCryptoMgr() {

}

HostCryptoMgr* HostCryptoMgr::GetInstance() {
    if (g_pKeyManager == NULL) {
        g_pKeyManager = new HostCryptoMgr();
    }
    return g_pKeyManager;
}

void HostCryptoMgr::Destroy() {
    if (g_pKeyManager != NULL) {
        delete g_pKeyManager;
        g_pKeyManager = NULL;
    }
}

bool HostCryptoMgr::Init() {

    if (false == m_pCypher->Initialize(ICryptoBSP::GetInstance())) {
        return false;
    }

    if (false == m_pCypher->GetAES()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetDES()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetSHA1()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetSHA256()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetSHA512()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetRAND()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetRSA()->Init(RSA_PKCS_V15, SIG_RSA_SHA1)) {
        return false;
    }

    if (false == m_pCypher->GetHMAC()->Init()) {
        return false;
    }

    if (false == m_pCypher->GetDUKPT()->Init()) {
        return false;
    }

    memset(m_ActiveKey, 0, sizeof(u2048));

    m_nActiveKeySlot    = 0;
    m_nActiveKeySz      = 0;
    m_nMode             = 0;
    m_nKeyMode          = KM_MODE_NONE;

    return true;
}

bool HostCryptoMgr::CalculateKCV(u8* pKey, u16 nKeySize, u32 nBytesToCheck, u8* pKCV, int nAlgo) {
    return CreateKeyCheckValue(pKey, nKeySize, 32, nBytesToCheck, pKCV, nAlgo);
}

bool HostCryptoMgr::CreateKeyCheckValue(u8* pKey, size_t nKey, size_t nZeros, size_t nVals, u8* pKCV, int nAlg) {

    IDES*       pDES        = ICypher::GetInstance()->GetDES();
    IAES*       pAES        = ICypher::GetInstance()->GetAES();
    ISHA256*    pSHA256     = ICypher::GetInstance()->GetSHA256();

    switch (nAlg) {

        case KCV_AES_128_CBC:
        case KCV_AES_192_CBC:
        case KCV_AES_256_CBC:
        case KCV_TDES_128_CBC:
        case KCV_TDES_192_CBC:
        case KCV_TDES_128_ECB:
        case KCV_TDES_192_ECB:
        case KCV_SHA_256_NONE:
            // Only KCV modes supported
            break;
        default:
            return false;
    }

    if ( (nZeros % 32 != 0) || (nVals > 32) || (nZeros < sizeof(u256)) ) {
        return false;
    }

    u8* zeros   = (u8*)malloc(nZeros);
    u8* tmp     = (u8*)malloc(nZeros);
    u8* pIV     = (u8*)malloc(16);

    if ((zeros == NULL) || (tmp == NULL) || (pIV == NULL)) {
        goto ABORT;
    }

    memset(zeros, 0, nZeros);
    memset(pIV, 0, 16);

    switch (nAlg) {

        case KCV_TDES_128_CBC:
            if (!pDES->Encrypt(zeros, tmp, (int)nZeros, pKey, &pKey[8], pKey, pIV, TDES_CBC))
                goto ABORT;
            break;

        case KCV_TDES_192_CBC:
            if (!pDES->Encrypt(zeros, tmp, (int)nZeros, pKey, &pKey[8], &pKey[16], pIV, TDES_CBC))
                goto ABORT;
            break;

        case KCV_TDES_128_ECB:
            if (!pDES->Encrypt(zeros, tmp, (int)nZeros, pKey, &pKey[8], pKey, pIV, TDES_ECB))
                goto ABORT;
            break;

        case KCV_TDES_192_ECB:
            if (!pDES->Encrypt(zeros, tmp, (int)nZeros, pKey, &pKey[8], &pKey[16], pIV, TDES_ECB))
                goto ABORT;
            break;

        case KCV_AES_128_CBC:
        case KCV_AES_192_CBC:
        case KCV_AES_256_CBC:
            if (!pAES->Encrypt(zeros, tmp, (int)nZeros, pKey, (int)nKey, pIV, AES_CBC))
                goto ABORT;
            break;

        case KCV_SHA_256_NONE:
            if (!pSHA256->Hash(pKey, (int)nKey, tmp))
                goto ABORT;
            break;

        default:
            goto ABORT;
    }

    memcpy(pKCV, tmp, nVals);

    free(zeros);
    free(tmp);
    free(pIV);

    return true;

ABORT:
    free(zeros);
    free(tmp);
    free(pIV);

    return false;

}

u16 HostCryptoMgr::GetKCVTypeFromSlot(u16 nSlot, u16 nKeyMode) {

    u16 nKCVType = 0;

    switch (nSlot) {

            // AES_256_CBC
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
            nKCVType = KCV_AES_256_CBC;
            break;

        // 128 bit keysAES_128_CBC
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
            nKCVType = KCV_AES_128_CBC;
            break;

        // TDES_192_CBC
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            nKCVType = KCV_TDES_192_CBC;
            break;

        // TDES_128_CBC
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
            nKCVType = KCV_TDES_128_CBC;
            break;

        // SHA_256_NONE
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
            nKCVType = KCV_SHA_256_NONE;
            break;

        // KCV type depends on KeyMode for these slots
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_EXT_BSK:
        case KT_KEY_CMPK:
            nKCVType = GetKCVTypeForVariableMode(nKeyMode);
            break;

        // No KCV
        case KT_EXT_SP:
        case KT_EXT_LAST:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        default:
            nKCVType = KCV_NONE;
            break;
    }

    return nKCVType;
}

u32 HostCryptoMgr::GetKCVTypeForVariableMode(u16 nKeyMode) {

    u32 KCVType = KCV_NONE;

    switch (nKeyMode) {

        case KM_AES_256_CBC:
            KCVType = KCV_AES_256_CBC;
            break;

        case KM_AES_128_CBC:
            KCVType = KCV_AES_128_CBC;
            break;

        case KM_TDES_ABA_ECB:
            KCVType = KCV_TDES_128_ECB;
            break;

        case KM_TDES_ABA_CBC:
            KCVType = KCV_TDES_128_CBC;
            break;

        case KM_TDES_ABC_ECB:
            KCVType = KCV_TDES_192_ECB;
            break;

        case KM_TDES_ABC_CBC:
            KCVType = KCV_TDES_192_CBC;
            break;

        default:
            break;
    }

    return KCVType;

}

u16 HostCryptoMgr::GetKeySizeFromSlot(u16 nSlot, u16 nKeyMode) {

    u16 nKeySize = 0;

    switch (nSlot) {

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

u32 HostCryptoMgr::GetKeySizeForVariableMode(u16 nKeyMode) {

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

void HostCryptoMgr::ReleaseMem(u8* pBuffer) {
    if (pBuffer) {
        free(pBuffer);
        pBuffer = NULL;
    }
}

bool HostCryptoMgr::Encrypt(u8* pIn, u8** pOut, uint* nSizeInOut, u256 pDigSigOut) {

    if (NULL == m_pCryptoAlgo) {
        return false;
    }

    if ((NULL == pIn) || (0 == *nSizeInOut)) {
        return false;
    }

    // Alloc in Buffer with N_Block aligned and copy
    uint nInAligSz = *nSizeInOut;

    if (nInAligSz % N_BLOCK) {
        int nAdj = N_BLOCK - nInAligSz % N_BLOCK;
        nInAligSz += nAdj;
    }

    u8* pInBuff = (u8*)malloc(nInAligSz);
    if (NULL == pInBuff) {
        return false;
    }

    memset(pInBuff, 0, nInAligSz);
    memcpy(pInBuff, pIn, *nSizeInOut);

    // Determine the size of the output buffer and allocate it
    uint nEncryptSz = nInAligSz;
    if ((m_nActiveKeySlot == KT_EXTKEY_TDES0) || (m_nActiveKeySlot == KT_EXTKEY_TDES1)) {
        // For DUKPT consider KSN Header
        nEncryptSz += sizeof(KSNType);
    }

    uint nOutSz = nEncryptSz;

    *pOut = (u8*)malloc(nOutSz);
    if (NULL == *pOut) {
        return false;
    }

    // Encrypt
    if (false == m_pCryptoAlgo->EncryptData(pInBuff, *pOut, (int)nInAligSz, (int)nEncryptSz, (u8*)m_ActiveKey, m_nActiveKeySz, NULL, m_nMode)) {
        free(*pOut);
        free(pInBuff);
        *pOut = NULL;
        pInBuff = NULL;
        return false;
    }

    // Add digsig if requested
    if (pDigSigOut != NULL) {
        if (false == m_pCypher->GetSHA256()->Hash(pInBuff, (int)nInAligSz, pDigSigOut)) {
            free(*pOut);
            free(pInBuff);
            *pOut = NULL;
            pInBuff = NULL;
            return false;
        }
    }

    *nSizeInOut = nOutSz;
    free(pInBuff);
    pInBuff = NULL;
    return true;
}

bool HostCryptoMgr::Decrypt(u8* pIn, u8* pOut, uint* nSizeInOut, u256 pDigSigIn) {

    if (m_nActiveKeySlot == 0) {
        return false;
    }

    if (NULL == m_pCryptoAlgo) {
        return false;
    }

    if ((NULL == pIn) || (NULL == pOut) || (0 == *nSizeInOut)) {
        return false;
    }

    uint nDecryptSz = *nSizeInOut;
    uint nActualOutSz = nDecryptSz;

    if ((m_nActiveKeySlot == KT_EXTKEY_TDES0) || (m_nActiveKeySlot == KT_EXTKEY_TDES1)) {
        // For DUKPT consider KSN Header in CGPkt
        nActualOutSz -= sizeof(KSNType);
    }

    // Decrypt
    if (false == m_pCryptoAlgo->DecryptData(pIn, pOut, (int)nDecryptSz, (int)nActualOutSz, (u8*)m_ActiveKey, m_nActiveKeySz, NULL, m_nMode)) {
        return false;
    }

    // Check digsig if requested
    if (pDigSigIn != NULL) {

        u256 pCalDigSig;

        if (false == m_pCypher->GetSHA256()->Hash(pOut, (int)nActualOutSz, pCalDigSig)) {
            return false;
        }

        if (memcmp(pDigSigIn, pCalDigSig, sizeof(u256)) != 0) {
            return false;
        }
    }

    *nSizeInOut = nActualOutSz;
    return true;
}

bool HostCryptoMgr::SetActiveKey(u16 nKeySlot, u8* pKeyVal, u16 nKeySize, u16 nKeyMode) {

    switch (nKeySlot) {

        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_AES_VEND: {
            m_pCryptoAlgo = m_pCypher->GetAES();
            m_nMode = AES_CBC;
        }   break;

        case KT_EXTKEY_TDES0: {
            m_pCryptoAlgo = m_pCypher->GetDUKPT();

            int nExpKeySize = GetKeySizeFromSlot(KT_EXTKEY_TDES0, KM_DUKPT_IPEK_128) + GetKeySizeFromSlot(KT_EXTKEY_KSN_0, KM_DUKPT_KSN_64);
            if (nKeySize != nExpKeySize)
            {
                return false;
            }

            nKeySize -= GetKeySizeFromSlot(KT_EXTKEY_KSN_0, KM_DUKPT_KSN_64);// Update only with TDES0 size

            u32 cnt = 0;
            if (!GetTransaction_Counter(&cnt, TC0))        // Get counter
            {
                return false;
            }
            m_pCypher->GetDUKPT()->SetContext(pKeyVal, pKeyVal + nKeySize, cnt);
            m_nMode = TDES_CBC;
        }   break;

        case KT_EXTKEY_TDES1: {
            m_pCryptoAlgo = m_pCypher->GetDUKPT();
            int nExpKeySize = GetKeySizeFromSlot(KT_EXTKEY_TDES1, KM_DUKPT_IPEK_128) + GetKeySizeFromSlot(KT_EXTKEY_KSN_1, KM_DUKPT_KSN_64);
            if (nKeySize != nExpKeySize)
            {
                return false;
            }

            nKeySize -= GetKeySizeFromSlot(KT_EXTKEY_KSN_1, KM_DUKPT_KSN_64);//  Update only with TDES1 size

            u32 cnt = 0;
            if (!GetTransaction_Counter(&cnt, TC1))        // Get counter
            {
                return false;
            }
            m_pCypher->GetDUKPT()->SetContext(pKeyVal, pKeyVal + nKeySize, cnt);
            m_nMode = TDES_CBC;
        }   break;

        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3: {
            m_pCryptoAlgo = m_pCypher->GetDES();
            m_nMode = TDES_CBC;
        }   break;

        case KT_EXT_SP: {
            m_pCryptoAlgo = m_pCypher->GetNULL();// GetCRYPTONONE();
        }   break;

        // KCV type depends on KeyMode for these slots
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_EXT_BSK:
        case KT_KEY_CMPK: {

            //Get the algorithm and mode from the KeyMode.
            switch (nKeyMode) {

                case KM_AES_256_CBC: {
                    m_pCryptoAlgo = m_pCypher->GetAES();
                    m_nMode = AES_CBC;
                }   break;

                case KM_AES_128_CBC: {
                    m_pCryptoAlgo = m_pCypher->GetAES();
                    m_nMode = AES_CBC;
                }   break;

                case KM_TDES_ABA_ECB:
                case KM_TDES_ABC_ECB: {
                    m_pCryptoAlgo = m_pCypher->GetDES();
                    m_nMode = TDES_ECB;
                }   break;

                case KM_TDES_ABA_CBC:
                case KM_TDES_ABC_CBC: {
                    m_pCryptoAlgo = m_pCypher->GetDES();
                    m_nMode = TDES_CBC;
                }   break;

                case KM_RSA_2048_v15: // For provisioning these are not used for encryption
                case KM_RSA_2048_v21:
                default: {
                    return false;
                }
            }
        }   break;

        // Not supported at this time
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_LAST:
        default: {
            m_nActiveKeySlot = 0;
            return false;
        }
    }

    m_nActiveKeySz = GetKeySizeFromSlot(nKeySlot, nKeyMode);

    if (m_nActiveKeySz != nKeySize) {
        // Invalid key size passed in
        m_nActiveKeySlot = 0;
        return false;
    }

    m_nActiveKeySlot = nKeySlot;
    m_nKeyMode = (_V100_ENC_KEY_MODE)nKeyMode;

    if (nKeySlot != KT_EXT_SP) {
        memcpy(m_ActiveKey, pKeyVal, m_nActiveKeySz);
    } else {
        memset(m_ActiveKey, 0, sizeof(u2048));
    }

    return true;
}

bool HostCryptoMgr::GetTransaction_Counter(u32* pCnt, int whichone) {

    switch (whichone) {

        case TC0: {
            if (gDUKPT_MODE == DUKPT_RUN) {
                gCnt0++;
            }
            if (gCnt0 >= 0x1FFFFF) {
                return false;
            }
            *pCnt = gCnt0;
        }   break;

        case TC1: {
            if (gDUKPT_MODE == DUKPT_RUN) {
                gCnt1++;
            }
            if (gCnt1 >= 0x1FFFFF) {
                return false;
            }
            *pCnt = gCnt1;
        }   break;

    }

    return true;
}

bool HostCryptoMgr::GetRandomNumber(u8* pRnd, int nSize) {
    return m_pCypher->GetRAND()->Random(pRnd, nSize);
}



#if 0

bool HostCryptoMgr::PerformSelfDiagnostic(bool bVerbose) {
    if(false == m_pCypher->GetDUKPT()->Self_Test(true)) { return false; }
    if(false == m_pCypher->GetAES()->Self_Test(true))     { return false; }
    if(false == m_pCypher->GetDES()->Self_Test(true))     { return false; }
    if(false == m_pCypher->GetSHA1()->Self_Test(true))     { return false; }
    if(false == m_pCypher->GetSHA256()->Self_Test(true)){ return false; }
    if(false == m_pCypher->GetSHA512()->Self_Test(true)){ return false; }
    if(false == m_pCypher->GetRAND()->Self_Test(true))     { return false; }
    if(false == m_pCypher->GetRSA()->Self_Test(true))     { return false; }

    return true;
}

bool HostCryptoMgr::GetSHA256(u8* InputData, size_t InputDataSize, u256 pHashValue) {

    memset(pHashValue, 0, sizeof(u256));
    bool bRes = m_pCypher->GetSHA256()->Hash(InputData, InputDataSize, pHashValue);
    if (!bRes) {
        throw "SHA256 Fail.";
    }
    return bRes;
}

bool HostCryptoMgr::WriteASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, char *pFilename ) {
    u8 Token;
    u16 size;
    u8 MSB, LSB;
    //u8 Exp[] = { 0x01, 0x00, 0x01 };

    FILE *pFID = fopen( pFilename,"wb") ;
    if ( pFID == NULL )
        return false;

    Token = 0x30;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // Start

    Token = 0x82;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // Size

    size = (4 + (int)nKeySize) + (1 + 1 + (int)nExpSize);
    MSB = (size & 0xFF00) >> 8;
    LSB = (size & 0x00FF) ;
    fwrite ( &MSB, sizeof(u8), 1, pFID );    // Size
    fwrite ( &LSB, sizeof(u8), 1, pFID );    // Size

    Token = 0x02;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // INTEGER
    Token = 0x82;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // Size

    size = (u16)nKeySize;                            // Key Size
    MSB = (size & 0xFF00) >> 8;
    LSB = (size & 0x00FF) ;
    fwrite ( &MSB, sizeof(u8), 1, pFID );    // Size
    fwrite ( &LSB, sizeof(u8), 1, pFID );    // Size

    fwrite ( pKey, sizeof(u8), nKeySize, pFID );// write key

    Token = 0x02;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // INTEGER

    Token = (int) nExpSize;
    fwrite ( &Token, sizeof(u8), 1, pFID );    // size of Exp

    fwrite ( pExp, sizeof(u8), nExpSize, pFID );    //

    fclose(pFID);

    // clean up
    return true;
}

bool HostCryptoMgr::GenerateASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, unsigned char** pASNBuff,  size_t &ASNSz) {

    u8 Token;
    u16 size;
    u8 MSB, LSB;
    unsigned char* pOutBuff;

    if((pKey == NULL) || (pExp == NULL)) return false;

    size = (4 + (int)nKeySize) + (1 + 1 + (int)nExpSize);
    ASNSz = 2+2+size;//2Tokens each 1 byte + 2bytes for storing size + size
    *pASNBuff = (unsigned char*)MALLOC(ASNSz);
    if(!(*pASNBuff)) return false;

    memset(*pASNBuff, 0, ASNSz);
    pOutBuff = *pASNBuff;

    Token = 0x30;
    *pOutBuff++ = Token;    // Start

    Token = 0x82;
    *pOutBuff++ = Token;    // Size

    MSB = (size & 0xFF00) >> 8;
    LSB = (size & 0x00FF) ;
    *pOutBuff++ = MSB;    // Size
    *pOutBuff++ = LSB;    // Size

    Token = 0x02;
    *pOutBuff++ = Token;    // INTEGER
    Token = 0x82;
    *pOutBuff++ = Token;

    size = (u16)nKeySize;    // Key Size
    MSB = (size & 0xFF00) >> 8;
    LSB = (size & 0x00FF) ;
    *pOutBuff++ = MSB;
    *pOutBuff++ = LSB;

    // write key
    memcpy(pOutBuff, pKey, nKeySize);
    pOutBuff += nKeySize;

    Token = 0x02;
    *pOutBuff++ = Token;    // INTEGER

    Token = (int) nExpSize;
    *pOutBuff++ = Token;    // Size

    memcpy(pOutBuff, pExp, nExpSize);

    // clean up
    return true;
}

bool HostCryptoMgr::ParseASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, u8* pASNBuff) {

    union {
        u8  byte[2];
        u16 size;
    } nSize;

    u8 Token;

    Token = *pASNBuff++;        // Start
    if (Token != 0x30 )
        return false;

    // fine 1st INTEGER field
    do
    {
        Token = *pASNBuff++;    // Start
        if (Token == 0x02 )
            break;
    } while(true);

    Token = *pASNBuff++;        // Start
    if (Token != 0x82 )
        return false;

    // read size of key
    nSize.byte[1] = *pASNBuff++;
    nSize.byte[0] = *pASNBuff++;

    /*
    ** check for byte stuffing
    */
    if ( nSize.size == nKeySize+1 )
    {
        Token = *pASNBuff++;    // read stuff byte

        if (Token != 0x00 )
            return false;
    }

    memcpy(pKey, pASNBuff, nKeySize);// read key

    pASNBuff += nKeySize;
    Token = *pASNBuff++;        // Start INTEGER
    if (Token != 0x02 )
        return false;

    Token = *pASNBuff++;        // size
    if (Token != nExpSize )
        return false;

    memcpy(pExp, pASNBuff++, nExpSize);// size

    return true;
}

bool HostCryptoMgr::ValidateKeyMode(u16 nSlot, u16 nKeyMode) {

    u16 nStaticKeyMode;

    switch (nSlot) {
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
        {
            nStaticKeyMode = KM_AES_256_CBC;
        } break;
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        {
                nStaticKeyMode = KM_AES_128_CBC;
        } break;
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
        {
                 nStaticKeyMode = KM_DUKPT_IPEK_128;
        } break;
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        {
                 nStaticKeyMode = KM_TDES_ABC_CBC;
        } break;
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:// only key value saved on MAXQ
        case KT_EXTKEY_DEVICE_Q:// only key value saved on MAXQ
        case KT_EXTKEY_PRIVATE_EXP:// only key value saved on MAXQ
        case KT_EXTKEY_PUBLIC_EXP:// only key value saved on MAXQ
        {
            if (nKeyMode != KM_RSA_2048_v15 && nKeyMode != KM_RSA_2048_v21)
            {
                return false;
            }
            else
            {
                return true;
            }

        }
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
        {
                 nStaticKeyMode = KM_DUKPT_KSN_64;
        } break;
        // variable key mode slots
        case KT_EXT_BSK:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_KEY_CMPK:
        {
            switch (nKeyMode)
            {
                case KM_AES_256_CBC:
                case KM_AES_128_CBC:
                case KM_TDES_ABA_ECB:
                case KM_TDES_ABA_CBC:
                case KM_TDES_ABC_ECB:
                case KM_TDES_ABC_CBC:
                {
                  return true;
                }
                default:
                {
                return false;
                }
            }
        }
        default:
        {
            return false;
        }
    }

    if (nKeyMode != nStaticKeyMode)
    {
        return false;
    }

    return true;

}

bool HostCryptoMgr::Set_DUKPT_Mode(int iMode) {

    switch (iMode) {

        case DUKPT_RUN: {        // default Incrementing Counter Mode
            gDUKPT_MODE = DUKPT_RUN;
        }   break;

        case DUKPT_HALT: {    // Halts Transaction Counter
            gDUKPT_MODE = DUKPT_HALT;
        }   break;

        case DUKPT_RESET_TC0: {    // Reset Counter 0
            gCnt0 = 0;
        }

        case DUKPT_RESET_TC1: { // Reset Counter 1
            gCnt1 = 0;
        } break;

        default:
            goto ABORT;
    }

    return true;

ABORT:
    return false;
}

#endif
