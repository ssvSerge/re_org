#pragma once
#include "CryptoMgr.h"
#include "CryptoMgrDefs.h"
#include "IMemMgr.h"
#include "V100_enc_types.h"


#define  ANBIO_SIZE_HYB02    ANBIO_LENGTH
#define  ANSOL_SIZE_HYB02     ANSOL_LENGTH
#define  BLOCK_SIZE_HYB02    BLOCK_LENGTH
#define  KCV_SIZE_HYB02        KCV_LENGTH
#define     ZEROS_SIZE_HYB02    32


/////////////////////////////////////////////////////////////////
//
//  Serves as the interface to ISecureElement.
//  Abstracts all encrypt/decrypt, keyloading etc..
//  To be used by the CmdExecutive
//
//
/////////////////////////////////////////////////////////////////


#define CM_ERROR_NOT_SUPPORTED  -2
#define CM_OK                    0
#define CM_ERROR                -1


class CryptoMgrHYB02 : public CryptoMgr
{
public:
    // Initialize all Crypto-Engines.
    bool Init();

    // Generates Random Number and sets ANBIO
    bool    GetRandomNumber(u256* pRnd);
    // Gets ANBIO(last random number generated)
    bool     GetANBIO(u256* pANBIO);
    // Query as to what state we are in. FKL locked or not
    bool     IsFactoryKeyLoadingLocked();
    // Lock Factory Mode
    bool    LockFactoryMode();
    // Unlock
    bool     UnlockFactoryMode();
    // Get Key
    bool      GetKey(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize);
    // Get Key info ver, KCV
    bool     GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType,u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32 );
    // Factory Key Load mode
    int     InitiateFactoryKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize);
    // Remote Key Load mode
    int     InitiateRemoteKeyLoad(_V100_ENC_KEY_TYPE nKeyType, u8*  pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize);
    // Set a key as active for biometric operations
    int SetActiveKey(_V100_ENC_KEY_TYPE nKeyType, bool bTCInc);
    // Encrypt with Active Key
    bool Encrypt( u8 *pIn, uint nInSize, u8 **pOutCG, uint* nOutCGSize, u256 pOutDigSig);
    // Decrypt with Active Key
    bool Decrypt( u8 *pInCG, uint nInCGSize, u8 **pOut, uint &nOutSize, u256 pInDigSig);

    bool ReleaseMemBuff(u8* pIn);

protected:
    // MEMBERS
    void ResetActiveKey();
    // Program all symmetric keys.        - [pCGKey]CTK/BTK . CTK in FKL, BTK in RKL
    bool ProgramKey(_V100_ENC_KEY_TYPE nTKKeyType, _V100_ENC_KEY_TYPE nPKKeyType, u8* pCGKey, uint nCGSize,u8** pOutCG, uint& nOutCGSize);
    // Program BSK  - [pCGKey]BTK. Only done in RKL
    bool ProgramBSK(_V100_ENC_KEY_TYPE nTKKeyType, u8* pCGKey, uint nCGSize, u8** pOutCG, uint& nOutCGSize);
    // Pulls fields out of plain-text
    bool DecodeKeyFromOpaque(u8* pPlainText, uint nPlainTextSize, u16 *nSlot, u16 *nKeyVer, u16 *nKeyMode, u8 *pKCV32, u16 *nKeyCryptoSize, u8* pANBIO, u8* pANSOL, u8** pKey, u32& nKeySize);
    // Validates fields
    bool ValidateOpaque(u16 nSlot,u16 nKeyVer, u16 nKeyMode, u8* pKCV32,u8* pANBIO,u8* pKey, u32 nKeySize);
    // Encrypt with given key
    //bool AlignAndEncrypt( ICRYPTOAlgo* pCryptoAlgo, u8* pKey, uint nKeySize, u16 nKeyType, int nAlgoMode, u8 *pIn, uint nInSize, u8 **pOutCG,   uint* nOutCGSize, u256 pOutDigSig);
    bool AlignAndEncrypt(u16 nKeyType, u8* pIn, uint nInSize, u8** pOutCG, uint* nOutCGSize, u256 pOutDigSig);
    int InvalidateBSK();


    u256     m_ANBIO;

    //// What key-type is being used currently?
    _V100_ENC_KEY_TYPE m_nActiveKeyType;

public:
    static CryptoMgrHYB02* GetInstance()
    {
        static CryptoMgrHYB02 instance;
        return &instance;
    }
    CryptoMgrHYB02();
    ~CryptoMgrHYB02();
};

