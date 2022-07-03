/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2014 Lumidigm, part of HID Global  (Unpublished Copyright) ALL RIGHTS RESERVED.
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
/******************************<%END LICENSE%>********************************/
#include "V100Cmd.h"
#include "string.h"

// ****************************  CMD_CANCEL_OPERATION  ***********************/

Atomic_Cancel_Operation::Atomic_Cancel_Operation()
{
    m_nCmd = CMD_CANCEL_OPERATION;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Cancel_Operation::~Atomic_Cancel_Operation()
{

}

bool Atomic_Cancel_Operation::PackChallenge(uchar** pPacket, uint& nSize)
{
    GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

bool Atomic_Cancel_Operation::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

bool Atomic_Cancel_Operation::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

bool Atomic_Cancel_Operation::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}

int  Atomic_Cancel_Operation::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

int  Atomic_Cancel_Operation::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_ERROR  **********************************/

Atomic_Error::Atomic_Error()
{
    m_nCmd = CMD_ERROR;
    // Challenge buffer size...
    m_nChallengeBufferSize = 0;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Error::~Atomic_Error()
{

}

bool Atomic_Error::PackChallenge(uchar** pPacket, uint& nSize)
{
    return false;
}

bool Atomic_Error::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    return false;
}

bool Atomic_Error::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;

    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

bool Atomic_Error::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}

int  Atomic_Error::GetChallengeBufferSize()
{
    return 0;
}

int  Atomic_Error::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_ACQ_STATUS  *************************/
Atomic_Get_Acq_Status::Atomic_Get_Acq_Status()
{
    m_nCmd = CMD_GET_ACQ_STATUS;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Get_Acq_Status::~Atomic_Get_Acq_Status()
{

}

bool Atomic_Get_Acq_Status::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

bool Atomic_Get_Acq_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

bool Atomic_Get_Acq_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    m_nResponseBufferSize += sizeof(_V100_ACQ_STATUS_TYPE);            // Matching Score
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    memcpy(pPtr, &m_ACQStatus, sizeof(m_ACQStatus));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

bool Atomic_Get_Acq_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    memcpy(&m_ACQStatus, pPtr, sizeof(m_ACQStatus));
    return true;
}

int  Atomic_Get_Acq_Status::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

int  Atomic_Get_Acq_Status::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_CONFIG  *****************************/
Atomic_Get_Config::Atomic_Get_Config()
{
     m_nCmd = CMD_GET_CONFIG;
     memset(&m_conf,0,sizeof(m_conf));
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE + sizeof(uint);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE + sizeof(_V100_INTERFACE_CONFIGURATION_TYPE);
     m_pResponseBuffer = NULL;
     m_nSizeOfStruct = sizeof(_V100_INTERFACE_CONFIGURATION_TYPE);
}

Atomic_Get_Config::~Atomic_Get_Config()
{
}

bool Atomic_Get_Config::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(0,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Opaque Data
    uint   sizeofStruct = sizeof(_V100_INTERFACE_CONFIGURATION_TYPE);
    memcpy(pPtr,&sizeofStruct,sizeof(uint));
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

bool Atomic_Get_Config::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    if(pPacket == NULL) return false;
    // Lets see what this header contains....
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    memcpy(&m_nSizeOfStruct, pPtr, sizeof(uint));
    return true;
}

bool Atomic_Get_Config::PackResponse(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Opaque Data
    memcpy(pPtr,&m_conf,sizeof(_V100_INTERFACE_CONFIGURATION_TYPE));
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

bool Atomic_Get_Config::UnpackResponse(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    memcpy(&m_conf, pPtr, sizeof(m_conf));
    return true;
}

int  Atomic_Get_Config::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

int  Atomic_Get_Config::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_CMD  ********************************/

Atomic_Get_Cmd::Atomic_Get_Cmd()
{
    m_nCmd = CMD_GET_CMD;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Get_Cmd::~Atomic_Get_Cmd()
{

}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Cmd::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Cmd::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Cmd::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    // Size field
    m_nSize = sizeof(m_nICT);
    //
    m_nResponseBufferSize += sizeof(m_nSize);
    // The Structure
    m_nResponseBufferSize += sizeof(m_nICT);
    //
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Pack size of structure
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    if (NULL == pPtr)
        return false;

    // Pack Structure
    pPtr = Pack(pPtr, &m_nICT, m_nSize);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Cmd::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    // Unpack size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    if (NULL == pPtr)
        return false;

    if (m_nSize != sizeof(m_nICT)) return false;
    pPtr = Unpack(&m_nICT, pPtr, m_nSize);
    if (NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Cmd::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Cmd::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_OP_STATUS  **************************/

Atomic_Get_OP_Status::Atomic_Get_OP_Status()
{
    m_nCmd = CMD_GET_OP_STATUS;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Get_OP_Status::~Atomic_Get_OP_Status()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_OP_Status::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_OP_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_OP_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize += sizeof(_V100_OP_STATUS);
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_OPStatus, sizeof(m_OPStatus));
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_OP_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_OPStatus, pPtr, sizeof(m_OPStatus));
    if (NULL == pPtr)
        return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Get_OP_Status::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Get_OP_Status::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// ****************************  CMD_GET_SERIAL_NUMBER  **********************/

Atomic_Get_Serial::Atomic_Get_Serial()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_GET_SERIAL_NUMBER;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize += sizeof(m_nSize);
    m_nResponseBufferSize += sizeof(m_nSerialNumber);
    m_pResponseBuffer = NULL;
    m_nSize = sizeof(m_nSerialNumber);
}

Atomic_Get_Serial::~Atomic_Get_Serial()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Serial::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Serial::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Serial::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nSerialNumber, m_nSize);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Serial::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nSerialNumber, pPtr, m_nSize);
    if (NULL == pPtr)
        return false;


    return true;
}

// How large is the Challenge packet?
int  Atomic_Get_Serial::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Get_Serial::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_STATUS  *****************************/

Atomic_Get_Status::Atomic_Get_Status()
{
    m_nCmd = CMD_GET_STATUS;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_nStatusSize = sizeof(_V100_INTERFACE_STATUS_TYPE);
}

Atomic_Get_Status::~Atomic_Get_Status()
{

}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Status::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize += sizeof(m_nStatusSize);
    m_nResponseBufferSize += sizeof(_V100_INTERFACE_STATUS_TYPE);
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nStatusSize, sizeof(m_nStatusSize));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nIST, m_nStatusSize);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nStatusSize, pPtr, sizeof(m_nStatusSize));
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nIST, pPtr, sizeof(_V100_INTERFACE_STATUS_TYPE));
    if (NULL == pPtr)
        return false;


    return true;
}

// How large is the Challenge packet?
int  Atomic_Get_Status::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Get_Status::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_SYSTEM_STATE  ***********************/

Atomic_Get_System_State::Atomic_Get_System_State()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_GET_SYSTEM_STATE;
    m_nArg = (short)0x9849;
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    //int x = sizeof(_V100_SYSTEM_DIAGNOSTICS);
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE + sizeof(_V100_SYSTEM_DIAGNOSTICS);
}

Atomic_Get_System_State::~Atomic_Get_System_State()
{

}

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_System_State::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pOpaqueDataStart = */GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    // Opaque Data
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure

bool Atomic_Get_System_State::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    if (pPacket == NULL) return false;
    // Lets see what this header contains....
    /*uchar* pOpaqueDataStart =*/ UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// Takes content of Command, and packs it into pPacket

bool Atomic_Get_System_State::PackResponse(uchar** pPacket, uint& nSize)
{
    uchar* pOpaqueDataStart = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    // Opaque Data
    memcpy(pOpaqueDataStart, &m_Diagnostics, sizeof(_V100_SYSTEM_DIAGNOSTICS));
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure

bool Atomic_Get_System_State::UnpackResponse(const uchar* pPacket, uint nSize)
{
    uchar* pOpaqueDataStart = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    memcpy(&m_Diagnostics, pOpaqueDataStart, sizeof(_V100_SYSTEM_DIAGNOSTICS));
    return true;
}
// How large is the Challenge packet?

int  Atomic_Get_System_State::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Get_System_State::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_GET_TAG  ********************************/

Atomic_Get_Tag::Atomic_Get_Tag()
{
    m_nCmd = CMD_GET_TAG;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_pTag = NULL;
}

Atomic_Get_Tag::~Atomic_Get_Tag()
{
    if (m_pTag)
    {
        FREE(m_pTag);
        m_pTag = NULL;
    }
}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Tag::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Tag::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Tag::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(m_nTagSize);
    m_nResponseBufferSize += m_nTagSize;
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nTagSize, sizeof(m_nTagSize));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, m_pTag, m_nTagSize);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Get_Tag::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nTagSize, pPtr, sizeof(m_nTagSize));
    if (NULL == pPtr)
        return false;

    m_pTag = (char*)MALLOC(m_nTagSize);
    pPtr = Unpack(m_pTag, pPtr, m_nTagSize);
    if (NULL == pPtr)
        return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Get_Tag::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Get_Tag::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool Atomic_Get_Tag::SetTag(char* pTag, ushort& nSize)
{
    if (m_pTag == NULL)
    {
        m_pTag = (char*)MALLOC(nSize);
    }
    memcpy(m_pTag, pTag, nSize);
    m_nTagSize = nSize;
    return true;
}

bool Atomic_Get_Tag::GetTag(char** pTag, ushort& nSize)
{
    *pTag = m_pTag;
    nSize = m_nTagSize;
    return true;
}

// ****************************  CMD_RESET  **********************************/

Atomic_Reset::Atomic_Reset()
{
    m_nCmd = CMD_RESET;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}

Atomic_Reset::~Atomic_Reset()
{

}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Reset::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Reset::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Reset::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Reset::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Reset::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Reset::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ****************************  CMD_SET_TAG  ********************************/

Atomic_Set_Tag::Atomic_Set_Tag()
{
    m_nCmd = CMD_SET_TAG;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_pTag = NULL;
}

Atomic_Set_Tag::~Atomic_Set_Tag()
{
    if (m_pTag)
    {
        FREE(m_pTag);
        m_pTag = NULL;
    }
}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Tag::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nTagSize);
    m_nChallengeBufferSize += m_nTagSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nTagSize, sizeof(m_nTagSize));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, m_pTag, m_nTagSize);
    if (NULL == pPtr)
        return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Set_Tag::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nTagSize, pPtr, sizeof(m_nTagSize));
    if (NULL == pPtr)
        return false;

    m_pTag = (char*)MALLOC(m_nTagSize);
    pPtr = Unpack(m_pTag, pPtr, m_nTagSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Tag::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Set_Tag::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Set_Tag::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Set_Tag::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool Atomic_Set_Tag::SetTag(char* pTag, ushort nSize)
{
    if (m_pTag == NULL)
    {
        m_pTag = (char*)MALLOC(nSize);
    }
    memcpy(m_pTag, pTag, nSize);
    m_nTagSize = nSize;
    return true;
}

bool Atomic_Set_Tag::GetTag(char** pTag, ushort& nSize)
{
    *pTag = m_pTag;
    nSize = m_nTagSize;
    return true;
}

// ****************************  CMD_UPDATE_FIRMWARE  ************************/

Macro_Update_Firmware::Macro_Update_Firmware()
{
    m_nCmd = CMD_UPDATE_FIRMWARE;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;

    m_pFWData = NULL;
    m_nDataSize = 0;

}

Macro_Update_Firmware::~Macro_Update_Firmware()
{
    if (m_pFWData)
    {
        FREE(m_pFWData);
        m_pFWData = NULL;
    }
}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Macro_Update_Firmware::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(unsigned int);
    m_nChallengeBufferSize += m_nDataSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nDataSize, sizeof(unsigned int));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, m_pFWData, m_nDataSize);
    if (NULL == pPtr)
        return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}

// Unpacks packet passed in into internal data structure
bool Macro_Update_Firmware::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nDataSize, pPtr, sizeof(unsigned int));
    if (NULL == pPtr)
        return false;

    if (m_pFWData) FREE(m_pFWData);
    if (m_nDataSize > MAX_FW_DATA_SIZE) return false;
    m_pFWData = (uchar*)MALLOC(m_nDataSize);
    pPtr = Unpack(m_pFWData, pPtr, m_nDataSize);
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Macro_Update_Firmware::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Macro_Update_Firmware::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}

// How large is the Challenge packet?
int  Macro_Update_Firmware::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Macro_Update_Firmware::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool   Macro_Update_Firmware::SetData(const uchar* pData, uint nDataSize)
{
    if (m_pFWData)
    {
        FREE(m_pFWData);
        m_pFWData = NULL;
    }
    if (nDataSize > MAX_FW_DATA_SIZE) return false;
    m_pFWData = (uchar*)MALLOC(nDataSize);
    memset(m_pFWData, 0, nDataSize);
    memcpy(m_pFWData, pData, nDataSize);
    m_nDataSize = nDataSize;
    return true;
}

uchar* Macro_Update_Firmware::GetData()
{
    return m_pFWData;
}

uint   Macro_Update_Firmware::GetDataSize()
{
    return m_nDataSize;
}

// ****************************  HID JAGUAR INIT  ****************************/



// ***************************************************************************/

// ***************************************************************************/
