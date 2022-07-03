#include "CryptoMgr.h"
#include "DataMgr.h"
#include "ISecureElement.h"
#include "CriticalErrorLog.h"
#include "V100_enc_types.h"
#include "CryptoMgrDefs.h"
#include "Configuration.h"
#include "CfgMgr.h"
#include <logging.h>

#include <cstring>
#include <fstream>

#define SECURE_ELEMENT ISecureElement::GetInstance()

bool CryptoMgr::CheckSEStatus() {
    ISecureElement* pSE = ISecureElement::GetInstance();
    return  CryptExecStatus::Successful == pSE->Execute_Get_Status();
}

bool CryptoMgr::CheckHWSupportsSE() {
    return false;
}

bool CryptoMgr::GetSESerialNumber(u64* pSN) {
    *pSN = 0;
    return true;
}

bool CryptoMgr::GetSEVersion(u32* pVer) {
    *pVer = 0;
    return true;
}

bool CryptoMgr::IsKeyLoaded(u16 nKeySlot) {
    ISecureElement* pSE = ISecureElement::GetInstance();
    return  CryptExecStatus::Successful == pSE->Execute_Check_Key_Exist(nKeySlot);
}

bool CryptoMgr::IsZeroVecKey(u8* pKey, u32 nKeySize) {
    return (!(pKey[0] || memcmp(&pKey[0], &pKey[1], nKeySize - 1)));
}

int CryptoMgr::GetKCVType(u16 nKeyType, u16 nKeyMode) {

    int KCVType = KCV_NONE;

    switch (nKeyType) {

        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
            KCVType = KCV_AES_256_CBC;
            break;

        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
            KCVType = KCV_AES_128_CBC;
            break;

        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
            KCVType = KCV_TDES_128_CBC;
            break;

        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            KCVType = KCV_TDES_192_CBC;
            break;

        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
            KCVType = KCV_SHA_256_NONE;
            break;

        // KCV type depends on KeyMode for these slots
        case KT_EXT_BSK:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
        case KT_MSK_MKD:
        case KT_MSK_SK:
            KCVType = GetKCVTypeForVariableMode(nKeyMode);
            break;

        default:
            err("GetKCVType:Invalid Key type %d", nKeyType);
            break;
    }

    return KCVType;
}

int CryptoMgr::GetKCVTypeForVariableMode(u16 nKeyMode) {

    int KCVType = KCV_NONE;

    switch (nKeyMode) {

        // only modes supported for variable mode key slots
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
            err("GetKCVTypeForVariableMode:Invalid Key mode %d", nKeyMode);
            break;
    }

    return KCVType;
}

u32 CryptoMgr::GetKeySize(u16 nKeyType, u16 nKeyMode) {

    u32 nKeySize = 0;

    switch (nKeyType) {

        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
            nKeySize = 256;
            break;

        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
            nKeySize = 32;
            break;

        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
            nKeySize = 16;
            break;

        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            nKeySize = 24;
            break;

        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
            nKeySize = 8;
            break;

        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
            nKeySize = 32;
            break;

        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
            nKeySize = 128;
            break;

        case KT_EXTKEY_PUBLIC_EXP:
            nKeySize = 4;
            break;

        case KT_EXT_DSK:
            nKeySize = 32;
            break;

        case KT_EXTKEY_PRIVATE_EXP:
            nKeySize = 4;
            break;

        //KeySize depends on mode for these slots
        case KT_EXT_BSK:
        case KT_EXTKEY_BTK:
        case KT_EXTKEY_CTK:
        case KT_MSK_MKD:
        case KT_MSK_SK:
            nKeySize = GetKeySizeForVariableMode(nKeyMode);
            break;

        default:
            err("GetKeySize:Invalid Key type %d", nKeyType);
            break;
    }

    return nKeySize;
}

u32 CryptoMgr::GetKeySizeForVariableMode(u16 nKeyMode) {

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
            err("GetKeySizeForVariableMode:Invalid Key mode %d", nKeyMode);
            break;
    }

    return nKeySize;
}

bool CryptoMgr::ValidateKeyMode(u16 nKeyType, u16 nKeyMode) {

    // ISecureElement* pSE = ISecureElement::GetInstance();

    u16 nStaticKeyMode;

    switch (nKeyType) {

        case KT_EXTKEY_VEND:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXT_DSK:
            nStaticKeyMode = KM_AES_256_CBC;
            break;

        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
            nStaticKeyMode = KM_AES_128_CBC;
            break;

        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
            nStaticKeyMode = KM_DUKPT_IPEK_128;
            break;

        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
            nStaticKeyMode = KM_TDES_ABC_CBC;
            break;

        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:        // only keyvalue saved on MAXQ
        case KT_EXTKEY_DEVICE_Q:        // only keyvalue saved on MAXQ
        case KT_EXTKEY_PRIVATE_EXP:     // only keyvalue saved on MAXQ
        case KT_EXTKEY_PUBLIC_EXP:      // only keyvalue saved on MAXQ
            if (nKeyMode != KM_RSA_2048_v15 && nKeyMode != KM_RSA_2048_v21) {
                return false;
            }
            return true;

        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
            nStaticKeyMode = KM_DUKPT_KSN_64;
            break;

        case KT_EXT_BSK:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_BTK:
            switch (nKeyMode) {
                case KM_AES_256_CBC:
                case KM_AES_128_CBC:
                case KM_TDES_ABA_ECB:
                case KM_TDES_ABA_CBC:
                case KM_TDES_ABC_ECB:
                case KM_TDES_ABC_CBC:
                    return true;
                default:
                    break;
            }
            return false;

        default:
            err("ValidateKeyMode:Unsupported Key %d", nKeyType);
            return false;
    }

    if (nKeyMode != nStaticKeyMode) {
        return false;
    }

    return true;
}

bool CryptoMgr::GetKeySlot(u16 nKeyType, u16& nKeySlot) {

    nKeySlot = -1;

    try {
        // nKeySlot = ISensorInstance::GetInstance()->GetCfgMgr()->GetKeyMap().at(nKeyType);
    } catch (...) {
        // Key not found in map, key not supported in current configuration or key map not configured correctly
        err("GetKeySlot:Failed to find key (%d) in key map. Returning false.", nKeyType);
        return false;
    }

    return true;
}

bool CryptoMgr::PullKeyInfo(u16 nKeySlot, u16& nKeyVer, u16& nKeyMode, u8* pKCV32) {

    KeyInfoStructure keyInfo;

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Key_Info(nKeySlot, keyInfo)) {
        err("PullKeyInfo:Get Key Info failed. Returning false.");
        return false;
    }

    nKeyVer  = keyInfo.key_ver;
    nKeyMode = (_V100_ENC_KEY_MODE)keyInfo.key_mode;
    memcpy(pKCV32, keyInfo.key_check_value, sizeof(keyInfo.key_check_value));

    return true;
}

bool CryptoMgr::CreateKCV(u8* pKey, uint nKeySize, uint nZeros, uint nVals, u8* pKCV, int nKCVType) {

    ISecureElement*     pSE         = ISecureElement::GetInstance();
    KeyInfoStructure    keyInfo     = {0};
    uint                tmpLength   = 0;
    u8*                 zeros       = nullptr;
    u8*                 tmp         = nullptr;

    // unsigned char ZERO_IV[BLOCK_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    if ((nZeros % BLOCK_SIZE != 0) || (nVals > 32)) {
        err("CreateKCV:nZeros %d is not multiple of 16 or nVals %d exceeds 32 ", nZeros, nVals);
        return false;
    }

    if (pKey == NULL) {
        err("CreateKCV:Invalid input argument");
        return false;
    }

    switch (nKCVType) {
        case KCV_AES_128_CBC:
        case KCV_AES_256_CBC:
        case KCV_TDES_128_CBC:
        case KCV_TDES_192_CBC:
        case KCV_SHA_256_NONE:
        case KCV_TDES_128_ECB:
        case KCV_TDES_192_ECB:
            // Only KCV modes supported
            break;

        case KCV_AES_192_CBC: //not used and we don't support this Keymode for encrypt/decrypt
        default:
            err("CreateKCV:Invalid KCV type %d", nKCVType);
            return false;
    }

    zeros = (u8*)MALLOC(nZeros);

    if ( zeros == NULL ) {
        err("CreateKCV:Failed to allocate memory");
        goto ABORT;
    }

    memset(zeros, 0, nZeros);

    tmpLength       = 0;
    keyInfo.slot    = KT_KEY_TMP;
    tmp             = nullptr;

    // Set temporary key for generating KCV.
    if (nKCVType != KCV_SHA_256_NONE) {

        keyInfo.key_size = nKeySize;

        switch ( nKCVType ) {
            case KCV_TDES_128_CBC:
                keyInfo.key_mode = KeyModes::TDES_ABA_CBC;
                break;
            case KCV_TDES_192_CBC:
                keyInfo.key_mode = KeyModes::TDES_ABC_CBC;
                break;
            case KCV_TDES_128_ECB:
                keyInfo.key_mode = KeyModes::TDES_ABA_ECB;
                break;
            case KCV_TDES_192_ECB:
                keyInfo.key_mode = KeyModes::TDES_ABC_ECB;
                break;
            case KCV_AES_128_CBC:
                keyInfo.key_mode = KeyModes::AES_128_CBC;
                break;
            case KCV_AES_256_CBC:
                keyInfo.key_mode = KeyModes::AES_256_CBC;
                break;
            default:
                err("CreateKCV:Invalid KCV type %d", nKCVType);
        }

        // Set Temporary key for generating KCV
        if (CryptExecStatus::Successful != pSE->Execute_Set_Key(keyInfo, pKey)) {
            pSE->Execute_Erase_Key(keyInfo.slot);
            err("CreateKCV:Failed to set key for encryption");
            goto ABORT;
        }

        if (false == EncryptBuffer(keyInfo.slot, zeros, nZeros, &tmp, tmpLength)) {
            err("CreateKCV:Failed to encrypt buffer");
            goto ABORT;
        }

    } else {

        if (CryptExecStatus::Successful != pSE->Execute_Hash_Data(pKey, nKeySize, HashAlgorithms::SHA2_256, &tmp, tmpLength)) {
            err("CreateKCV:Hash step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
    }

    if (tmpLength < nVals) {
        err("CreateKCV:KCV size %d generated is less than requested size %d", tmpLength, nVals);
        goto ABORT;
    }

    memcpy(pKCV, tmp, nVals);

    if (nKCVType != KCV_SHA_256_NONE) {
        if (CryptExecStatus::Successful != pSE->Execute_Erase_Key(keyInfo.slot)) {
            err("CreateKCV:Failed to erase tmp key.");
            FREE(zeros);
            pSE->Delete_Buffer(&tmp);
            return false;
        }
    }

    FREE(zeros);
    pSE->Delete_Buffer(&tmp);
    return true;

ABORT:
    pSE->Execute_Erase_Key(keyInfo.slot);
    FREE(zeros);
    pSE->Delete_Buffer(&tmp);
    return false;
}


/*************************************************************************************/
//*** Encrypt the input buffer with given Key and Key mode                         ***/
//***   NOTE: This function will not support (Host)RSA or DUKPT key encryption,    ***/
//***    instead user should call SE->Select_<DUKPT/RSA>_Key then call SE->Encrypt  ***/
/*************************************************************************************/
bool CryptoMgr::EncryptBuffer(u16 nKeySlot, u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize) {

    if ((NULL == pIn) || (0 == nInSize)) {
        err("EncryptBuffer:Initial input arugment validation failed");
        return false;
    }

    /* if not Padded to BLOCK_SIZE return error */
    if (nInSize % BLOCK_SIZE != 0) {
        err("EncryptBuffer:Invalid input size %d (should be multiple of %d).", nInSize, BLOCK_SIZE);
        return false;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(nKeySlot)) {
        err("EncryptBuffer:Selecting for KeySlot %d returned error", nKeySlot);
        return false;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Encrypt(pIn, nInSize, pOutCG, nOutCGSize, nullptr, 0)) {
        err("EncryptBuffer:Encrypting with KeySlot %d returned error", nKeySlot);
        return false;
    }

    return true;
}

/**************************************************************************************/
//*** Decrypt the input buffer with given Key and Key mode                          ***/
//***   NOTE: This function will not support (Host)RSA or DUKPT key decryption,     ***/
//***    instead user should call SE->Select_<DUKPT/RSA>_Key then call SE->Decrypt   ***/
/**************************************************************************************/
bool CryptoMgr::DecryptBuffer(u16 nKeySlot, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize) {

    if ((NULL == pInCG) || (0 == nInCGSize)) {
        err("DecryptBuffer:Initial input argument validation failed");
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

void CryptoMgr::ReleaseSEBuff (u8** pIn) {
    SECURE_ELEMENT->Delete_Buffer(pIn);
}
