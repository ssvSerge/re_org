/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2013 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.lumidigm.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/******************************<%END LICENSE%>******************************/
#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

//#include <string>

#ifdef __GNUC__
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#endif

// per MSDN
#define MAX_PATH 260

typedef uint8_t     BYTE;
typedef uint8_t     UCHAR;
typedef uint8_t *   PUCHAR;
typedef uint16_t    WORD;
typedef uint16_t    USHORT;
typedef uint32_t    DWORD;
typedef uint64_t    QWORD;
typedef uint32_t    ULONG;
typedef uint32_t *  ULONG_PTR;

typedef int8_t      CHAR;
typedef int8_t *    PCHAR;
typedef int16_t     SHORT;
typedef int32_t     LONG;

typedef void *      PVOID;
typedef void *        LPVOID;

#ifdef __cplusplus
typedef bool        BOOL;
typedef wchar_t     WCHAR;
typedef wchar_t*    LPCWSTR;
#endif

#ifndef _VDSP
 #define TRUE  true
 #define FALSE false
#endif

// according to MSDN, the following typedef is valid
// for both 32-bit and 64-bit Windows OSs.
typedef void *      HANDLE;
#define INVALID_HANDLE_VALUE NULL
typedef HANDLE      HKEY;

#define WINAPI
#define _STDCALL
#define __stdcall

#define ZeroMemory(p,l) memset(p, 0, l);
#define sprintf_s sprintf


typedef int         errno_t;

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };

        PVOID Pointer;
    };

    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

inline int fopen_s(FILE ** pFile, const char * filename, const char * mode)
{
    *pFile = fopen(filename, mode);
    if (!(*pFile))
    #ifdef _VDSP
        return -1;
    #else
        return errno;
    #endif
    return 0;
}

#ifdef __GNUC__

inline uint32_t GetTickCount()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t u64mSJulian = tv.tv_sec * 1000;
    u64mSJulian += tv.tv_usec / 1000;
    return (uint32_t) (u64mSJulian & 0xFFFFFFFF);
}

inline void Sleep(uint32_t millis)
{
    usleep(millis * 1000);
}
#endif



inline void AssertNumericTypes()
{
    // Windows unsigned types
    assert(sizeof(BYTE)   == 1);
    assert(sizeof(UCHAR)  == 1);
    assert(sizeof(WORD)   == 2);
    assert(sizeof(USHORT) == 2);
    assert(sizeof(DWORD)  == 4);
    assert(sizeof(ULONG)  == 4);
    assert(sizeof(QWORD)  == 8);

    // Windows signed types
    assert(sizeof(CHAR)   == 1);
    assert(sizeof(SHORT)  == 2);
    assert(sizeof(LONG)   == 4);

    // Windows HANDLE
    assert(sizeof(void *) == sizeof(HANDLE));

    // Standard unsigned types
    assert(sizeof(uint8_t)  == 1);
    assert(sizeof(uint16_t) == 2);
    assert(sizeof(uint32_t) == 4);
    assert(sizeof(uint64_t) == 8);

    // Standard unsigned types
    assert(sizeof(int8_t)  == 1);
    assert(sizeof(int16_t) == 2);
    assert(sizeof(int32_t) == 4);
    assert(sizeof(int64_t) == 8);
}

