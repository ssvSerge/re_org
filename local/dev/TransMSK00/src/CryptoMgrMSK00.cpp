#include "CryptoMgrMSK00.h"
#include "V100_internal_types.h"
#include "DataMgr.h"
#include "CfgMgr.h"
//#include "lumi_random.h"
#include "AutoHeapBuffer.h"
#include "CriticalErrorLog.h"

#include <unordered_map>

//legacy  mapping to report key status
#define MKD_ID 1  // Diversified Master Key
#define TK_ID 2  // Transport Key
#define SK_ID 3  // Session Key
#define TMP_ID 4

#define MKD_ID_KEY_IS_SET                (1<<MKD_ID)
#define TK_ID_KEY_IS_SET                (1<<TK_ID)
#define SK_ID_KEY_IS_SET                (1<<SK_ID)
#define TMP_ID_KEY_IS_SET                (1<<TMP_ID)

std::string MKD_PROP_STRING = "PARTIAL_MKD_FOR_TRANSPORT";

typedef struct
{
    u128
        DevID;        // No Padding
    u128
        ANSOL;        // Host Challenge
} BioDataHdr;

// Mapping of key mode to software encryption/decryption
// function arguments:
//
//    _V100_ENC_KEY_MODE    Algo / Mode            Key Size (bytes)
//    --------------------------------------------------------
//    KM_MODE_NONE        NO_MODE                -
//    ---------------        AES_MODE
//        ?                    AES_ECB
//    KM_AES_128_CBC            AES_CBC         16
//    KM_AES_256_CBC            AES_CBC *        32
//        ?                    AES_CTR
//    ---------------        TDES_MODE
//    KM_TDES_ABA_ECB            DES_ECB            16
//    KM_TDES_ABA_CBC            DES_CBC            16
//    KM_TDES_ABC_ECB            TDES_ECB        24
//    KM_TDES_ABC_CBC            TDES_CBC *        24
//    --------------------------------------------------------
//     *these modes were supported in legacy code that used "crypto mode" setting
//
// The key mode is now stored with the key and accessed when needed.


CryptoMgrMSK00::CryptoMgrMSK00()
{
    memset(m_ANBIO, 0, sizeof(u128));
    memset(m_ANSOL, 0, sizeof(u128));
}

CryptoMgrMSK00::~CryptoMgrMSK00()
{

}

bool CryptoMgrMSK00::Init()
{
    // set ANBIO and ANSOL to a random number - do not set to all zeros - guessable...
    CryptExecStatus rt1 = SECURE_ELEMENT->Execute_GetRandomBuffer(m_ANBIO, sizeof(m_ANBIO));
    if (rt1 != CryptExecStatus::Successful)
    {
        LOGMSG("Init:Get random number for ANBIO returned error.");
    }
    CryptExecStatus rt2 = SECURE_ELEMENT->Execute_GetRandomBuffer(m_ANSOL, sizeof(m_ANSOL));
    if (rt2 != CryptExecStatus::Successful)
    {
        LOGMSG("Init:Get random number for ANSOL returned error.");
    }

    return (rt1 == CryptExecStatus::Successful) && (rt2 == CryptExecStatus::Successful);
}

bool CryptoMgrMSK00::IsMKDLoaded()
{
    u16 nSlot = 0;
    if (!GetKeySlot(KT_MSK_MKD, nSlot))
    {
        return false;
    }
    return IsKeyLoaded(nSlot);
}

bool CryptoMgrMSK00::PerformSelfDiagnostic(bool bVerbose)
{
//    return CryptoMgrSymmetric::GetInstance()->PerformSelfDiagnostic(bVerbose);
    return false;
}

bool CryptoMgrMSK00::GetRandomNumber(u8* pRnd, int nSize)
{
    if (nSize != ANBIO_SIZE)
    {
        LOGMSG("GetRandomNumber:Invalid size (%d).", nSize);
        return false;
    }

    CryptExecStatus rt = SECURE_ELEMENT->Execute_GetRandomBuffer(pRnd, nSize);
    if (rt != CryptExecStatus::Successful)
    {
        LOGMSG("GetRandomNumber:Get random number returned error.");
    }

    // always returns true...
    SetANBIO((u128*)pRnd);

    return rt == CryptExecStatus::Successful;
}

bool CryptoMgrMSK00::SetANBIO(u128* pANBIO)
{
    memcpy(m_ANBIO, pANBIO, ANBIO_SIZE);
    return true;
}
bool CryptoMgrMSK00::GetANBIO(u128* pANBIO)
{
    memcpy(pANBIO, m_ANBIO, ANBIO_SIZE);
    return true;
}
bool CryptoMgrMSK00::SetANSOL(u128* pANSOL)
{
    memcpy(m_ANSOL, pANSOL, ANSOL_SIZE);
    return true;
}
bool CryptoMgrMSK00::GetANSOL(u128* pANSOL)
{
    memcpy(pANSOL, m_ANSOL, ANSOL_SIZE);
    return true;
}



// pCG = KeyModeCTK[SK KCV] TK
// KCV = KeyModeCTK[Zeros(16)]SK
// KCV calculated and saved on FS is 4 bytes but compared only 3 bytes to validate the sentin KCV
// TK = [ ANSOL + DeviceID + MKd[0-15] ]SHA256
bool CryptoMgrMSK00::SetSK(u8 *pCG, uint nCGSize, u8* pANSOL, int nANSOLSize)
{
    // Hand off Cryptogram
    u8* Kmkd = nullptr;
    uint32_t nMKDLength = 0;
    u8* Ktk = nullptr; //Calculated as sha256
    u16 nCTKVer, nCTKMode, nSKCVType;
    u32 nTKDataSize = 0;
    u8 pCTKKCV[KCV_SIZE], pSKKCV[KCV_SIZE];
    char devID[DEVID_SIZE];
    nTKDataSize = nANSOLSize + DEVID_SIZE + 16;
    AutoHeapBuffer auto_pTKData(nTKDataSize);
    u8* pTKData = auto_pTKData.u8Ptr();
    u8* pSKData = nullptr;
    // TODO: TEMP DEFINITION FOR DEBUGGING WITH NO TRANSCEIVER
    //u8* pSKData = nullptr;

    KeyInfoStructure kSKInfo, kTKInfo;
    kSKInfo.key_size = 0;


    if ((nANSOLSize != 16) || pCG == NULL || pANSOL == NULL || (nCGSize % BLOCK_SIZE != 0))
    {
        LOGMSG("SetSK:Invalid input pointer or size (ANSOL size = %d, CG size = %d).",
            nANSOLSize, nCGSize);
        goto ON_FAIL;
    }

    // Retrieve MKD to generate transport key
    // Disallow SK udpate if MKD is not loaded
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Property((uint8_t*)MKD_PROP_STRING.c_str(), MKD_PROP_STRING.length(), &Kmkd, nMKDLength))
    {
        LOGMSG("SetSK:Get MKD returned error.");
        goto ON_FAIL;
    }

    //Retrieve CTK to find the mode of TK and SK
    if (false == GetKeyInfo(KT_EXTKEY_CTK, nCTKVer, nCTKMode, pCTKKCV))
    {
        LOGMSG("SetSK:Get CTK key info returned error.");
        goto ON_FAIL;
    }

    // Generate Transport Key TK
    // TK = [ ANSOL + DeviceID + MKd[0-15] ]SHA256
    // Get DeviceID
    if (!GetDeviceID(devID))
    {
        LOGMSG("SetSK:Get device ID returned error.");
        goto ON_FAIL;
    }
    memcpy(pTKData, pANSOL, nANSOLSize);
    memcpy(pTKData + nANSOLSize, devID, sizeof(devID));
    memcpy(pTKData + nANSOLSize + sizeof(devID), Kmkd, 16); // only use 16 bytes of MKDkey

    // Generate transport key
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pTKData, nANSOLSize + sizeof(devID) + 16, HashAlgorithms::SHA2_256, &Ktk, nTKDataSize))
    {
        LOGMSG("SetSK:Hash computation step returned error.");
        goto ON_FAIL;
    }


    // Decrypt CG using TK
    // [SK KCV] TK
    kTKInfo.key_mode = (KeyModes)nCTKMode;
    kTKInfo.key_size = CryptoMgr::GetKeySize(KT_MSK_SK, (u16)kTKInfo.key_mode);
    if (!GetKeySlot(KT_KEY_TMP, kTKInfo.slot))
    {
        goto ON_FAIL;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(kTKInfo, Ktk))
    {
        SECURE_ELEMENT->Execute_Erase_Key(KT_KEY_TMP);
        LOGMSG("SetSK:Failed to set transport key.");
        goto ON_FAIL;
    }

    if (false == DecryptBuffer(kTKInfo.slot, pCG, nCGSize, &pSKData, kTKInfo.key_size))
    {
        SECURE_ELEMENT->Execute_Erase_Key(KT_KEY_TMP);
        LOGMSG("SetSK:Failed to select Decrypt.");
        goto ON_FAIL;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(kTKInfo.slot))
    {
        LOGMSG("SetSK:Failed to erase transport key.");
        goto ON_FAIL;
    }


    //Use CTKmode as SK mode
    kSKInfo.key_ver = 0x0A01;
    kSKInfo.key_mode = (KeyModes)nCTKMode;
    kSKInfo.key_size = CryptoMgr::GetKeySize(KT_MSK_SK, (u16)kSKInfo.key_mode);
    if (!GetKeySlot(KT_MSK_SK, kSKInfo.slot))
    {
        goto ON_FAIL;
    }
    nSKCVType = CryptoMgr::GetKCVType(KT_MSK_SK, (u16)kSKInfo.key_mode);


    // KCV = [ 0 ] SK
    if (!CreateKCV(pSKData, kSKInfo.key_size, sizeof(u128), KCV_SIZE, pSKKCV, nSKCVType))
    {
        LOGMSG("SetSK:Create KCV step returned error.");
        goto ON_FAIL;
    }

    // Validate key check value with recovered SK
    // CG = [SK KCV] TK
    if (memcmp(pSKData + kSKInfo.key_size, pSKKCV, 3) != 0)
    {
        LOGMSG("SetSK:Key check value validation step returned error.");
        goto ON_FAIL;
    }

    // Disallow a session key of zeroVec regardless of correctness
    if (IsZeroVecKey(pSKData, kSKInfo.key_size))
    {
        LOGMSG("SetSK:Invalid key value -- zero vector not allowed.");
        goto ON_FAIL;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(kSKInfo, pSKData))
    {
        LOGMSG("SetSK:Set key returned error.");
        goto ON_FAIL;
    }

    if (false == UpdateKeyState())
    {
        LOGMSG("SetSK:Update key state returned error.");
        goto ON_FAIL;
    }

    memset(Ktk, 0, nTKDataSize);
    memset(Kmkd, 0, nMKDLength);
    memset(pSKData, 0, kSKInfo.key_size);
    ReleaseSEBuff(&Ktk);
    ReleaseSEBuff(&Kmkd);
    ReleaseSEBuff(&pSKData);
    //AutoHeapBuffer takes care of pSKData and pTKData
    return true;

ON_FAIL:
    if (Ktk != nullptr)
    {
        memset(Ktk, 0, nTKDataSize);
    }
    if (Kmkd != nullptr)
    {
        memset(Kmkd, 0, nMKDLength);
    }
    if (pSKData != nullptr)
    {
        memset(pSKData, 0, kSKInfo.key_size);
    }

    ReleaseSEBuff(&Ktk);
    ReleaseSEBuff(&Kmkd);
    ReleaseSEBuff(&pSKData);
    //AutoHeapBuffer takes care of pTKData
    return false;
}

bool CryptoMgrMSK00::GetSKCV(u8* pKCV, uint nKCVSize)
{
    if (nKCVSize != KCV_SIZE)
    {
        LOGMSG("GetSKCV:Invalid KCV size parameter % d(must be % d).",
            nKCVSize, KCV_SIZE);
        return false;
    }

    KeyInfoStructure keyInfo;
    u16 nSlot = 0;
    if (!GetKeySlot(KT_MSK_SK, nSlot))
    {
        return false;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Key_Info(nSlot, keyInfo))
    {
        LOGMSG("GetSKCV:Get Key Info failed. Returning false.");
        return false;
    }

    memcpy(pKCV, keyInfo.key_check_value, nKCVSize);

    return true;
}

// pKey = [MKD] CTK
// pDigSig = SHA256[ MKD]
// KCV saved on  to sensor  = [Zeros(16)]MKD
// Sets MKD
bool CryptoMgrMSK00::SetMKD(u8* pCG, uint nCGSize , u256 pDigSig)
{
    // the cryptogram is always packed in a u256 (32-byte) buffer, *without*
    // a size parameter (unlike when setting the SK).

    u8  pMKDKCV[KCV_SIZE];
    int nMKDKCVType;
    u32  keyStatus = 0;
    u8* pMKD = nullptr;
    KeyInfoStructure kCTKInfo, kMKDInfo;

    u8* hashval = nullptr;
    uint hashsize;

    AutoHeapBuffer auto_pCTK(sizeof(u2048));
    u8* pCTK = auto_pCTK.u8Ptr();
    AutoHeapBuffer auto_PartialMKD(sizeof(u128));
    u8* pPartialMKD = auto_PartialMKD.u8Ptr();

    AutoHeapBuffer auto_pInAlig(sizeof(u256));
    u8* pInAlig = auto_pInAlig.u8Ptr();
    memset(pInAlig, 0, sizeof(u256));

    if (pCG == NULL || pDigSig == NULL || (nCGSize % BLOCK_SIZE != 0))
    {
        LOGMSG("SetMKD:Invalid input pointer or size (CG size = %d).",
            nCGSize);
        return false;
    }

    // Make sure MKD doesn't exist
    if (IsMKDLoaded())
    {
        LOGMSG("SetMKD:Key is already loaded.");
        return false;
    }

    // Select CTK as active key
    u16 nSlot = 0;
    if (!GetKeySlot(KT_EXTKEY_CTK, nSlot))
    {
        goto ON_FAIL;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(nSlot))
    {
        LOGMSG("SetMKD:Failed to select key for decryption.");
        goto ON_FAIL;
    }

    // Get CTK info for later
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Key_Info(nSlot, kCTKInfo))
    {
        LOGMSG("SetMKD:Failed to get CTK info.");
        goto ON_FAIL;
    }

    // Decrypt MKD using CTK
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Decrypt(pCG, nCGSize, &pMKD, kMKDInfo.key_size, nullptr, 0))
    {
        LOGMSG("SetMKD:Decryption step returned error.");
        goto ON_FAIL;
    }


    kMKDInfo.key_ver = 0x0A01;
    kMKDInfo.key_mode = kCTKInfo.key_mode; // Use CTKMode for MKD
    kMKDInfo.key_size = GetKeySize(KT_MSK_MKD, (u16)kMKDInfo.key_mode);
    if (!GetKeySlot(KT_MSK_MKD, kMKDInfo.slot))
    {
        goto ON_FAIL;
    }
    nMKDKCVType = GetKCVType(KT_MSK_MKD, (u16)kMKDInfo.key_mode);

    // Calculate the Hash of Kmkd
    if (kMKDInfo.key_size == 16)
    {
        // For keys that are 16 bytes, nCGSize will be 32 however we only want to hash the first 16 bytes to get the correct DigSig
        nCGSize = 16;
    }
    memcpy(pInAlig, pMKD, nCGSize);
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pInAlig, nCGSize, HashAlgorithms::SHA2_256, &hashval, hashsize))
    {
        LOGMSG("SetMKD:Hash computation step returned error.");
        goto ON_FAIL;
    }
    memset(pInAlig, 0, sizeof(u256));

    // Check Digital Signature
    if (memcmp(hashval, pDigSig, sizeof(u256)))
    {
        LOGMSG("SetMKD:Digital signature validation step returned error.");
        goto ON_FAIL;
    }

    // Disallow a MKD key of zeroVec regardless of correctness
    if (IsZeroVecKey(pMKD, kMKDInfo.key_size))
    {
        LOGMSG("SetMKD:Invalid key value -- zero vector not allowed.");
        goto ON_FAIL;
    }

    // Create KCV to save with the key on FS.
    if (!CreateKCV(pMKD, kMKDInfo.key_size, 16, KCV_SIZE, pMKDKCV, nMKDKCVType)) // nZeros = 16, save 4 bytes
    {
        LOGMSG("SetMKD:Create KCV step returned error.");
        goto ON_FAIL;
    }
    memcpy(kMKDInfo.key_check_value, pMKDKCV, sizeof(kMKDInfo.key_check_value));


    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(kMKDInfo, pMKD))
    {
        SECURE_ELEMENT->Execute_Erase_Key(kMKDInfo.slot);


        LOGMSG("SetMKD:Set key %d returned error.", KT_MSK_MKD);
        goto ON_FAIL;
    }

    // Store partial MKD for loading session key(s)
    memcpy(pPartialMKD, pMKD, sizeof(u128));
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Property((uint8_t*)MKD_PROP_STRING.c_str(), MKD_PROP_STRING.length(), pPartialMKD, sizeof(u128)))
    {
        SECURE_ELEMENT->Execute_Erase_Key(kMKDInfo.slot);
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)MKD_PROP_STRING.c_str(), MKD_PROP_STRING.length());

        LOGMSG("SetMKD:Failed to set property for MKD.");
        goto ON_FAIL;
    }

    if (false == UpdateKeyState())
    {
        LOGMSG("SetMKD:Update key state returned error.");
        goto ON_FAIL;
    }

    memset(pMKD, 0, kMKDInfo.key_size);
    ReleaseSEBuff(&pMKD);
    memset(hashval, 0, hashsize);
    ReleaseSEBuff(&hashval);

    return true;

ON_FAIL:
    if (pMKD != nullptr)
    {
        memset(pMKD, 0, kMKDInfo.key_size);
    }
    if (hashval != nullptr)
    {
        memset(hashval, 0, hashsize);
    }
    ReleaseSEBuff(&pMKD);
    ReleaseSEBuff(&hashval);

    return false;
}

bool CryptoMgrMSK00::UpdateKeyState()
{
    u16 KeyStatus = 0;
    u16 nSlot = 0;
    if (GetKeySlot(KT_MSK_MKD, nSlot))
    {
        if (IsKeyLoaded(nSlot))
        {
            KeyStatus |= MKD_ID_KEY_IS_SET;
            KeyStatus |= TMP_ID_KEY_IS_SET;    //To support legacy simulator which looks for 0x12 for Kmkd_PROGRAMMED (enabled by MaxQ in V)
        }
    }
    if (GetKeySlot(KT_MSK_SK, nSlot))
    {
        if (IsKeyLoaded(nSlot))
        {
            KeyStatus |= SK_ID_KEY_IS_SET;
            KeyStatus |= TK_ID_KEY_IS_SET;    //To support legacy simulator which looks for 0x0c for SK_PROGRAMMED (enabled by MaxQ in V)
        }
    }

    _V100_INTERFACE_CONFIGURATION_TYPE* pICT = ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration();
    pICT->pImageBuffer = KeyStatus;
    return true;
}

bool CryptoMgrMSK00::GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, u16& nKeyMode, u8* pKCV32)
{
    if ((nKeyType != KT_EXTKEY_CTK) && (nKeyType != KT_MSK_MKD) && (nKeyType != KT_MSK_SK))
    {
        LOGMSG("GetKeyInfo:Invalid key type %d.", nKeyType);
        return false;
    }
    u16 nSlot = 0;
    if (!GetKeySlot(nKeyType, nSlot))
    {
        return false;
    }
    if (!PullKeyInfo(nSlot, nKeyVer, nKeyMode, pKCV32))
    {
        LOGMSG("GetKeyInfo:Failed to pull key info for key %d. Returning false.", nKeyType);
        return false;
    }

    return true;
}

// DigSig =[ [pIn] SHA256] SK
bool CryptoMgrMSK00::CreateDigSig(u8* pIn, uint nInSize, u256 pOutDigSig)
{
    u8* pHash = nullptr;
    uint nHashLength;

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pIn, nInSize, HashAlgorithms::SHA2_256, &pHash, nHashLength))
    {
        ReleaseSEBuff(&pHash);
        LOGMSG("CreateDigSig:Compute hash returned error.");
        return false;
    }

    u8* pOutCG = nullptr;
    uint nOutSize;

    if (false == Encrypt(KT_MSK_SK, pHash, sizeof(u256), &pOutCG, nOutSize))
    {
        ReleaseSEBuff(&pHash);
        if (pOutCG)
        {
            FREE(pOutCG);
        }
        LOGMSG("CreateDigSig:Encryption step returned error.");
        return false;
    }
    ReleaseSEBuff(&pHash);
    if (pOutCG == nullptr)
    {
        LOGMSG("CreateDigSig:Encryption step failed to allocate memory.");
        return false;
    }
    memcpy(pOutDigSig, pOutCG, sizeof(u256));
    FREE(pOutCG);

    return true;
}

bool CryptoMgrMSK00::ValidateDigSig(u8* pIn, uint nInSize, u256 pInDigSig)
{
    u8* pHashCal = nullptr;
    uint nHashLength;

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pIn, nInSize, HashAlgorithms::SHA2_256, &pHashCal, nHashLength))
    {
        ReleaseSEBuff(&pHashCal);
        LOGMSG("CreateDigSig:Compute hash returned error.");
        return false;
    }

    u8* pOutPT = NULL;
    uint nOutSize;

    if (false == Decrypt(KT_MSK_SK, pInDigSig, sizeof(u256), &pOutPT, nOutSize))
    {
        if (pOutPT)
        {
            ReleaseSEBuff(&pHashCal);
            FREE(pOutPT);
            pOutPT = NULL;
        }
        LOGMSG("ValidateDigSig:Decryption step returned error.");
        return false;
    }
    if (pOutPT == NULL)
    {
        ReleaseSEBuff(&pHashCal);
        LOGMSG("ValidateDigSig:Decryption step failed to allocate memory.");
        return false;
    }


    if (memcmp(pHashCal, pOutPT, sizeof(u256)) !=0)
    {
        ReleaseSEBuff(&pHashCal);
        FREE(pOutPT);
        LOGMSG("ValidateDigSig:Computed hash does not match digital signature.");
        return false;
    }
    ReleaseSEBuff(&pHashCal);
    FREE(pOutPT);

    return true;
}

/*
    CryptoMgrMSK00::EncryptBioData
    NOTE: *pOutCG should be nullptr, *pOutCG will be allocated with MALLOC
*/
bool CryptoMgrMSK00::EncryptBioData(u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize, u256 pDigSig)
{
    // Pad Indata
    // Create Digsig [BiodataHdr + Indata + padding]
    // Encrypt [Indata + padding]

    if ((NULL == pIn) || (0 == nInSize))
    {
        LOGMSG("EncryptBioData:Null input or zero size input parameter (size = %d).",
                nInSize);
        return false;
    }

    *pOutCG = NULL;
    nOutCGSize =0;
    // Alloc in Buffer with BLOCK_SIZE aligned and copy
    uint nInAligSz = nInSize;
    if (nInAligSz % BLOCK_SIZE)
    {
        nInAligSz += BLOCK_SIZE - (nInAligSz % BLOCK_SIZE);
    }
    nInAligSz += sizeof(BioDataHdr);

    AutoHeapBuffer pInAutoBuffer(nInAligSz);
    u8* pInBuff = pInAutoBuffer.u8Ptr();
    if (NULL == pInBuff)
    {
        LOGMSG("EncryptBioData:Failed to allocate memory.");
        return false;
    }
    memset(pInBuff, 0, nInAligSz);

    BioDataHdr* pBioHdr = (BioDataHdr*)pInBuff;
    GetDeviceID((char*)pBioHdr->DevID);

    memcpy(pBioHdr->ANSOL, m_ANSOL, ANSOL_SIZE);
    memcpy(pInBuff + sizeof(BioDataHdr), pIn, nInSize);

    //DigSig = [[DevID ANSOL InData padding]Hash]SK
    if (false == CreateDigSig(pInBuff, nInAligSz, pDigSig))
    {
        LOGMSG("EncryptBioData:Create digital signature returned error.");
        return false;
    }

    // Encrypt [InData + padding] SK
    if (false == Encrypt(KT_MSK_SK, pInBuff + sizeof(BioDataHdr),nInAligSz - sizeof(BioDataHdr), pOutCG, nOutCGSize))
    {
        LOGMSG("EncryptBioData:Encryption step returned error.");
        return false;
    }

    return true;
}

/*
    CryptoMgrMSK00::Encrypt
    NOTE: *pOutCG should be nullptr, *pOutCG will be allocated with MALLOC
*/
bool CryptoMgrMSK00::Encrypt(u16 KeyType, u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize)
{
    if (KeyType != KT_MSK_SK && KeyType != KT_MSK_MKD)
    {
        LOGMSG("Encrypt:Invalid key type for encryption %d (must be SK or MKD).",
            KeyType);
        return false;
    }
    u8* pSEOutCG = nullptr;
    u16 nSlot = 0;
    if (!GetKeySlot(KeyType, nSlot))
    {
        return false;
    }
    if (!EncryptBuffer(nSlot, pIn, nInSize, &pSEOutCG, nOutCGSize))
    {
        LOGMSG("Encrypt:Failed to encrypt buffer. Returning false.",
            nInSize);
        ReleaseSEBuff(&pSEOutCG);
        return false;
    }
    *pOutCG = (u8*)MALLOC(nOutCGSize);
    if (*pOutCG == nullptr)
    {
        nOutCGSize = 0;
        LOGMSG("Encrypt:Failed to allocate memory.");
        return false;
    }
    memcpy(*pOutCG, pSEOutCG, nOutCGSize);
    nOutCGSize = nOutCGSize;
    memset(pSEOutCG, 0, nOutCGSize);
    ReleaseSEBuff(&pSEOutCG);

    return true;
}

/*
    CryptoMgrMSK00::Decrypt
    NOTE: *pOut should be nullptr, *pOut will be allocated with MALLOC
*/
bool CryptoMgrMSK00::Decrypt(u16 KeyType, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize)
{
    if ((pInCG == NULL) || (nInCGSize == 0))
    {
        LOGMSG("Decrypt:NULL input or zero size input parameter (size = %d).",
            nInCGSize);
        return false;
    }
    u8* pSEOut = nullptr;
    u16 nSlot = 0;
    if (!GetKeySlot(KeyType, nSlot))
    {
        return false;
    }

    if (!DecryptBuffer(nSlot, pInCG, nInCGSize, &pSEOut, nOutSize))
    {
        LOGMSG("Decrypt:Failed to decrypt buffer. Returning false.");
        return false;
    }

    *pOut = (u8*)MALLOC(nOutSize);
    if (*pOut == nullptr)
    {
        nOutSize = 0;
        LOGMSG("Decrypt:Failed to allocate memory.");
        return false;
    }
    memcpy(*pOut, pSEOut, nOutSize);
    nOutSize = nOutSize;
    memset(pSEOut, 0, nOutSize);
    ReleaseSEBuff(&pSEOut);

    return true;
}

bool CryptoMgrMSK00::GetDeviceID(char pDevIDBuff[DEVID_SIZE])
{
    /*
    **    Retrieve Unique SN from secure element
    */
    u64 SN = 0;
    CryptoMgr::GetSESerialNumber(&SN);

    /*
    ** Convert to Serial Number String
    */
    unsigned short VID = ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration()->Vendor_Id;
    unsigned short PID = ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration()->Product_Id;
    unsigned char* pt = (unsigned char*)&VID;
    pDevIDBuff[0] = pt[1];
    pDevIDBuff[1] = pt[0];

    pt = (unsigned char*)&PID;
    pDevIDBuff[2] = pt[1];
    pDevIDBuff[3] = pt[0];

    pt = (unsigned char*)&SN;
    pDevIDBuff[4] = 0xee;
    pDevIDBuff[5] = pt[5];
    pDevIDBuff[6] = pt[4];
    pDevIDBuff[7] = pt[3];
    pDevIDBuff[8] = pt[2];
    pDevIDBuff[9] = pt[1];
    pDevIDBuff[10] = pt[0];
    pDevIDBuff[11] = 0x23;
    pDevIDBuff[12] = 0x01;

    pDevIDBuff[13] = '\0';
    pDevIDBuff[14] = '\0';
    pDevIDBuff[15] = '\0';
    return true;
}
