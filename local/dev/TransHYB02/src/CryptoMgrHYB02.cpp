#include "CryptoMgrHYB02.h"
#include "ISecureElement.h"
#include "CriticalErrorLog.h"
#include "AutoHeapBuffer.h"
#include "CfgMgr.h"


std::string FKL_LOCK_PROP_STRING = "FKL_LOCK_STATUS";


CryptoMgrHYB02::CryptoMgrHYB02()
{

}

CryptoMgrHYB02::~CryptoMgrHYB02()
{
}

bool CryptoMgrHYB02::Init()
{
    // Set ANBIO to a random number - do not set to all zeros - guessable...
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_GetRandomBuffer(reinterpret_cast<u8*>(&m_ANBIO), sizeof(m_ANBIO)))
    {
        LOGMSG("Init:Get random number returned error.");
        return false;
    }

    m_nActiveKeyType = KT_EXT_LAST;
    InvalidateBSK();
    return true;
}

/******************************************************************************/
/***    Get random number which sets ANBIO ***/
/******************************************************************************/
bool CryptoMgrHYB02::GetRandomNumber(u256* pRnd)
{
    if (!pRnd)
    {
        LOGMSG("GetRandomNumber:NULL input Rnd buffer");
        return false;
    }

    CryptExecStatus CrypStat = SECURE_ELEMENT->Execute_GetRandomBuffer((u8*)pRnd, 32);
    if (CryptExecStatus::Successful != CrypStat)
    {
        LOGMSG("GetRandomNumber:Getting Random number returned error");
        return false;
    }

    memcpy(m_ANBIO, pRnd, sizeof(u256));
    return true;
}

/******************************************************************************/
/***    Get ANBIO    ***/
/******************************************************************************/
bool CryptoMgrHYB02::GetANBIO(u256* pRnd)
{
    if (!pRnd)
    {
        LOGMSG("GetANBIO:NULL input Rnd buffer");
        return false;
    }
    memcpy(pRnd, m_ANBIO, sizeof(u256));

    return true;
}

/******************************************************************************/
/***    Check if FKL locked        ***/
/******************************************************************************/
bool CryptoMgrHYB02::IsFactoryKeyLoadingLocked()
{
    // Retrieve the FKL Lock property
    u32 nFKLLock;
    u8* pFKLLock = nullptr;
    uint32_t nFKLLockLength = 0;
    CryptExecStatus rc = SECURE_ELEMENT->Execute_Get_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length(), &pFKLLock, nFKLLockLength);
    if (CryptExecStatus::Successful != rc)
    {
        if (CryptExecStatus::Not_Exist != rc)
        {
            LOGMSG("IsFactoryKeyLoadingLocked: Get FKL Lock returned error.");
        }
        return false;
    }

    if (nFKLLockLength != sizeof(u32))
    {
        ReleaseSEBuff(&pFKLLock);
        return false;
    }

    memcpy(&nFKLLock, pFKLLock, sizeof(u32));

    ReleaseSEBuff(&pFKLLock);
    return (nFKLLock == 1) ? true : false;
}

/******************************************************************************/
/***    Lock FKL ***/
/******************************************************************************/
bool CryptoMgrHYB02::LockFactoryMode()
{
    // Make sure the BTK has been loaded
    u16 key_slot = 0;
    if (!GetKeySlot(KT_EXTKEY_BTK, key_slot))
    {
        LOGMSG("LockFactoryMode:Failed to find key (%d) in key map!", KT_EXTKEY_BTK);
        return false;
    }
    if (false == IsKeyLoaded(key_slot))
    {
        LOGMSG("LockFactoryMode: The BTK is not loaded. Locking FKL is not allowed in this state.");
        return false;
    }


    // Store the FKL Lock as a property
    u32 nFKLLock = 1;
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length(), (uint8_t*)&nFKLLock, sizeof(u32)))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length());

        LOGMSG("LockFactoryMode:Failed to set property for FKL Lock.");
        return false;
    }

    // Check it
    u8* pTmp = nullptr;
    u32 valLength = 0;
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Get_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length(), &pTmp, valLength))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length());

        LOGMSG("LockFactoryMode:Failed to get property for FKL Lock.");
        return false;
    }
    if (valLength != sizeof(u32))
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length());

        LOGMSG("LockFactoryMode:FKLlock size read didn't match.");
        ReleaseSEBuff(&pTmp);
        return false;
    }
    if (*(u32*)pTmp != nFKLLock)
    {
        SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length());

        LOGMSG("LockFactoryMode:Set property for FKL Lock does not match expected value.");
        ReleaseSEBuff(&pTmp);
        return false;
    }

    ReleaseSEBuff(&pTmp);
    return true;
}

/******************************************************************************/
/***    Unlock FKL    ***/
/******************************************************************************/
bool CryptoMgrHYB02::UnlockFactoryMode()
{
    // Remove the property from the secure element
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Remove_Property((uint8_t*)FKL_LOCK_PROP_STRING.c_str(), FKL_LOCK_PROP_STRING.length()))
    {
        LOGMSG("UnlockFactoryMode: Removing lock returned error");
        return false;
    }

    // Check it
//<TODO>: Validate property was removed
    //if(false == m_pKeyMgr->Get_Key_Lock_Status(&bVal))
    //{
    //    return false;
    //}

    //if(bVal != false)
    //{
    //    return false;
    //}

    return true;
}

/******************************************************************************/
/***    Get Key. Only allowed keys are DSK(returned encrypted)
/        and Device Public    ***/
/******************************************************************************/
bool CryptoMgrHYB02::GetKey(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize)
{
    memset(pKey, 0, sizeof(pKey));
    if (pKCV32 == NULL)
    {
        return false;
    }

    //Only Keys allowed to get in provisioning
    if (nKeyType != KT_EXTKEY_DEVICE_PUBLIC)
    {
        LOGMSG("GetKey:Retrieving Key %d is not supported", nKeyType);
        return false;
    }

    uint8_t* pOut = nullptr;
    uint32_t nOutSize = 0;
    CryptExecStatus CrypStat = SECURE_ELEMENT->Execute_Get_RSA_PublicKey(&pOut, nOutSize);
    if (CrypStat != CryptExecStatus::Successful)
    {
        LOGMSG("GetKey:Retrieving Key %d returned error", nKeyType);
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
}

/******************************************************************************/
/***    Get Key info    ***/
/******************************************************************************/
bool CryptoMgrHYB02::GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType,u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32 )
{
    // Make sure we have access to this key
    u16 key_slot;

    if (!GetKeySlot(nKeyType, key_slot))
    {
        LOGMSG("GetKeyInfo: Key (%d) not supported for this configuration!", nKeyType);
        return false;
    }

    u16 nTempKeyMode = 0;
    if (!PullKeyInfo(key_slot, nKeyVer, nTempKeyMode, pKCV32))
    {
        LOGMSG("\nGetKeyInfo: Failed to get key info for key %d (slot %d)!", nKeyType, key_slot);
        return false;
    }
    nKeyMode = (_V100_ENC_KEY_MODE)nTempKeyMode;
    return true;
}

/******************************************************************************/
/***    FKL        ***/
/*** CG = KeyModeCTK [ KeyBlock ] KCTK ***/
/*** pCGKey = [ CG ] ***/
/*** nCGSize = KeyBlockSize ***/
/******************************************************************************/
int CryptoMgrHYB02::InitiateFactoryKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize)
{
    int nRes = CM_ERROR;
    if((pCGKey == NULL) || (nCGSize ==0))
    {
        return nRes;
    }

    u8* pTmpOutCG = nullptr;
    uint nTmpOutCGSz = 0;

    switch(nKeyType)
    {
        case KT_EXTKEY_BTK:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        case KT_EXTKEY_AES_VEND:
        {
            if(true == ProgramKey(KT_EXTKEY_CTK, nKeyType, pCGKey, nCGSize, &pTmpOutCG, nTmpOutCGSz))    // [AES][pCGKey]CTK
            {
                nRes = CM_OK;
            }
        } break;
        case KT_EXTKEY_TDES0:// DUKPT keys not supported
        case KT_EXTKEY_TDES1:// DUKPT keys not supported
        case KT_EXTKEY_KSN_0:// DUKPT keys not supported
        case KT_EXTKEY_KSN_1:// DUKPT keys not supported
        case KT_EXT_BSK: // only allowed in RKL
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_PUBLIC_EXP:    // This is programmed as part of KT_EXTKEY_HOST_PUBLIC
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_DSK:
        default:
        {
            nRes = CM_ERROR_NOT_SUPPORTED;        // These keys cannot be programmed externally or through this method
        } break;
    };

    if (nRes == CM_OK)
    {
        *pOutCG = (u8*)MALLOC(nTmpOutCGSz);
        memcpy(*pOutCG, pTmpOutCG, nTmpOutCGSz);
        nOutCGSize = nTmpOutCGSz;
    }
    ReleaseSEBuff(&pTmpOutCG);
    return nRes;
}

/******************************************************************************/
/***    RKL     ***/
/*** CG = KeyModeBTK [ KeyBlock ] KBTK ***/
/*** pCGKey = [ CG ] ***/
/*** nCGSize = KeyBlockSize ***/
/******************************************************************************/
int CryptoMgrHYB02::InitiateRemoteKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8*  pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize)
{
    int nRes = CM_ERROR;
    if((pCGKey == NULL) || (nCGSize ==0))
    {
        return nRes;
    }

    u8* pTmpOutCG = nullptr;
    uint nTmpOutCGSz = 0;

    switch(nKeyType)
    {
        case KT_EXTKEY_BTK:
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        case KT_EXTKEY_AES_VEND:

        {
            // [AES][pCGKey]BTK
            if( true == ProgramKey(KT_EXTKEY_BTK, nKeyType, pCGKey, nCGSize, &pTmpOutCG, nTmpOutCGSz))
            {
                nRes = CM_OK;
            }
        } break;
        case KT_EXT_BSK:
        {
            if(true ==  ProgramBSK(KT_EXTKEY_BTK, pCGKey, nCGSize, &pTmpOutCG, nTmpOutCGSz))
            {
                nRes = CM_OK;
            }
        }break;
        case KT_EXTKEY_TDES0:// DUKPT keys not supported
        case KT_EXTKEY_TDES1:// DUKPT keys not supported
        case KT_EXTKEY_KSN_0:// DUKPT keys not supported
        case KT_EXTKEY_KSN_1:// DUKPT keys not supported
        case KT_EXTKEY_SPARE_2:
        case KT_EXTKEY_SPARE_3:
        case KT_EXTKEY_VEND:
        case KT_EXTKEY_CTK:
        case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_PUBLIC_EXP: // this is program as part of case KT_EXTKEY_HOST_PUBLIC:
        case KT_EXTKEY_DEVICE_PUBLIC:
        case KT_EXTKEY_DEVICE_PRIVATE:
        case KT_EXTKEY_DEVICE_P:
        case KT_EXTKEY_DEVICE_Q:
        case KT_EXTKEY_PRIVATE_EXP:
        case KT_EXT_DSK:
        default:
        {
            nRes = CM_ERROR_NOT_SUPPORTED;        // These keys cannot be programmed in RKL or through this method.
        } break;
    };

    if (nRes == CM_OK)
    {
        *pOutCG = (u8*)MALLOC(nTmpOutCGSz);
        memcpy(*pOutCG, pTmpOutCG, nTmpOutCGSz);
        nOutCGSize = nTmpOutCGSz;

    }
    ReleaseSEBuff(&pTmpOutCG);
    return nRes;

}

/******************************************************************************/
/***    Set Active Key for biometric operations        ***/
/******************************************************************************/
int CryptoMgrHYB02::SetActiveKey(_V100_ENC_KEY_TYPE nKeyType, bool bTCInc)
{
    ResetActiveKey();
    // Invalidate BSK when client sets any key as active key.
    InvalidateBSK();

    u16 key_slot = 0;

    switch(nKeyType)
    {
        case KT_EXTKEY_AES0:
        case KT_EXTKEY_AES1:
        case KT_EXTKEY_AES2:
        case KT_EXTKEY_AES3:
        case KT_EXTKEY_TDES2:
        case KT_EXTKEY_TDES3:
        case KT_EXTKEY_AES_VEND:
        {
            if (!GetKeySlot(nKeyType, key_slot))
            {
                LOGMSG("SetActiveKey: Failed to find key (%d) in key map!", nKeyType);
                return CM_ERROR;
            }
        }break;
        // These can't be set as active keys by themselves
        case KT_EXT_DSK:// Not supported
        case KT_EXTKEY_TDES0:// DUKPT keys not supported
        case KT_EXTKEY_TDES1:// DUKPT keys not supported
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

    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(key_slot))
    {
        ResetActiveKey();
        LOGMSG("GetKeyInfo:Selecting for Key %d returned error", nKeyType);
        return CM_ERROR;
    }

    m_nActiveKeyType = nKeyType;

    return CM_OK;
}

/******************************************************************************/
/***    Encrypt the input buffer with active key     ***/
/******************************************************************************/
bool CryptoMgrHYB02::Encrypt( u8 *pIn, uint nInSize, u8 **pOutCG,   uint* nOutCGSize, u256 pOutDigSig)
{
    if((NULL == pIn) || (0 == nInSize) )
    {
        return false;
    }

    u8* pTmpOutCG = nullptr;
    uint nTmpOutCGSz = 0;

    if(false == AlignAndEncrypt(m_nActiveKeyType, pIn, nInSize, &pTmpOutCG, &nTmpOutCGSz, pOutDigSig))
    {
        return false;
    }
    *pOutCG = (u8*)MALLOC(nTmpOutCGSz);
    memcpy(*pOutCG, pTmpOutCG, nTmpOutCGSz);
    *nOutCGSize = nTmpOutCGSz;
    ReleaseSEBuff(&pTmpOutCG);
    return true;
}

/******************************************************************************/
/***    Decrypt input buffer with active key     ***/
/******************************************************************************/
bool CryptoMgrHYB02::Decrypt( u8 *pInCG, uint nInCGSize, u8 **pOut, uint &nOutSize, u256 pInDigSig)
{
    if ((NULL == pInCG) || (0 == nInCGSize))
    {
        LOGMSG("Decrypt:Initial input arugment validation failed");
        return false;
    }
    u8* clearBuf = nullptr;
    uint nActualOutSz = 0;
    u16 key_slot = 0;
    if (!GetKeySlot(m_nActiveKeyType, key_slot))
    {
        LOGMSG("Decrypt: Failed to find key (%d) in key map!", m_nActiveKeyType);
        return false;
    }

    if (!DecryptBuffer(key_slot, pInCG, nInCGSize, &clearBuf, nActualOutSz))
    {
        LOGMSG("Decrypt: Failed to decrypt buffer!");
        return false;
    }

    *pOut = (u8*)MALLOC(nActualOutSz);
    memcpy(*pOut, clearBuf, nActualOutSz);
    ReleaseSEBuff(&clearBuf);
    // Check digsig if requested
    if(pInDigSig)
    {
        u8* pCalDigSig = nullptr;
        uint nDSSize = 0;

        // Calculate the signature
        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(*pOut, nActualOutSz, HashAlgorithms::SHA2_256, &pCalDigSig, nDSSize))
        {
            FREE(*pOut);*pOut = nullptr;
            LOGMSG("Decrypt: Hash computation step returned error.");
            return false;
        }

        if (nDSSize != sizeof(u256))
        {
            FREE(*pOut); *pOut = nullptr;
            ReleaseSEBuff(&pCalDigSig);
            return false;
        }

        if(memcmp(pInDigSig, pCalDigSig, sizeof(u256)) != 0)
        {
            FREE(*pOut); *pOut = nullptr;
            ReleaseSEBuff(&pCalDigSig);
            return false;
        }
        ReleaseSEBuff(&pCalDigSig);
    }

    nOutSize = nActualOutSz;
    return true;
}

/******************************************************************************/
/***    Release mem Buff***/
/******************************************************************************/
bool CryptoMgrHYB02::ReleaseMemBuff(u8* pIn)
{
    if(pIn) FREE(pIn);

    return true;

}


/******************************************************************************/
//*** Helper functions ***//
/******************************************************************************/

/******************************************************************************/
/***    Reset Active Key     ***/
/******************************************************************************/
void CryptoMgrHYB02::ResetActiveKey()
{
    m_nActiveKeyType = KT_EXT_LAST;
}

/******************************************************************************/
/***    Decrypt input key cryptogram with given Key type and
/        program the key provided in cryptogram    ***/
/******************************************************************************/
bool CryptoMgrHYB02::ProgramKey(_V100_ENC_KEY_TYPE nTKKeyType, _V100_ENC_KEY_TYPE nPKKeyType, u8* pCGKeyBlock, uint nCGKeyBlockSize,
                                u8** pOutCG, uint& nOutCGSize)
{
    if((pCGKeyBlock == NULL) || (nCGKeyBlockSize ==0))
    {
        LOGMSG("ProgramKey: Invalid Parameters!");
        return false;
    }

    bool bRes = false;

    int     nTKAlgoMode;
    u16  nTKVer        = 0;
    u16  nTKKeyMode = KM_MODE_NONE;
    u8      nTKKCV32[KCV_SIZE_HYB02];
    u8      nKCV32[KCV_SIZE_HYB02];
    u8   pANBIO[ANBIO_SIZE_HYB02];
    u8   pANSOL[ANSOL_SIZE_HYB02];
    u8*  pCGKey = NULL;
    u8*  pKey = NULL;//Allocated by DecodeKeyFromOpaque
    u32  nKeySize = 0;
    u16  nKeyType = 0;
    u16  nKeyMode = KM_MODE_NONE;
    u16  nKeyVer = 0;
    u16  nKeyCryptoSize =0;

    u8* pKeyBlockClear = nullptr;

    uint nKeyBlockSize = 0;

    // Decrypt the KeyBlock cryptogram with the specified Transport Key slot
    u16 key_slot = 0;
    if (!GetKeySlot(nTKKeyType, key_slot))
    {
        LOGMSG("ProgramKey: Failed to find key (%d) in key map!", nTKKeyType);
        return false;
    }
    bRes = DecryptBuffer(key_slot, pCGKeyBlock, nCGKeyBlockSize, &pKeyBlockClear, nKeyBlockSize);

    if (bRes == false)
    {
        LOGMSG("ProgramKey: Decrypting the KeyBlock for Key %d returned error", nPKKeyType);
        return false;
    }

    // Decode decrypted contents
    bRes = DecodeKeyFromOpaque(pKeyBlockClear, nKeyBlockSize, &nKeyType, &nKeyVer, &nKeyMode, nKCV32, &nKeyCryptoSize, pANBIO, pANSOL, &pCGKey, nKeySize);
    ReleaseSEBuff(&pKeyBlockClear);
    if( false == bRes )
    {
        LOGMSG("ProgramKey: Decoding the decrypted KeyBlock for Key %d returned error", nKeyType);
        return false;
    }
    AutoFreePtr afp_CGKey(pCGKey);

    // If the key sent is not the one said return error.
    if(nPKKeyType != nKeyType)
    {
        LOGMSG("ProgramKey: The KeyBlock is not valid");
        return false;
    }

    AutoHeapBuffer Auto_pKey(nKeyCryptoSize);// Gets allocated only if nKeyCryptoSize >0
    // Key provided in Keyblock is encrypted if the nKeyCryptoSize is not zero
    if(nKeyCryptoSize != 0)
    {
        pKey = Auto_pKey.u8Ptr();
        uint nKeyDecryptedSize = 0;
        u8* pKeyDecrypted = nullptr;
        if (!GetKeySlot(nTKKeyType, key_slot))
        {
            LOGMSG("Decrypt: Failed to find key (%d) in key map!", nTKKeyType);
            return false;
        }
        bRes = DecryptBuffer(key_slot, pCGKey, nKeyCryptoSize, &pKeyDecrypted, nKeyDecryptedSize);

        if (bRes == false)
        {
            LOGMSG("ProgramKey: Decrypting the KeyBlock for Key %d returned error", nPKKeyType);
            return false;
        }
        memcpy(pKey, pKeyDecrypted, nKeyDecryptedSize);
        ReleaseSEBuff(&pKeyDecrypted);

    }else
    {
        pKey = pCGKey;
    }

    // Validate
    bRes = ValidateOpaque(nKeyType, nKeyVer, nKeyMode, nKCV32, pANBIO, pKey, nKeySize);
    if( false == bRes )
    {
        return false;
    }

    KeyInfoStructure KeyInfo;
    memcpy(KeyInfo.key_check_value, nKCV32, KCV_SIZE);
    KeyInfo.key_mode = (KeyModes)nKeyMode;
    KeyInfo.key_size = nKeySize;
    KeyInfo.key_ver  = nKeyVer;
    if (!GetKeySlot(nKeyType, KeyInfo.slot))
    {
        LOGMSG("ProgramKey: Failed to find key (%d) in key map!", nKeyType);
        return false;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Set_Key(KeyInfo, pKey))
    {
        LOGMSG("ProgramKey: Setting Key %d returned error", nKeyType);
        return false;
    }

    uint nTempOutCGSize;
    // Encrypt ANSOL with new key sent in
    bRes = AlignAndEncrypt(nKeyType, pANSOL, ANSOL_SIZE_HYB02, pOutCG, &nTempOutCGSize, NULL);
    //Note: check for bRes is done after revoking the key in case of zero Vec Key.
    //
    //Revoke the Key if its a zeroVecKey
    if (IsZeroVecKey(pKey, KeyInfo.key_size))
    {
        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(KeyInfo.slot))
        {
            ReleaseSEBuff(pOutCG);
            return false;
        }
    }

    //Check here if AlignAndEncrypt failed before
    if (false == bRes)
    {
        return false;
    }
    // we are here means everything went correct
    nOutCGSize = nTempOutCGSize;
    memset(pKey, 0, nKeySize);// zeroize
    return true;
}

/******************************************************************************/
/***    Program BSK which also sets BSK as active key     ***/
/******************************************************************************/
bool CryptoMgrHYB02::ProgramBSK(_V100_ENC_KEY_TYPE nTKKeyType, u8* pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize)
{
    _V100_ENC_KEY_TYPE nBSKKeyType = KT_EXT_BSK;
    // Program the key
    if(false  == ProgramKey(nTKKeyType, nBSKKeyType, pCGKey, nCGSize, pOutCG, nOutCGSize))
    {
        return false;
    }

    //Set BSK as active key
    u16 key_slot = 0;
    if (!GetKeySlot(KT_EXT_BSK, key_slot))
    {
        LOGMSG("ProgramBSK: Failed to find key (%d) in key map!", KT_EXT_BSK);
        return false;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Select_Key(key_slot))
    {
        LOGMSG("ProgramBSK:Selecting for KeySlot %d returned error", KT_EXT_BSK);
        return false;
    }
    m_nActiveKeyType = KT_EXT_BSK;

    return true;
}

/******************************************************************************/
//*** Pulls fields out of plain-text from KeyLoadBuffer ***//
//*** KeyBlock = [ ANBIO + ANSOL + Slot + Version + KeyMode + KCV + KeyCryptoSize + KeyVal + PAD ] ***//
/******************************************************************************/
bool CryptoMgrHYB02::DecodeKeyFromOpaque(u8* pKeyBlockClear, uint nKeyBlockSize, u16 *nSlot, u16 *nKeyVer, u16 *nKeyMode, u8 *pKCV32, u16 *nKeyCryptoSize, u8* pANBIO, u8* pANSOL, u8** pKey, u32& nKeySize)
{
    nKeySize = 0;

    if( nKeyBlockSize < ( ANBIO_SIZE_HYB02 + ANSOL_SIZE_HYB02 + sizeof(u16)*3 + KCV_SIZE_HYB02 ) )
    {
        LOGMSG("DecodeKeyFromOpaque:Input buffer size is not valid");
        return false;
    }
    if( (pKeyBlockClear == NULL) || (pKCV32 == NULL))
    {
        LOGMSG("DecodeKeyFromOpaque:Initial input argument validation failed");
        return false;
    }


    u8* pPtr = pKeyBlockClear;
    memcpy(pANBIO,         pPtr, ANBIO_SIZE_HYB02);  pPtr+=ANBIO_SIZE_HYB02;
    memcpy(pANSOL,         pPtr, ANSOL_SIZE_HYB02);  pPtr+=ANSOL_SIZE_HYB02;
    memcpy(nSlot,          pPtr, sizeof(u16));       pPtr+=sizeof(u16);
    memcpy(nKeyVer,        pPtr, sizeof(u16));       pPtr+=sizeof(u16);
    memcpy(nKeyMode,       pPtr, sizeof(u16));       pPtr+=sizeof(u16);
    memcpy(pKCV32,         pPtr, KCV_SIZE_HYB02);      pPtr+=KCV_SIZE_HYB02;
    memcpy(nKeyCryptoSize, pPtr, sizeof(u16));       pPtr+=sizeof(u16);

    nKeySize = GetKeySize(*nSlot, *nKeyMode);
    if (nKeySize == 0)
    {
        LOGMSG("DecodeKeyFromOpaque:Key %d is not supported");
        return false;
    }

    // Now we have the key size expected. check again if the input buffer size is correct
    if( nKeyBlockSize < ( ANBIO_SIZE_HYB02 + ANSOL_SIZE_HYB02 + sizeof(u16)*4 + KCV_SIZE_HYB02 + nKeySize) )
    {
        LOGMSG("DecodeKeyFromOpaque:Input buffer size is not valid");;
        return false;
    }

    // Make sure if the key is encrypted , encyrptedSize provided is atleast KeySize calculated from KeyMode
    if((*nKeyCryptoSize!=0) && (*nKeyCryptoSize < nKeySize))
    {
        LOGMSG("DecodeKeyFromOpaque:KeyCryptoSize provided in KeyBlock is not valid");
        return false;
    }


    //If key is encrypted allocate Key buffer of size nKeyCryptoSize otherwise nKeySize
    uint nKeyBufferSize = nKeySize;
    if(*nKeyCryptoSize != 0)
    {
        nKeyBufferSize = *nKeyCryptoSize;
    }

    *pKey = (u8*)MALLOC(nKeyBufferSize);
    if (NULL == *pKey)
    {
        LOGMSG("DecodeKeyFromOpaque:Failed to allocate memory.");
        return false;
    }

    memcpy(*pKey, pPtr, nKeyBufferSize);

    return true;
}

/******************************************************************************/
//*** Validates fields ***//
/******************************************************************************/
bool CryptoMgrHYB02::ValidateOpaque(u16 nSlot,u16 nKeyVer,u16 nKeyMode, u8* pKCV32,u8* pANBIO,u8* pKey, u32 nKeySize)
{
    u8    pCalculatedKCV[KCV_SIZE_HYB02];

    // Validate KCV - now Slot/KeyMode dependent.
    int nKCVType = GetKCVType(nSlot, nKeyMode);

    if(pKCV32 == NULL || pKey == NULL || nKeySize ==0 )
    {
        return false;
    }

    //Validate KeyMode -- Depends static key modes depends on slot
    if(false  == ValidateKeyMode(nSlot, nKeyMode))
    {
        return false;
    }

    if(nKCVType != KCV_NONE) // validate only if we support
    {
        if(false == CreateKCV( pKey, (uint)nKeySize, ZEROS_SIZE_HYB02, KCV_SIZE_HYB02, pCalculatedKCV, nKCVType))
        {
            return false;
        }
        // Validate KCV
        if( memcmp( pKCV32, pCalculatedKCV, KCV_SIZE_HYB02) != 0 )
        {
            return false;
        }
    }
    // Validate ANBIO
    if( memcmp( pANBIO, m_ANBIO, ANBIO_SIZE_HYB02) != 0 )
    {
        return false;
    }
    return true;

}

/******************************************************************************/
/***    Encrypt the input buffer with provided key     ***/
/******************************************************************************/
bool CryptoMgrHYB02::AlignAndEncrypt(u16 nKeyType, u8 *pIn, uint nInSize, u8 **pOutCG, uint* nOutCGSize, u256 pOutDigSig)
{

    if((NULL == pIn) || (0 == nInSize) )
    {
        LOGMSG("AlignAndEncrypt: Initial input arugment validation failed");
        return false;
    }

    // Alloc in Buffer with N_Block aligned and copy
    uint nInAligSz = nInSize;
    if( nInAligSz%BLOCK_SIZE_HYB02 )
    {
        int nAdj = BLOCK_SIZE_HYB02 - (nInAligSz%BLOCK_SIZE_HYB02);
        nInAligSz+=nAdj;
    }

    AutoHeapBuffer Auto_pInBuff(nInAligSz);
    u8* pInBuff = Auto_pInBuff.u8Ptr();
    if (NULL == pInBuff)
    {
        LOGMSG("AlignAndEncrypt:Failed to allocate memory.");
        return false;
    }
    memset(pInBuff, 0, nInAligSz);
    memcpy(pInBuff, pIn, nInSize);

    uint nOutSize = 0;
    u16 key_slot = 0;
    if (!GetKeySlot(nKeyType, key_slot))
    {
        LOGMSG("AlignAndEncrypt: Failed to find key (%d) in key map!", nKeyType);
        return false;
    }
    if (!EncryptBuffer(key_slot, pInBuff, nInAligSz, pOutCG, nOutSize))
    {
        ReleaseSEBuff(pOutCG);
        LOGMSG("AlignAndEncrypt:Failed to encrypt buffer.");
        return false;
    }

    // Cal digsig if requested
    if(pOutDigSig)
    {
        u8*  pHash       = nullptr;
        uint nHashLength = 0;

        if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Hash_Data(pInBuff, nInAligSz, HashAlgorithms::SHA2_256, &pHash, nHashLength))
        {
            ReleaseSEBuff(pOutCG);
            ReleaseSEBuff(&pHash);
            LOGMSG("AlignAndEncrypt: Compute hash returned error.");
            return false;
        }

        memcpy(pOutDigSig, pHash, sizeof(u256));
        ReleaseSEBuff(&pHash);
    }

    *nOutCGSize = nOutSize;
    return true;
}

// Helper method to invalidate BSK
int CryptoMgrHYB02::InvalidateBSK()
{
    u16 nSlot = 0;
    if (!GetKeySlot(KT_EXT_BSK, nSlot))
    {
        LOGMSG("InvalidateBSK: Couldn't find BSK in key map!");
        return CM_ERROR;
    }
    if (CryptExecStatus::Successful != SECURE_ELEMENT->Execute_Erase_Key(nSlot))
    {
        LOGMSG("InvalidateBSK:Removing Key returned error");
        return CM_ERROR;
    }
}

