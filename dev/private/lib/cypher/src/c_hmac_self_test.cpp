/*******************************************************************************
**
**  V300b Updated Board Support Package
**  $Id: v300b_hmac.cpp 22877 2014-03-03 03:38:32Z spcorcoran $
**
**  COPYRIGHT INFORMATION:  
**      This software is proprietary and confidential.  
**      By using this software you agree to the terms and conditions of the 
**      associated Lumidigm Inc. License Agreement.
**
**      (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
**  Keyed-Hash Message Authentication Code (HMAC)
**  FIPS PUB 198, 198-1
*******************************************************************************/
/*
**  Includes
*/

#include "ICypher.h"
#include "Auto_Crypto_Objects.h"

#include <iostream>
#include <fstream>

#include "c_hmac.h"
#include "c_hmac_tvec.h"  // Test Vectors   

/******************************************************************************
** built-in Self Test Using FIPS Test Vectors
******************************************************************************/

/*
TEMP for checking HMAC generation on MAXQ
static u256 testKey = { 
    0xee, 0x0a, 0x81, 0xa8, 0xbd, 0x52, 0xc9, 0xb1, 0x42, 0x20, 0x83, 0x52, 0x2d, 0x37, 0xf8, 0x07,
    0x18, 0x96, 0xba, 0x62, 0x5f, 0xfa, 0x22, 0xad, 0x32, 0xa4, 0xfd, 0xd1, 0xe8, 0x5c, 0x83, 0x77 
};

static u8 testMsg[] = {
    0x0c, 0x24, 0x5d, 0xe3, 0xb2, 0x50, 0xc3, 0x32, 0x82, 0xea, 0x1a, 0x02, 0xd0, 0x07, 0xf0, 0x3b, 
    0x34, 0xed, 0x42, 0x76, 0x31, 0x28, 0x3e, 0xb6, 0x14, 0xdb, 0x4d, 0x52, 0x1f, 0x55, 0x51, 0x36, 
    0xe7, 0xe4, 0x2b, 0x4c, 0xfb, 0xee, 0x81, 0x34, 0xc6, 0x3d, 0xbe, 0x3b, 0xb7, 0x9b, 0x5a, 0x8b, 
    0x9f, 0x9f, 0x5b, 0x9f, 0x5a, 0xc6, 0x1c, 0xfa, 0xb1, 0xc5, 0x4d, 0x19, 0x7f, 0x1e, 0x3b, 0xa6, 
};
*/

bool oHMAC::Self_Test( bool verbose )
{
    int MACSize;
    AutoHeapBuffer Auto_MAC(32);
    u8 *MAC = Auto_MAC.u8Ptr();
    if (MAC == NULL)
        return false;
    
#if 0
    // check HMAC gen on MAXQ   
    if(!HMAC( testMsg, sizeof(testMsg), testKey, sizeof(testKey), MAC, &MACSize, SHA256_MODE) )
        return false;
#endif  
    
    /*
    ** KAT1 SHA-1 with 64 Byte Key
    */
    MACSize = sizeof(KAT1_MAC);
    if(!HMAC( KAT1_MSG, sizeof(KAT1_MSG), KAT1_KEY, sizeof(KAT1_KEY), MAC, &MACSize, SHA1_MODE) )
        return false;
    
    if( memcmp( KAT1_MAC, MAC, MACSize) != 0 )
        return false;
        
    /*
    ** KAT2 SHA-1 with 20 Byte Key
    */
    MACSize = sizeof(KAT2_MAC);
    if(!HMAC( KAT2_MSG, sizeof(KAT2_MSG), KAT2_KEY, sizeof(KAT2_KEY), MAC, &MACSize, SHA1_MODE) )
        return false;
    
    if( memcmp( KAT2_MAC, MAC, MACSize) != 0 )
        return false;
        
    /*
    ** A.3 SHA-1 with 100 Byte Key
    */
    MACSize = sizeof(KAT3_MAC);
    if(!HMAC( KAT3_MSG, sizeof(KAT3_MSG), KAT3_KEY, sizeof(KAT3_KEY), MAC, &MACSize, SHA1_MODE) )
        return false;
            
    if( memcmp( KAT3_MAC, MAC, MACSize) != 0 )
        return false;

    /*
    ** KAT4 SHA-1 with 49 Byte Key
    ** ONLY check leftmost bytes
    */
    MACSize = sizeof(KAT4_MAC);
    if(!HMAC( KAT4_MSG, sizeof(KAT4_MSG), KAT4_KEY, sizeof(KAT4_KEY), MAC, &MACSize, SHA1_MODE) )
        return false;
            
    if( memcmp( KAT4_MAC, MAC, sizeof(KAT4_MAC)) != 0 )
        return false;
        
    /*
    ** Example #5 SHA-256 with 40 Byte Key
    ** ONLY check leftmost bytes
    */
    MACSize = sizeof(KAT5_MAC);
    if(!HMAC( KAT5_MSG, sizeof(KAT5_MSG), KAT5_KEY, sizeof(KAT5_KEY), MAC, &MACSize, SHA256_MODE) )
        return false;
            
    if( memcmp( KAT5_MAC, MAC, sizeof(KAT5_MAC)) != 0 )
        return false;
        
    /*
    ** Example #5 SHA-256 with 40 Byte Key, 24 Byte MAC
    ** ONLY check leftmost bytes
    */
    MACSize = sizeof(KAT6_KEY);
    if(!HMAC( KAT6_MSG, sizeof(KAT6_MSG), KAT6_KEY, sizeof(KAT6_KEY), MAC, &MACSize, SHA256_MODE) )
        return false;
            
    if( memcmp( KAT6_MAC, MAC, sizeof(KAT6_MAC)) != 0 )
        return false;
        
        
    /*
    **  Test Entity Authenticate
    */
#if 0 // Sang: Disabled for now as this function accesses Hardware
    if (!HMAC_Authentication( (u256*)KAT1_HMAC, (u256*)KAT1_KEY, (u256*)&KAT1_KEY[32], KAT5_MSG, sizeof(KAT5_MSG)) )
        return false;
#endif
        
    return true;
}//end
