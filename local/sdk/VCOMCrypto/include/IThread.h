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

// ------------------------------------------------------------------------------
// Thread
class IThread
{
public:
    virtual ~IThread(){};
    // Thread life-cycle
    virtual bool Initialize() = 0;
    // Majority of time spent here
    virtual bool Run() = 0;
    // Any type of shut-down happens here.
    virtual bool Close() = 0;
    // Set Handle
    virtual void SetSystemThreadHandle(void* pHandle) = 0;
    // Get Handle
    virtual void* GetSystemThreadHandle() = 0;
    // Quit Message
    virtual void PostQuitMessage() = 0;
    // Wait for Completion
    virtual bool WaitForCompletion(unsigned int nMS) = 0;
};

// ------------------------------------------------------------------------------
// objects that can be locked and unlocked should implement this interface
// so that the RAII scope based Lock object can be used for mutual-exclusion.
// the IMutex and ISemaphore both derive and implement this interface.
class ILockable
{
public:
    virtual ~ILockable() {}

    virtual bool Acquire() = 0;    // acquire the lockable resource - no timeout
    virtual bool Release() = 0;    // release the lockable resource
};

// ------------------------------------------------------------------------------
// EVENT
class IEvent
{
public:
    virtual ~IEvent() {}
    //
    virtual void InitializeEvent(bool bManualReset, bool bInitialState) = 0;
    // Close Handle
    virtual void CloseHandle() = 0;
    virtual bool GetManualReset() = 0;
    virtual bool GetInitialState() = 0;
    // Whatever the OS uses as a handle can be expressed as a null pointer 
    virtual void  SetSystemEventHandle(void* pHandle) = 0;
    virtual void* GetSystemEventHandle() = 0;
    // returns true if event signalled, false if timeout.   A timeout of 0 means infinite.
    virtual bool PendEvent(unsigned int nTimeoutMS) = 0;
    // Set Event
    virtual bool SetEvent() = 0;
};

// ------------------------------------------------------------------------------
// SEMAPHORE
class ISemaphore : public ILockable
{
public:
    virtual ~ISemaphore() {}
    //
    virtual void InitializeSemaphore(unsigned int nInitialCount, unsigned int nMaxCount, const char* pName, bool bMakeProcessUnique = false) = 0;
    // Close Handle
    virtual void CloseHandle() = 0;
    // 
    virtual bool IsProcessUnique() = 0;
    // Accessors
    virtual unsigned int GetInitialCount() = 0;
    virtual unsigned int GetMaxCount() = 0;
    // Some type of identifier.  
    virtual const char*  GetName() = 0;
    // Whatever the OS uses as a handle can be expressed as a null pointer 
    virtual void  SetSystemSemaphoreHandle(void* pHandle) = 0;
    virtual void* GetSystemSemaphoreHandle() = 0;
    // returns true if sem signalled, false if timeout.   A timeout of 0 means infinite.
    virtual bool PendSemaphore(unsigned int nTimeoutMS) = 0;
    // returns true if any of the handles signalled for bWaitAll = false, false if timeout.
    // returns true if all the handles are signalled for bWaitAll = true, false if timeout.
    //  A timeout of 0 means infinite.
    //virtual bool PendMultipleObjects(unsigned int nCount, void** pHandles, bool bWaitAll,  unsigned int nTimeoutMS, int* index)=0;
    // Set Event
    virtual bool SetEvent() = 0;

    virtual bool Acquire()
    { return this->PendSemaphore(0); }
    virtual bool Release()
    { return this->SetEvent(); }
};

// ------------------------------------------------------------------------------
// This object is a scope-based RAII lock for any lockable object,
// including BaseMutex and BaseSemaphore.
//
// here is an example of use:
//
// {
//     Lock l(m_pUSBLock);      // a lock on m_pUSBLock is acquired here...
//     ....perform USB I/O...
// } // the lock on m_pUSBLock is released here...
//
class Lock
{
public:
            Lock(ILockable & m) :        // pass in a reference...
                mm(m)
            { mm.Acquire(); }
            Lock(ILockable * pm) :        // ...or a pointer...
                mm(*pm)
            { mm.Acquire(); }
    virtual ~Lock()
            { mm.Release(); }

private:
            // disallow default instantiation
            Lock();
            // disallow shallow copy
            Lock(const Lock &);
    void    operator=(const Lock &);

    ILockable & mm;
};
