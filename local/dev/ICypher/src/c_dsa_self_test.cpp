/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_dsa.cpp 20311 2013-07-25 13:58:06Z spcorcoran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
/*
**    Includes
*/

#include "ICypher.h"
#include "c_dsa.h"
#include <iostream>
#include <fstream>
#include <math.h>

/******************************************************************************
** Self Test
******************************************************************************/
bool oDSA::Self_Test( void )
{
    if (!GenerateDomainParameters( 2048, 256 ) )
        return false;

    return true;
}//end
