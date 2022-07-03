#pragma once
#include "CryptoMgr.h"
#include "IMemMgr.h"
#include "IMsgListener.h"
#include "V100_enc_types.h"
#include "CryptoMgrDefs.h"
#include "ISecureElement.h"


/////////////////////////////////////////////////////////////////
//
//  Serves as the interface to ICypher.
//  Abstracts all encrypt/decrypt, etc.. calls
//  To be used by the CmdExecutive
//
//
/////////////////////////////////////////////////////////////////

#define  DEVID_SIZE     16                // Device ID Length in Bytes

//class IKeyMgrMSK00;
class CryptoMgrMSK00 : public CryptoMgr
{
public:
    // Initialize all Crypto-Engines.
    bool Init();
    // Get Random Number
    bool    GetRandomNumber(u8* pRnd, int nSize);
    // Set ANBIO
    bool    SetANBIO(u128* pANBIO);
    // Get ANBIO
    bool    GetANBIO(u128* pANBIO);
    // Set ANSOL
    bool    SetANSOL(u128* pANSOL);
    // Get ANSOL
    bool    GetANSOL(u128* pANSOL);
    // Perform Self-diagnostic
    bool    PerformSelfDiagnostic(bool bVerbose);

    bool Encrypt(u16 KeyType, u8 *pIn, uint nInSize, u8 **pOutCG, uint& nOutCGSize);
    bool EncryptBioData(u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize, u256 pOutDigSig);
    bool Decrypt(u16 KeyType, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize);

    bool CreateDigSig(u8* pIn, uint nInSize, u256 pOutDigSig);
    bool ValidateDigSig(u8* pIn, uint nInSize, u256 pInDigSig);
    bool GetDeviceID( char pDevIDBuff[DEVID_SIZE]);

    bool SetSK( u8 *pCG, uint nCGSize, u8* pANSOL, int nANSOLSize );
    bool GetSKCV(u8* pKCV, uint nKCVSize);
    bool SetMKD(u8* pCG, uint nCGSize, u256 pDigSig);
    bool UpdateKeyState();
    bool GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, u16& nKeyMode, u8* pKCV32);

    bool IsMKDLoaded();



    static CryptoMgrMSK00* GetInstance()
    {
        static CryptoMgrMSK00 instance;
        return &instance;
    }

    CryptoMgrMSK00(const CryptoMgrMSK00&) = delete;
    CryptoMgrMSK00& operator=(const CryptoMgrMSK00&) = delete;


private:

    u128 m_ANBIO;
    u128 m_ANSOL;

public:
    CryptoMgrMSK00();
    ~CryptoMgrMSK00();
};

