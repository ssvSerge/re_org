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

#include <cassert>
#include "IThread.h"

#ifdef WIN32
    //#include "winnt.h"
    #define MUTEX_ASSERT(o) (o)
    #define XPLAT_MUTEX_TYPE HANDLE
    #define ASSERT_MUTEX_CREATE(m)          MUTEX_ASSERT((m = CreateMutex(NULL, FALSE, NULL)) != NULL)
    #define ASSERT_MUTEX_DESTROY(m)         MUTEX_ASSERT(m); MUTEX_ASSERT((CloseHandle(m) != 0))
                                            // XXX: vvv absolutely hates ABANDONNED
    #define ASSERT_MUTEX_LOCK(m)            MUTEX_ASSERT(m); MUTEX_ASSERT((WaitForSingleObject((m),INFINITE) == WAIT_OBJECT_0))
    #define ASSERT_MUTEX_UNLOCK(m)          MUTEX_ASSERT(m); MUTEX_ASSERT((ReleaseMutex((m)) != 0))
#else   // WIN32
    #include <pthread.h>
    #define MUTEX_ASSERT(o) assert(o)
    #define XPLAT_MUTEX_TYPE pthread_mutex_t
    #define ASSERT_MUTEX_CREATE(m)          pthread_mutex_init(&(m),NULL)               // cannot fail
    #define ASSERT_MUTEX_DESTROY(m)         MUTEX_ASSERT((pthread_mutex_destroy(&(m)) == 0))
    #define ASSERT_MUTEX_LOCK(m)            MUTEX_ASSERT((pthread_mutex_lock(&(m)) == 0))
    #define ASSERT_MUTEX_UNLOCK(m)          MUTEX_ASSERT((pthread_mutex_unlock(&(m)) == 0))
#endif  // WIN32

class BaseMutex : public ILockable
{
public:
                        BaseMutex()
                        { ASSERT_MUTEX_CREATE(m); }
    virtual             ~BaseMutex()
                        { ASSERT_MUTEX_DESTROY(m); }
    virtual bool        Acquire()
                        { ASSERT_MUTEX_LOCK(m); return true; }
    virtual bool        Release()
                        { ASSERT_MUTEX_UNLOCK(m); return true; }
protected:

    XPLAT_MUTEX_TYPE    m;
};
