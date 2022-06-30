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

#include "stdlib.h"

#define MALLOC                     malloc
#define FREE                       free
#define AFREE                      free
#define REALLOC                    realloc
#define AMALLOC(alignment, size)   _aligned_malloc(size, alignment)

typedef int (*cbFunc)(char* );

class IMemMgr {
	public:
		// Initialize it
		virtual bool Init(void* pMemPool, int nSize) = 0;
		virtual bool Close() = 0;
		// Access the memory manager
		static  IMemMgr* GetMemoryManager(int nIndex = 0);
		// Debug Functions for heap management.
		virtual int		GetPeakMem() = 0;
		virtual int		GetCurrentMem() = 0;
		virtual int		GetRemaining() = 0;
		virtual void	DisplayMemoryUsage() = 0;
		virtual void 	ImprintBlockGraph(char* pImage, int nWidth, int nHeight) = 0;
		virtual void	ExportMemoryMetrics(char* pBuffer, int nBufferLength) = 0;
		virtual void	DisplayBlockGraph() = 0;
		virtual void	DisplaySummary() = 0;
		virtual void	DisplaySummary(cbFunc func) = 0;
		virtual void    DisplayAllocatedMemBlocks() = 0;
		virtual bool	CheckHeapIntegrity() = 0;
		static  void*   lrealloc(void* pOld, size_t size);
		static  void*   lmalloc(size_t size);
		static  void  	lfree(void* p);
		static  bool	CreateBootImage(void* pStart, void* pEnd);
		static  bool	ValidateBootImage(void* pStart, void* pEnd);
};

	
class MemoryBase {
	protected:
		~MemoryBase() = default;
	public:
		void* operator new(size_t size);
		void* operator new(size_t size, void* pPlacement);
		void  operator delete (void* p);
};


