/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2008 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
//
//
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

#if defined(_WIN32) && defined(_DEBUG)
#    ifndef _CRTDBG_MAP_ALLOC
#        define _CRTDBG_MAP_ALLOC
#    endif
#    include <stdlib.h>
#    include <crtdbg.h>
#endif

#include "stdlib.h"

// This must be defined at the project level to ensure consistency
// between all object files.
#ifndef __OVERRIDE_NEW__
//#warning __OVERRIDE_NEW__ must be defined to either 1 (enabled) or 0 (disabled) before inclusion of this header.
#define __OVERRIDE_NEW__ 1
#endif

#ifdef __GNUC__
// alignment == # bits to align to
// size      == size of user block required
void * _aligned_malloc(size_t alignment, size_t size);
#define _aligned_free(p) free(p)
#endif

// Grab the memory manager
#if __OVERRIDE_NEW__
#    define MALLOC MEM_NAMESPACE::IMemMgr::lmalloc
#    define FREE   MEM_NAMESPACE::IMemMgr::lfree
#    define REALLOC MEM_NAMESPACE::IMemMgr::lrealloc
#error _OVERRIDE_NEW_ called!!!!
#else
#    define MALLOC malloc
#    define FREE   free
#    define REALLOC realloc
#    define AMALLOC(alignment, size) _aligned_malloc(size, alignment)
#    define AFREE free
#endif

#ifdef _VDSP
#include "string.h"        // for size_t
#endif

#ifndef MEM_NAMESPACE
#error The value MEM_NAMESPACE must be defined
#endif

#define MemoryBase MEM_NAMESPACE::MemoryBase_1

typedef int (*cbFunc)(char* );

namespace MEM_NAMESPACE
{
    class IMemMgr
    {
    public:
        // Initialize it
        virtual bool Init(void* pMemPool, int nSize) = 0;
        virtual bool Close() = 0;
        // Access the memory manager
        static  IMemMgr* GetMemoryManager(int nIndex = 0);
        // Debug Functions for heap management.
        virtual int        GetPeakMem() = 0;
        virtual int        GetCurrentMem() = 0;
        virtual int        GetRemaining() = 0;
        virtual void    DisplayMemoryUsage() = 0;
        virtual void     ImprintBlockGraph(char* pImage, int nWidth, int nHeight) = 0;
        virtual void    ExportMemoryMetrics(char* pBuffer, int nBufferLength) = 0;
        virtual void    DisplayBlockGraph() = 0;
        virtual void    DisplaySummary() = 0;
        virtual void    DisplaySummary(cbFunc func) = 0;
        virtual void    DisplayAllocatedMemBlocks() = 0;
        virtual bool    CheckHeapIntegrity() = 0;
        static  void*   lrealloc(void* pOld, size_t size);
        static  void*   lmalloc(size_t size);
        static  void      lfree(void* p);
        static  bool    CreateBootImage(void* pStart, void* pEnd);
        static  bool    ValidateBootImage(void* pStart, void* pEnd);
    };

    
    class MemoryBase_1
    {
    protected:
        ~MemoryBase_1() = default;
    public:
        void* operator new(size_t size);
        void* operator new(size_t size, void* pPlacement);
        void  operator delete (void* p);
    };
    
    
    
};


