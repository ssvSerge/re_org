/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id$
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        Lumidigm Inc Copyright 2014 All Rights Reserved.
**
*******************************************************************************/
#pragma once
#include "ICypher.h"
#include "IMemMgr.h"

class oKEYMAN : public IKEYMAN, public MemoryBase
{
    public:

        virtual bool Init( void );
        virtual bool Self_Test( void );

        virtual bool PKI_Cipher_Device_PubKey( u8 *pPlainText, u8 *pCipherText, u32 nBytes );
        virtual bool PKI_Decipher_Device_PrivKey( u8 *pCipherText, u8 *pPlainText, u32 nOutMaxBytes, u32* nOutBytes);
        virtual bool KeyGen( u8 *pSK, int nSKSize, u8 *pCG, int nCGSize );
        virtual bool Check_Host_DigSig(u8* pPlainText, u32 nBytes, u2048 pInDigSig, u8* pKey, u32 nKeySize, u32 Exponent);
        virtual bool Create_Device_DigSig(u8* pPlainText, u32 nBytes, u2048 pOutDigSig);
        virtual bool Self_Provision_Device( void );

    private:
        bool AsymPackKey( AsymKeyFormatType *pIn, u16 Version, u2048 *pKey );
        bool SymPackKey(  SymKeyFormatType  *pIn, u16 Version, u256 *pKey, EXTKeyNameType nSlotName );

        AsymKeyFormatType tAKey;
        AsymKeyFormatType iAKey;
        SymKeyFormatType  iSKey;

        u32
            m_Exponent;

        u32
            eTime[8],
            sTime;

        u256
            m_hbuf,
            tSKold,
            tSK;

        u1024
            tP,
            tQ,
            tT;

        u2048
            m_PlainText,
            m_Cryptogram,
            tN,
            tD;

    public:
        oKEYMAN();
        ~oKEYMAN();
};



