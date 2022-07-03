/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
**                                                                                                                      **
** For a list of applicable patents and patents pending, visit www.hidglobal.com/patents                                **
**                                                                                                                      **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                           **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS                                     **
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR                                       **
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER                                       **
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                                              **
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                           **
**                                                                                                                      **
*************************************************************************************************************************/
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "string.h"

#define UNUSED(x)            (void)(x)


// **************************************  CMD_ENC_GET_KEY  ************************************** //
Atomic_Enc_Get_Key::Atomic_Enc_Get_Key()
{
    m_nCmd = CMD_ENC_GET_KEY;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    memset(m_pKey, 0, sizeof(u2048));
    memset(m_KCV, 0, sizeof(m_KCV));
    m_nKeyLength = 0;
    m_nKeyMode = KM_MODE_NONE;
}
Atomic_Enc_Get_Key::~Atomic_Enc_Get_Key()
{

}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Key::PackChallenge(uchar** pPacket, uint& nSize)
{

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Key::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Key::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(m_nKeyLength);
    m_nResponseBufferSize += sizeof(m_KCV);
    m_nResponseBufferSize += sizeof(m_nKeyVersion);
    m_nResponseBufferSize += sizeof(m_nKeyMode);
    m_nResponseBufferSize += sizeof(u2048);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;

    pPtr = Pack(pPtr, &m_nKeyLength, sizeof(m_nKeyLength));
    pPtr = Pack(pPtr, m_KCV, sizeof(m_KCV));
    pPtr = Pack(pPtr, &m_nKeyVersion, sizeof(m_nKeyVersion));
    pPtr = Pack(pPtr, &m_nKeyMode, sizeof(m_nKeyMode));
    pPtr = Pack(pPtr, m_pKey, sizeof(u2048));
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Key::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nKeyLength, pPtr, sizeof(m_nKeyLength));
    pPtr = Unpack(m_KCV, pPtr, sizeof(m_KCV));
    pPtr = Unpack(&m_nKeyVersion, pPtr, sizeof(m_nKeyVersion));
    pPtr = Unpack(&m_nKeyMode, pPtr, sizeof(m_nKeyMode));
    pPtr = Unpack(m_pKey, pPtr, sizeof(u2048));
    return (pPtr != NULL) ? true : false;
}
// How large is the Challenge packet?
int  Atomic_Enc_Get_Key::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Get_Key::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// **************************************  CMD_ENC_GET_KEYVERSION  ************************************** //
Atomic_Enc_Get_KeyVersion::Atomic_Enc_Get_KeyVersion() {
    m_nCmd = CMD_ENC_GET_KEYVERSION;
    // Challenge buffer size...
    m_nChallengeBufferSize    = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer        = NULL;
    m_nResponseBufferSize    = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer        = NULL;

    m_nKeySlot    = KT_EXT_LAST;
    m_nKeyVersion = 0;
    m_nKeyMode    = KM_MODE_NONE;

    memset(m_pKCV, 0, sizeof(m_pKCV));
}

Atomic_Enc_Get_KeyVersion::~Atomic_Enc_Get_KeyVersion() {

}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_KeyVersion::PackChallenge(uchar** pPacket, uint& nSize) {

    m_nChallengeBufferSize += sizeof(m_nKeySlot);

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) {
        return false;
    }

    // Fill in some stuff
    pPtr = Pack(pPtr, &m_nKeySlot, sizeof(m_nKeySlot));

    *pPacket = m_pChallengeBuffer;
    nSize     = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_KeyVersion::UnpackChallenge(const uchar* pPacket, uint nSize) {

    UNUSED(nSize);
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nKeySlot, pPtr, sizeof(m_nKeySlot));
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_KeyVersion::PackResponse(uchar** pPacket, uint& nSize) {

    UNUSED(nSize);

    m_nResponseBufferSize += sizeof(m_nKeyVersion);
    m_nResponseBufferSize += sizeof(m_nKeyMode);
    m_nResponseBufferSize += sizeof(m_pKCV);

    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) {
        return true;
    }

    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) {
        return false;
    }

    pPtr = Pack(pPtr, &m_nKeyVersion, sizeof(m_nKeyVersion));
    pPtr = Pack(pPtr, &m_nKeyMode, sizeof(m_nKeyMode));
    pPtr = Pack(pPtr, m_pKCV, sizeof(m_pKCV));

    if (NULL == pPtr) {
        return false;
    }

    // Response
    *pPacket = m_pResponseBuffer;
    nSize     = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_KeyVersion::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nKeyVersion, pPtr, sizeof(m_nKeyVersion));
    pPtr = Unpack(&m_nKeyMode, pPtr, sizeof(m_nKeyMode));
    pPtr = Unpack(m_pKCV, pPtr, sizeof(m_pKCV));
    return (pPtr != NULL) ? true : false;
}
// How large is the Challenge packet?
int  Atomic_Enc_Get_KeyVersion::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Get_KeyVersion::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// **************************************  CMD_ENC_GET_RND_NUMBER  ************************************** //
Atomic_Enc_Get_Rnd_Number::Atomic_Enc_Get_Rnd_Number()
{
    m_nCmd = CMD_ENC_GET_RND_NUMBER;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Enc_Get_Rnd_Number::~Atomic_Enc_Get_Rnd_Number()
{

}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Rnd_Number::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Rnd_Number::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Rnd_Number::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(m_Rand);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, m_Rand, sizeof(m_Rand));
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Rnd_Number::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(m_Rand, pPtr, sizeof(m_Rand));
    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_Get_Rnd_Number::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Get_Rnd_Number::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// **************************************  CMD_ENC_GET_SERIAL_NUMBER  ************************************** //
Atomic_Enc_Get_Serial_Number::Atomic_Enc_Get_Serial_Number()
{
     m_nCmd = CMD_ENC_GET_SERIAL_NUMBER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Enc_Get_Serial_Number::~Atomic_Enc_Get_Serial_Number()
{

}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Serial_Number::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr) return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Serial_Number::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_Serial_Number::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize+=sizeof(m_SerialNum);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_SerialNum, sizeof(m_SerialNum));
    if(NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_Serial_Number::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(NULL == pPtr) return false;
    pPtr = Unpack(&m_SerialNum, pPtr, sizeof(m_SerialNum));
    if(NULL == pPtr) return false;
    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_Get_Serial_Number::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Get_Serial_Number::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_SET_KEY  ************************************** //
Atomic_Enc_Set_Key::Atomic_Enc_Set_Key() {

    m_nCmd = CMD_ENC_SET_KEY;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;

    m_pBuffer     = NULL;
    m_nBufferSize = 0;
    m_nKeyType    = KT_EXT_LAST;
}

Atomic_Enc_Set_Key::~Atomic_Enc_Set_Key() {
    if (m_pBuffer) {
        FREE(m_pBuffer);
        m_pBuffer = NULL;
    }
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Set_Key::PackChallenge(uchar** pPacket, uint& nSize) {

    m_nChallengeBufferSize  +=  sizeof(m_nBufferSize);    // Size of opaque packet (int)
    m_nChallengeBufferSize  +=  m_nBufferSize;            // The opaque packet
    m_nChallengeBufferSize  +=  sizeof(m_nKeyType);

    uchar* pPtr;
    pPtr  =  GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    pPtr  =  Pack(pPtr, &m_nKeyType, sizeof(m_nKeyType));
    pPtr  =  Pack(pPtr, &m_nBufferSize, sizeof(m_nBufferSize));
    pPtr  =  Pack(pPtr, m_pBuffer, m_nBufferSize);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize    = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Set_Key::UnpackChallenge(const uchar* pPacket, uint nSize) {

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nKeyType, pPtr, sizeof(m_nKeyType));
    pPtr = Unpack(&m_nBufferSize, pPtr, sizeof(m_nBufferSize));

    UNUSED(nSize);

    // Allocate memory
    if (m_pBuffer) {
        FREE(m_pBuffer);
       m_pBuffer = NULL;
    }

    m_pBuffer = (uchar*)MALLOC(m_nBufferSize);
    pPtr = Unpack(m_pBuffer, pPtr, m_nBufferSize);

    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Set_Key::PackResponse(uchar** pPacket, uint& nSize) {

    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) {
        return true;
    }

    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) {
        return true;
    }

    pPtr  =  Pack(pPtr,  &m_nBufferSize,   sizeof(m_nBufferSize) );
    pPtr  =  Pack(pPtr,  m_pBuffer,        m_nBufferSize );

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Set_Key::UnpackResponse(const uchar* pPacket, uint nSize) {

    UNUSED(nSize);

    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr) {
        return false;
    }

    pPtr = Unpack(&m_nBufferSize, pPtr, sizeof(m_nBufferSize));
    // Allocate memory
    if (m_pBuffer) {
        FREE(m_pBuffer);
        m_pBuffer = NULL;
    }

    m_pBuffer = (uchar*)MALLOC(m_nBufferSize);
    pPtr      = Unpack(m_pBuffer, pPtr, m_nBufferSize);

    return (pPtr != NULL) ? true : false;
}

// How large is the Challenge packet?
int  Atomic_Enc_Set_Key::GetChallengeBufferSize() {
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Enc_Set_Key::GetResponseBufferSize() {
    return m_nResponseBufferSize;
}

bool Atomic_Enc_Set_Key::SetBuffer(u8* pBuffer, uint nBufferSize) {

    if (m_pBuffer) {
        FREE(m_pBuffer);
        m_pBuffer = NULL;
    }

    m_pBuffer = (u8*)MALLOC(nBufferSize);
    memcpy(m_pBuffer, pBuffer, nBufferSize);
    m_nBufferSize = nBufferSize;

    return true;
}

