/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: IBSP.h 22877 2014-03-03 03:38:32Z spcorcoran $
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
#include "CypherTypes.h"

class ICryptoBSP
{
    public:
        virtual ~ICryptoBSP() {};
        // Instance Access
        //virtual IMAXQ     *GetMAXQ()   = 0;
        //virtual IATSHA     *GetATSHA()  = 0;

        virtual u32  CRC32_Calc(u8 *buffer, u32 size, u32 seed )=0;
        virtual void CRC32_CreateTable( void )=0;
        virtual u32  CRC32_Reflect(u32 ref, char ch)=0;

        virtual u32 GetErrorCode(void)=0;
        /*
        **    Static Singleton
        */
        static ICryptoBSP* GetInstance();
};

