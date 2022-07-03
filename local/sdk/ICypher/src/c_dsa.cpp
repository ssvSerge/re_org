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


typedef struct
{
    u32
        p,            // Prime Modulus
        q,            // Prime Divisor
        g,            // Generator
        seedlength,    // length in bits
        counter;
} DSA_Domain_Parameters_Type;


/******************************************************************************
** Initialization
******************************************************************************/
bool oDSA::Init( void )
{
    return true;
}//end

/******************************************************************************
**    Generate Finite Field Cryptograpy (FFC) Domain Parameters
**    SHALL use APPROVED HASH Function (SHA256 ok,  SHA-1 NOT APPROVED)
** L Desired length of Prime p in Bits
** N Desired length of Prime q in Bits
******************************************************************************/

#define OUTLEN (32 * 8 ) // bit length of HASH function

bool oDSA::GenerateDomainParameters( int L, int N )
{

    DSA_Domain_Parameters_Type DP = {0,0,0,256,0};
    u256 Domain_Parameter_Seed, tmp;

    /*
    **    A.1.1.2 Generate Primes p and q
    */
    switch ( L )
    {
        case 1024:                // Approved Sizes
            if ( N != 160 )
                return false;
            break;
        case 2048:
            if (!( (N == 224 || N == 256) ))
                return false;
            break;
        case 3072:
            if ( N != 256 )
                return false;
            break;
        default:
            return false;
    }

    if ( DP.seedlength < (u32)N )
        return false;

    //u32 n = (u32) ceil( (double)L / (double)OUTLEN ) - 1;
    //u32 b = L -1 - (n * OUTLEN);

    // Get Random Number
    if (!Rand( Domain_Parameter_Seed, sizeof(u256) ) )
        return false;

    if (!Hash( Domain_Parameter_Seed, sizeof(u256), tmp ) )
        return false;

//    u32 U = tmp % (u32)pow(2.0,(double)N-1.0);

    return true;

}

/******************************************************************************
**    Hash Function Support
******************************************************************************/
bool oDSA::Hash( u8 *Msg, int Nbytes, u8 *hashval)
{
//    if( !oSHA256::Hash( Msg, Nbytes, hashval ) )
//        return false;
    return true;
}//end

/******************************************************************************
** Random Number Support
******************************************************************************/
bool oDSA::Rand( u8 *rNum, int Nbytes)
{
//    IATSHA* pATSHA  = ICypher::GetInstance()->GetATSHA();
//    if( !pATSHA->GetRandomNumber( rNum, RANDOM_NO_UPDATE_SEED ) )
//        return false;
    return true;
}//end

/******************************************************************************
**
******************************************************************************/
oDSA::oDSA()
{
}//end

/******************************************************************************
**
******************************************************************************/
oDSA::~oDSA()
{
}//end



