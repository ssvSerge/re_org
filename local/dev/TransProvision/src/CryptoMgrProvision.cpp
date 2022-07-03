#include "CryptoMgrProvision.h"
#include "CryptoMgrDefs.h"
#include "ISecureElement.h"
#include "CriticalErrorLog.h"
#include "AutoHeapBuffer.h"
#include "CfgMgr.h"


static const size_t        RSA_BITS = 2048;

CryptoMgrProvision::CryptoMgrProvision() {

}

CryptoMgrProvision::~CryptoMgrProvision() {

}

/////////////////////////////////////////////////////////////////////////////
// Initialize
//
bool CryptoMgrProvision::Init() {

    // set ANBIO to a random number - do not set to all zeros - guessable...
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_GetRandomBuffer(reinterpret_cast<u8*>(&m_ANBIO), sizeof(m_ANBIO))) {
        LOGMSG("Init:Get random number returned error.");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Generate randoms.
//
bool CryptoMgrProvision::GetRandomNumber(u256* pRnd) {

    CryptExecStatus status;

    if (!pRnd) {
        return false;
    }

    status = SECURE_ELEMENT->Execute_GetRandomBuffer((u8*)pRnd, 32);

    if (CryptExecStatus::Successful != status) {
        return false;
    }

    memcpy ( m_ANBIO, pRnd, sizeof(u256) );
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Get GetANBIO from
//
bool CryptoMgrProvision::GetANBIO(u256* pRnd) {

    if ( ! pRnd ) {
        return false;
    }

    memcpy ( pRnd, m_ANBIO, sizeof(u256) );
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Generate and save device asymmetric keys
//
bool CryptoMgrProvision::GenerateAsymmetricKeys() {

    CryptExecStatus status;

    status = SECURE_ELEMENT->Execute_Generate_RSA_Key();

    if (CryptExecStatus::Successful != CrypStat) {
        LOGMSG("GetRandomNumber:Getting Random number returned error");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Get KT_EXTKEY_DEVICE_PUBLIC key.
//
bool CryptoMgrProvision::GetKey ( _V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize ) {

    memset ( pKey, 0, sizeof(u2048) );

    if (pKCV32 == NULL) {
        return false;
    }

    // Only Keys allowed to get in provisioning.
    if (nKeyType != KT_EXTKEY_DEVICE_PUBLIC) {
        LOGMSG("GetKey:Retrieving Key %d is not supported", nKeyType);
        return false;
    }

    uint8_t*    pOut        = nullptr;
    uint32_t    nOutSize    = 0;

    CryptExecStatus CrypStat = SECURE_ELEMENT->Execute_Get_RSA_PublicKey(&pOut, nOutSize);
    if (CrypStat != CryptExecStatus::Successful) {
        LOGMSG("GetKey:Retrieving Key %d returned error", nKeyType);
        return false;
    }

    if (nOutSize != sizeof(u2048)) {
        LOGMSG("GetKey:Keysize retrieved for Key %d is not valid", nKeyType);
        ReleaseSEBuff(&pOut);
        return false;
    }

    memcpy (pKey, pOut, nOutSize);
    nKeySize = nOutSize;
    ReleaseSEBuff(&pOut);

    return GetKeyInfo(nKeyType, nKeyVer, nKeyMode, pKCV32);
}

/////////////////////////////////////////////////////////////////////////////
// Returns nKeyVer, nKeyMode and pKCV32 for
//  KT_EXTKEY_DEVICE_PUBLIC or KT_EXTKEY_CTK.
//
bool CryptoMgrProvision::GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32) {

    CryptExecStatus status;

    if ((nKeyType != KT_EXTKEY_DEVICE_PUBLIC) && (nKeyType != KT_EXTKEY_CTK)) {
        LOGMSG("GetKeyInfo:Retrieving KeyInfo for Key %d is not supported", nKeyType);
        return false;
    }

    KeyInfoStructure KeyInfo;
    u16 nSlot = 0;
    if (!GetKeySlot(nKeyType, nSlot)) {
        return false;
    }

    status = SECURE_ELEMENT->Execute_Get_Key_Info(nSlot, KeyInfo);
    if (CryptExecStatus::Successful != status) {
        LOGMSG("GetKeyInfo:Retrieving KeyInfo for Key %d returned error", nKeyType);
        return false;
    }

    nKeyVer = KeyInfo.key_ver;
    nKeyMode = (_V100_ENC_KEY_MODE)KeyInfo.key_mode;
    memcpy(pKCV32, KeyInfo.key_check_value, KCV_SIZE);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Programming the KT_EXTKEY_CTK (Customer Transportation Key)
//      nKeyType                only KT_EXTKEY_CTK supported.
//      pCG, nCGSize,           Cypher text and Length of message
//      pOutCG, nOutCGSize      Cypher text of ANSOL encrypted with KT_EXTKEY_CTK
//
//  Plain text contains [KeyType] [KeyVersion] [KCV] [ANBIO] [RawKey]
//  Plain text might (not must) be encrypted with the Device Public Key.
//
bool CryptoMgrProvision::SetKey(_V100_ENC_KEY_TYPE nKeyType, u8* pCG, uint nCGSize, u8** pOutCG, uint& nOutCGSize) {

    bool bRes = false;

    if ( (pCG == NULL) || (nCGSize == 0) ) {
        LOGMSG("SetKey:Initial input arguments validation failed");
        return false;
    }

    if ( nKeyType != KT_EXTKEY_CTK ) {
        LOGMSG("SetKey:Setting Key %d is not supported", nKeyType);
        return false;
    }

    bRes = IsKeyLoaded(KT_EXTKEY_CTK);

    if ( true == bRes ) {
        LOGMSG("SetKey: Key %d is already loaded. Reloading not allowed", KT_EXTKEY_CTK);
        return false;
    }

    // We must decrypt using RSA.
    // First check if we have two cryptograms or one cryptogram
    uint nCG1Size = 0;
    uint nCG2Size = 0;

    if ( nCGSize == (RSA_BITS/8) * 1 ) {
        nCG1Size = nCGSize;
        nCG2Size = 0;
    } else
    if ( nCGSize == (RSA_BITS/8) * 2 ) {
        nCG1Size = (RSA_BITS / 8);
        nCG2Size = (RSA_BITS / 8);
    } else {
        return false;
    }

    // Allocate space for plain text Key block
    AutoHeapBuffer Auto_pPlainText(nCG1Size);

    u8* pPlainText = Auto_pPlainText.u8Ptr();

    if (pPlainText == NULL) {
        return false;
    }

    uint nPTSize = 0;

    // Decrypt pCG with KT_EXTKEY_DEVICE_PRIVATE.
    bRes = PKIDecryptWithDevicePrivKey(pCG, pPlainText, nCG1Size, (u32*)&nPTSize);

    if ( bRes == false ) {
        LOGMSG("SetKey:Decrypting the input CG1 step for setting Key %d returned error", nKeyType);
        return false;
    }

    // Takes the non-validate pPlainText packet, and fills out appropriate fields.
    u16     nKeyTypeRead        = 0;
    u16     nKeyVer             = 0;
    u16     nKeyMode            = KM_MODE_NONE;
    u8      nKCV32[KCV_SIZE]    = {0};
    u8      pANBIO[ANBIO_SIZE]  = {0};
    u8      pANSOL[ANSOL_SIZE]  = {0};
    u8*     pCGKey              = NULL;
    u8*     pKey                = NULL;
    u32     nKeySize            = 0;
    u16     nKeyCryptoSize      = 0;

    // De-serialize key parameters from internal buffer.
    // Key material encrypted with KT_EXTKEY_CTK
    //  nKeySize            -> length of the key.
    //  nKeyCryptoSize == 0 -> pCGKey is in the plain form
    //  nKeyCryptoSize != 0 -> pCGKey encrypted with RSA Device Public Key
    bRes = DecodeKeyFromOpaque ( pPlainText, nPTSize, &nKeyTypeRead, &nKeyVer, &nKeyMode, nKCV32, &nKeyCryptoSize, pANBIO, pANSOL, &pCGKey, nKeySize );

    if ( false == bRes ) {
        LOGMSG("SetKey:Decoding the decrypted CG1 step for setting Key %d returned error", nKeyType);
        return false;
    }

    AutoFreePtr afp_CGKey(pCGKey);

    // Only KT_EXTKEY_CTK allowed here.
    if ( nKeyTypeRead != KT_EXTKEY_CTK ) {
        LOGMSG("SetKey:KeyType %d provided in KeyBlock is not supported", nKeyTypeRead);
        return false;
    }

    // If key is encrypted nKeyCryptoSize is non zero and equal to nCG2Size otherwise both should be zero.
    if ( nKeyCryptoSize != nCG2Size ) {
        LOGMSG("SetKey:KeyCryptoSize provided in KeyBlock is not valid");
        return false;
    }

    AutoHeapBuffer Auto_pKey(nCG2Size);

    if ( nKeyCryptoSize != 0 ) {

        pKey = Auto_pKey.u8Ptr();

        if (pKey == NULL) {
            return false;
        }

        if (false == PKIDecryptWithDevicePrivKey(pCG + nCG1Size, pKey, nCG2Size, &nKeySize)) {
            LOGMSG("SetKey:Decrypting the CG2 step for setting Key %d returned error", nKeyType);
            return false;
        }

    } else {
        pKey = pCGKey;
    }

    // Validate the key information.
    //  [nKeyTypeRead nKeyMode] must be valid
    //  [nKCV32] must match to the calculated one.
    //  [pANBIO] must be the same like m_ANBIO
    bRes = ValidateOpaque(nKeyTypeRead, nKeyVer, nKeyMode, nKCV32, pANBIO, pKey, nKeySize);
    if (false == bRes) {
        LOGMSG("SetKey:Validating the opaque data step for setting Key %d returned error", nKeyTypeRead);
        return false;
    }

    // Zero values aren't allowed. Just reject the request.
    if (true == IsZeroVecKey(pKey, nKeySize)) {
        LOGMSG("SetKey:Setting Key %d with ZeroVec is not allowed ", nKeyType);
        return false;
    }

    KeyInfoStructure KeyInfo;
    memcpy(KeyInfo.key_check_value, nKCV32, KCV_SIZE);

    KeyInfo.key_mode    = (KeyModes)nKeyMode;
    KeyInfo.key_size    = nKeySize;
    KeyInfo.key_ver     = nKeyVer;

    {   // Setup the KT_EXTKEY_CTK (Customer Transportation Key)
        bRes = GetKeySlot(nKeyType, KeyInfo.slot);
        if ( false == bRes ) {
            LOGMSG("SetKey:Setting Key %d returned error", nKeyType);
            return false;
        }

        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(KeyInfo, pKey)) {
            LOGMSG("SetKey:Setting Key %d returned error", nKeyType);
            return false;
        }

    }

    {   //
        u8* pTmpOutCG = nullptr;
        uint nTmpOutCGSz = 0;

        bRes = Encrypt(nKeyType, pANSOL, ANSOL_SIZE, &pTmpOutCG, nTmpOutCGSz);
        if (false == bRes) {
            ReleaseSEBuff(&pTmpOutCG);
            LOGMSG("SetKey:Encrypting ANSOL step returned error");
            return false;
        }

        *pOutCG = (u8*)MALLOC(nTmpOutCGSz);
        memcpy(*pOutCG, pTmpOutCG, nTmpOutCGSz);
        nOutCGSize = nTmpOutCGSz;
        ReleaseSEBuff(&pTmpOutCG);

        memset(pKey, 0, nKeySize);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
//*** Validate packet and remove CTK ***//
//
bool CryptoMgrProvision::Decommission(u8* pInCG, uint nInCGSize) {

    if ((NULL == pInCG) || (0 == nInCGSize)) {
        LOGMSG("Decommission:Initial input argument validation failed");
        return false;
    }

    uint8_t* pBuffer = nullptr;
    uint32_t nBufferSize = 0;

    if (false == Decrypt(KT_EXTKEY_CTK, pInCG, nInCGSize, &pBuffer, nBufferSize))
    {
        LOGMSG("Decommission:Decrypting with Key %d returned error", KT_EXTKEY_CTK);
        return false;
    }

    if (nBufferSize < ANBIO_SIZE)
    {
        ReleaseSEBuff(&pBuffer);
        LOGMSG("Decommission:Decrypted buffer size is not valid");
        return false;
    }
    //Validate ANBIO sent in
    if (memcmp(pBuffer, m_ANBIO, ANBIO_SIZE) != 0)
    {
        ReleaseSEBuff(&pBuffer);
        LOGMSG("Decommission:ANBIO validation failed");
        return false;
    }

    ReleaseSEBuff(&pBuffer);
    u16 nSlot = 0;
    if (!GetKeySlot(KT_EXTKEY_CTK, nSlot))
    {
        return false;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(nSlot))
    {
        LOGMSG("Decommission:Removing Key %d returned error", KT_EXTKEY_CTK);
        return false;
    }

    return true;
}


/******************************************************************************/
//*** Helper functions ***//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////
//*** Decrypt input CG with Device asymmetric keys ***//
//
bool CryptoMgrProvision::PKIDecryptWithDevicePrivKey(u8* pCipherText, u8* pPlainText, u32 nOutMaxBytes, u32* nOutBytes) {

    *nOutBytes = 0;

    if (nOutMaxBytes > sizeof(u2048) || !pCipherText || !pPlainText) {
        LOGMSG("PKIDecryptWithDevicePrivKey:Initial input arguments validation failed");
        return false;
    }

    uint8_t* pBuffer = nullptr;
    uint32_t nBufferSize = 0;
    if (false == Decrypt(KT_EXTKEY_DEVICE_PRIVATE, pCipherText, nOutMaxBytes, &pBuffer, nBufferSize)) {
        LOGMSG("PKIDecryptWithDevicePrivKey:Decrypting with  Key %d returned error", KT_EXTKEY_DEVICE_PRIVATE);
        return false;
    }

    if (nBufferSize > nOutMaxBytes) {
        LOGMSG("PKIDecryptWithDevicePrivKey:Decrypt output size %d exceeds maxbytes %d", nBufferSize, nOutMaxBytes);
        ReleaseSEBuff(&pBuffer);
        return false;
    }

    memcpy(pPlainText, pBuffer, nBufferSize);
    *nOutBytes = nBufferSize;
    ReleaseSEBuff(&pBuffer);
    return true;

}

/////////////////////////////////////////////////////////////////////////////
// Pulls fields out from plain text buffer and store to variables
//  [ANBIO][ANSOL][KeyType][KeyVer][KeyMode][KCV][KeySize][KeyMaterial][PAD]
//      nKeyType        = [KeyType]
//      nKeyVer         = [KeyVer]
//      nKeyMode        = [KeyMode]
//      pKCV32          = [KCV]
//      nKeyCryptoSize  = [KeySize] Might be zero.
//                                  Zero: Key is in the plain form.
//                                  Non zero: Key material is encrypted.
//      pANBIO          = [ANBIO]
//      pANSOL          = [ANSOL]
//      pKey            = [KeyMaterial] encrypted with RSA public key
//      nKeySize        = Key length calculated from [nKeyType & nKeyMode]
//
bool CryptoMgrProvision::DecodeKeyFromOpaque(u8* pPlainText, uint nPlainTextSize, u16* nKeyType, u16* nKeyVer, u16* nKeyMode, u8* pKCV32, u16* nKeyCryptoSize, u8* pANBIO, u8* pANSOL, u8** pKey, u32& nKeySize) {

    nKeySize = 0;

    if ( nPlainTextSize < (ANBIO_SIZE + ANSOL_SIZE + sizeof(u16) * 3 + KCV_SIZE) ) {
        LOGMSG("DecodeKeyFromOpaque: Input buffer size is not valid");
        return false;
    }

    if ( (pPlainText == NULL) || (pKCV32 == NULL) ) {
        LOGMSG("DecodeKeyFromOpaque: Initial input argument validation failed");
        return false;
    }

    u8* pPtr = pPlainText;

    memcpy ( pANBIO,         pPtr,   ANBIO_SIZE );  pPtr += ANBIO_SIZE;   // [ANBIO]
    memcpy ( pANSOL,         pPtr,   ANSOL_SIZE );  pPtr += ANSOL_SIZE;   // [ANSOL]
    memcpy ( nKeyType,       pPtr,  sizeof(u16) );  pPtr += sizeof(u16);  // [KeyType]
    memcpy ( nKeyVer,        pPtr,  sizeof(u16) );  pPtr += sizeof(u16);  // [KeyVer]
    memcpy ( nKeyMode,       pPtr,  sizeof(u16) );  pPtr += sizeof(u16);  // [KeyMode]
    memcpy ( pKCV32,         pPtr,     KCV_SIZE );  pPtr += KCV_SIZE;     // [KCV]
    memcpy ( nKeyCryptoSize, pPtr,  sizeof(u16) );  pPtr += sizeof(u16);  // [KeySize]

    nKeySize = GetKeySize( *nKeyType, *nKeyMode );
    if (nKeySize == 0) {
        LOGMSG("DecodeKeyFromOpaque: Key %d is not supported", *nKeyType);
        return false;
    }

    // Validate values to prevent the buffer overrun.
    // Now we have the key size expected. check again if the input buffer size is correct.
    if ( nPlainTextSize < (ANBIO_SIZE + ANSOL_SIZE + sizeof(u16) * 3 + KCV_SIZE + nKeySize) ) {
        LOGMSG("DecodeKeyFromOpaque: Input buffer size is not valid");;
        return false;
    }

    // Make sure if the key is encrypted, encyrptedSize provided is at least KeySize calculated from KeyMode
    if ( (*nKeyCryptoSize != 0) && (*nKeyCryptoSize < nKeySize) ) {
        LOGMSG("DecodeKeyFromOpaque: KeyCryptoSize provided in KeyBlock is not valid");
        return false;
    }

    *pKey = (u8*)MALLOC(nKeySize);

    if (NULL == *pKey) {
        LOGMSG("DecodeKeyFromOpaque: Failed to allocate memory.");
        return false;
    }

    memcpy ( *pKey, pPtr, nKeySize );

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Validate Key parameters after of plain text parsing.
//   nKeyVer not used.
//   Pair [KeyType KeyMode] must be valid.
//   KCV calculated must be the same like pKCV32
//   pANBIO must be the same like m_ANBIO
//
bool CryptoMgrProvision::ValidateOpaque(u16 nKeyType, u16 nKeyVer, u16 nKeyMode, u8* pKCV32, u8* pANBIO, u8* pKey, u32 nKeySize) {

    u8        pCalculatedKCV[KCV_SIZE];
    int     nKCVType;
    bool    ioRes;

    if (pKCV32 == NULL || pKey == NULL || nKeySize == 0) {
        LOGMSG("CryptoMgrProvision::ValidateOpaque: Bad input parameters");
        return false;
    }

    // Check the KeyMode supported by KeyType specified.
    ioRes = ValidateKeyMode(nKeyType, nKeyMode);
    if (false == ioRes) {
        LOGMSG("CryptoMgrProvision::ValidateOpaque: Wrong key mode for the key type specified.");
        return false;
    }

    // Get the KCV on the base of [KeyType + KeyMode]
    //      KCV_AES_256_CBC
    //      KCV_AES_128_CBC
    //      KCV_TDES_128_CBC
    //      KCV_TDES_128_ECB
    //      KCV_TDES_192_CBC
    //      KCV_TDES_192_ECB
    //      KCV_SHA_256_NONE
    //      KCV_NONE
    nKCVType = GetKCVType(nKeyType, nKeyMode);

    if (nKCVType != KCV_NONE) {
        ioRes = CreateKCV( pKey, (uint)nKeySize, ZEROS_SIZE, KCV_SIZE, pCalculatedKCV, nKCVType);
        if ( false == ioRes ) {
            LOGMSG("CryptoMgrProvision::ValidateOpaque CreateKCV failed");
            return false;
        }
        // Validate KCV
        if (memcmp(pKCV32, pCalculatedKCV, KCV_SIZE) != 0) {
            LOGMSG("CryptoMgrProvision::ValidateOpaque wrong KCV");
            return false;
        }
    }

    // Validate ANBIO
    if (memcmp(pANBIO, m_ANBIO, ANBIO_SIZE) != 0) {
        LOGMSG("CryptoMgrProvision::ValidateOpaque wrong ANBIO");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
//*** Encrypt the input buffer with given Key and Keymode ***//
//
bool CryptoMgrProvision::Encrypt(u16 nKeyType, u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize) {

    if ((NULL == pIn) || (0 == nInSize)) {
        LOGMSG("Encrypt:Initial input argument validation failed");
        return false;
    }

    *pOutCG     = NULL;
    nOutCGSize  = 0;

    // Allocate in Buffer with BLOCK_SIZE aligned and copy
    uint nInAligSz = nInSize;
    if (nInAligSz % BLOCK_SIZE) {
        nInAligSz += BLOCK_SIZE - (nInAligSz % BLOCK_SIZE);
    }

    AutoHeapBuffer pInAutoBuffer(nInAligSz);
    u8* pInBuff = pInAutoBuffer.u8Ptr();
    if (NULL == pInBuff) {
        LOGMSG("Encrypt:Failed to allocate memory.");
        return false;
    }

    memset(pInBuff, 0, nInAligSz);
    memcpy(pInBuff, pIn, nInSize);
    u16 nSlot = 0;

    if (!GetKeySlot(nKeyType, nSlot)) {
        return false;
    }

    if (!EncryptBuffer(nSlot, pInBuff, nInAligSz, pOutCG, nOutCGSize)) {
        LOGMSG("Encrypt:Failed to encrypt buffer");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
//*** Decrypt the input buffer with given Key and Keymode ***//
//
bool CryptoMgrProvision::Decrypt(u16 nKeyType, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize) {

    if ((NULL == pInCG) || (0 == nInCGSize)) {
        LOGMSG("Decrypt:Initial input argument validation failed");
        return false;
    }

    u16 nSlot = 0;

    if (!GetKeySlot(nKeyType, nSlot)) {
        return false;
    }

    return DecryptBuffer(nSlot, pInCG, nInCGSize, pOut, nOutSize);
}
