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

#include "IThread.h"

class IThread;
class ISemaphore;
class IStreamDvc;
class ISELogger;
class Configuration;


#define COMM_PIPE        1000
#define SVC_PIPE        1001
#define DVC_SVC_PIPE    1002


class IXServiceProvider
{
public:
    virtual ~IXServiceProvider(){};

    // -----------------------------------------------------------------------------------
    // THREAD
    // Returns thread identifier, NULL on failure.
    virtual void* CreateThread(IThread* pTO) = 0 ;
    // Wait with timeout for thread completion.
    virtual bool PendThreadCompletion(IThread* pThread, unsigned int nTimeoutMS) = 0;
    virtual void CloseHandle(IThread* pThread) = 0;

    // ----------------------------------------------------------------------------------------------
    // EVENT - acts like a Windows EVENT, even if you're not on Windows.
    // Returns system event identifier - which you may use, as certain Windows-only APIs require it.
    // POSIX has an independent implementation.  Under POSIX requires a POSIX-only equivalent API.
    virtual void* InitEvent(IEvent* pEvent) = 0;
    // Don't wait for Event to be signalled.
    virtual bool TryPendEvent(IEvent* pEvent) = 0;
    // Wait for Event to be signalled.
    virtual bool PendEvent(IEvent* pEvent, unsigned int nTimeoutMS) = 0;
    // Signal the Event
    virtual bool SetEvent(IEvent* pEvent) = 0;
    virtual void CloseHandle(IEvent* pEvent) = 0;

    // ----------------------------------------------------------------------------------------------
    // SEMAPHORE - use this when you need a synchronization mechanism that can count
    //             or if you need to Pend() with a timeout.
    // Returns system semaphore identifier - which you SHOULD NOT USE.
    virtual void* InitSemaphore(ISemaphore* pSemaphore) = 0;
    // Wait for Semaphore to be signalled.
    virtual bool PendSemaphore(ISemaphore* pSem, unsigned int nTimeoutMS) = 0;
    // Test if Semaphore is signalled without blocking.
    virtual bool TryPendSemaphore(ISemaphore* pSem) = 0;
    // Signal the Semaphore
    virtual bool SetEvent(ISemaphore* pSem) = 0;
    virtual void CloseHandle(ISemaphore* pSem) = 0;
    // ----------------------------------------------------------------------------------------------

    // Pends multiple objects
    // DEPRECATION WARNING : this method is slated to disappear in the future.
    // DEPRECATION NOTE    : this functionality was purged on 2013/12/11 by Jack Bates
    //virtual bool PendMultipleObjects(unsigned int, void**, bool,  unsigned int, int*) = 0;

    // Timing
    virtual bool Sleep(unsigned int nMSToSleep) = 0;
    // Attain Stream device
    virtual IStreamDvc* GetStreamDvc(int nIdentifier) = 0;
    // Get Logger
    virtual ISELogger* GetLogger() = 0;
    // Timing
    virtual unsigned int GetTime() = 0;
    virtual unsigned int GetDurationMS(unsigned int nTime) = 0;
#if 0
    // General Critical Section - this is in regards to this object only!!!
    virtual bool Lock()    = 0;
    virtual bool Release() = 0;
#endif
    // Config
    virtual bool SetConfiguration(Configuration* pConfiguration)=0;
    // Reset streaming device
    virtual bool ResetDvc(Configuration* pConfiguration, ISemaphore* pSem)=0;
    // Notify that the SEngine is ready for a streaming device
    virtual bool NotifySEReady(Configuration* pConfiguration, ISemaphore* pSem)=0;

    // Chris - remove dll dependancy
    //virtual ISEngineProc*  GetProcessorInterface() = 0;
    //virtual ISEngineOneToN*        GetOneToNInterface() = 0;

};

#define    QUIT_REQUEST    0
#define TRANSFER_OK        1
#define TRANSFER_ERR    2

typedef enum
{
    ServerMode,
    ClientMode
} StreamMode;

class IStreamDvc
{
public:
    virtual ~IStreamDvc(){};
    virtual int ConnectStream(  const char *, ISemaphore* pQuitEvent, StreamMode streamMode = ServerMode, unsigned int nTimeout = 0 ) = 0;
    virtual int ReadStream(unsigned char* pStream, unsigned int& nSize, unsigned int nTimeoutMS) = 0;
    virtual int WriteStream(unsigned char* pStream, unsigned int nSize, unsigned int nTimeoutMS) = 0;
};
//
extern "C" IXServiceProvider* GetServiceProvider();
extern "C" void ReleaseServiceProvider();
