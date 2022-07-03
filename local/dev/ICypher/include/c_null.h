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
#include "ICypher.h"
#include "CypherTypes.h"


class oNULL : public INULL, public MemoryBase
{
    virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode);
    virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode);
};
