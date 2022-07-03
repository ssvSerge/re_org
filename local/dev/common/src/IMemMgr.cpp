/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2009 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
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
#ifdef WIN32
#pragma warning(disable : 4311 4313)
#include "crtdbg.h"
#endif

#ifdef _VDSP
//
//#include <services/services.h>
#endif

#include "lumi_stdint.h"
#include "IMemMgr.h"
#include "MemoryManager.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "parallel.h"
#include <assert.h>

#ifdef _BSP_LAYER
#include "..\..\Build\VDK.h"
#else
#ifdef _VDSP_VDK
#include "VDK.h"
#endif
#endif


// Static IMemMgr functions
using namespace MEM_NAMESPACE;

#ifdef _VDSP
//section("sdram0_bank1")
#endif

#if 0
namespace MEM_NAMESPACE
{
    MemoryManager mem[3];
};
#endif

void* MemoryBase_1::operator new(size_t size)
{
#if __OVERRIDE_NEW__
    return MEM_NAMESPACE::IMemMgr::lmalloc(size);
#else
    return malloc(size);
#endif
}
void  MemoryBase_1::operator delete (void* p)
{
#if __OVERRIDE_NEW__
    return MEM_NAMESPACE::IMemMgr::lfree(p);
#else
    return free(p);
#endif
}

//from new.h
void* MemoryBase_1::operator new( size_t, void* what )
{
   return what;
}

void* IMemMgr::lmalloc(size_t size)
{
#ifdef IN_PARALLEL
#    ifdef _DEBUG
    // leave it just in case; ARM doesn't use custom memory manager
    assert(is_main_thread());            // IMemMgr isn't thread safe
#    endif
#endif

#ifdef _VDSP_VDK
    VDK::PushUnscheduledRegion();
#endif
    void *p=MemoryManager::mem[0].alloc((int)size);

#ifndef __GNUC__
    if((uint64_t)(p)%4)
    {
         fprintf(stdout,"Memory %p not aligned.\n",p);
    }
    #endif
    if( p == NULL)
    {
        if ( MemoryManager::mem[1].GetRemaining() ) {
            p = MemoryManager::mem[1].alloc((int)size);
        }
    }

    if( p == NULL)
    {
        if ( MemoryManager::mem[2].GetRemaining() ) {
            p = MemoryManager::mem[2].alloc((int)size);
        }
    }

    if( p == NULL)
    {
        if ( MemoryManager::mem[3].GetRemaining() ) {
            p = MemoryManager::mem[3].alloc((int)size);
        }
    }

    if( p == NULL)
    {
        //int breakme = 1;
        fprintf(stdout,"mem[0] Cannot allocate %d bytes, %d remaining.\n",
            (int)size,IMemMgr::GetMemoryManager(0)->GetRemaining());
        fprintf(stdout,"mem[1] Cannot allocate %d bytes, %d remaining.\n",
            (int)size,IMemMgr::GetMemoryManager(1)->GetRemaining());
#ifdef _DEBUG_
        IMemMgr::GetMemoryManager()->DisplayBlockGraph();
#endif
    }
#ifdef _VDSP_VDK

    VDK::PopUnscheduledRegion();
#endif
    return p;
}
void* IMemMgr::lrealloc(void* pOld, size_t size)
{
#ifdef _VDSP_VDK
    VDK::PushUnscheduledRegion();
#endif
    void* pNew = MemoryManager::mem[0].alloc(size);
    if(pNew == NULL)
    {
#ifdef _VDSP_VDK
        VDK::PopUnscheduledRegion();
#endif
        return NULL;
    }
    memcpy(pNew,pOld, size);
    lfree(pOld);

#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();
#endif
    return pNew;
}
void IMemMgr::lfree(void* p)
{
    if( NULL == p )
    {
        return;
    }
#ifdef _VDSP_VDK
    VDK::PushUnscheduledRegion();
#endif

    if(true == MemoryManager::mem[0].release(p))
    {
#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();
#endif
        return;
    }
    //mem[0].DisplayAllocatedMemBlocks();
    if(true == MemoryManager::mem[1].release(p))
    {
#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();;
#endif
       return;
    }
    if(true == MemoryManager::mem[2].release(p))
    {
#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();
#endif
       return;
    }
    if(true == MemoryManager::mem[3].release(p))
    {
#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();
#endif
       return;
    }

#ifdef _VDSP_VDK
    VDK::PopUnscheduledRegion();
#endif
    return;
}

IMemMgr* IMemMgr::GetMemoryManager(int nIndex)
{
    return &MemoryManager::mem[nIndex];
}

#ifdef __GNUC__
#include <assert.h>
//#include <stddef.h>
//#include "lumi_stdint.h"
//#include "stdalign.h"
void * _aligned_malloc(size_t alignment, size_t size)
{
    // enforce sensible request
         if (alignment == 64)
             alignment = 8;
    else if (alignment == 32)
             alignment = 4;
    else if (alignment == 16)
             alignment = 2;
    else if (alignment == 8)
             alignment = 1;
    else if (alignment == 32768)
             alignment = 4096;
    else
             return NULL;

    // oversized alloc...
    void * p = malloc(size + alignment);
    size_t slop = ((uintptr_t) p) % alignment;
    if (slop != 0)
    {
        // there was slop, so offset return value to be aligned
        p = (void *) (((uint8_t *) p) + (alignment - slop));
    }
    slop = ((uintptr_t) p) % alignment;
    assert(slop == 0);
    return p;
}
#endif


