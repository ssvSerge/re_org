/***************************************************************************************/
// ©Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/
#ifndef _lumi_stdint_h_
#define _lumi_stdint_h_

// INTEGER TYPE AND RANGE DEFINITIONS FOR VARIOUS OSs/COMPILERs

#if defined(_WIN32)
    // 32-bit Windows
    #if (_MSC_VER >= 1600) //VS2010 and up include stdint.h
        #include <stdint.h>
    #else
        typedef char             int8_t;
        typedef short            int16_t;
        typedef int              int32_t;
        typedef __int64          int64_t;
        typedef unsigned char    uint8_t;
        typedef unsigned short   uint16_t;
        typedef unsigned int     uint32_t;
        typedef unsigned __int64 uint64_t;
        #define INT8_MAX   ((int8_t)   0x7F)
        #define INT16_MAX  ((int16_t)  0x7FFF)
        #define INT32_MAX  ((int32_t)  0x7FFFFFFF)
        #define INT64_MAX  ((int64_t)  0x7FFFFFFFFFFFFFFFLL)
        #define INT8_MIN   ((int8_t)   0x80)
        #define INT16_MIN  ((int16_t)  0x8000)
        #define INT32_MIN  ((int32_t)  0x80000000)
        #define INT64_MIN  ((int64_t)  0x8000000000000000LL)
        #define UINT8_MAX  ((uint8_t)  0xFF)
        #define UINT16_MAX ((uint16_t) 0xFFFF)
        #define UINT32_MAX ((uint32_t) 0xFFFFFFFF)
        #define UINT64_MAX ((uint64_t) 0xFFFFFFFFFFFFFFFFLLU)
    #endif
	#include <basetsd.h>
	typedef SSIZE_T ssize_t;
#elif defined(__GNUC__)
    // we do not define *INT*_MAX and *INT*MIN because we use gcc CL param -D__STDC_LIMIT_MACROS
    // GNU C, 32 or 64 bit
    #include <stdint.h>
    #if defined(__i386__)
        // 32-bit GNU C
    #elif defined(__x86_64__)
        // 64-bit GNU C
    #elif defined(__arm__)
        // 32-bit arm
    #elif defined (__aarch64__)
        // 64-bit arm
    #else
        #error "Unknown GNU platform"
    #endif
#elif defined(__ADSPBLACKFIN__)
    #include <stdint.h>
	#include <stdlib.h>
    typedef int32_t ssize_t;
#elif defined(FX2_H)
    // Cypress FX2 8051
    typedef char             int8_t;
    typedef short            int16_t;
    typedef int              int32_t;
    typedef unsigned char    uint8_t;
    typedef unsigned short   uint16_t;
    typedef unsigned long    uint32_t;
    #define INT8_MAX   ((int8_t)  0x7F)
    #define INT16_MAX  ((int16_t) 0x7FFF)
    #define INT32_MAX  ((int32_t) 0x7FFFFFFF)
    #define INT8_MIN   ((int8_t)  0x80)
    #define INT16_MIN  ((int16_t) 0x8000)
    #define INT32_MIN  ((int32_t) 0x80000000)
    #define UINT8_MAX  ((uint8_t)  0xFF)
    #define UINT16_MAX ((uint16_t) 0xFFFF)
    #define UINT32_MAX ((uint32_t) 0xFFFFFFFF)
#endif

#define __STDC_FORMAT_MACROS
#include <lumi_inttypes.h>

#ifdef _VDSP
	#include <services_types.h>
#else
	typedef uint32_t		u32;
#endif

typedef uint8_t			u8;
typedef uint16_t        u16;
#ifndef FX2_H
typedef uint64_t 		u64;
#endif
typedef uint8_t			uchar;
typedef uint16_t        ushort;
typedef uint32_t        uint;

#ifndef __KEIL_C51__          //exclude this type for Keil compiler for now - NN
    typedef uint64_t     u64;
#endif

typedef u8 u128 [ 16 ];  // 128-bits data buffer (16 bytes)
typedef u8 u256 [ 32 ];  // 256-bits data buffer (32 bytes)
	typedef u8 u192  [  24 ];		// Fundamental Key Type
	typedef u8 u512  [  64 ];		// 64 Byte Type
	typedef u8 u1024 [ 128 ];
typedef u8 u2048[ 256 ]; // 2048-bits data buffer (256 bytes)
	typedef u8 u4096 [ 512 ];
	typedef u8 u64B  [   8 ];		// Byte Access to SN
typedef u8 u160[20];
#endif	// _lumi_stdint_h_
