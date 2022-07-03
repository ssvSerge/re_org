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
#pragma warning(disable : 4311 4312)
#endif
#include "lumi_stdint.h"
#include "MemoryManager.h"
#include "string.h"
#include "stdio.h"

using namespace MEM_NAMESPACE;
#define _DEBUG_MEMORY_    0

//#pragma optimize_off
#ifdef _VDSP
//section("sdram0_bank1")
#endif
MemoryManager MemoryManager::mem[4];

MemoryManager::MemoryManager()
{
    m_pCurMemStart        = NULL;
    m_pStartMemPool        = NULL;
    m_nMemPoolLength    = 0;
    m_nPeakAllocation     = 0;
    m_pHolePtr            = NULL;
    m_bInitialized = false;
}
bool MemoryManager::Init(void* pMemPool, int nSize)
{
    if(m_bInitialized) return false ;
    // Initialize Memory Pool on 16 byte boundary
    // XXX - jbates - use uintptr_t
    m_pStartMemPool = (char*)(((uintptr_t) (pMemPool) + 0xF) & (~0xF) );
    // may have to make buffer smaller
    // XXX - jbates - use uintptr_t
    // XXX - jbates - cast to int before subtract.
    nSize -= (int) ((uintptr_t)m_pStartMemPool - (uintptr_t)pMemPool); //((uint) (pMemPool) & 0x3) ? 4 : 0;
    m_nMemPoolLength = nSize;
    m_pEndMemPool = m_pStartMemPool + nSize;

    // XXX - jbates - type mismatch
    //memset(m_pStartMemPool, NULL, nSize);
    memset(m_pStartMemPool, 0, nSize);
    // Set "CurPtr"
    m_pCurMemStart = m_pStartMemPool;
    m_pFirstMemoryNode = (MemoryNode*)m_pStartMemPool;
    m_pFirstMemoryNode->pPrevious = NULL;
    m_pFirstMemoryNode->pNext = NULL;
    m_pFirstMemoryNode->pad[0] = 'l'; m_pFirstMemoryNode->pad[1] = 'u'; m_pFirstMemoryNode->pad[2] = 'm'; m_pFirstMemoryNode->pad[3] = 'i';
    m_bInitialized = true;
    m_pLastFilled = m_pFirstMemoryNode;

    #if    _DEBUG_MEMORY_
    if(false == CheckHeapIntegrity())
    {
        fprintf(stdout,"\n *** Heap Corrupted ****" );
    }
    #endif
    return true;
}
bool  MemoryManager::Close()
{
    memset(m_pStartMemPool, 0 , m_nMemPoolLength);
    m_pStartMemPool = 0;
    m_pEndMemPool= 0;
    m_pCurMemStart = 0;
    m_pFirstMemoryNode = NULL;
    m_nMemPoolLength = 0;
    m_nCurrentlyAllocated = 0;
    m_nPeakAllocation = 0;
    m_bInitialized = false;

    return true;
}
void* MemoryManager::alloc(size_t nSize_t)
{
    #if    _DEBUG_MEMORY_
    if(false == CheckHeapIntegrity())
    {
        fprintf(stdout,"\n *** Heap Corrupted ****" );
        exit(0);
    }
    #endif
    if(m_bInitialized == false) return NULL;
    if (m_pFirstMemoryNode == NULL) return NULL;
    //nSize = nSize+(8-nSize%8);
    uint nSize = (uint)(nSize_t+3)&(~0x3);
    MemoryNode* pAllocateAfter = FindHole(nSize,m_pLastFilled);
    if( pAllocateAfter == NULL )
    {
        pAllocateAfter = FindHole(nSize,m_pFirstMemoryNode);
    }
    if(pAllocateAfter == NULL) return NULL;

    m_pLastFilled = pAllocateAfter;
    // calc end of pAllocateAfter data is the start of our new node
    MemoryNode* pThisNode = (MemoryNode*)((char*)pAllocateAfter + pAllocateAfter->nSize + sizeof(MemoryNode));
    pThisNode->nSize = nSize;
//    pThisNode->nMarker = (uint)pThisNode;
    // If the AllocateAfter MemoryNode contains a link to next one, insert this in middle.
    // and relink.
    pThisNode->pNext = pAllocateAfter->pNext;
    pThisNode->pPrevious = pAllocateAfter;
    pThisNode->pad[0]     = 'l';
    pThisNode->pad[1]     = 'u';
    pThisNode->pad[2]     = 'm';
    pThisNode->pad[3]     = 'i';
    // Now, patch it in.
    pAllocateAfter->pNext = pThisNode;
    if(pThisNode->pNext)
    {
        pThisNode->pNext->pPrevious = pThisNode;
    }
    // Add this to current mem.
    m_nCurrentlyAllocated += nSize + sizeof(MemoryNode);
    if(m_nCurrentlyAllocated > m_nPeakAllocation)
    {
        m_nPeakAllocation = m_nCurrentlyAllocated;
    }
    //m_pCurMemStart = (char*)pThisNode;
    return ((char*)pThisNode) + sizeof(MemoryNode);        // = start of data
}
bool MemoryManager::release(void* pMemToRelease)
{
    #if    _DEBUG_MEMORY_
    if(false == CheckHeapIntegrity())
    {
        fprintf(stdout,"\n *** Heap Corrupted ****" );
        fflush(stdout);
        exit(0);
    }
    #endif
    if(m_bInitialized == false) return false;
    if( ( pMemToRelease < m_pStartMemPool ) || (pMemToRelease > m_pEndMemPool )) return false;
    MemoryNode* pFreeBird = (MemoryNode*)((char*)pMemToRelease - sizeof(MemoryNode));
    // Link "Previous" to "Next"
    if(strncmp(pFreeBird->pad,"lumi",4) != 0 )
    {
        fprintf(stdout,"\n Freebird has broken wing.");
    }
    if(pFreeBird->pNext && pFreeBird->pPrevious)
    {
        MemoryNode* nodeBefore = pFreeBird->pPrevious;
        MemoryNode* nodeAfter  = pFreeBird->pNext;

        nodeBefore->pNext = nodeAfter;
        nodeAfter->pPrevious = nodeBefore;
        if(m_pLastFilled == pFreeBird) m_pLastFilled = nodeBefore;
    } else
    {
        if( pFreeBird->pPrevious)
        {
            pFreeBird->pPrevious->pNext = NULL;
        }
        if(m_pLastFilled == pFreeBird) m_pLastFilled = m_pFirstMemoryNode;
    }
    m_nCurrentlyAllocated -= pFreeBird->nSize  + sizeof(MemoryNode);
    memset(pFreeBird, 0, sizeof(MemoryNode));
    return true;
}
int    MemoryManager::GetRemaining()
{
    return m_nMemPoolLength - m_nCurrentlyAllocated;
}
int    MemoryManager::GetPeakMem()
{
    return m_nPeakAllocation;
}
int    MemoryManager::GetCurrentMem()
{
    return m_nCurrentlyAllocated;
}
bool MemoryManager::CheckHeapIntegrity()
{
     //
     // XXX - jbates - unused.
    //bool bIntegrityError = false;

    MemoryNode* pLooker = m_pFirstMemoryNode;
    while(pLooker && true)
    {
        // Node isn't marked.
        if( strncmp(pLooker->pad,"lumi",4) != 0 )
        {
            fprintf(stdout,"\nNode corrupted.");
            return false;
        }
        // Nodes aren't linked.
        if(pLooker->pNext == NULL) // Last one
        {
            break;
        }
        if( pLooker->pNext->pPrevious != pLooker )
        {
             fprintf(stdout,"\nNode corrupted.");
             return false;
        }
        pLooker = (MemoryNode*)pLooker->pNext;        // No, check next node    }
    }
    return true;
}

#include "stdio.h"
void MemoryManager::DisplayMemoryUsage()
{
    // Iterate through
    // XXX - jbates - unused.
    //unsigned long nTotalMemoryUsed = 0;
    MemoryNode* pPreviousNode = m_pFirstMemoryNode;
    bool end = false;

    fprintf(stdout,"\n**** Current Memory Usage.\n%10d bytes allocated.",m_nCurrentlyAllocated);
    fprintf(stdout,"\n%10d Peak Usage\n",m_nPeakAllocation);
    fprintf(stdout,"\n%10d bytes remaining\n",m_nMemPoolLength-m_nCurrentlyAllocated);

    while(!end)
    {
        fprintf(stdout,"\n %9d allocated at 0x%p", pPreviousNode->nSize, ((char*)(pPreviousNode)+sizeof(MemoryNode)));
        pPreviousNode = (MemoryNode*)pPreviousNode->pNext;
        if(pPreviousNode == NULL) end = true;
    }

}
void MemoryManager::DisplayAllocatedMemBlocks()
{
    // Iterate through
    // XXX - jbates - unused.
    //unsigned long nTotalMemoryUsed = 0;
    MemoryNode* pPreviousNode = m_pFirstMemoryNode;
    bool end = false;

    fprintf(stdout,"\n**** Current Memory Usage.\n%10d bytes allocated.",m_nCurrentlyAllocated);
    fprintf(stdout,"\n%10d bytes remaining\n",m_nMemPoolLength-m_nCurrentlyAllocated);

    int nCnt = 0;
    while(!end)
    {
        fprintf(stdout,"\n(%3d) %9d allocated at 0x%p", nCnt++, pPreviousNode->nSize, ((char*)(pPreviousNode)+sizeof(MemoryNode)));
        pPreviousNode = (MemoryNode*)pPreviousNode->pNext;
        if(pPreviousNode == NULL) end = true;
    }
}
void MemoryManager::ImprintBlockGraph(char* pImageIn, int nWidth, int nHeight)
{

    // I want to display the entire memory block in one page.
    int nXResolution = nWidth/2;
    int nYResolution = nHeight/2;

    // fprintf(stdout,"\n\n ************** Displaying Block Graph *****************");
    unsigned char* pImage = (unsigned char*)(pImageIn);
    unsigned char* pPtr = (unsigned char*)pImage;

    pPtr = (unsigned char*)PackHeader((char*)pPtr, nWidth*nHeight);

    uint   nLineCtr = 1;
    unsigned char   nColor    = 255;
    unsigned char    BorderColor = 128;

    int BlockSize = m_nMemPoolLength / (nXResolution*nYResolution);
    // fprintf(stdout,  "\n Block Size:  %d bytes.",BlockSize);

    // Iterate through...
    MemoryNode* pFirstNode = m_pFirstMemoryNode;
    bool bDone = false;

    // XXX - jbates - unused.
    //int nXBlocksDrawn = nXResolution;
    int nBlkDraw = 0;
    unsigned long nTotalBytesAllocated = 0;
    while(!bDone)
    {
        // How many "blocks" does this node correspond to?
        nTotalBytesAllocated += pFirstNode->nSize + sizeof(MemoryNode);
        int nDrawBlocks = (pFirstNode->nSize + sizeof(MemoryNode))/BlockSize;
        if( nDrawBlocks == 0 )
        {
            nDrawBlocks = 1;
        }

        int nTmp = nDrawBlocks;
        while(nTmp--)
        {
            if( (nBlkDraw%nXResolution) == 0)
            {
                *pPtr = BorderColor;
                pPtr = pImage + (nLineCtr++)*nWidth;
                nBlkDraw = 0;
            }
            *pPtr++ = nColor;
            nBlkDraw++;
        }
        if(pFirstNode->pNext == NULL)
        {
            bDone = true;
            continue;
        }
        // If there are holes between this node and the next, draw the hole as well.
        int nHoleSize = (int)((char*)pFirstNode->pNext - (char*)pFirstNode) - pFirstNode->nSize - sizeof(MemoryNode);
        nDrawBlocks = nHoleSize/BlockSize;
        while(nDrawBlocks --)
        {
            if( (nBlkDraw%nXResolution) == 0)
            {
                *pPtr = BorderColor;
                pPtr = pImage + (nLineCtr++)*nWidth;
                nBlkDraw = 0;
            }
            *pPtr++ = 0;
            nBlkDraw++;
        }
        pFirstNode = (MemoryNode*)pFirstNode->pNext;
    }
    // Draw pFirstNode to end....
    int nBytesToEnd = (int)((char*)m_pEndMemPool - ((char*)pFirstNode + pFirstNode->nSize + sizeof(MemoryNode)));
    int nBlocksToDraw = nBytesToEnd/BlockSize;
    while(nBlocksToDraw --)
    {
        if( (nBlkDraw%nXResolution) == 0)
        {
            *pPtr = BorderColor;
            pPtr = pImage + (nLineCtr++)*nWidth;
            nBlkDraw = 0;
        }
        *pPtr++ = 0;
        nBlkDraw++;
    }
    pPtr = pImage + (nLineCtr++)*nWidth;
    while(nXResolution--) *pPtr++ = BorderColor;

    //fprintf(stdout,"\n\n******** Total Bytes Allocated:   %d\n\n",nTotalBytesAllocated);


}
char* MemoryManager::PackHeader(char* pPtr, unsigned int nBufferLength)
{
    // Pack the metrics
    // XXX - jbates - use uintptr_t
    uintptr_t  TempPtrValue = 0;
    // XXX - jbates - unused.
    //char* pMemLimit = pPtr+nBufferLength;

    // XXX - jbates - use uintptr_t
    TempPtrValue = (uintptr_t)m_pStartMemPool;
    pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));
    // XXX - jbates - use uintptr_t
    TempPtrValue = (uintptr_t)m_pEndMemPool;
    pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));
    // XXX - jbates - use uintptr_t
    TempPtrValue = (uintptr_t)m_pCurMemStart;
    pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));
    // XXX - jbates - use uintptr_t
    TempPtrValue = (uintptr_t)m_pFirstMemoryNode;
    pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));


    pPtr = Pack(pPtr,(char*)&m_nMemPoolLength,sizeof(m_nMemPoolLength));
    pPtr = Pack(pPtr,(char*)&m_nCurrentlyAllocated,sizeof(m_nCurrentlyAllocated));
    pPtr = Pack(pPtr,(char*)&m_nPeakAllocation,sizeof(m_nPeakAllocation));

    MemoryNode* pFirstNode = m_pFirstMemoryNode;
    uint nCnt = 0;

    while(pFirstNode)
    {
        nCnt++;
        pFirstNode = (MemoryNode*)pFirstNode->pNext;
    }
    // Reset node.
    pFirstNode = m_pFirstMemoryNode;
    // Pack node count
    pPtr = Pack(pPtr,(char*)&nCnt,sizeof(nCnt));
    while(pFirstNode)
    {
        // XXX - jbates - use uintptr_t
        TempPtrValue = (uintptr_t)pFirstNode;
        pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));
        // XXX - jbates - no-op
        //if(pPtr >= pMemLimit )
        //{
        //    int error = 1;
        //}
        // XXX - jbates - use uintptr_t
        TempPtrValue = (uintptr_t)(pFirstNode->pNext);
        pPtr = Pack(pPtr,(char*)&TempPtrValue,sizeof(TempPtrValue));
        // XXX - jbates - no-op
        //if(pPtr >= pMemLimit )
        //{
        //    int error = 1;
        //}
        pPtr = Pack(pPtr,(char*)&(pFirstNode->nSize),sizeof(pFirstNode->nSize));
        // XXX - jbates - no-op
        //if(pPtr >= pMemLimit )
        //{
        //    int error = 1;
        //}
        pFirstNode = (MemoryNode*)pFirstNode->pNext;
    }

    return pPtr;
}
char* MemoryManager::Pack(char* pDst, void* pSrc, uint nSize)
{
    memcpy(pDst,(char*)pSrc,nSize);
     return pDst+nSize;
}
void MemoryManager::ExportMemoryMetrics(char* pBuffer, int nBufferLength)
{
    char* pPtr = pBuffer;
    PackHeader(pPtr, nBufferLength);
}
void MemoryManager::DisplayBlockGraph()
{
    // I want to display the entire memory block in one page.
    int nXResolution = 60;
    int nYResolution = 80;

    fprintf(stdout,"\n\n ************** Displaying Block Graph *****************");


    int BlockSize = m_nMemPoolLength / (nXResolution*nYResolution);
    fprintf(stdout,  "\n Block Size:  %d bytes.",BlockSize);

    // Iterate through...
    MemoryNode* pFirstNode = m_pFirstMemoryNode;
    bool bDone = false;

    // XXX - jbates - unused
    //int nXBlocksDrawn = nXResolution;
    int nBlkDraw = 0;
    unsigned long nTotalBytesAllocated = 0;
    while(!bDone)
    {
        // How many "blocks" does this node correspond to?
        nTotalBytesAllocated += pFirstNode->nSize + sizeof(MemoryNode);
        int nDrawBlocks = (pFirstNode->nSize + sizeof(MemoryNode))/BlockSize;
        if( nDrawBlocks == 0 )
        {
            nDrawBlocks = 1;
        }

        int nTmp = nDrawBlocks;
        while(nTmp--)
        {
            if( (nBlkDraw%nXResolution) == 0)
            {
                fprintf(stdout,"\n");
                nBlkDraw = 0;
            }
            fprintf(stdout,"X");
            nBlkDraw++;
        }
        if(pFirstNode->pNext == NULL)
        {
            bDone = true;
            continue;
        }
        // If there are holes between this node and the next, draw the hole as well.
        int nHoleSize = (int)((char*)pFirstNode->pNext - (char*)pFirstNode) - pFirstNode->nSize - sizeof(MemoryNode);
        nDrawBlocks = nHoleSize/BlockSize;
        while(nDrawBlocks --)
        {
            if( (nBlkDraw%nXResolution) == 0)
            {
                fprintf(stdout,"\n");
                nBlkDraw = 0;
            }
            fprintf(stdout,"-");
            nBlkDraw++;
        }
        pFirstNode = (MemoryNode*)pFirstNode->pNext;
    }
    // Draw pFirstNode to end....
    int nBytesToEnd = (int)((char*)m_pEndMemPool - ((char*)pFirstNode + pFirstNode->nSize + sizeof(MemoryNode)));
    int nBlocksToDraw = nBytesToEnd/BlockSize;
    while(nBlocksToDraw --)
    {
        if( (nBlkDraw%nXResolution) == 0)
        {
            fprintf(stdout,"\n");
            nBlkDraw = 0;
        }
        fprintf(stdout,"-");
        nBlkDraw++;
    }


    fprintf(stdout,"\n\n******** Total Bytes Allocated:   %lu\n\n",nTotalBytesAllocated);


}
void MemoryManager::DisplaySummary()
{
    fprintf(stdout,"\n**** Current Memory Usage.\n%10d bytes allocated.",m_nCurrentlyAllocated);
    fprintf(stdout,"%10d bytes remaining\n",m_nMemPoolLength-m_nCurrentlyAllocated);
}
void MemoryManager::DisplaySummary(cbFunc func)
{
    sprintf(m_strScratch,"**** Current Memory Usage.");
    func(m_strScratch);
    sprintf(m_strScratch,"%10d bytes allocated",m_nCurrentlyAllocated);
    func(m_strScratch);
    sprintf(m_strScratch,"%10d bytes remaining",m_nMemPoolLength-m_nCurrentlyAllocated);
    func(m_strScratch);
}

MemoryNode* MemoryManager::FindHole(uint nSize, MemoryNode* pSeed)
{
    // Iterate through MemoryNodes until you find hole between end of data and next node
    MemoryNode* pLooker = pSeed;
    while(true)
    {
        if(pLooker->pNext == NULL) // Last one
        {
            // Calculate "real" memory left over, including the header that would be necessary.
            int RealBytesOpen = (int)(m_pEndMemPool - ((char*)pLooker + pLooker->nSize + 2*sizeof(MemoryNode)));
            // XXX - jbates - casted away signed/unsigned mismatch
            if( (RealBytesOpen > 0) && (nSize <= (uint) RealBytesOpen))
            {
                return pLooker;
            }
            return NULL;
        }
        // Calculate size of hole, should be >= 0
        // XXX - jbates - (int) cast
        int HoleSize = (int) ((char*)pLooker->pNext - ((char*)pLooker + pLooker->nSize + sizeof(MemoryNode)));
        if(nSize + sizeof(MemoryNode) <= (uint)HoleSize)
        {
            return pLooker;
        }

        pLooker = (MemoryNode*)pLooker->pNext;        // No, check next node
    }
}
typedef unsigned char uchar;

#ifdef _THISDOESNTWORK
section ("sdram0_no_cache")
static volatile uchar* pBI = NULL;
bool IMemMgr::CreateBootImage(void* pStart, void* pEnd)
{
    unsigned int sizeImage = (unsigned int)((uchar*)pEnd-(uchar*)pStart);
    if( pBI == NULL )
    {
        pBI = (uchar*)MALLOC(sizeImage);
    }
    memcpy((void*)pBI, pStart, sizeImage);
    ValidateBootImage(pStart, pEnd);
}
bool IMemMgr::ValidateBootImage(void* pStart, void* pEnd)
{
    if(pBI == NULL) return false;
    unsigned int sizeImage = (unsigned int)((uchar*)pEnd-(uchar*)pStart);

    volatile uchar* pCmp = (uchar*)pStart;

    for(int ii = 0; ii < sizeImage; ii++)
    {
        if( *(pBI + ii) != *(pCmp+ii) )
        {
            fprintf(stdout,"\nAddress 0x%x different", (uchar*)pStart+ii);
        }
    }
    return true;
}
#endif
