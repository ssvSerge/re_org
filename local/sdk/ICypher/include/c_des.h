//
// THIS FILE IS FROZEN FOR FIPS VALIDATION!
// NO CHANGES ALLOWED.
//

/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_des.h 22612 2014-01-25 14:18:56Z spcorcoran $
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


class oDES : public IDES,  public MemoryBase
{
    public:

                      oDES() {}
        virtual      ~oDES() {}
        virtual bool Init( void );
        virtual bool Self_Test(bool verbose);
        virtual u32  Get_Revision(void);

        virtual bool Encrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode  );
        virtual bool Decrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode  );

    private:

};

