#pragma once
#include "CryptoMgr.h"
#include "IMemMgr.h"
#include "V100_enc_types.h"

/////////////////////////////////////////////////////////////////
//
//  Customer provisioning CryptoMgr( Mainly used to load/remove CTK on/from to device )
//
//
/////////////////////////////////////////////////////////////////

class CryptoMgrProvision: public CryptoMgr
{
public:

    bool Init();

    // Generates Random Number and sets ANBIO
    bool GetRandomNumber(u256* pRnd);
    // Gets ANBIO(last random number generated)
    bool GetANBIO(u256* pANBIO);

    //Generate Device asymmetrickeys
    bool GenerateAsymmetricKeys();

    bool GetKey(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32, u2048 pKey, u32& nKeySize);
    // This is a static call since ENC_GET_KEY_VERSION is supported in TransBase that means its supported in all config.
    // So it needs to be static as CmdExecutiveBase needs this call.
    static bool GetKeyInfo(_V100_ENC_KEY_TYPE nKeyType,u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV32);
    bool SetKey(_V100_ENC_KEY_TYPE nKeyType, u8 *pCG, uint nCGSize, u8** pOutCG, uint& nOutCGSize);

    //CTK
    bool Decommission(u8* pInCG, uint nInCGSize);

    static CryptoMgrProvision& GetInstance()
    {
        static CryptoMgrProvision instance;
        return instance;
    }

    CryptoMgrProvision(const CryptoMgrProvision&) = delete;
    CryptoMgrProvision& operator=(const CryptoMgrProvision&) = delete;

private:
    bool PKIDecryptWithDevicePrivKey( u8 *pCipherText, u8 *pPlainText, u32 nOutMaxBytes, u32* nOutBytes);
    bool Encrypt(u16 nKeyType, u8 *pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize);
    bool Decrypt(u16 nKeyType, u8 *pInCG, uint nInCGSize, u8 **pOut,   uint& nOutSize);
    bool DecodeKeyFromOpaque(u8* pPlainText, uint nPlainTextSize, u16* nKeyType, u16* nVerNum, u16* nKeyMode, u8* pKCV32, u16* nKeyCryptoSize, u8* pANBIO, u8* pANSOL, u8** pKey, u32& nKeySize);
    bool ValidateOpaque(u16 nKeyType, u16 nVerNum, u16 nKeyMode, u8* pKCV32, u8* pANBIO, u8* pKey, u32 nKeySize);
    u256 m_ANBIO;



    CryptoMgrProvision();
    ~CryptoMgrProvision();
};

