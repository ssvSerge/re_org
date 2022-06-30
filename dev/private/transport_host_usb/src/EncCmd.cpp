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
#include "string.h"
#include "EncCmd.h"
#include "EncTypes.h"

// **************************************  CMD_ENC_CAPTURE  ************************************** //
// ICmd
Atomic_Enc_Capture::Atomic_Enc_Capture()
{
    m_nCmd = CMD_ENC_CAPTURE;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    //
    m_Type = CAPTURE_IMAGE;
}
Atomic_Enc_Capture::~Atomic_Enc_Capture()
{

}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Capture::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(_V100_ENC_CAPTURE_TYPE);
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_Type, sizeof(m_Type));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Capture::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_Type, pPtr, sizeof(m_Type));
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Capture::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Capture::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// How large is the Challenge packet?
int  Atomic_Enc_Capture::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Capture::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_DECRYPT  ************************************** //
Atomic_Enc_Decrypt::Atomic_Enc_Decrypt()
{
    m_nCmd = CMD_ENC_DECRYPT;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    //
    m_nDataBufferSize = 0;
    m_pDataBuffer = NULL;
}
Atomic_Enc_Decrypt::~Atomic_Enc_Decrypt()
{
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Decrypt::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nDataBufferSize);
    m_nChallengeBufferSize += m_nDataBufferSize;
    m_nChallengeBufferSize += sizeof(u256);
    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    //
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));

    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Decrypt::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));

    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Decrypt::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += (sizeof(m_nDataBufferSize) + m_nDataBufferSize) + sizeof(u256);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Decrypt::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));

    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_Decrypt::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Decrypt::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_ENROLL  ************************************** //
Atomic_Enc_Enroll::Atomic_Enc_Enroll()
{
    m_nCmd = CMD_ENC_ENROLL;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Enc_Enroll::~Atomic_Enc_Enroll()
{

}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Enroll::PackChallenge(uchar** pPacket, uint& nSize)
{

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Enroll::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Enroll::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Enroll::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// How large is the Challenge packet?
int  Atomic_Enc_Enroll::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Enroll::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_FACTORY_SET_KEY  ************************************** //
Atomic_Enc_Factory_Set_Key::Atomic_Enc_Factory_Set_Key()
{
    m_nCmd = CMD_ENC_FACTORY_SET_KEY;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Enc_Factory_Set_Key::~Atomic_Enc_Factory_Set_Key()
{

}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Factory_Set_Key::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(u256);
    m_nChallengeBufferSize += sizeof(u256);

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;

    pPtr = Pack(pPtr, m_Key, sizeof(u256));
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Factory_Set_Key::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_Key, pPtr, sizeof(u256));
    pPtr = Unpack(&m_DigSig, pPtr, sizeof(u256));

    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Factory_Set_Key::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Factory_Set_Key::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr) return false;
    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_Factory_Set_Key::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Factory_Set_Key::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_IDENTIFYMATCH  ************************************** //
Atomic_Enc_IdentifyMatch::Atomic_Enc_IdentifyMatch()
{
    m_nCmd = CMD_ENC_IDENTIFYMATCH;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    //
    m_nDataBufferSize = 0;
    m_pDataBuffer = NULL;
}
Atomic_Enc_IdentifyMatch::~Atomic_Enc_IdentifyMatch()
{
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_IdentifyMatch::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nDataBufferSize);
    m_nChallengeBufferSize += m_nDataBufferSize;
    m_nChallengeBufferSize += sizeof(u256);
    m_nChallengeBufferSize += sizeof(m_nFMR);
    m_nChallengeBufferSize += sizeof(u256);    // puszAleatoryNumberBB

    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    //
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    pPtr = Pack(pPtr, &m_nFMR, sizeof(m_nFMR));
    pPtr = Pack(pPtr, puszAleatoryNumberBB, sizeof(u256));

    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_IdentifyMatch::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));
    pPtr = Unpack(&m_nFMR, pPtr, sizeof(m_nFMR));
    pPtr = Unpack(puszAleatoryNumberBB, pPtr, sizeof(u256));
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_IdentifyMatch::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += (sizeof(m_nDataBufferSize) + m_nDataBufferSize) + sizeof(int) + sizeof(unsigned long) + sizeof(u256);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, &m_Result, sizeof(m_Result));
    pPtr = Pack(pPtr, &m_nFMR, sizeof(m_nFMR));
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_IdentifyMatch::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(&m_Result, pPtr, sizeof(m_Result));
    pPtr = Unpack(&m_nFMR, pPtr, sizeof(m_nFMR));
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));

    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_IdentifyMatch::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_IdentifyMatch::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_RETURNCAPTUREDBIR  ************************************** //
Atomic_Enc_ReturnCapturedBIR::Atomic_Enc_ReturnCapturedBIR()
{
    m_nCmd = CMD_ENC_RETURNCAPTUREDBIR;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    //
    m_nDataBufferSize = 0;
    m_pDataBuffer = NULL;
}
Atomic_Enc_ReturnCapturedBIR::~Atomic_Enc_ReturnCapturedBIR()
{
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_ReturnCapturedBIR::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nDataBufferSize);
    m_nChallengeBufferSize += m_nDataBufferSize;
    m_nChallengeBufferSize += sizeof(ST_BBXBIO_BIR_HEADER);
    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    //
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, &m_BIRHdr, sizeof(m_BIRHdr));

    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_ReturnCapturedBIR::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(&m_BIRHdr, pPtr, sizeof(m_BIRHdr));
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_ReturnCapturedBIR::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += (sizeof(m_nDataBufferSize) + m_nDataBufferSize) + sizeof(u256);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_ReturnCapturedBIR::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));

    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_ReturnCapturedBIR::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_ReturnCapturedBIR::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_ENC_RETURNCAPTUREDBIR_IM  ************************************** //
Atomic_Enc_ReturnCapturedBIR_IM::Atomic_Enc_ReturnCapturedBIR_IM()
{
    m_nCmd = CMD_ENC_RETURNCAPTUREDBIR_IM;

    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;

    m_nDataBufferSize = 0;
    m_pDataBuffer = NULL;

    m_pImageCGBuffer = NULL;
    m_nImageCGBufferSize = 0;
    memset(m_ImageCGDigSig, 0, sizeof(m_ImageCGDigSig));
}

Atomic_Enc_ReturnCapturedBIR_IM::~Atomic_Enc_ReturnCapturedBIR_IM()
{
    FREE(m_pDataBuffer);
    m_pDataBuffer = NULL;

    FREE(m_pImageCGBuffer);
    m_pImageCGBuffer = NULL;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_ReturnCapturedBIR_IM::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Atomic_Enc_ReturnCapturedBIR
    //     Since there isn't a good way to use subclassing with this
    //     implementation, we'll just do a copy-n-paste job here
    m_nChallengeBufferSize += sizeof(m_nDataBufferSize);
    m_nChallengeBufferSize += m_nDataBufferSize;
    m_nChallengeBufferSize += sizeof(ST_BBXBIO_BIR_HEADER);
    m_nChallengeBufferSize += sizeof(m_nImage_Type);
    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    //
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, &m_BIRHdr, sizeof(m_BIRHdr));
    pPtr = Pack(pPtr, &m_nImage_Type, sizeof(m_nImage_Type));

    // Why even check this?, Pack will only return NULL if pPtr & sizeof(BIRHrd) are both zero!
    //if(NULL == pPtr) return false;

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_ReturnCapturedBIR_IM::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));

    FREE(m_pDataBuffer);
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(&m_BIRHdr, pPtr, sizeof(m_BIRHdr));
    pPtr = Unpack(&m_nImage_Type, pPtr, sizeof(m_nImage_Type));

    // Why even check this?
    //return (pPtr!=NULL)? true:false;
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_ReturnCapturedBIR_IM::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(m_nDataBufferSize);
    m_nResponseBufferSize += m_nDataBufferSize;
    m_nResponseBufferSize += sizeof(m_DigSig);
    m_nResponseBufferSize += sizeof(m_nImageCGBufferSize);
    m_nResponseBufferSize += m_nImageCGBufferSize;
    m_nResponseBufferSize += sizeof(m_ImageCGDigSig);

    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;

    // BIR
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(m_DigSig));

    // Image
    pPtr = Pack(pPtr, &m_nImageCGBufferSize, sizeof(m_nImageCGBufferSize));
    pPtr = Pack(pPtr, m_pImageCGBuffer, m_nImageCGBufferSize);
    pPtr = Pack(pPtr, m_ImageCGDigSig, sizeof(m_ImageCGDigSig));

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_ReturnCapturedBIR_IM::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    // BIR
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    FREE(m_pDataBuffer);
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(m_DigSig));

    // Image
    pPtr = Unpack(&m_nImageCGBufferSize, pPtr, sizeof(m_nImageCGBufferSize));
    FREE(m_pImageCGBuffer);
    m_pImageCGBuffer = (uchar*)MALLOC(m_nImageCGBufferSize);
    pPtr = Unpack(m_pImageCGBuffer, pPtr, m_nImageCGBufferSize);
    pPtr = Unpack(m_ImageCGDigSig, pPtr, sizeof(m_ImageCGDigSig));

    return true;
}

bool Atomic_Enc_ReturnCapturedBIR_IM::SetImageCG(uchar* pImageCG, uint nSize, u256 digSig)
{
    FREE(m_pImageCGBuffer);
    m_pImageCGBuffer = (uchar*)MALLOC(nSize);
    if (m_pImageCGBuffer == NULL)
    {
        return false;
    }

    memcpy(m_pImageCGBuffer, pImageCG, nSize);
    m_nImageCGBufferSize = nSize;

    memcpy(m_ImageCGDigSig, digSig, sizeof(u256));
    return true;
}


// **************************************  CMD_ENC_SET_PARAMETERS  ************************************** //
Atomic_Enc_Set_Parameters::Atomic_Enc_Set_Parameters()
{
    m_nCmd = CMD_ENC_SET_PARAMETERS;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_pSKCG = NULL;
    m_nSKCGSize = 0;
    m_pANSOL = NULL;
    m_nANSOLSize = 0;
    m_nSPL = SPOOF_PROTECT_NONE;
    m_nTimeoutSeconds = 0;
}
Atomic_Enc_Set_Parameters::~Atomic_Enc_Set_Parameters()
{
    if (m_pSKCG) { FREE(m_pSKCG); m_pSKCG = NULL; }
    if (m_pANSOL) { FREE(m_pANSOL); m_pANSOL = NULL; }
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Set_Parameters::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nSKCGSize);    // Size of opaque packet (int)
    m_nChallengeBufferSize += m_nSKCGSize;            // The opaque packet
    m_nChallengeBufferSize += sizeof(m_nANSOLSize);    // Size of opaque packet (int)
    m_nChallengeBufferSize += m_nANSOLSize;            // The opaque packet
    m_nChallengeBufferSize += sizeof(m_nSPL);
    m_nChallengeBufferSize += sizeof(m_nTimeoutSeconds);
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nSKCGSize, sizeof(m_nSKCGSize));
    pPtr = Pack(pPtr, m_pSKCG, m_nSKCGSize);
    pPtr = Pack(pPtr, &m_nANSOLSize, sizeof(m_nANSOLSize));
    pPtr = Pack(pPtr, m_pANSOL, m_nANSOLSize);
    pPtr = Pack(pPtr, &m_nSPL, sizeof(m_nSPL));
    pPtr = Pack(pPtr, &m_nTimeoutSeconds, sizeof(m_nTimeoutSeconds));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Set_Parameters::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSKCGSize, pPtr, sizeof(m_nSKCGSize));
    // Allocate memory
    m_pSKCG = (uchar*)MALLOC(m_nSKCGSize);
    pPtr = Unpack(m_pSKCG, pPtr, m_nSKCGSize);
    pPtr = Unpack(&m_nANSOLSize, pPtr, sizeof(m_nANSOLSize));
    // Allocate memory
    m_pANSOL = (uchar*)MALLOC(m_nANSOLSize);
    pPtr = Unpack(m_pANSOL, pPtr, m_nANSOLSize);
    pPtr = Unpack(&m_nSPL, pPtr, sizeof(m_nSPL));
    pPtr = Unpack(&m_nTimeoutSeconds, pPtr, sizeof(m_nTimeoutSeconds));
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Set_Parameters::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_Set_Parameters::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// How large is the Challenge packet?
int  Atomic_Enc_Set_Parameters::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_Set_Parameters::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_Enc_Set_Parameters::SetParameters(uchar* pSK, int szSK, uchar* pANSOL, int szANSOL,
    _V100_ENC_SPOOF_PROTECTION_LEVEL protLevel, int nTimeoutSeconds)
{
    if (m_pSKCG) { delete m_pSKCG; m_pSKCG = NULL; }
    m_pSKCG = (uchar*)MALLOC(szSK);
    memcpy(m_pSKCG, pSK, szSK);
    if (m_pANSOL) { delete m_pANSOL; m_pANSOL = NULL; }
    m_pANSOL = (uchar*)MALLOC(szANSOL);
    memcpy(m_pANSOL, pANSOL, szANSOL);
    m_nSKCGSize = szSK;
    m_nANSOLSize = szANSOL;
    m_nSPL = protLevel;
    m_nTimeoutSeconds = nTimeoutSeconds;
    return true;
}


// **************************************  CMD_ENC_VERIFYMATCH  ************************************** //
Atomic_Enc_VerifyMatch::Atomic_Enc_VerifyMatch()
{
    m_nCmd = CMD_ENC_VERIFYMATCH;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    //
    m_nDataBufferSize = 0;
    m_pDataBuffer = NULL;
}
Atomic_Enc_VerifyMatch::~Atomic_Enc_VerifyMatch()
{
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_VerifyMatch::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nDataBufferSize);
    m_nChallengeBufferSize += m_nDataBufferSize;
    m_nChallengeBufferSize += sizeof(u256);
    m_nChallengeBufferSize += sizeof(m_nFMR);
    m_nChallengeBufferSize += sizeof(u256);    // puszAleatoryNumberBB

    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    //
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, m_DigSig, sizeof(u256));
    pPtr = Pack(pPtr, &m_nFMR, sizeof(m_nFMR));
    pPtr = Pack(pPtr, puszAleatoryNumberBB, sizeof(u256));

    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_VerifyMatch::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);
    pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));
    pPtr = Unpack(&m_nFMR, pPtr, sizeof(m_nFMR));
    pPtr = Unpack(puszAleatoryNumberBB, pPtr, sizeof(u256));
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_VerifyMatch::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += (sizeof(m_nDataBufferSize) + m_nDataBufferSize) + sizeof(m_Result) + sizeof(m_nFMR);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDataBufferSize, sizeof(m_nDataBufferSize));
    pPtr = Pack(pPtr, m_pDataBuffer, m_nDataBufferSize);
    pPtr = Pack(pPtr, &m_Result, sizeof(m_Result));
    pPtr = Pack(pPtr, &m_nFMR, sizeof(m_nFMR));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Enc_VerifyMatch::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_nDataBufferSize, pPtr, sizeof(m_nDataBufferSize));
    if (m_pDataBuffer) { FREE(m_pDataBuffer); m_pDataBuffer = NULL; }
    m_pDataBuffer = (uchar*)MALLOC(m_nDataBufferSize);
    pPtr = Unpack(m_pDataBuffer, pPtr, m_nDataBufferSize);

    //
    // ##### MAV - 2012Nov21 #####
    //
    //pPtr = Unpack(m_DigSig, pPtr, sizeof(u256));

    pPtr = Unpack(&m_Result, pPtr, sizeof(m_Result));
    pPtr = Unpack(&m_nFMR, pPtr, sizeof(m_nFMR));


    return true;
}
// How large is the Challenge packet?
int  Atomic_Enc_VerifyMatch::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Enc_VerifyMatch::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
