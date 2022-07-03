#include "CryptoMgrCP001.h"
#include "AutoHeapBuffer.h"
#include "CriticalErrorLog.h"
#include "CfgMgr.h"

#define TRANS_COUNTER_0 101
#define TRANS_COUNTER_1 102

std::string FKL_LOCK_PROP_STRING_CP001 = "FKL_LOCK_STATUS_CP001";
std::string DSK_PROP_STRING_CP001 = "DSK_CRYPTOGRAM_CP001";

// Helper functions
u16 GetKeyMode(u16 nSlot);


static _V100_ENC_KEY_TYPE TranslateLegacySlotToUniversal(_V100_ENC_KEY_TYPE legacy_key_type)
{
    enum _LEGACY_KEY_TYPES
    {
        LEGACY_KT_EXTKEY_WK    = 1,
        LEGACY_KT_EXT_TK = 4097
    };

    // only these two get translated
    switch(legacy_key_type)
    {
        case LEGACY_KT_EXTKEY_WK:        return KT_EXT_BSK;        // aka slot 4097
        case LEGACY_KT_EXT_TK:            return KT_EXTKEY_CTK;    // aka slot 1
    }

    // the remaining key types are identical
    return legacy_key_type;
}


CryptoMgrCP001::CryptoMgrCP001()
{

}

CryptoMgrCP001::~CryptoMgrCP001()
{

}

bool CryptoMgrCP001::Init()
{
    // set ANBIO to a random number - do not set to all zeros - guessable...
    CryptExecStatus rc = SECURE_ELEMENT->Execute_GetRandomBuffer(m_ANBIO, sizeof(m_ANBIO));
    if (rc != CryptExecStatus::Successful)
    {
        LOGMSG("Init:Get random number for ANBIO returned error.");
    }

    m_nActiveKeyType = KT_EXT_LAST;

    // From KeyMgr
    InvalidateDSK();
    InvalidateBSK();

    return rc == CryptExecStatus::Successful;
}

/////////////////////////////////////////////////////////////////////////////
// Get random number which sets ANBIO
//
bool CryptoMgrCP001::GetRandomNumber(u256* pRnd)
{
    if (!pRnd)
    {
        return false;
    }

    CryptExecStatus rc = SECURE_ELEMENT->Execute_GetRandomBuffer((u8*)pRnd, 32);
    if (rc == CryptExecStatus::Successful)
    {
        memcpy(m_ANBIO, pRnd, 32);
    }

    return rc == CryptExecStatus::Successful ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
// Indicates whether FKL has already been performed, or not
//
bool CryptoMgrCP001::IsFactoryKeyLoadingLocked()
{
    // Retrieve the FKL Lock property
    u32 nFKLLock;
    u8* pFKLLock = nullptr;
    uint32_t nFKLLockLength = 0;
    CryptExecStatus rc = SECURE_ELEMENT->Execute_Get_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length(), &pFKLLock, nFKLLockLength);
    if (CryptExecStatus::Successful != rc)
    {
        if (CryptExecStatus::Not_Exist != rc)
        {
            LOGMSG("IsFactoryKeyLoadingLocked: Get FKL Lock returned error.");
        }
        return false;
    }
    if(nFKLLockLength != sizeof(u32))
    {
        ReleaseSEBuff(&pFKLLock);
        return false;
    }
    memcpy(&nFKLLock, pFKLLock, sizeof(u32));

    ReleaseSEBuff(&pFKLLock);

    return (nFKLLock == 1) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
// Locks FKL so that it cannot be performed more than once
//
bool CryptoMgrCP001::LockFactoryMode()
{
    bool bVal = true;
    // check if BTK is loaded, if not, return error
    u16 nKeySlot = 0;
    if (!GetKeySlot(KT_EXTKEY_BTK, nKeySlot))
    {
        return false;
    }
    if (!IsKeyLoaded(nKeySlot))
    {
        return false;
    }

    // Store the FKL Lock as a property
    u32 nFKLLock = 1;
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length(), (uint8_t*)&nFKLLock, sizeof(u32)))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length());

        LOGMSG("LockFactoryMode:Failed to set property for FKL Lock.");
        return false;
    }

    // Check it
    u8* pTmp = nullptr;
    u32 valLength = 0;
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length(), &pTmp, valLength))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length());

        LOGMSG("LockFactoryMode:Failed to get property for FKL Lock.");
        return false;
    }
    if(valLength != sizeof(u32))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length());
        LOGMSG("LockFactoryMode:FKLlock size read didn't match.");
        ReleaseSEBuff(&pTmp);
        return false;
    }
    if (*(u32 *)pTmp != nFKLLock)
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING_CP001.c_str(), FKL_LOCK_PROP_STRING_CP001.length());

        LOGMSG("LockFactoryMode:Set property for FKL Lock does not match expected value.");
        ReleaseSEBuff(&pTmp);
        return false;
    }

    ReleaseSEBuff(&pTmp);
    return true;
}


/////////////////////////////////////////////////////////////////////////////
//    Device-generated Session Key
//
bool CryptoMgrCP001::GenerateSessionKey()
{
    InvalidateDSK();

    //    Generate a New Session Key
    //    Use 1/2 of Random Data to Hash and 1/2 as Key
    //
    int nBytes = 2 * sizeof(u256);
    u8 pData[2 * sizeof(u256)] = {};
    u8* pSK = nullptr;
    uint32_t nSKSize = 0;
    u8* pCG = nullptr;
    uint nCGLength = 0;
    KeyInfoStructure dskInfo;
    KeyInfoStructure tmpKeyInfo;
    PublicRSAKeySlots pSlots;
    // Select RSA key
    u16 nSlotPublic = 0;
    u16 nSlotExp = 0;

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_GetRandomBuffer(pData, nBytes))
    {
        return false;
    }

    // Create New Session Key
    tmpKeyInfo.key_size = sizeof(u256);
    tmpKeyInfo.key_mode = KeyModes::AES_256_CBC;
    if (!GetKeySlot(KT_KEY_TMP, tmpKeyInfo.slot))
    {
        return false;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(tmpKeyInfo, &pData[sizeof(u256)]))
    {
        goto ABORT;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(tmpKeyInfo.slot))
    {
        SECURE_ELEMENT->Execute_Erase_Key(tmpKeyInfo.slot);
        goto ABORT;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pData, sizeof(u256), HashAlgorithms::HMAC_SHA256, &pSK, nSKSize))
    {
        SECURE_ELEMENT->Execute_Erase_Key(tmpKeyInfo.slot);
        goto ABORT;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(tmpKeyInfo.slot))
    {
        goto ABORT;
    }

    if (nSKSize != sizeof(u256))
    {
        goto ABORT;
    }



    if (!GetKeySlot(KT_EXTKEY_HOST_PUBLIC, nSlotPublic))
    {
        return false;
    }
    if (!GetKeySlot(KT_EXTKEY_PUBLIC_EXP, nSlotExp))
    {
        return false;
    }
    pSlots.public_key_slot = nSlotPublic;
    pSlots.public_key_exp_slot = nSlotExp;
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Public_RSA_Keys(pSlots))
    {
        goto ABORT;
    }

    // Encrypt
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Encrypt(pSK, sizeof(u256), &pCG, nCGLength, nullptr, 0))
    {
        goto ABORT;
    }


    // Set DSK (store cryptogram as property as client can request this)
    dskInfo.key_mode = KeyModes::AES_256_CBC;
    dskInfo.key_size = sizeof(u256);
    dskInfo.key_ver = 0;
    memset(dskInfo.key_check_value, 0, KCV_SIZE);
    if (!GetKeySlot(KT_EXT_DSK, dskInfo.slot))
    {
        goto ABORT;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(dskInfo, pSK))
    {
        goto ABORT;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Property((u8*)DSK_PROP_STRING_CP001.c_str(), DSK_PROP_STRING_CP001.length(), pCG, sizeof(u2048)))
    {
        goto ABORT;
    }

    memset(pSK, 0, nSKSize);
    ReleaseSEBuff(&pSK);
    ReleaseSEBuff(&pCG);

    return true;

ABORT:
    if(pSK != nullptr)
    {
        memset(pSK, 0, nSKSize);
    }
    ReleaseSEBuff(&pSK);
    ReleaseSEBuff(&pCG);
    InvalidateDSK();
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Get Key
//   Only allowed keys are DSK (returned encrypted) and Device Public
//
bool CryptoMgrCP001::GetKey(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize)
{
    nKeyType = TranslateLegacySlotToUniversal(nKeyType);

    memset(pKey, 0, sizeof(pKey));
    if (pKCV32 == NULL)
    {
        return false;
    }

    switch(nKeyType)
    {
        case KT_EXT_DSK:
        {
            u8* pCG = nullptr;
            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Property((u8*)DSK_PROP_STRING_CP001.c_str(), DSK_PROP_STRING_CP001.length(), &pCG, nKeySize))
            {
                ReleaseSEBuff(&pCG);
                return false;
            }
            memcpy(pKey, pCG, nKeySize);
            ReleaseSEBuff(&pCG);
            nKeySize = sizeof(u2048);
            return GetKeyInfo(nKeyType, nKeyVer, nKeyMode, pKCV32);
        };
        case KT_EXTKEY_DEVICE_PUBLIC:
        {
            uint8_t* pOut = nullptr;
            uint32_t nOutSize = 0;
            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_RSA_PublicKey(&pOut, nOutSize))
            {
                LOGMSG("GetKey: Retrieving key %d returned error", nKeyType);

                ReleaseSEBuff(&pOut);
                return false;
            }
            if (nOutSize != sizeof(pKey))
            {
                LOGMSG("GetKey:Keysize retrieved for Key %d is not valid", nKeyType);
                ReleaseSEBuff(&pOut);
                return false;
            }
            memcpy(pKey, pOut, nOutSize);
            nKeySize = nOutSize;
            ReleaseSEBuff(&pOut);

            return GetKeyInfo(nKeyType, nKeyVer, nKeyMode, pKCV32);
            break;
        }
        default:
        {
            return false;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Gets version, mode, size and KCV
//
bool CryptoMgrCP001::GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32)
{
    u16 nTempKeyMode;

    nKeyType = TranslateLegacySlotToUniversal(nKeyType);

    if (nKeyType >= KT_MSK_MKD)
    {
        return false;
    }

    switch (nKeyType)
    {
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_SP:
        {
            // Return zeros for these for backwards compatibility
            nKeyVer = 0;
            nKeyMode = KM_MODE_NONE;
            // NOTE: pKCV32 already zeros, allocated by Broker
            return true;
        }
        default:
            break;
    }

    u16 nSlot = 0;
    if (!GetKeySlot(nKeyType, nSlot))
    {
        return false;
    }
    if (!PullKeyInfo(nSlot, nKeyVer, nTempKeyMode, pKCV32))
    {
        return false;
    }

    nKeyMode = (_V100_ENC_KEY_MODE)nTempKeyMode;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//    FKL
// CG = KeyModeCTK [ KeyBlock ] KCTK
// pCG = [ CG ]
// nCGSize = KeyBlockSize
//
int CryptoMgrCP001::InitiateFactoryKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCG, int nCGSize)
{
    if ((pCG == NULL) || (nCGSize <=0))
    {
        return false;
    }

    nKeyType = TranslateLegacySlotToUniversal(nKeyType);

    switch(nKeyType)
    {
        case KT_EXTKEY_VEND:
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
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_HOST_PUBLIC:
        {
            if (false == ProgramKey(KT_EXTKEY_CTK, nKeyType, pCG, nCGSize))    // [AES][pCG]CTK
            {
                return CM_ERROR;
            }
            break;
        }
        case KT_EXT_BSK:
        {
            if (false == ProgramBSK(pCG, nCGSize))
            {
                return CM_ERROR;
            }
            break;
        }
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_PUBLIC_EXP:    // This is programmed as part of KT_EXTKEY_HOST_PUBLIC
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PRIVATE_EXP:
        default:
        {
            return CM_ERROR_NOT_SUPPORTED;    // These keys cannot be programmed externally.
        }
    }

    return CM_OK;
}

/////////////////////////////////////////////////////////////////////////////
//    RKL
// CG = KeyModeBTK [ KeyBlock ] KBTK
// pCG = [ CG ]
// nCGSize = KeyBlockSize
//
int CryptoMgrCP001::InitiateRemoteKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCG, int nCGSize)
{
    if ((pCG == NULL) || (nCGSize <= 0))
    {
        return CM_ERROR;
    }

    nKeyType = TranslateLegacySlotToUniversal(nKeyType);

    switch(nKeyType)
    {
        case KT_EXTKEY_VEND:
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
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_HOST_PUBLIC:
        {
            if (false == ProgramKey(KT_EXTKEY_BTK, nKeyType, pCG, nCGSize))    // [AES][pCG]BTK
            {
                return CM_ERROR;
            }
            break;
        }
        case KT_EXT_BSK:
        {
            if (false == ProgramBSK(pCG, nCGSize))
            {
                return CM_ERROR;
            }
            break;
        }
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_PUBLIC_EXP: // this is program as part of case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PRIVATE_EXP:
        default:
        {
            return CM_ERROR_NOT_SUPPORTED;        // These keys cannot be programmed externally or through this method
        }
    };

    return CM_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Backend-generated Session Key. Programs BSK and sets it as an active Key
//
bool CryptoMgrCP001::ProgramBSK(u8* pCG, u16 nKeySize)
{
    // Ensure that one of the AES keys are active
    int WKSize = NULL;
    //AutoHeapBuffer Auto_NewKey(sizeof(u2048));
    u8* pNewKey = nullptr;// Auto_NewKey.u8Ptr();
    uint nSizeOut = 0;// sizeof(u2048);
    KeyInfoStructure keyInfo;

    switch(m_nActiveKeyType)
    {
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES_VEND:
        {
            WKSize = sizeof(u256);
        } break;
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        {
            WKSize = sizeof(u128);
        } break;
        default:
        {
            return false;
        }
    }

    // Make certain that expected key size lines up
    if (nKeySize != WKSize)
    {
        return false;
    }

    // Decrypt the key
    if (false == Decrypt(pCG, nKeySize, &pNewKey, &nSizeOut, NULL))
    {
        goto Abort;
    }

    // We now have our new key
    if (nSizeOut > sizeof(u2048))
    {
        goto Abort;
    }

    keyInfo.key_size = nSizeOut;
    keyInfo.key_ver = 0;
    keyInfo.key_mode = (KeyModes)GetKeyMode(m_nActiveKeyType);
    if (!GetKeySlot(KT_EXT_BSK, keyInfo.slot))
    {
        goto Abort;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(keyInfo, pNewKey))
    {
        LOGMSG("ProgramBSK:Failed to save key.");
        goto Abort;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(keyInfo.slot))
    {
        goto Abort;
    }
    m_nActiveKeyType = KT_EXT_BSK;

    ReleaseSEBuff(&pNewKey);
    return true;
Abort:
    ReleaseSEBuff(&pNewKey);
    return false;
}


/////////////////////////////////////////////////////////////////////////////
// Set Active Key for biometric operations
//
int CryptoMgrCP001::SetActiveKey(_V100_ENC_KEY_TYPE nKeyType, bool bTCInc)
{
    ResetActiveKey();

    // Invalidate BSK when client sets any key as active key.
    InvalidateBSK();

    u16 key_slot;

    switch(nKeyType)
    {
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        {
            break;
        }
        case KT_EXTKEY_TDES0:
        case KT_EXTKEY_TDES1:
        {
            // setting any other key as active will make SK invalid
            if (KT_EXT_DSK != nKeyType)
            {
                InvalidateDSK();
            }

            DUKPTKeySlots slots;
            u16 nIPEKSlot = 0, nKSNSlot = 0, nTCSlot = 0;
            if (!GetKeySlot(nKeyType, nIPEKSlot))
            {
                return false;
            }
            if (nKeyType == KT_EXTKEY_TDES0)
            {
                if (!GetKeySlot(KT_EXTKEY_KSN_0, nKSNSlot))
                {
                    return false;
                }
                if (!GetKeySlot(TRANS_COUNTER_0, nTCSlot))
                {
                    return false;
                }
            }
            else
            {
                if (!GetKeySlot(KT_EXTKEY_KSN_1, nKSNSlot))
                {
                    return false;
                }
                if (!GetKeySlot(TRANS_COUNTER_1, nTCSlot))
                {
                    return false;
                }
            }
            slots.DUKPT_IPEK_slot = nIPEKSlot;
            slots.DUKPT_KSN_slot = nKSNSlot;
            slots.DUKPT_TC_slot = nTCSlot;
            slots.increase_transaction_counter = bTCInc;

            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_DUKPT_Keys(slots))
            {
                ResetActiveKey();
                LOGMSG("SetActiveKey:Selecting Key %d returned error", nKeyType);
                return CM_ERROR;
            }

            m_nActiveKeyType = nKeyType;

            return CM_OK;
            break;
        }
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        case KT_EXTKEY_AES_VEND:
        case KT_EXT_DSK:
        {
            break;
        }

        // These can't be set as active keys by themselves
        case KT_EXT_SP:
        case KT_EXTKEY_KSN_0:
        case KT_EXTKEY_KSN_1:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_VEND:
        case KT_EXT_BSK:  // This is not set externally but when backend generated SK is loaded it gets set as Active automatically
        case KT_EXTKEY_BTK:    // Not allowed to use for biometric operations. only for Key loading
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PUBLIC_EXP:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_LAST:
        case KT_EXTKEY_CTK:    // Not allowed to use for biometric operations. only for Key loading
        default:
        {
            return CM_ERROR_NOT_SUPPORTED;
        }
    }

    if (!GetKeySlot(nKeyType, key_slot))
    {
        return CM_ERROR;
    }

    // setting any other key as active will make SK invalid
    if (KT_EXT_DSK != nKeyType)
    {
        InvalidateDSK();
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(key_slot))
    {
        ResetActiveKey();
        LOGMSG("SetActiveKey:Selecting for Key %d returned error", nKeyType);
        return CM_ERROR;
    }

    m_nActiveKeyType = nKeyType;

    return CM_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Encrypt the input buffer with active key
//
bool CryptoMgrCP001::Encrypt( u8 *pIn, uint nInSize, u8 **pOutCG,   uint* nOutCGSize, u256 pOutDigSig)
{
    if((NULL == pIn) || (0 == nInSize) )
    {
        return false;
    }

    u8* pTmpOutCG = nullptr;
    uint nTmpOutCGSz = 0;
    if (false == AlignAndEncrypt(pIn, nInSize, &pTmpOutCG, &nTmpOutCGSz, pOutDigSig))
    {
        return false;
    }

    *pOutCG = (u8*)MALLOC(nTmpOutCGSz);
    memcpy(*pOutCG, pTmpOutCG, nTmpOutCGSz);
    *nOutCGSize = nTmpOutCGSz;
    ReleaseSEBuff(&pTmpOutCG);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Decrypt input buffer with active key
//
//DUKPT keys are not used in this config for decrypting. Returns error for DUKPT keys
bool CryptoMgrCP001::Decrypt( u8 *pInCG, uint nInCGSize, u8 **pOut,   uint* nOutSize, u256 pInDigSig)
{
    if((NULL == pInCG) || (NULL == pOut) || (0 == nInCGSize) )
    {
        return false;
    }

    if ((m_nActiveKeyType == KT_EXTKEY_TDES0) || (m_nActiveKeyType ==  KT_EXTKEY_TDES1))
    {
        return false;
        // For DUKPT consider KSN Header in CGPkt and make sure to select the DUKPT keys on SE
    }

    u8* clearBuf = nullptr;
    uint nActualOutSz = 0;
    u16 nSlot = 0;
    if (!GetKeySlot(m_nActiveKeyType, nSlot))
    {
        return false;
    }

    if (false == DecryptBuffer(nSlot, pInCG, nInCGSize, &clearBuf, nActualOutSz))
    {
        ReleaseSEBuff(&clearBuf);
        return false;
    }

    // Check digsig if requested
    if (pInDigSig)
    {
        u8* pCalDigSig = nullptr;
        uint32_t nDigSigLength = 0;
        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(clearBuf, nActualOutSz, HashAlgorithms::SHA2_256, &pCalDigSig, nDigSigLength))
        {
            ReleaseSEBuff(&clearBuf);
            return false;
        }

        if (nDigSigLength != sizeof(u256))
        {
            ReleaseSEBuff(&clearBuf);
            ReleaseSEBuff(&pCalDigSig);
            return false;
        }
        if (memcmp(pInDigSig,pCalDigSig, sizeof(u256)) != 0)
        {
            ReleaseSEBuff(&clearBuf);
            ReleaseSEBuff(&pCalDigSig);
            return false;
        }
        ReleaseSEBuff(&pCalDigSig);
    }
    *pOut = clearBuf;
    *nOutSize = nActualOutSz;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Resets the active key, irrespective of how it got set
//
void CryptoMgrCP001::ResetActiveKey()
{
    m_nActiveKeyType = KT_EXT_LAST;
}

/////////////////////////////////////////////////////////////////////////////
// Decrypts the Spoof Protection Level cryptogram and checks the ANBIO
//
bool CryptoMgrCP001::DecryptAndValidateSPLCG(u8 *pIn, u8 **pOut, uint nSizeIn, uint* nSizeOut)
{
    switch(m_nActiveKeyType)
    {
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_AES_VEND:
        case KT_EXT_BSK:
        {
            break;
        }
        default:
        {
            return false;
        }
    }

    u8* pOutBuff = nullptr;
    uint nOutBuffSz = 0;
    if (false == Decrypt(pIn, nSizeIn, &pOutBuff, &nOutBuffSz, NULL))
    {
        return false;
    }

    if(nOutBuffSz < ANBIO_SIZE_CP001)
    {
        ReleaseSEBuff(&pOutBuff);
        return false;
    }

    if (memcmp(pOutBuff, m_ANBIO, ANBIO_SIZE_CP001) != 0)
    {
        ReleaseSEBuff(&pOutBuff);
        return false;
    }

    *pOut = (u8*)MALLOC(nOutBuffSz);
    memcpy(*pOut, pOutBuff, nOutBuffSz);
    *nSizeOut = nOutBuffSz;
    ReleaseSEBuff(&pOutBuff);
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// Decrypt input key cryptogram with given Transport key type and program
// the key provided in cryptogram
//
bool CryptoMgrCP001::ProgramKey(_V100_ENC_KEY_TYPE nTKKeyType, _V100_ENC_KEY_TYPE nKeyType, u8* pCG, uint nCGSize)
{
    if ((pCG == NULL) || (nCGSize == 0))
    {
        return false;
    }

    if ((nTKKeyType != KT_EXTKEY_CTK) && (nTKKeyType != KT_EXTKEY_BTK))
    {
        return false;
    }

    // Decrypt using TKMode[pCG]TK, where TK is either BTK or CTK
    u16 nTKVer = 0;
    u16 nTKKeyMode = KM_MODE_NONE;
    u8 nTKKCV32[KCV_SIZE] = {};
    AutoHeapBuffer Auto_TK(sizeof(u2048));
    u8* pTK = Auto_TK.u8Ptr();
    u32 nTKSize = 0;

    u8* pPlainText = nullptr;
    u16 nSlot = 0;
    if (!GetKeySlot(nTKKeyType, nSlot))
    {
        return false;
    }
    if (!DecryptBuffer(nSlot, pCG, nCGSize, &pPlainText, nCGSize))
    {
        ReleaseSEBuff(&pPlainText);
        return false;
    }

    // Validate
    u16 nNewKeyType = 0;
    u16 nKeyVersion = 0;
    u8     pKCV[KCV_SIZE] = {};
    u8  pANBIO[ANBIO_SIZE_CP001] = {};
    u8* pKey = NULL;
    u16 nKeySize = 0;
    if (false == DecodeKeyFromOpaque(pPlainText, nCGSize, &nNewKeyType, &nKeyVersion, pKCV, pANBIO, &pKey, nKeySize))
    {
        ReleaseSEBuff(&pPlainText);
        return false;
    }
    ReleaseSEBuff(&pPlainText);
    AutoFreePtr afp_Key(pKey);

    // Validate
    if (false == ValidateOpaque(nNewKeyType, nKeyVersion, pKCV, pANBIO, pKey, nKeySize))
    {
        return false;
    }

    if (nKeyType != nNewKeyType)
    {
        return false;
    }

    // If its the BPK, extract actual Key
    bool bRes = false;
    if (nNewKeyType == KT_EXTKEY_HOST_PUBLIC)
    {
        u8* pASNKey = (u8*)MALLOC(256);
        u8  exponent[3];
        u32 exponentInt = 0;
        ParseASNFromBuffer(pASNKey, 256, exponent, 3, pKey, nKeySize);
        // Set the Key.
        KeyInfoStructure keyInfo;
        memcpy(keyInfo.key_check_value, pKCV, KCV_SIZE);
        keyInfo.key_mode = (KeyModes)KM_RSA_2048_v15;
        keyInfo.key_size = 256;
        keyInfo.key_ver = nKeyVersion;
        if (!GetKeySlot(KT_EXTKEY_HOST_PUBLIC, keyInfo.slot))
        {
            return false;
        }
        keyInfo.rsa_sign_mode = SIG_RSA_SHA1;

        if (IsZeroVecKey(pASNKey, keyInfo.key_size))
        {
            printf("\nDEBUGGING... Zero vec key received, revoking KT_EXTKEY_HOST_PUBLIC...\n");
            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(keyInfo.slot))
            {
                FREE(pASNKey);
                return false;
            }
            bRes = true;
        }
        else
        {
            bRes = (CryptExecStatus::Successful == SECURE_ELEMENT->Execute_Set_Key(keyInfo, pASNKey));
        }



        FREE(pASNKey);
        pASNKey = nullptr;
        if (bRes == false)
        {
            return false;
        }
        // Set the exponent
        memcpy(&exponentInt, exponent, 3);
        // Set this key
        KeyInfoStructure keyInfoExp;
        memcpy(keyInfoExp.key_check_value, pKCV, KCV_SIZE);
        keyInfoExp.key_mode = (KeyModes)KM_RSA_2048_v15;
        keyInfoExp.key_size = sizeof(u32);
        keyInfoExp.key_ver = 0;
        if (!GetKeySlot(KT_EXTKEY_PUBLIC_EXP, keyInfoExp.slot))
        {
            return false;
        }

        if (IsZeroVecKey((u8*)&exponentInt, keyInfo.key_size))
        {
            printf("\nDEBUGGING... Zero vec key received, revoking KT_EXTKEY_PUBLIC_EXP...\n");
            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(keyInfo.slot))
            {
                return false;
            }
            bRes = true;
        }
        else
        {
            bRes = (CryptExecStatus::Successful == SECURE_ELEMENT->Execute_Set_Key(keyInfoExp, (u8*)&exponentInt));
        }
    }
    else
    {
        u16 nKeyMode = GetKeyMode(nNewKeyType);

        KeyInfoStructure keyInfo;
        memcpy(keyInfo.key_check_value, pKCV, KCV_SIZE);
        keyInfo.key_mode = (KeyModes)nKeyMode;
        keyInfo.key_size = nKeySize;
        keyInfo.key_ver = nKeyVersion;
        if (!GetKeySlot(nNewKeyType, keyInfo.slot))
        {
            return false;
        }
        //Revoke the Key if its a zeroVecKey
        if (IsZeroVecKey(pKey, keyInfo.key_size))
        {
            if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(keyInfo.slot))
            {
                return false;
            }
            bRes = true;
        }
        else {
            bRes = (CryptExecStatus::Successful == SECURE_ELEMENT->Execute_Set_Key(keyInfo, pKey));
        }
    }

    // When the TDES0/1 keys are loaded, the transaction counter is set to zero.
    // Typically, this value increments with every capture, but if the active
    // key is set to one of these keys /prior/ to a capture, the transaction
    // counter will be at zero, which is an invalid value.  The solution is
    // to increment the counter here so that this error does not occur.
    if (nNewKeyType == KT_EXTKEY_TDES0)
    {
        KeyInfoStructure keyInfo;
        keyInfo.key_size = sizeof(uint);
        if (!GetKeySlot(TRANS_COUNTER_0, keyInfo.slot))
        {
            return false;
        }
        uint nVal = 1;
        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(keyInfo, (u8*)&nVal))
        {
            LOGMSG("ProgramKey:Failed to set transaction counter!");
            return false;
        }
    }
    else if (nNewKeyType == KT_EXTKEY_TDES1)
    {
        KeyInfoStructure keyInfo;
        keyInfo.key_size = sizeof(uint);
        if (!GetKeySlot(TRANS_COUNTER_1, keyInfo.slot))
        {
            return false;
        }
        uint nVal = 1;
        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(keyInfo, (u8*)&nVal))
        {
            LOGMSG("ProgramKey:Failed to set transaction counter!");
            return false;
        }
    }

    return bRes;
}

/////////////////////////////////////////////////////////////////////////////
// Used by ProgramKey to extract values from the decrypted packet
//
bool CryptoMgrCP001::DecodeKeyFromOpaque(u8* pPlainText, int nPlainTextSize, u16 *nKeyType, u16 *nKeyVersion, u8 *pKCV, u8* pANBIO, u8** pKey, u16& nKeySize)
{
    bool bRes = true;
    nKeySize = 0;

    if (nPlainTextSize < ( sizeof(u16)*2 + KCV_SIZE + ANBIO_SIZE_CP001 + nKeySize))
    {
        return false;
    }
    u8* pPtr = pPlainText;
    memcpy(nKeyType, pPtr, sizeof(u16));     pPtr += sizeof(u16);
    memcpy(nKeyVersion, pPtr, sizeof(u16)); pPtr += sizeof(u16);
    memcpy(pKCV, pPtr, KCV_SIZE);     pPtr += KCV_SIZE;
    memcpy(pANBIO, pPtr, ANBIO_SIZE_CP001);    pPtr += ANBIO_SIZE_CP001;

    *nKeyType = TranslateLegacySlotToUniversal((_V100_ENC_KEY_TYPE)*nKeyType);

    size_t nExpSize = 0 ;
    u8       exponent[3];
    // Is it the BPK?
    if (*nKeyType == KT_EXTKEY_HOST_PUBLIC)
    {
        // Decode ASN so we know the key size
        GetASNSize(pPtr, nKeySize);
        *pKey = (u8*)MALLOC(nKeySize);
        memcpy(*pKey, pPtr, nKeySize);
    }
    else
    {
        u16 nKeyMode = GetKeyMode(*nKeyType);
        nKeySize = GetKeySize(*nKeyType, nKeyMode);

        if (nKeySize == 0)
        {
            return false;
        }
        *pKey = (u8*)MALLOC(nKeySize);
        memcpy(*pKey, pPtr, nKeySize);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Validates fields
//
bool CryptoMgrCP001::ValidateOpaque(u16 nKeyType, u16 nKeyVersion, u8* pKCV, u8* pANBIO, u8* pKey, u16 nKeySize)
{
    // Validate KCV - now Slot dependent.
    u16 nKeyMode = GetKeyMode(nKeyType);
    int nKCVType = GetKCVType(nKeyType, nKeyMode);

    if (nKCVType != KCV_NONE) // validate only if we support
    {
        u8 pCalculatedKCV[KCV_SIZE];

        if (false == CreateKCV(pKey, (uint)nKeySize, ZEROS_SIZE, KCV_SIZE, pCalculatedKCV, nKCVType))
        {
            return false;
        }

        // Validate KCV
        if (0 != memcmp(pKCV, pCalculatedKCV, KCV_SIZE))
        {
            return false;
        }
    }

    // Validate ANBIO
    if (memcmp(pANBIO, m_ANBIO, ANBIO_SIZE_CP001) != 0)
    {
        return false;
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// Encrypt the input buffer with provided key
//
bool CryptoMgrCP001::AlignAndEncrypt(u8 *pIn, uint nInSize, u8 **pOutCG,   uint* nOutCGSize, u256 pOutDigSig)
{
    if((NULL == pIn) || (0 == nInSize) )
    {
        return false;
    }

    // Alloc in Buffer with N_Block aligned and copy
    uint nInAligSz = nInSize;
    if( nInAligSz%BLOCK_SIZE )
    {
        int nAdj = BLOCK_SIZE - (nInAligSz%BLOCK_SIZE);
        nInAligSz+=nAdj;
    }

     AutoHeapBuffer Auto_pInBuff(nInAligSz);
    u8* pInBuff = Auto_pInBuff.u8Ptr();
    if(NULL == pInBuff) return false;
    memset(pInBuff, 0, nInAligSz);
    memcpy(pInBuff, pIn, nInSize);

    *pOutCG = nullptr;
    uint nOutSz = 0;

    //Select Key on SE
    if (SelectKeyOnSE(m_nActiveKeyType))
    {
        return false;

    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Encrypt(pInBuff, nInAligSz, pOutCG, nOutSz, nullptr, 0))
    {
        ReleaseSEBuff(pOutCG);
        return false;
    }

    // Cal digsig if requested
    if(pOutDigSig)
    {
        u8* hashData = nullptr;
        uint32_t hashLength = 0;
        if(CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pInBuff, nInAligSz, HashAlgorithms::SHA2_256, &hashData, hashLength))
        {
            ReleaseSEBuff(&hashData);
            ReleaseSEBuff(pOutCG);
            return false;
        }
        memcpy(pOutDigSig, hashData, sizeof(u256));
        ReleaseSEBuff(&hashData);
    }

    *nOutCGSize = nOutSz;
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// Extract Keys and Exponent from ASN.1 Buff
//
bool CryptoMgrCP001::ParseASNFromBuffer(u8 *pKey, u16 nKeySize, u8 *pExp, u16 nExpSize, unsigned char *pASNBuff, u16 nASNSz)
{
    union
    {
        u8  byte[2];
        u16 size;
    } nSize;

    unsigned char* pInBuff = pASNBuff;

    // Start
    if (*pInBuff++ != 0x30 )
        return false;

    size_t nSz = nASNSz-1;
    // find 1st INTEGER field
    do
    {
        // Start
        if (*pInBuff++ == 0x02 )
            break;

        nSz--;
    } while(nSz);

    if(nSz == 0) return false; // We can't find return

    // Start
    if (*pInBuff++ != 0x82 )
        return false;

    // read size of key
    nSize.byte[1]= *pInBuff++;
    nSize.byte[0] = *pInBuff++;

    // check for byte stuffing
    //
    if (nSize.size == nKeySize + 1)
    {
        // read stuff byte
        if (*pInBuff++ != 0x00)
            return false;
    }

    // read key
    memcpy(pKey, pInBuff, nKeySize);
    pInBuff += nKeySize;

    // Start INTEGER
    if (*pInBuff++ != 0x02)
        return false;

    // size
    if (*pInBuff++ != nExpSize)
        return false;

    memcpy(pExp, pInBuff, nExpSize);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Extracts the size from an ASN-format key
//
bool CryptoMgrCP001::GetASNSize(unsigned char* pASNBuff, u16 &nASNSz)
{
    union
    {
        u8  byte[2];
        u16 size;
    } nSize;

    if (pASNBuff == NULL)
    {
        return false;
    }

    unsigned char* pInBuff = pASNBuff;
    // Start
    if (*pInBuff++ != 0x30)
    {
        return false;
    }
    if (*pInBuff++ != 0x82)
    {
        return false;
    }

    // read size of remaining ASNBuffer
    nSize.byte[1]= *pInBuff++;
    nSize.byte[0] = *pInBuff++;

    nASNSz = 2 + 2 + nSize.size;    // 2 Tokens each 1 byte + 2bytes for storing size + size

    return true;
}


// Helper method to invalidate DSK
int CryptoMgrCP001::InvalidateDSK()
{
    int nErr = 0;
    u16 nSlot = 0;
    if (!GetKeySlot(KT_EXT_DSK, nSlot))
    {
        nErr = CM_ERROR;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(nSlot))
    {
        nErr = CM_ERROR;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Remove_Property((u8*)DSK_PROP_STRING_CP001.c_str(), DSK_PROP_STRING_CP001.length()))
    {
        nErr = CM_ERROR;
    }
    return nErr;
}

// Helper method to invalidate BSK
int CryptoMgrCP001::InvalidateBSK()
{
    u16 nSlot = 0;
    if (!GetKeySlot(KT_EXT_BSK, nSlot))
    {
        return CM_ERROR;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(nSlot))
    {
        return CM_ERROR;
    }
}

int CryptoMgrCP001::SelectKeyOnSE(_V100_ENC_KEY_TYPE nKeyType)
{

    u16 key_slot;

    switch (nKeyType)
    {
    case KT_EXTKEY_AES0:
    case KT_EXTKEY_AES1:
    case KT_EXTKEY_AES2:
    case KT_EXTKEY_AES3:
    {
        break;
    }
    case KT_EXTKEY_TDES0:
    case KT_EXTKEY_TDES1:
    {

        DUKPTKeySlots slots;
        u16 nIPEKSlot = 0, nKSNSlot = 0, nTCSlot = 0;
        if (!GetKeySlot(nKeyType, nIPEKSlot))
        {
            return false;
        }
        if (nKeyType == KT_EXTKEY_TDES0)
        {
            if (!GetKeySlot(KT_EXTKEY_KSN_0, nKSNSlot))
            {
                return false;
            }
            if (!GetKeySlot(TRANS_COUNTER_0, nTCSlot))
            {
                return false;
            }
        }
        else
        {
            if (!GetKeySlot(KT_EXTKEY_KSN_1, nKSNSlot))
            {
                return false;
            }
            if (!GetKeySlot(TRANS_COUNTER_1, nTCSlot))
            {
                return false;
            }
        }
        slots.DUKPT_IPEK_slot = nIPEKSlot;
        slots.DUKPT_KSN_slot = nKSNSlot;
        slots.DUKPT_TC_slot = nTCSlot;
        slots.increase_transaction_counter = false;

        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_DUKPT_Keys(slots))
        {
            LOGMSG("SelectKeyOnSE:Selecting Key %d returned error", nKeyType);
            return CM_ERROR;
        }
        return CM_OK;
        break;
    }
    case KT_EXTKEY_TDES2:
    case KT_EXTKEY_TDES3:
    case KT_EXTKEY_AES_VEND:
    case KT_EXT_DSK:
    case KT_EXT_BSK:
    {
        break;
    }

    // These can't be set as active keys by themselves
    case KT_EXT_SP:
    case KT_EXTKEY_KSN_0:
    case KT_EXTKEY_KSN_1:
    case KT_EXTKEY_SPARE_2:
    case KT_EXTKEY_SPARE_3:
    case KT_EXTKEY_VEND:
        // This is not set externally but when backend generated SK is loaded it gets set as Active automatically
    case KT_EXTKEY_BTK:    // Not allowed to use for biometric operations. only for Key loading
    case KT_EXTKEY_HOST_PUBLIC:
    case KT_EXTKEY_DEVICE_PUBLIC:
    case KT_EXTKEY_DEVICE_PRIVATE:
    case KT_EXTKEY_DEVICE_P:
    case KT_EXTKEY_DEVICE_Q:
    case KT_EXTKEY_PUBLIC_EXP:
    case KT_EXTKEY_PRIVATE_EXP:
    case KT_EXT_LAST:
    case KT_EXTKEY_CTK:    // Not allowed to use for biometric operations. only for Key loading
    default:
    {
        return CM_ERROR_NOT_SUPPORTED;
    }
    }

    if (!GetKeySlot(nKeyType, key_slot))
    {
        return CM_ERROR;
    }

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(key_slot))
    {

        LOGMSG("SelectKeyOnSE:Selecting for Key %d returned error", nKeyType);
        return CM_ERROR;
    }


    return CM_OK;
}

// Helper method to get key mode based on key type
u16 GetKeyMode(u16 nSlot)
{
    switch (nSlot)
    {
    case KT_EXTKEY_VEND:
    case KT_EXTKEY_AES0:
    case KT_EXTKEY_AES1:
    case KT_EXTKEY_AES_VEND:
    case KT_EXTKEY_SPARE_2:
    case KT_EXTKEY_SPARE_3:
    case KT_EXT_DSK:
    {
        return KM_AES_256_CBC;
    }
    case KT_EXTKEY_AES2:
    case KT_EXTKEY_AES3:
    {
        return KM_AES_128_CBC;
    }
    case KT_EXTKEY_TDES0:
    case KT_EXTKEY_TDES1:
    {
        return KM_DUKPT_IPEK_128;
    }
    case KT_EXTKEY_TDES2:
    case KT_EXTKEY_TDES3:
    {
        return KM_TDES_ABC_CBC;
    }
    case KT_EXTKEY_HOST_PUBLIC:
    case KT_EXTKEY_DEVICE_PUBLIC:
    case KT_EXTKEY_DEVICE_PRIVATE:
    case KT_EXTKEY_DEVICE_P:// only keyvalue saved on MAXQ
    case KT_EXTKEY_DEVICE_Q:// only keyvalue saved on MAXQ
    case KT_EXTKEY_PRIVATE_EXP:// only keyvalue saved on MAXQ
    case KT_EXTKEY_PUBLIC_EXP:// only keyvalue saved on MAXQ
    {
        return KM_RSA_2048_v15;
        //            return KM_RSA_2048_v21;
    }
    case KT_EXTKEY_KSN_0:
    case KT_EXTKEY_KSN_1:
    {
        return KM_DUKPT_KSN_64;
    }
    // variable keymode slots (in other configs, but not CP001)
    case KT_EXT_BSK:// Depends on the activekey slot which sets this key. Usually one should not call this function with BSK
    case KT_EXTKEY_CTK:
    case KT_EXTKEY_BTK:
    {
        return KM_AES_256_CBC;
    }
    }
    return KM_MODE_NONE;
}

