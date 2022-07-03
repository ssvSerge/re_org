#pragma once
#include "lumi_mem_mgr.h"
#if defined(WIN32) || defined(__GNUC__)
#define LUMI_USE_POLARSSL_AES_ECB
#endif
#include "polarssl/config.h"
