// ================================================================================================================================
//
// AutoHeapBuffer.h
//
// ability to declare a buffer on the heap that is automatically erased and free'd when the stack frame pops.
//
// ================================================================================================================================

#include <cstring>
#include <cstdlib>

#include "Platform.h"
#include "IMemMgr.h"

// ================================================================================================================================
// Use an auto declaration to get a buffer in heap
class AutoHeapBuffer
{
public:
    // create a new buffer of specified size
    AutoHeapBuffer(size_t l)    :
        m_p(NULL),
        m_l(l)
    {
        if (m_l)
            m_p = MALLOC(m_l);
    }
    // copy passed buffer of specified size
    AutoHeapBuffer(const uint8_t * p, size_t l)   :
        m_p(NULL),
        m_l(l)
    {
        if (m_l)
        {
            m_p = MALLOC(m_l);
            if (m_p)
                memcpy(m_p, p, m_l);
        }
    }
    // destroy with built-in zeroize...
    ~AutoHeapBuffer()
    {
        Clear();
        if (m_p)
            FREE(m_p); 
    }

    uint8_t * u8Ptr()
    { return reinterpret_cast<uint8_t *>(m_p); }
    char * charPtr()
    { return reinterpret_cast<char *>(m_p); }
    size_t Len()
    { return m_l; }
    void Clear()
    {
        if (m_p && m_l)
            memset(m_p, 0, m_l);
    }
    void Release()
    {
        Clear();
        if (m_p)
            FREE(m_p);
        m_p = NULL;
    }

private:
    void    * m_p;
    size_t    m_l;

    // pre-c++11: disallow default instantiation by not providing an implementation
    AutoHeapBuffer();
    // pre-c++11: disallow copy constructor by not providing an implementation
    AutoHeapBuffer(const AutoHeapBuffer &);
    AutoHeapBuffer & operator=(const AutoHeapBuffer &);
};

// ================================================================================================================================
// Use an auto declaration to get an object in heap
// use like this, for example:
// typdef AutoHeapObject<aes_context,Create_aes_context,Delete_aes_context> Auto_aes_context
// then implement the Create_aes_context and Delete_aes_context adaptors.
// always be sure to call Ptr() to do a NULL check, as this template does not check status of heap allocation.
template<class T, T * (*Create)(), void (*Destroy)(T *)>
class AutoHeapObject
{
public:
    AutoHeapObject()
    { m_pT = Create(); }
    virtual ~AutoHeapObject()
    { Destroy(m_pT); m_pT = NULL; }

    virtual T* Ptr()
    { return (T*) m_pT; }
private:
        T * m_pT;
};

class AutoFreePtr
{
public:
    AutoFreePtr() : m_ptr(NULL){};
    AutoFreePtr(void* ptr) : m_ptr(ptr){};
    ~AutoFreePtr()
    {
        if (m_ptr)
        {
            FREE(m_ptr);
            m_ptr = NULL;
        }
    }
private:
    void* m_ptr;
};

// ================================================================================================================================
//
// Easy way to deal with object derived from MemoryBase
//
//

template<class T>
class AutoDestructObject
{
public:
    AutoDestructObject() : m_pT(NULL){};
    AutoDestructObject(T* pObject) : m_pT(pObject){};
    ~AutoDestructObject()
    {
        if(m_pT)
        {
            delete m_pT;
            m_pT = NULL;
        }
    }
private:
    T* m_pT;
};

template<class T>
class AutoDestructPODArray
{
public:
    AutoDestructPODArray() : m_pT(NULL){};
    AutoDestructPODArray(T* pObject) : m_pT(pObject){};
    ~AutoDestructPODArray()
    {
        if (m_pT)
        {
            delete [] m_pT;
            m_pT = NULL;
        }
    }
private:
    T* m_pT;
};

// ================================================================================================================================


