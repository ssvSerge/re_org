/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_crypto.cpp 20311 2013-07-25 13:58:06Z spcorcoran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
#pragma once
#include "IMemMgr.h"
#include "CypherTypes.h" //Sang:CryptoCert
#include "polarssl/sha512.h"

class oSHA512 : public ISHA512,  public MemoryBase
{

public:
                  oSHA512() {}
    virtual      ~oSHA512() {}
    virtual bool Init(void) { return true; }
    virtual bool Self_Test(bool verbose);
    virtual u32  Get_Revision(void);

    virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue, int is384 = false);
    virtual bool HMAC_Hash(const u8 *pKey, int nKeySize, const u8 *pMsg, int nMsgSize, u8 *HashValue, int is384 = false);

private:
    u8 hashval[64];
    sha512_context ctx;
};
