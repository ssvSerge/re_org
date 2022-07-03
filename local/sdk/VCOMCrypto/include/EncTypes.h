#pragma once
#ifndef _CFG_SPEC_ENCTYPES
#define _CFG_SPEC_ENCTYPES
#include "V100_enc_types.h"
#include "V100_shared_types.h"

typedef enum
{
    RAW_MODE = 0x00,        // Records will consist of just the biometric data as specified by the data type without
                            // wrapping such data in any specified structure (i.e. biometric template, WSQ compressed
                            // image, etc.)
    BIR_MODE = 0x01    ,        // Records will consist of the biometric data populated in a _V100_ENC_BIR structure
} _V100_ENC_RECORD_MODE;


#endif