/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_hmac.h 22877 2014-03-03 03:38:32Z spcorcoran $
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



class oHMAC : public IHMAC, public MemoryBase
{
    public:

                      oHMAC() {}
        virtual      ~oHMAC() {}
        virtual bool Init( void );
        virtual bool Self_Test(bool verbose );
        virtual u32  Get_Revision(void);

        virtual bool HMAC( u8 *Msg, int MsgSize, u8 *Key, int KeySize, u8 *MAC, int *MACSize, int Mode);
        //virtual bool HMAC_Authentication( u256 *pHMAC, u256 *pHostRNC, u256 *pSensorRNC, void *pOtherData, size_t nOtherSize);
        //virtual bool HMAC_Authentication_Generic( u256 *pHMAC, void *pOtherData, size_t nOtherSize);

    private:
};


