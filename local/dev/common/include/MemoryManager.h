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
#include "IMemMgr.h"
#include <vector>

typedef unsigned int uint;

typedef struct _MemoryNode
{
    uint    nSize;
    //uint     nMarker;
    _MemoryNode*    pNext;
    _MemoryNode*    pPrevious;
    char    pad[4];        // we need sizeof(MemoryNode) = 16 for mem align
} MemoryNode;

namespace MEM_NAMESPACE
{

typedef std::vector<MemoryNode*>    MemoryHoles;

class MemoryManager : public MEM_NAMESPACE::IMemMgr
{
public:
    MemoryManager();
    virtual ~MemoryManager(){};
    // Allocate and release.
    void* alloc(size_t nSize);
    bool  release(void*);

    // Access the memory manager
    static  IMemMgr* GetMemoryManager(int Index = 0);
    // Debug Functions for heap management.
    virtual bool    Init(void* pMemPool, int nSize);
    virtual bool     Close();
    virtual int        GetPeakMem();
    virtual int        GetCurrentMem();
    virtual int        GetRemaining();
    virtual void    DisplayMemoryUsage();
    virtual void     ImprintBlockGraph(char* pImage, int nWidth, int nHeight);
    virtual void    ExportMemoryMetrics(char* pBuffer, int nBufferLength);
    virtual void    DisplayBlockGraph();
    virtual void    DisplaySummary();
    virtual void    DisplaySummary(cbFunc func);
    virtual void    DisplayAllocatedMemBlocks();
    virtual bool    CheckHeapIntegrity();
private:
    // private functions
    MemoryNode*        FindHole(uint nSize, MemoryNode* pSeed);
    char*            PackHeader(char*, uint nSize);
    char*              Pack(char* pDst, void* pSrc, uint nSize);
    //
    char*            m_pStartMemPool;
    char*            m_pEndMemPool;
    char*            m_pCurMemStart;
    MemoryNode*        m_pFirstMemoryNode;
    MemoryNode*        m_pLastFilled;
    MemoryNode*        m_pHolePtr;
    uint            m_nMemPoolLength;
    uint            m_nCurrentlyAllocated;
    uint            m_nPeakAllocation;
    bool            m_bInitialized;
    char            m_strScratch[255];
public:
    static  MemoryManager mem[4];
};
};

