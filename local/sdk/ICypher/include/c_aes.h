/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_aes.h 21029 2013-09-06 22:16:59Z spcorcoran $
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


class oAES : public IAES, public MemoryBase
{
    public:

                      oAES() {}
        virtual      ~oAES() {}
        virtual bool Init(void);
        virtual bool Self_Test(bool verbose);
        virtual u32  Get_Revision(void);

        virtual bool Encrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode);
        virtual bool Decrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode);

    private:

        u256
            KAT_Tmp,
            KAT_Key,
            KAT_IV,
            KAT_PT;
};
