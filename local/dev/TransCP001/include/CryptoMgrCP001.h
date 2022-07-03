#pragma once

#include "IMemMgr.h"
#include "V100_enc_types.h"
#include "ICypher.h"
#include "CryptoMgr.h"
#include "CryptoMgrDefs.h"
#include "HBSEClient.h"

enum class CryptoMgrState {             //
    CM_STATE_UNKNOWN            = 0,    // Used to indicate error state.
    CM_STATE_DECOMMISIONED      = 1,    // Decommissioned state. All keys are deleted.
    CM_STATE_CTK                = 1,    // [CTK] installed.
    CM_STATE_BTK                = 2,    // [BTK] installed.
    CM_STATE_PROVISIONED        = 3,    //
    CM_STATE_END                = 4     //
};                                      //

enum class CryptoMgrRes {
    CM_ERROR_NONE               = 0,
    CM_ERROR_KEY_NOT_EXISTS     = 1,
    CM_ERROR_NOT_SUPPORTED      = 2,
    CM_ERROR_BAD_PARAM          = 3,
    CM_ERROR_BUSY               = 4,
    CM_ERROR_GENERAL            = 5
};



class CryptoMgrCP001 {

    public:
        CryptoMgrCP001();
        ~CryptoMgrCP001();

        CryptoMgrCP001(const CryptoMgrCP001&)               = delete;
        CryptoMgrCP001& operator=(const CryptoMgrCP001&)    = delete;

    public:
        static CryptoMgrCP001* GetInstance() {
            static CryptoMgrCP001 instance;
            return &instance;
        }

        // Initialize all Crypto-Engines.
        bool Init();


    public:
        CryptoMgrRes GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32);
        CryptoMgrRes GetRandomNumber(u256* pRnd);

    private:
        CryptoMgrRes GetKeyInfoInt(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32);
        CryptoMgrRes GetRandomNumberInt(u256* pRnd);

    private:
        CryptoMgrRes GetActiveSlot(u16 nKeyType, u16& nKeySlot);
        CryptoMgrRes PullKeyInfo(u16 nKeySlot, u16& nKeyVer, u16& nKeyMode, u8* pKCV32);
        CryptoMgrRes InvalidateDSK();
        CryptoMgrRes InvalidateBSK();


    private:
        u16                     m_ActiveSlot;
        bool                    m_MgrBusy;
        ISecureElement*         m_SecureClient;
        u256                    m_ANBIO;
        _V100_ENC_KEY_TYPE      m_nActiveKeyType;

#if 0

        // Returns the state of Crypto Manager.
        bool GetState(CryptoMgrState& state);

        // Chapter 4.5 "Customer decommissioning of a device"
        bool TamperDetected();

        // Lock Factory Mode
        bool LockFactoryMode();

        // Generate Session Key [SK]BTK
        bool GenerateSessionKey();

        // Get Key
        bool GetKey(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize);

        // Factory Key Load mode
        int  InitiateFactoryKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCG, int nCGSize);

        // Remote Key Load mode
        int  InitiateRemoteKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCG, int nCGSize);

        // Set a key as active for biometric operations
        int  SetActiveKey(_V100_ENC_KEY_TYPE nKeyType, bool bTCInc);

        // Encrypt with Active Key
        bool Encrypt(u8* pIn, uint nInSize, u8** pOutCG, uint* nOutCGSize, u256 pOutDigSig);

        // Decrypt SpoofCG with Active Key and validate ANBIO
        bool DecryptAndValidateSPLCG(u8* pIn, u8** pOut, uint nSizeIn, uint* nSizeOut);

        void ReleaseSEBuff(u8** pIn);
        int  GetKCVType(u16 nKeyType, u16 nKeyMode);
        u32  GetKeySize(u16 nKeyType, u16 nKeyMode);
        bool IsZeroVecKey(u8* pKey, u32 nKeySize);
        bool CreateKCV(u8* pKey, uint nKeySize, uint nZeros, uint nVals, u8* pKCV, int nKCVType);
        bool DecryptBuffer(u16 nKeySlot, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize);
        bool EncryptBuffer(u16 nKeySlot, u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize);
        bool IsKeyLoaded(u16 nKeySlot);
        int  GetKCVTypeForVariableMode(u16 nKeyMode);
        u32  GetKeySizeForVariableMode(u16 nKeyMode);
        void ResetActiveKey();
        bool ProgramBSK(u8* pCGKey, u16 nKeySize);
        bool ProgramKey(_V100_ENC_KEY_TYPE nTKKeyType, _V100_ENC_KEY_TYPE nKeyType, u8* pCG, uint nCGSize);
        bool DecodeKeyFromOpaque(u8* pPlainText, int nPlainTextSize, u16 *nKeyType, u16 *nKeyVersion, u8 *pKCV, u8* pANBIO, u8** pKey, u16& nKeySize);
        bool ValidateOpaque(u16 nKeyType, u16 nKeyVersion, u8* pKCV, u8* pANBIO, u8* pKey, u16 nKeySize);
        bool AlignAndEncrypt(u8 *pIn, uint nInSize, u8 **pOutCG, uint* nOutCGSize, u256 pOutDigSig);
        bool ParseASNFromBuffer( u8 *pKey, u16 nKeySize, u8 *pExp, u16 nExpSize, unsigned char *pASNBuff, u16 nASNSz);
        bool GetASNSize(unsigned char* pASNBuff, u16 &nASNSz);
        bool Decrypt(u8* pInCG, uint nInCGSize, u8** pOut, uint* nOutSize, u256 pInDigSig);
        int  SelectKeyOnSE(_V100_ENC_KEY_TYPE nKeyType);
#endif

};

