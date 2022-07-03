
/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: c_ecdsa.cpp 28352 2016-02-19 03:38:32Z sillendu $
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
#include "CypherTypes.h"
#include "c_ecdsa.h"


/******************************************************************************
** Built-In Test
******************************************************************************/
bool oECDSA::Self_Test(bool verbose)
{
    if (ecdsa_self_test(verbose))
        return false;

    return true;
}
