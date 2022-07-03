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

class oDUKPT : public IDUKPT, public MemoryBase
{

public:

    virtual bool Init(void);
    virtual bool Self_Test(bool verbose);
    virtual u32  Get_Revision(void);
    virtual    bool Encrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize);
    virtual bool Decrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize); // debug ONLY, HOST SIMULATION
    virtual bool Decrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize, const u8 *Key, int KeyLength); // debug ONLY, HOST SIMULATION
    //virtual bool SetContext(int WhichOne, bool bTCInc);
    virtual bool SetContext(u8* IPEK, u8* KSN, u32 cnt);
    virtual bool Update_KSN(u32 cnt);

private:


    void ClearContext();
    bool Derive_SK(KSNType *pKSN, u128 *pIPEK);
    bool Derive_IPEK(u128 *pIPEK);
    bool cipher(const u8 *pIn, u8 *pOut, const u8 *pKey, size_t nBytes, size_t nKeySize);
    bool decipher(const u8 *pIn, u8 *pOut, const u8 *pKey, size_t nBytes, size_t nKeySize);
    //bool print( u8 *pPtr, size_t nBytes, EndianType );
    void swap_bytes(const u8 *pIn, u8 *pOut, size_t nBytes);
    bool GenLRC(u8 *pIn, size_t nBytes, u8 *pLRC);
    bool ToLittleEndian(const u8 *pIn, u8 *pOut, size_t nBytes);

    //SymKeyFormatType    m_iSKey;

    KSNType
        m_dftKSN,        // 8 MSB's from vendor
        m_iKSN;        // Current KSN

    u128
        m_tmp,
        m_iBDK,        // Dummy, Not Real Base Derivation Key, used for test
        m_iIPEK,        // Loaded IPEK
        m_iMAC,
        m_iDATA,
        m_iSK;        // Current Session Key

public:
    oDUKPT();
    virtual ~oDUKPT();

};
