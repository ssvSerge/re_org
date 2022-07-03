#pragma once
#include <unordered_map>
#include "V100_enc_types.h"

#define  ANBIO_SIZE        ANBIO_LENGTH
#define  ANSOL_SIZE     ANSOL_LENGTH
#define  BLOCK_SIZE        BLOCK_LENGTH
#define  KCV_SIZE        KCV_LENGTH
#define  ZEROS_SIZE        32
#define     NUM_KEY_SLOTS_PROVISION    100
#define  KT_KEY_TMP    0xffff // make sure this value is not used in _V100_ENC_KEY_TYPE

#define  ANBIO_SIZE_CP001    8
