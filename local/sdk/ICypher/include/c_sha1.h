/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_sha1.h 21029 2013-09-06 22:16:59Z spcorcoran $
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
#include "CypherTypes.h"

class oSHA1 : public ISHA1,public MemoryBase
{
    public:

                      oSHA1() {}
        virtual      ~oSHA1() {}
        virtual bool Init(void) { return true; }
        virtual bool Self_Test(bool verbose);
        virtual u32  Get_Revision(void);

        virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue);

    private:
        u8 hashval[20];
        sha1_context ctx;
};
