/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_rand.cpp 22716 2014-02-06 23:53:54Z spcorcoran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
**    FIPS MODE
**        SP 800- 90A  CTR_DRBG
**         key length 256b
**        seed length 384b
*******************************************************************************/
/*
**    Includes
*/
#include <cassert>
#include "AutoHeapBuffer.h"
#include "ICypher.h"
#include "lumi_random.h"
#include "c_rand.h"

/******************************************************************************
** Built-In Test
******************************************************************************/
bool oRAND::Self_Test( bool verbose )
{
    return (ctr_drbg_self_test(verbose) == 0);
}


