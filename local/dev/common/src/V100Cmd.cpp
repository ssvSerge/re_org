/***************************************************************************************/
// ©Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/

#include "string.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "V100IDCmd.h"

#include <application/stuff.h>

/***************************************************************************************/

Atomic_Get_Config::Atomic_Get_Config()
{
     m_nCmd = CMD_GET_CONFIG;
     memset(&m_conf,0,sizeof(m_conf));
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE + sizeof(uint);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE + sizeof(_V100_INTERFACE_CONFIGURATION_TYPE);
     m_pResponseBuffer = NULL;
}
Atomic_Get_Config::~Atomic_Get_Config()
{
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Config::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

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
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Config::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    if(pPacket == NULL) return false;
    // Lets see what this header contains....
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    memcpy(&m_nSizeOfStruct, pPtr, sizeof(uint));
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Config::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Opaque Data
    memcpy(pPtr,&m_conf,sizeof(_V100_INTERFACE_CONFIGURATION_TYPE));
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Config::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    memcpy(&m_conf, pPtr, sizeof(m_conf));
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Config::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Config::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// ************************************** CMD_ARM_TRIGGER ************************************** //

Atomic_Arm_Trigger::Atomic_Arm_Trigger()
{
     m_nCmd = CMD_ARM_TRIGGER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_nArg = 1;
}
Atomic_Arm_Trigger::~Atomic_Arm_Trigger()
{
}
bool Atomic_Arm_Trigger::GetTriggerType(uint& _Trigger)
{
    _Trigger = (uint)m_nArg;
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Arm_Trigger::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    uchar* pPtr = GenerateChallengeHeader(0,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;
    // Opaque Data
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Arm_Trigger::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    if(pPacket == NULL) return false;
    // Lets see what this header contains....
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Arm_Trigger::PackResponse(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;
    // Opaque Data
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Arm_Trigger::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;
    return true;
}
// How large is the Challenge packet?
int  Atomic_Arm_Trigger::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Arm_Trigger::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_MATCH *************************

Macro_Match::Macro_Match()
{
     m_nCmd = CMD_MATCH;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pGalleryTemplate = NULL;
     m_pProbeTemplate = NULL;
     m_nGalleryTemplateSize = 0;
     m_nProbeTemplateSize = 0;
     m_nScore = 0;

}
Macro_Match::~Macro_Match()
{
    if(m_pGalleryTemplate)
    {
        FREE(m_pGalleryTemplate);
        m_pGalleryTemplate = NULL;
    }
    if(m_pProbeTemplate)
    {
        FREE(m_pProbeTemplate);
        m_pProbeTemplate = NULL;
    }

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_Match::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Calculate Challenge Buffer Size. Envelope + Template + sizeof(TemplateSize)
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += m_nGalleryTemplateSize + sizeof(uint);
    m_nChallengeBufferSize += m_nProbeTemplateSize + sizeof(uint);
    // Create Opaque Data
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Copy size
    memcpy(pPtr, &m_nProbeTemplateSize, sizeof(m_nProbeTemplateSize));
    pPtr+=sizeof(m_nProbeTemplateSize);
    memcpy(pPtr,m_pProbeTemplate, m_nProbeTemplateSize);
    pPtr+=m_nProbeTemplateSize;

    memcpy(pPtr, &m_nGalleryTemplateSize, sizeof(m_nGalleryTemplateSize));
    pPtr+=sizeof(m_nGalleryTemplateSize);
    memcpy(pPtr,m_pGalleryTemplate, m_nGalleryTemplateSize);
    pPtr+=m_nGalleryTemplateSize;

    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Match::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    memcpy(&m_nProbeTemplateSize, pPtr, sizeof(m_nProbeTemplateSize));
    pPtr+=sizeof(m_nProbeTemplateSize);
    if(m_nProbeTemplateSize > nSize) return false; // simple sanity check
    m_pProbeTemplate = (uchar*)MALLOC(m_nProbeTemplateSize);
    memcpy(m_pProbeTemplate, pPtr, m_nProbeTemplateSize);
    pPtr+=m_nProbeTemplateSize;

    memcpy(&m_nGalleryTemplateSize, pPtr, sizeof(m_nGalleryTemplateSize));
    pPtr+=sizeof(m_nGalleryTemplateSize);
    if(m_nGalleryTemplateSize > nSize) return false; // simple sanity check
    m_pGalleryTemplate = (uchar*)MALLOC(m_nGalleryTemplateSize);
    memcpy(m_pGalleryTemplate, pPtr, m_nGalleryTemplateSize);
    pPtr+=m_nGalleryTemplateSize;


    return true;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Match::PackResponse(uchar** pPacket, uint& nSize)
{    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    m_nResponseBufferSize += sizeof(m_nScore);                        // Matching Score
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    memcpy(pPtr, &m_nScore, sizeof(m_nScore));
    pPtr+=sizeof(m_nScore);
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;

    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Match::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    memcpy(&m_nScore, pPtr, sizeof(m_nScore));
    pPtr+=sizeof(m_nScore);
    return true;
}
// How large is the Challenge packet?
int  Macro_Match::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Match::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Macro_Match::SetGalleryTemplateData(uchar* pTemplate, uint nTemplateSize)
{
    if(m_pGalleryTemplate)
    {
        FREE(m_pGalleryTemplate);
        m_pGalleryTemplate = NULL;
    }
    m_pGalleryTemplate = (uchar*)MALLOC(nTemplateSize);
    memcpy(m_pGalleryTemplate, pTemplate, nTemplateSize);
    m_nGalleryTemplateSize = nTemplateSize;

    return true;
}
bool Macro_Match::SetProbeTemplateData(uchar* pTemplate, uint nTemplateSize)
{
    if(m_pProbeTemplate)
    {
        FREE(m_pProbeTemplate);
        m_pProbeTemplate = NULL;
    }
    m_pProbeTemplate = (uchar*)MALLOC(nTemplateSize);
    memcpy(m_pProbeTemplate, pTemplate, nTemplateSize);
    m_nProbeTemplateSize = nTemplateSize;

    return true;
}
uchar*  Macro_Match::GetGalleryTemplate()
{
    return m_pGalleryTemplate;
}
uint Macro_Match::GetGalleryTemplateSize()
{
    return m_nGalleryTemplateSize;
}
uchar*  Macro_Match::GetProbeTemplate()
{
    return m_pProbeTemplate;
}
uint Macro_Match::GetProbeTemplateSize()
{
    return m_nProbeTemplateSize;
}

// VID STREAM

Macro_Vid_Stream::Macro_Vid_Stream()
{
     m_nCmd = CMD_VID_STREAM;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_Vid_Stream::~Macro_Vid_Stream()
{

}
void Macro_Vid_Stream::SetVidStreamMode(_V100_VID_STREAM_MODE mode)
{
    m_nVidStreamMode = mode;

}
void Macro_Vid_Stream::GetVidStreamMode(_V100_VID_STREAM_MODE& mode)
{
    mode = m_nVidStreamMode;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_Vid_Stream::PackChallenge(uchar** pPacket, uint& nSize)
{
    uint nStructSize = sizeof(_V100_VID_STREAM_MODE);
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize+=sizeof(nStructSize );                        // integer holding sizeof _V100...
    m_nChallengeBufferSize+=sizeof(_V100_VID_STREAM_MODE);        // Actual Enumerated type
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &nStructSize , sizeof(nStructSize ));
    pPtr = Pack(pPtr, &m_nVidStreamMode, sizeof(_V100_VID_STREAM_MODE));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Vid_Stream::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&nSize, pPtr, sizeof(nSize));
    pPtr = Unpack(&m_nVidStreamMode, pPtr, sizeof(_V100_VID_STREAM_MODE));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Vid_Stream::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Vid_Stream::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_Vid_Stream::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Vid_Stream::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_ERROR

// CMD_XXX_XXXX *************************

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
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Error::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(pPacket);
    UNUSED(nSize);

    return false;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Error::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(pPacket);
    UNUSED(nSize);

    return false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Error::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr)
        return false;

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Error::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Error::GetChallengeBufferSize()
{
    return 0;
}
// How large is the Response packet?
int  Atomic_Error::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

//**************** CMD_GET_IMAGE *******************/

Atomic_Get_Image::Atomic_Get_Image()
{
     m_nCmd = CMD_GET_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE + sizeof(m_nImage_Type);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     //
     m_pImage = NULL;
     m_nImageSize = 0;
}
Atomic_Get_Image::~Atomic_Get_Image()
{
    if(m_pResponseBuffer == NULL)
    {
        if(m_pImage)
        {
            FREE(m_pImage);
            m_pImage = NULL;
        }
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nImage_Type, sizeof(m_nImage_Type));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(pPacket);
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nImage_Type, pPtr, sizeof(m_nImage_Type));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Image::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize  = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize += sizeof(m_nImage_Type);
    m_nResponseBufferSize += sizeof(m_nImageSize);
    m_nResponseBufferSize += m_nImageSize;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    pPtr = Pack(pPtr, &m_nImage_Type, sizeof(m_nImage_Type));
    pPtr = Pack(pPtr, &m_nImageSize, sizeof(m_nImageSize));
    pPtr = Pack(pPtr, m_pImage, m_nImageSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Image Type
    pPtr = Unpack(&m_nImage_Type,pPtr, sizeof(m_nImage_Type));
        if(NULL == pPtr)
           return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nImageSize,pPtr, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Allocate Image Area
    m_pImage = (uchar*)MALLOC(m_nImageSize);
    pPtr = Unpack(m_pImage, pPtr, m_nImageSize);
        if(NULL == pPtr)
           return false;


    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_Get_Image::SetImageType(_V100_IMAGE_TYPE type)
{
     m_nImage_Type = type;
     return true;
}
bool Atomic_Get_Image::GetImageType(_V100_IMAGE_TYPE& type)
{
     type = m_nImage_Type;
    return true;
}
// Set/Get Image Buffer
bool Atomic_Get_Image::SetImageMetrics(uchar* pImage, uint nImageSize)
{
     m_pImage = pImage;
     m_nImageSize = nImageSize;
     return true;
}
bool Atomic_Get_Image::GetImageMetrics(uchar** pImage, uint& nImageSize)
{
     *pImage = m_pImage;
     nImageSize = m_nImageSize;
     return true;
}

Atomic_Set_Image::Atomic_Set_Image()
{
     m_nCmd = CMD_SET_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pImage = NULL;
}
Atomic_Set_Image::~Atomic_Set_Image()
{
    if(m_pChallengeBuffer == NULL)
    {
        if(m_pImage)
        {
            FREE(m_pImage);
        }
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    
    // Image Size
    m_nChallengeBufferSize += sizeof(m_nImageSize);
    // Image Data
    m_nChallengeBufferSize += m_nImageSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Generate Response Header
    // Pack Image Size
    pPtr = Pack(pPtr, &m_nImageSize, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Pack Image
    pPtr = Pack(pPtr, m_pImage, m_nImageSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Fill in some stuff
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nImageSize, pPtr, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Allocate space, and Unpack Image
    m_pImage = (uchar*)MALLOC(m_nImageSize);
    pPtr = Unpack(m_pImage, pPtr, m_nImageSize);
        if(NULL == pPtr)
           return false;


    return (pPtr !=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Image::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}




Atomic_Get_Composite_Image::Atomic_Get_Composite_Image()
{
     m_nCmd = CMD_GET_COMPOSITE_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pImage = NULL;

}
Atomic_Get_Composite_Image::~Atomic_Get_Composite_Image()
{
    if(m_pImage)
    {
        FREE(m_pImage);
    }
}
void Atomic_Get_Composite_Image::SetImage(uchar* _Image, uint _ImageSize)
{
    if(m_pImage)
    {
        FREE(m_pImage);
        m_pImage = NULL;
    }
    m_pImage = (uchar*)MALLOC(_ImageSize);
    memcpy(m_pImage, _Image, _ImageSize);
    m_nImageSize = _ImageSize;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Composite_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Composite_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Composite_Image::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize += sizeof(m_nSpoofValue);
    // Image Size
    m_nResponseBufferSize += sizeof(m_nImageSize);
    // Image Data
    m_nResponseBufferSize += m_nImageSize;
    // Generate Response Header
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack Image Size
    pPtr = Pack(pPtr, &m_nSpoofValue, sizeof(m_nSpoofValue));
        if(NULL == pPtr)
           return false;

    // Pack Spoof
    pPtr = Pack(pPtr, &m_nImageSize, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Pack Image
    pPtr = Pack(pPtr, m_pImage, m_nImageSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Composite_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Spoof
    pPtr = Unpack(&m_nSpoofValue, pPtr, sizeof(m_nSpoofValue));
        if(NULL == pPtr)
           return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nImageSize, pPtr, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Allocate space, and Unpack Image
    m_pImage = (uchar*)MALLOC(m_nImageSize);
    pPtr = Unpack(m_pImage, pPtr, m_nImageSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Composite_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Composite_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Set_Composite_Image::Atomic_Set_Composite_Image()
{
     m_nCmd = CMD_SET_COMPOSITE_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pImage = NULL;
}
Atomic_Set_Composite_Image::~Atomic_Set_Composite_Image()
{
    if(m_pChallengeBuffer == NULL)
    {
        if(m_pImage)
        {
            FREE(m_pImage);
        }
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Composite_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Image Size
    m_nChallengeBufferSize += sizeof(m_nImageSize);
    // Image Data
    m_nChallengeBufferSize += m_nImageSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Generate Response Header
    // Pack Image Size
    pPtr = Pack(pPtr, &m_nImageSize, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Pack Image
    pPtr = Pack(pPtr, m_pImage, m_nImageSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Fill in some stuff
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Composite_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nImageSize, pPtr, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    // Allocate space, and Unpack Image
    m_pImage = (uchar*)MALLOC(m_nImageSize);
    pPtr = Unpack(m_pImage, pPtr, m_nImageSize);
        if(NULL == pPtr)
           return false;


    return (pPtr !=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Composite_Image::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Composite_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Composite_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Composite_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Get_Template::Atomic_Get_Template()
{
     m_nCmd = CMD_GET_TEMPLATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pTemplate = NULL;
     m_nTemplateSize = 0;
}
Atomic_Get_Template::~Atomic_Get_Template()
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
}
// ICmd
void Atomic_Get_Template::SetTemplate(uchar* pTemplate, uint nTemplateSize)
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
    m_pTemplate = (uchar*)MALLOC(nTemplateSize);
    memcpy(m_pTemplate, pTemplate, nTemplateSize);
    m_nTemplateSize = nTemplateSize;
}
uchar*    Atomic_Get_Template::GetTemplate(uint& nTemplateSize)
{
    nTemplateSize = m_nTemplateSize;
    return m_pTemplate;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Template::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Template::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Template::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Calculate Response Buffer Size.
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nTemplateSize);
    m_nResponseBufferSize+=m_nTemplateSize;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Populate Response Buffer
    pPtr = Pack(pPtr, &m_nTemplateSize, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplate, m_nTemplateSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Template::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
        if(NULL == pPtr)
           return false;


    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Template::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Template::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Set_Template::Atomic_Set_Template()
{
     m_nCmd = CMD_SET_TEMPLATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pTemplate = NULL;
     m_nTemplateSize = 0;
}
Atomic_Set_Template::~Atomic_Set_Template()
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
}
void Atomic_Set_Template::SetTemplate(uchar* pTemplate, uint nTemplateSize)
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
    m_pTemplate = (uchar*)MALLOC(nTemplateSize);
    memcpy(m_pTemplate, pTemplate, nTemplateSize);
    m_nTemplateSize = nTemplateSize;
}
uchar*    Atomic_Set_Template::GetTemplate(uint& nTemplateSize)
{
    nTemplateSize = m_nTemplateSize;
    return m_pTemplate;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Template::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_nTemplateSize);
    m_nChallengeBufferSize+=m_nTemplateSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    pPtr = Pack(pPtr, &m_nTemplateSize, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr,  m_pTemplate, m_nTemplateSize);
        if(NULL == pPtr)
           return false;

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Template::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    if(m_pTemplate)
    {
        FREE(m_pTemplate);
    }
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Template::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Template::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Template::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Template::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
/***************** CMD_ACQ_STATUS *********************/
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
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Acq_Status::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Acq_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Acq_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    m_nResponseBufferSize += sizeof(_V100_ACQ_STATUS_TYPE);            // Matching Score
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    memcpy(pPtr, &m_ACQStatus, sizeof(m_ACQStatus));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Acq_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    memcpy(&m_ACQStatus, pPtr, sizeof(m_ACQStatus));
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Acq_Status::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Acq_Status::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
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
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nStatusSize);
    m_nResponseBufferSize+=sizeof(_V100_INTERFACE_STATUS_TYPE);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nStatusSize, sizeof(m_nStatusSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nIST, m_nStatusSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nStatusSize, pPtr, sizeof(m_nStatusSize));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nIST, pPtr, sizeof(_V100_INTERFACE_STATUS_TYPE));
        if(NULL == pPtr)
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
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Cmd::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Cmd::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
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
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack size of structure
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    // Pack Structure
    pPtr = Pack(pPtr, &m_nICT, m_nSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Cmd::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    if( m_nSize != sizeof(m_nICT) ) return false;
    pPtr = Unpack(&m_nICT, pPtr, m_nSize);
        if(NULL == pPtr)
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
Atomic_Set_Cmd::Atomic_Set_Cmd()
{
     m_nCmd = CMD_SET_CMD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_nChallengeBufferSize += sizeof(m_nSize);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Set_Cmd::~Atomic_Set_Cmd()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Cmd::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += m_nSize;
    uchar* pPtr= GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack Size
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    // Pack Structure
    pPtr = Pack(pPtr, &m_nICT, m_nSize);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Cmd::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    // Unpack Structure
    pPtr = Unpack(&m_nICT, pPtr, m_nSize);
    //
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Cmd::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Cmd::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Cmd::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Cmd::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_Set_LED::Atomic_Set_LED()
{
     m_nCmd = CMD_SET_LED;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_nLEDControlSize = sizeof(m_nV100_LED_CONTROL);
}
Atomic_Set_LED::~Atomic_Set_LED()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_LED::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize =  ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_nLEDControlSize);
    m_nChallengeBufferSize += sizeof(m_nV100_LED_CONTROL);

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nLEDControlSize, sizeof(m_nLEDControlSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nV100_LED_CONTROL, sizeof(m_nV100_LED_CONTROL));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_LED::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nLEDControlSize, pPtr, sizeof(m_nLEDControlSize));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nV100_LED_CONTROL, pPtr, sizeof(m_nV100_LED_CONTROL));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_LED::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_LED::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_LED::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_LED::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}




Atomic_Config_Comport::Atomic_Config_Comport()
{
     m_nCmd = CMD_CONFIG_COMPORT;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_nBaudSize = sizeof(m_nBaudRate);
}
Atomic_Config_Comport::~Atomic_Config_Comport()
{

}
bool Atomic_Config_Comport::SetBaudRate(uint  nBaudRate)
{
    m_nBaudRate = nBaudRate;
    return true;
}
bool Atomic_Config_Comport::GetBaudRate(uint& nBaudRate)
{
    nBaudRate = m_nBaudRate;
    return true;
}

// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Config_Comport::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    m_nChallengeBufferSize+=sizeof(m_nBaudSize);
    m_nChallengeBufferSize+=m_nBaudSize;

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nBaudSize, sizeof(m_nBaudSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nBaudRate, m_nBaudSize);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Config_Comport::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nBaudSize, pPtr, sizeof(m_nBaudSize));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nBaudRate, pPtr, m_nBaudSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Config_Comport::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Config_Comport::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Config_Comport::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Config_Comport::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
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
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Reset::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Reset::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Reset::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
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

// CMD_SET_OPTION *************************

Atomic_Set_Option::Atomic_Set_Option()
{
     m_nCmd = CMD_SET_OPTION;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_pOptData = NULL;
}
Atomic_Set_Option::~Atomic_Set_Option()
{
    if(m_pOptData){
        FREE(m_pOptData);
        m_pOptData = NULL;
    }
}
// ICmd
bool Atomic_Set_Option::SetOption(_V100_OPTION_TYPE OptType, uchar* pOptData, uint nSizeOptData)
{
    m_nArg = (short)OptType;
    if(m_pOptData){
        FREE(m_pOptData);
        m_pOptData = NULL;
    }
    m_pOptData = (uchar*)MALLOC(nSizeOptData);
    memcpy(m_pOptData, pOptData, nSizeOptData);
    m_nSizeOptData = nSizeOptData;

    return true;
}
bool Atomic_Set_Option::GetOption(_V100_OPTION_TYPE& OptType, uchar** pOptData, uint& nSizeOptData)
{
    OptType = (_V100_OPTION_TYPE)m_nArg;
    *pOptData = m_pOptData;
    nSizeOptData = m_nSizeOptData;

    return true;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Option::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize+=sizeof(m_nSizeOptData);
    m_nChallengeBufferSize+=m_nSizeOptData;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nSizeOptData, sizeof(m_nSizeOptData));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pOptData, m_nSizeOptData);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Option::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSizeOptData, pPtr, sizeof(m_nSizeOptData));
        if(NULL == pPtr)
           return false;

    if(m_pOptData){
        FREE(m_pOptData);
        m_pOptData = NULL;
    }
    m_pOptData = (uchar*)MALLOC(m_nSizeOptData);
    pPtr = Unpack(m_pOptData, pPtr, m_nSizeOptData);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Option::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Option::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Option::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Option::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// CMD_MATCH_EX *************************

Atomic_Match_Ex::Atomic_Match_Ex()
{
     m_nCmd = CMD_MATCH_EX;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     //
     m_pProbeTemplate        = NULL;
     m_nSzProbeTemplate        = 0;
     m_pGalleryTemplate        = NULL;
     m_nSzGalleryTemplate    = 0;
     m_nSpoofScore            = 0;
     m_nMatchScore            = 0;
}
Atomic_Match_Ex::~Atomic_Match_Ex()
{
    if(m_pProbeTemplate)
    {
        FREE(m_pProbeTemplate);
        m_pProbeTemplate = NULL;
    }
    if(m_pGalleryTemplate)
    {
        FREE(m_pGalleryTemplate);
        m_pGalleryTemplate = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Match_Ex::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Create Opaque Data
    if( m_pProbeTemplate )
    {
        m_nArg++;
        m_nChallengeBufferSize += m_nSzProbeTemplate + sizeof(uint);
    }
    if( m_pGalleryTemplate )
    {
        m_nArg++;
        m_nChallengeBufferSize += m_nSzGalleryTemplate + sizeof(uint);
    }

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Copy size

    pPtr = Pack(pPtr, &m_nSzProbeTemplate, sizeof(m_nSzProbeTemplate));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr,m_pProbeTemplate, m_nSzProbeTemplate);
        if(NULL == pPtr)
           return false;


    if( m_pGalleryTemplate == NULL) return true;

    pPtr = Pack(pPtr, &m_nSzGalleryTemplate, sizeof(m_nSzGalleryTemplate));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr,m_pGalleryTemplate, m_nSzGalleryTemplate);
        if(NULL == pPtr)
           return false;


    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Match_Ex::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;


    pPtr = Unpack(&m_nSzProbeTemplate, pPtr, sizeof(m_nSzProbeTemplate));
        if(NULL == pPtr)
           return false;

    if(m_nSzProbeTemplate > nSize) return false; // simple sanity check
    m_pProbeTemplate = (uchar*)MALLOC(m_nSzProbeTemplate);
    pPtr = Unpack(m_pProbeTemplate, pPtr, m_nSzProbeTemplate);
        if(NULL == pPtr)
           return false;

    // If this is the only template, return
    if(m_nArg <= 1) return true;
    // Unpack Challenge
    pPtr = Unpack(&m_nSzGalleryTemplate, pPtr, sizeof(m_nSzGalleryTemplate));
        if(NULL == pPtr)
           return false;

    if(m_nSzGalleryTemplate > nSize) return false; // simple sanity check
    m_pGalleryTemplate = (uchar*)MALLOC(m_nSzGalleryTemplate);
    pPtr = Unpack(m_pGalleryTemplate, pPtr, m_nSzGalleryTemplate);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Match_Ex::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Response
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    m_nResponseBufferSize += sizeof(m_nMatchScore);                        // Matching Score
    m_nResponseBufferSize += sizeof(m_nSpoofScore);
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nMatchScore, sizeof(m_nMatchScore));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nSpoofScore, sizeof(m_nMatchScore));
        if(NULL == pPtr)
           return false;

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Match_Ex::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nMatchScore, pPtr, sizeof(m_nMatchScore));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSpoofScore, pPtr, sizeof(m_nSpoofScore));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Match_Ex::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Match_Ex::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// Client
bool Atomic_Match_Ex::SetProbeTemplate(uchar* pTpl, uint Size)
{
    if( m_pProbeTemplate != NULL )
    {
        FREE(m_pProbeTemplate);
        m_pProbeTemplate = NULL;
    }
    m_pProbeTemplate = (uchar*)MALLOC(Size);
    memcpy(m_pProbeTemplate, pTpl, Size);
    m_nSzProbeTemplate = Size;
    return true;
}
bool Atomic_Match_Ex::SetGalleryTemplate(uchar* pTpl, uint Size)
{
    if( m_pGalleryTemplate != NULL )
    {
        FREE(m_pGalleryTemplate);
        m_pGalleryTemplate = NULL;
    }
    m_pGalleryTemplate = (uchar*)MALLOC(Size);
    memcpy(m_pGalleryTemplate, pTpl, Size);
    m_nSzGalleryTemplate = Size;
    return true;
}
// Host
uchar* Atomic_Match_Ex::GetProbeTemplate()
{
    return m_pProbeTemplate;
}
uchar* Atomic_Match_Ex::GetGalleryTemplate()
{
    return m_pGalleryTemplate;
}

// CMD_SPOOF_GET_TEMPLATE *************************

Atomic_Spoof_Get_Template::Atomic_Spoof_Get_Template()
{
     m_nCmd = CMD_SPOOF_GET_TEMPLATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_nArg = -1;

     m_pData                    = NULL;
     m_nDataSize                = 0;
     m_pSpoofTemplate            = NULL;
     m_nSpoofTemplateSize       = 0;
}
Atomic_Spoof_Get_Template::~Atomic_Spoof_Get_Template()
{
    if(m_pData)
    {
        FREE(m_pData);
        m_pData = NULL;
    }
    if(m_pSpoofTemplate)
    {
        FREE(m_pSpoofTemplate);
        m_pSpoofTemplate = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Spoof_Get_Template::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Calculate Challenge Buffer Size. Envelope + Template + sizeof(TemplateSize)
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    // If we are sending data in, add that to buffer
    if(m_nArg != 0)
    {
        m_nChallengeBufferSize += sizeof(m_nDataSize);
        m_nChallengeBufferSize += m_nDataSize;
    }
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Pack appropriate data.
    switch(m_nArg)
    {
        case 0:
        {
        } break;
        case 1:
        case 2:
        {
            pPtr = Pack(pPtr, &m_nDataSize, sizeof(m_nDataSize));
                        if(NULL == pPtr)
                            return false;

            pPtr = Pack(pPtr, m_pData, m_nDataSize);
                        if(NULL == pPtr)
                            return false;

        } break;
    }
    nSize = m_nChallengeBufferSize;
    *pPacket = m_pChallengeBuffer;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Spoof_Get_Template::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    if(m_nArg != 0)
    {
        pPtr = Unpack(&m_nDataSize, pPtr, sizeof(m_nDataSize));
                if(NULL == pPtr)
                    return false;

        m_pData = (uchar*)MALLOC(m_nDataSize);
        pPtr = Unpack(m_pData, pPtr, m_nDataSize);
    }
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Spoof_Get_Template::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    m_nResponseBufferSize += sizeof(m_nSpoofTemplateSize);                        // Matching Score
    m_nResponseBufferSize += m_nSpoofTemplateSize;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nSpoofTemplateSize, sizeof(m_nSpoofTemplateSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pSpoofTemplate, m_nSpoofTemplateSize);
        if(NULL == pPtr)
           return false;

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Spoof_Get_Template::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSpoofTemplateSize, pPtr, sizeof(m_nSpoofTemplateSize));
        if(NULL == pPtr)
           return false;

    m_pSpoofTemplate = (uchar*)MALLOC(m_nSpoofTemplateSize);
    pPtr = Unpack(m_pSpoofTemplate, pPtr, m_nSpoofTemplateSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Spoof_Get_Template::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Spoof_Get_Template::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// Set Data
bool   Atomic_Spoof_Get_Template::SetData(uchar* pData, uint nDataSize)
{
    if(m_pData)
    {
        FREE(m_pData);
        m_pData = NULL;
    }
    m_pData = (uchar*)MALLOC(nDataSize);
    memcpy(m_pData, pData, nDataSize);
    m_nDataSize = nDataSize;
    return true;
}
// Get Data
uchar* Atomic_Spoof_Get_Template::GetData()
{
    return m_pData;
}
// Get Data Size
uint   Atomic_Spoof_Get_Template::GetDataSize()
{
    return m_nDataSize;
}
// Get Spoof Template
uchar* Atomic_Spoof_Get_Template::GetSpoofTemplate()
{
    return m_pSpoofTemplate;
}
// Get Spoof Template size
uint   Atomic_Spoof_Get_Template::GetSpoofTemplateSize()
{
    return m_nSpoofTemplateSize;
}
// Set Spoof Template
bool   Atomic_Spoof_Get_Template::SetSpoofTemplate(uchar* pData, uint nDataSize)
{
    if(m_pSpoofTemplate)
    {
        FREE(m_pSpoofTemplate);
        m_pSpoofTemplate = NULL;
    }
    m_pSpoofTemplate = (uchar*)MALLOC(nDataSize);
    memcpy(m_pSpoofTemplate, pData, nDataSize);
    m_nSpoofTemplateSize = nDataSize;
    return true;
}

// CMD_XXX_XXXX *************************

Atomic_Format_DB::Atomic_Format_DB()
{
     m_nCmd = CMD_FORMAT_DB;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Format_DB::~Atomic_Format_DB()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Format_DB::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Format_DB::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Format_DB::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Format_DB::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Format_DB::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Format_DB::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

//

// CMD_XXX_XXXX *************************

Atomic_Get_DB_Metrics::Atomic_Get_DB_Metrics()
{
     m_nCmd = CMD_GET_DB_METRICS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_DB_Metrics::~Atomic_Get_DB_Metrics()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_DB_Metrics::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_DB_Metrics::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_DB_Metrics::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    m_nResponseBufferSize+=sizeof(_V100_DB_METRICS);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_DBMetrics, sizeof(m_DBMetrics));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_DB_Metrics::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_DBMetrics, pPtr, sizeof(m_DBMetrics));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_DB_Metrics::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_DB_Metrics::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

//
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
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_OP_Status::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_OP_Status::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    m_nResponseBufferSize += sizeof(_V100_OP_STATUS);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr,&m_OPStatus,sizeof(m_OPStatus));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_OP_Status::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_OPStatus, pPtr,sizeof(m_OPStatus));
        if(NULL == pPtr)
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
// CMD_XXX_XXXX *************************


// CMD_SET_TAG *************************

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
    if(m_pTag)
    {
        FREE(m_pTag);
        m_pTag = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Tag::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_nTagSize);
    m_nChallengeBufferSize+=m_nTagSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nTagSize, sizeof(m_nTagSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTag, m_nTagSize);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Tag::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTagSize, pPtr, sizeof(m_nTagSize));
        if(NULL == pPtr)
           return false;

    m_pTag = (char*)MALLOC(m_nTagSize);
    pPtr = Unpack(m_pTag, pPtr, m_nTagSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Tag::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Tag::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
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
bool Atomic_Set_Tag::SetTag(char* pTag, ushort& nSize)
{
    if(m_pTag == NULL)
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
// CMD_GET_TAG

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
    if(m_pTag)
    {
        FREE(m_pTag);
        m_pTag = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Tag::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Tag::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Tag::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize+=sizeof(m_nTagSize);
    m_nResponseBufferSize+=m_nTagSize;
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nTagSize, sizeof(m_nTagSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTag, m_nTagSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Tag::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTagSize, pPtr, sizeof(m_nTagSize));
        if(NULL == pPtr)
           return false;

    m_pTag = (char*)MALLOC(m_nTagSize);
    pPtr = Unpack(m_pTag, pPtr, m_nTagSize);
        if(NULL == pPtr)
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
    if(m_pTag == NULL)
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
// CMD_XXX_XXXX *************************


Atomic_Truncate_378::Atomic_Truncate_378()
{
     m_nCmd = CMD_TRUNCATE_378;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // Init vars
    m_pTemplate                = NULL;
    m_nTemplateSize            = 0;
    m_nTemplateMaxSize        = 0;
    m_nTemplateActualSize    = 0;
    //
}
Atomic_Truncate_378::~Atomic_Truncate_378()
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Truncate_378::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_nTemplateMaxSize);
    m_nChallengeBufferSize+=sizeof(m_nTemplateSize);
    m_nChallengeBufferSize+=m_nTemplateSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    //
    pPtr = Pack(pPtr, &m_nTemplateMaxSize, sizeof(m_nTemplateMaxSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nTemplateSize, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplate, m_nTemplateSize);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Truncate_378::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateMaxSize, pPtr, sizeof(m_nTemplateMaxSize));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Truncate_378::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize+=sizeof(m_nTemplateSize);
    m_nResponseBufferSize+=m_nTemplateSize;
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nTemplateSize, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplate, m_nTemplateSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Truncate_378::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    if(m_pTemplate) FREE(m_pTemplate); m_pTemplate = NULL;
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Truncate_378::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Truncate_378::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_Truncate_378::GetMaxTemplateSize(uint& nSizeToTruncate)
{
    nSizeToTruncate = m_nTemplateMaxSize;
    return true;
}

bool Atomic_Truncate_378::SetMaxTemplateSize(uint nSizeToTruncate)
{
    m_nTemplateMaxSize = nSizeToTruncate;
    return true;
}
bool Atomic_Truncate_378::SetTemplate(const uchar* pTemplate, uint nTemplateSize)
{
    m_nTemplateSize = nTemplateSize;
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
    m_pTemplate = (uchar*)MALLOC(nTemplateSize);
    memcpy(m_pTemplate, pTemplate, nTemplateSize);
    return true;
}
bool Atomic_Truncate_378::GetTemplate(uchar* pTemplate, uint& nTemplateSize)
{
    nTemplateSize = m_nTemplateSize;
    memcpy(pTemplate, m_pTemplate, m_nTemplateSize);
    return true;
}

// Set GPIO

// CMD_XXX_XXXX *************************

Atomic_Set_GPIO::Atomic_Set_GPIO()
{
     m_nCmd = CMD_SET_GPIO;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Set_GPIO::~Atomic_Set_GPIO()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_GPIO::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_Mask);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_Mask, sizeof(m_Mask));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_GPIO::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_Mask, pPtr, sizeof(m_Mask));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_GPIO::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_GPIO::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_GPIO::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_GPIO::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// Get GPIO

Atomic_Get_GPIO::Atomic_Get_GPIO()
{
     m_nCmd = CMD_GET_GPIO;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_GPIO::~Atomic_Get_GPIO()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_GPIO::PackChallenge(uchar** pPacket, uint& nSize)
{
    GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_GPIO::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_Mask, pPtr, sizeof(m_Mask));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_GPIO::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    m_nResponseBufferSize+=sizeof(uchar);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_Mask, sizeof(uchar));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_GPIO::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_Mask, pPtr, sizeof(uchar));
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_GPIO::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_GPIO::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/*
** Cancel Operation
*/

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
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Cancel_Operation::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Cancel_Operation::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Cancel_Operation::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Cancel_Operation::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Cancel_Operation::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Cancel_Operation::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_FILE_DELETE
Atomic_File_Delete::Atomic_File_Delete()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_DELETE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_Delete::~Atomic_File_Delete()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Delete::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_szFileLength);
    m_nChallengeBufferSize += m_szFileLength;
    m_nChallengeBufferSize += sizeof(Attribute);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
    pPtr = Pack(pPtr, &Attribute, sizeof(Attribute));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_File_Delete::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
    pPtr = Unpack(&Attribute, pPtr, sizeof(Attribute));
    return (pPtr!=NULL)? true:false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_File_Delete::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_File_Delete::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Delete::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Delete::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// SPEC
char* Atomic_File_Delete::GetName()
{
    return (char*)m_pFileName;
}
uint* Atomic_File_Delete::GetSize()
{
    return &m_szFileLength;
}
_V100_FILE_ATTR Atomic_File_Delete::GetAttr()
{
    return Attribute;
}
bool Atomic_File_Delete::SetName(char* pObjectName, uint nObjectLength, _V100_FILE_ATTR attr)
{
    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
    memcpy(m_pFileName, pObjectName, nObjectLength);
    m_pFileName[nObjectLength] = 0;
    m_szFileLength = nObjectLength;
    Attribute = attr;
    return true;
}


// Atomic_Get_FIR_Image
Atomic_Get_FIR_Image::Atomic_Get_FIR_Image()
{
     m_nCmd = CMD_GET_FIR_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_nFIRType = LUMI_FIR_LAST;
     m_nFingerType = LUMI_FINGER_POSITION_UNKNOWN_FINGER;
     m_nFIRImageSz = 0;
     m_FIRImage = NULL;

}
Atomic_Get_FIR_Image::~Atomic_Get_FIR_Image()
{
    if(m_FIRImage)
    {
        FREE(m_FIRImage); m_FIRImage = NULL;
    }
}

 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_FIR_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    m_nChallengeBufferSize += sizeof(m_nFIRType);
    m_nChallengeBufferSize += sizeof(m_nFingerType);

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nFIRType, sizeof(m_nFIRType));
    pPtr = Pack(pPtr, &m_nFingerType, sizeof(m_nFingerType));

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_FIR_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nFIRType, pPtr, sizeof(m_nFIRType));
    pPtr = Unpack(&m_nFingerType, pPtr, sizeof(m_nFingerType));

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_FIR_Image::PackResponse(uchar** pPacket, uint& nSize)
{

    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;

    m_nResponseBufferSize += sizeof(m_nFIRImageSz);
    m_nResponseBufferSize += m_nFIRImageSz;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nFIRImageSz, sizeof(m_nFIRImageSz));
    pPtr = Pack(pPtr, m_FIRImage, m_nFIRImageSz);

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_FIR_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nFIRImageSz,pPtr, sizeof(m_nFIRImageSz));
    if(m_FIRImage)
    {
        FREE(m_FIRImage); m_FIRImage = NULL;
    }
    m_FIRImage = (uchar*)MALLOC(m_nFIRImageSz);
    pPtr = Unpack(m_FIRImage,pPtr, m_nFIRImageSz);

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_FIR_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_FIR_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool Atomic_Get_FIR_Image::SetFIRType(_V100_FIR_RECORD_TYPE FIRType)
{
    if(FIRType >= LUMI_FIR_LAST)
        return false;

    m_nFIRType = FIRType;
    return true;
}

bool Atomic_Get_FIR_Image::SetFingerType(_V100_FINGER_PALM_POSITION FingerType)
{
    if(FingerType >= LUMI_FINGER_PALM_POSITION_LAST )
        return false;

    m_nFingerType = FingerType;
    return true;
}
bool Atomic_Get_FIR_Image::SetFIRImage(uchar* pFIRImage, uint nFIRImageSz)
{
    if(m_FIRImage)
    {
        FREE(m_FIRImage); m_FIRImage = NULL;
    }

    m_FIRImage =(uchar*)MALLOC(nFIRImageSz);
    memcpy(m_FIRImage, pFIRImage, nFIRImageSz);

    m_nFIRImageSz = nFIRImageSz;
    return true;
}


// **************************************  CMD_SET_LICENSE_KEY **************************************
// SET_LICENSE
// ICmd
Atomic_Set_License_Key::Atomic_Set_License_Key()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_LICENSE_KEY;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     //
     m_nSizeLK = sizeof(_V100_DEVICE_POLICY);
}
Atomic_Set_License_Key::~Atomic_Set_License_Key()
{

}
// Set License Key
bool Atomic_Set_License_Key::SetLicenseKey(_V100_DEVICE_POLICY* pLC)
{
    memcpy(&m_LK, pLC, sizeof(m_LK));
    return true;
}
// Get License Key
bool Atomic_Set_License_Key::GetLicenseKey(_V100_DEVICE_POLICY* pLC)
{
    memcpy(pLC,&m_LK,sizeof(m_LK));
    return true;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_License_Key::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize+=sizeof(m_nSizeLK);
    m_nChallengeBufferSize+=m_nSizeLK;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nSizeLK, sizeof(m_nSizeLK));
    pPtr = Pack(pPtr, &m_LK, m_nSizeLK);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_License_Key::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSizeLK, pPtr, sizeof(m_nSizeLK));
    pPtr = Unpack(&m_LK, pPtr, sizeof(m_LK));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_License_Key::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_License_Key::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_License_Key::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_License_Key::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// **************************************  CMD_SET_RECORD **************************************

Atomic_Set_Record::Atomic_Set_Record()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_Record_Type = RECORD_TYPE_NONE;
     m_nRecordSize = 0;
     m_pRecordData = NULL;
}
Atomic_Set_Record::~Atomic_Set_Record()
{
    if(m_pRecordData)
    {
        FREE(m_pRecordData);
    }
    m_pRecordData = NULL;
}
bool Atomic_Set_Record::SetRecord(_V100_RECORD_TYPE type, uint nRecordSize, uchar* pRecordData)
{
    m_Record_Type = type;
    m_nRecordSize = nRecordSize;
    m_pRecordData = (uchar*)MALLOC(m_nRecordSize);
    memcpy(m_pRecordData, pRecordData, m_nRecordSize);
    return true;
}
bool Atomic_Set_Record::GetRecord(_V100_RECORD_TYPE& type, uint& nRecordSize, uchar** pRecordData)
{
    type = m_Record_Type;
    nRecordSize = m_nRecordSize;
    *pRecordData = m_pRecordData;
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize+=sizeof(m_Record_Type);
    m_nChallengeBufferSize+=sizeof(m_nRecordSize);
    m_nChallengeBufferSize+=m_nRecordSize;

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    pPtr = Pack(pPtr, &m_Record_Type, sizeof(m_Record_Type));
    pPtr = Pack(pPtr, &m_nRecordSize, sizeof(m_nRecordSize));
    pPtr = Pack(pPtr, m_pRecordData, m_nRecordSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_Record_Type, pPtr, sizeof(m_Record_Type));
    pPtr = Unpack(&m_nRecordSize, pPtr, sizeof(m_nRecordSize));
    //
    m_pRecordData = (uchar*)MALLOC(m_nRecordSize);
    pPtr = Unpack(m_pRecordData, pPtr, m_nRecordSize);

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// **************************************  CMD_SET_RECORD **************************************

Atomic_Write_Flash::Atomic_Write_Flash()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_WRITE_FLASH;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_eWhat_To_Flash = RECORD_TYPE_NONE;
     m_nSizeRecordType = sizeof(m_eWhat_To_Flash);
}
Atomic_Write_Flash::~Atomic_Write_Flash()
{

}
void Atomic_Write_Flash::SetFlashType(_V100_RECORD_TYPE ft)
{
    m_eWhat_To_Flash = ft;
}
void Atomic_Write_Flash::GetFlashType(_V100_RECORD_TYPE& ft)
{
    ft = m_eWhat_To_Flash;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Write_Flash::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize  = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += m_nSizeRecordType;
    m_nChallengeBufferSize += sizeof(m_eWhat_To_Flash);

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr,&m_nSizeRecordType, sizeof(m_nSizeRecordType));
    pPtr = Pack(pPtr,&m_eWhat_To_Flash, m_nSizeRecordType);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Write_Flash::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSizeRecordType, pPtr, sizeof(m_nSizeRecordType));
    pPtr = Unpack(&m_eWhat_To_Flash, pPtr, m_nSizeRecordType);

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Write_Flash::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Write_Flash::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Write_Flash::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Write_Flash::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// **************************************  CMD_GET_SERIAL **************************************

Atomic_Get_Serial::Atomic_Get_Serial()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_GET_SERIAL_NUMBER;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nSize);
    m_nResponseBufferSize+=sizeof(m_nSerialNumber);
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
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Serial::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Serial::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nSerialNumber, m_nSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Serial::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSerialNumber, pPtr, m_nSize);
        if(NULL == pPtr)
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

Atomic_Clear::Atomic_Clear()
{
     m_nCmd = CMD_ENC_CLEAR;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Clear::~Atomic_Clear()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Clear::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Clear::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Clear::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Clear::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Clear::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Clear::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// Get, clear, enable, disable Log
Atomic_Log::Atomic_Log()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_LOG;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     m_pLog = NULL;
}
Atomic_Log::~Atomic_Log()
{
    if(m_pLog)
    {
        FREE(m_pLog);
        m_pLog = NULL;
    }
}
void Atomic_Log::SetLog(uchar* pLog, uint LogSize)
{
    m_pLog = (uchar*)MALLOC(LogSize);
    if (!m_pLog)
        return;
    memcpy(m_pLog, pLog, LogSize);
    m_LogSize = LogSize;
}
void Atomic_Log::GetLog(uchar** hLog, uint* pLogSize)
{
    *hLog = m_pLog;
    *pLogSize = m_LogSize;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Log::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_LogType);
    m_nChallengeBufferSize += sizeof(m_LogOption);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    pPtr = Pack(pPtr, &m_LogType, sizeof(m_LogType));
    pPtr = Pack(pPtr, &m_LogOption, sizeof(m_LogOption));

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Log::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_LogType, pPtr, sizeof(m_LogType));
    pPtr = Unpack(&m_LogOption, pPtr, sizeof(m_LogOption));
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Log::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket,nSize) == false)
        return true;

    // Fill in
    m_nResponseBufferSize += sizeof(m_LogType);
    m_nResponseBufferSize += sizeof(m_LogSize);
    m_nResponseBufferSize += m_LogSize;
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);

    // Response
    pPtr = Pack(pPtr, &m_LogType, sizeof(m_LogType));
    pPtr = Pack(pPtr, &m_LogSize, sizeof(m_LogSize));
    pPtr = Pack(pPtr, m_pLog, m_LogSize);

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Log::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    pPtr = Unpack(&m_LogType, pPtr, sizeof(m_LogType));
    pPtr = Unpack(&m_LogSize, pPtr, sizeof(m_LogSize));

    m_pLog = (uchar*)MALLOC(m_LogSize);
    if (m_pLog)
        pPtr = Unpack(m_pLog, pPtr, m_LogSize);
    else
        return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Log::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Log::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// CMD_GET_VERSION
Atomic_Get_Version::Atomic_Get_Version()
{
     m_nCmd = CMD_GET_VERSION;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pVersionString = NULL;
     m_nSzVersionString = 0;
}
Atomic_Get_Version::~Atomic_Get_Version()
{
    if(m_pVersionString)
    {
        FREE(m_pVersionString);
        m_pVersionString = NULL;
    }
}
// ICmd
//   Note: caller should include terminating 'null' character in the size
//   as we are using it like a character array rather than a string.
void Atomic_Get_Version::SetVersion(char* pVersion, uint nSize)
{
    if(m_pVersionString)
    {
        FREE(m_pVersionString);
        m_pVersionString = NULL;
    }
    m_pVersionString = (char*)MALLOC(nSize);
    memcpy(m_pVersionString, pVersion, nSize);
    m_nSzVersionString = nSize;
}
char*    Atomic_Get_Version::GetVersion(uint& nSize)
{
    nSize = m_nSzVersionString;
    return m_pVersionString;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Version::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Version::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Version::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Calculate Response Buffer Size.
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nSzVersionString);
    m_nResponseBufferSize+=m_nSzVersionString;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Populate Response Buffer
    pPtr = Pack(pPtr, &m_nSzVersionString, sizeof(m_nSzVersionString));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pVersionString, m_nSzVersionString);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Version::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSzVersionString, pPtr, sizeof(m_nSzVersionString));
        if(NULL == pPtr)
           return false;

    if(m_pVersionString)
    {
        FREE(m_pVersionString);
        m_pVersionString = NULL;
    }
    m_pVersionString = (char*)MALLOC(m_nSzVersionString);
    pPtr = Unpack(m_pVersionString, pPtr, m_nSzVersionString);
        if(NULL == pPtr)
           return false;


    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Version::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Version::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_UPDATE_FIRMWARE *************************

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
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nDataSize, pPtr, sizeof(unsigned int));
    if (NULL == pPtr)
        return false;

    if (m_pFWData) FREE(m_pFWData);
    if (m_nDataSize > MAX_FW_DATA_SIZE) return false;
    m_pFWData = (uchar*)MALLOC(m_nDataSize);
    if( m_pFWData == NULL){
        return false;
    }
    pPtr = Unpack(m_pFWData, pPtr, m_nDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Update_Firmware::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
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
    UNUSED(nSize);

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

bool   Macro_Update_Firmware::SetData(uchar* pData, uint nDataSize)
{
    if (m_pFWData)
    {
        FREE(m_pFWData);
        m_pFWData = NULL;
    }
    if (nDataSize > MAX_FW_DATA_SIZE) return false;
    m_pFWData = (uchar*)MALLOC(nDataSize);
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
Atomic_Enc_Get_KeyVersion::Atomic_Enc_Get_KeyVersion()
{
    m_nCmd = CMD_ENC_GET_KEYVERSION;

    // Challenge buffer size...
    m_nChallengeBufferSize     = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer     = NULL;
    m_nResponseBufferSize     = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer     = NULL;

    m_nKeySlot    = KT_EXT_LAST;
    m_nKeyVersion    = 0;
    m_nKeyMode    = KM_MODE_NONE;

    memset(m_pKCV, 0, sizeof(m_pKCV));
}

Atomic_Enc_Get_KeyVersion::~Atomic_Enc_Get_KeyVersion()
{

}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_KeyVersion::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nKeySlot);

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    pPtr = Pack(pPtr, &m_nKeySlot, sizeof(m_nKeySlot));

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Get_KeyVersion::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nKeySlot, pPtr, sizeof(m_nKeySlot));
    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Get_KeyVersion::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(m_nKeyVersion);
    m_nResponseBufferSize += sizeof(m_nKeyMode);
    m_nResponseBufferSize += sizeof(m_pKCV);
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) {
        return true;
    }
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nKeyVersion, sizeof(m_nKeyVersion));
    pPtr = Pack(pPtr, &m_nKeyMode, sizeof(m_nKeyMode));
    pPtr = Pack(pPtr, m_pKCV, sizeof(m_pKCV));
    if (NULL == pPtr) return false;
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
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


// **************************************  CMD_ENC_SET_KEY  ************************************** //
Atomic_Enc_Set_Key::Atomic_Enc_Set_Key() {
    //
    m_nCmd                   = CMD_ENC_SET_KEY;
    // Challenge buffer size...
    m_nChallengeBufferSize   = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer       = NULL;
    m_nResponseBufferSize    = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer        = NULL;

    m_pBuffer                 = NULL;
    m_nBufferSize            = 0;

    m_nKeyType               = KT_EXT_LAST;
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
    m_nChallengeBufferSize  +=  sizeof(m_nKeyType);       //

    uchar* pPtr;
    pPtr  =  GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    pPtr  =  Pack ( pPtr, &m_nKeyType,     sizeof(m_nKeyType) );
    pPtr  =  Pack ( pPtr, &m_nBufferSize,  sizeof(m_nBufferSize) );
    pPtr  =  Pack ( pPtr, m_pBuffer,       m_nBufferSize );

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize    = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Enc_Set_Key::UnpackChallenge(const uchar* pPacket, uint nSize) {

    uchar* pPtr;

    UNUSED(nSize);

    pPtr = UnpackChallengeHeader ( pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize );
    pPtr = Unpack ( &m_nKeyType,    pPtr, sizeof(m_nKeyType) );
    pPtr = Unpack ( &m_nBufferSize, pPtr, sizeof(m_nBufferSize) );

    // Allocate memory
    if (m_pBuffer) {
        FREE(m_pBuffer);
        m_pBuffer = NULL;
    }

    m_pBuffer = (uchar*)MALLOC(m_nBufferSize);
    pPtr      = Unpack(m_pBuffer, pPtr, m_nBufferSize);

    return (pPtr != NULL) ? true : false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Enc_Set_Key::PackResponse(uchar** pPacket, uint& nSize) {

    m_nResponseBufferSize += m_nBufferSize;

    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) {
        return true;
    }

    // Fill in
    uchar*pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) {
        return true;
    }

    pPtr  =  Pack(pPtr,  &m_nBufferSize,   sizeof(m_nBufferSize));
    pPtr  =  Pack(pPtr,  m_pBuffer,        m_nBufferSize);

    // Response
    *pPacket = m_pResponseBuffer;
    nSize    = m_nResponseBufferSize;

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
    pPtr = Unpack(m_pBuffer, pPtr, m_nBufferSize);

    return (pPtr != NULL) ? true : false;
}

// How large is the Challenge packet?
int Atomic_Enc_Set_Key::GetChallengeBufferSize() {
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int Atomic_Enc_Set_Key::GetResponseBufferSize() {
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

Atomic_Add_User::Atomic_Add_User()
{
     m_nCmd = CMD_ADD_USER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
    //
    memset(&m_UserRecord, 0, sizeof(m_UserRecord));
    m_pOpaqueData = NULL;
}

Atomic_Add_User::~Atomic_Add_User()
{
    if(m_pOpaqueData)
    {
         FREE(m_pOpaqueData);
    }
}

//  ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Add_User::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_UserRecord);
    m_nChallengeBufferSize += m_UserRecord.nSizeRecord;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pOpaqueData, m_UserRecord.nSizeRecord);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Add_User::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    m_pOpaqueData = (char*)MALLOC(m_UserRecord.nSizeRecord);
    pPtr = Unpack(m_pOpaqueData, pPtr, m_UserRecord.nSizeRecord);
    return (pPtr!=NULL)? true:false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Add_User::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Add_User::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Add_User::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Add_User::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool Atomic_Add_User::SetOpaqueData(char* pOpData)
{
     if(m_UserRecord.nSizeRecord == 0) return false;
     if(m_UserRecord.nSizeRecord > MAX_USER_RECORD_SIZE) return false;
     if(m_pOpaqueData) FREE(m_pOpaqueData);
     m_pOpaqueData = (char*)MALLOC(m_UserRecord.nSizeRecord);
     memcpy(m_pOpaqueData, pOpData, m_UserRecord.nSizeRecord);
     return true;
}

Atomic_Delete_User::Atomic_Delete_User()
{
     m_nCmd = CMD_DELETE_USER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Delete_User::~Atomic_Delete_User()
{

}

// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Delete_User::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Delete_User::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_Delete_User::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}

// Unpacks packet passed in into internal data structure
bool Atomic_Delete_User::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}

// How large is the Challenge packet?
int  Atomic_Delete_User::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}

// How large is the Response packet?
int  Atomic_Delete_User::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_Get_User::Atomic_Get_User()
{
     m_nCmd = CMD_GET_USER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pRecordData = NULL;
     //
     memset(&m_UserRecord, 0, sizeof(m_UserRecord));
}
Atomic_Get_User::~Atomic_Get_User()
{
    if(m_pRecordData)
    {
        FREE(m_pRecordData);
        m_pRecordData = NULL;
    }
}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_User::PackChallenge(uchar** pPacket, uint& nSize)
{
    switch(m_nArg)
    {
    case 0:
        {
            m_nChallengeBufferSize+=sizeof(_V100_USER_RECORD);
            uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
                        if(NULL == pPtr)
                            return false;

            pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
                        if(NULL == pPtr)
                            return false;


        } break;
    case 1:
        {
            m_nChallengeBufferSize+=sizeof(m_UID_Index);
            uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
                        if(NULL == pPtr)
                            return false;

            pPtr = Pack(pPtr, &m_UID_Index, sizeof(m_UID_Index));
                        if(NULL == pPtr)
                            return false;

        } break;
    }
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_User::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    switch(m_nArg)
    {
    case 0:
        {
            pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
        } break;
    case 1:
        {
            pPtr = Unpack(&m_UID_Index, pPtr, sizeof(m_UID_Index));
        } break;
    }
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_User::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize+=sizeof(m_UserRecord);
    m_nResponseBufferSize+=m_UserRecord.nSizeRecord;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

     pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pRecordData, m_UserRecord.nSizeRecord);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_User::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    m_pRecordData = (char*)MALLOC(m_UserRecord.nSizeRecord);
    pPtr = Unpack(m_pRecordData, pPtr, m_UserRecord.nSizeRecord);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_User::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_User::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_Get_User::SetRecordData(char* pDataToSet)
{
    if(m_UserRecord.nSizeRecord == 0) return false;
    m_pRecordData = (char*)MALLOC(m_UserRecord.nSizeRecord);
    memcpy(m_pRecordData, pDataToSet, m_UserRecord.nSizeRecord);
    return true;
}

Atomic_Get_Verification_Rules::Atomic_Get_Verification_Rules()
{
     m_nCmd = CMD_GET_VERIFICATION_RULES;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_Verification_Rules::~Atomic_Get_Verification_Rules()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Verification_Rules::PackChallenge(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Verification_Rules::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Verification_Rules::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);

    m_nResponseBufferSize+=sizeof(m_VerificationRules);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_VerificationRules, sizeof(m_VerificationRules));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Get_Verification_Rules::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_VerificationRules, pPtr, sizeof(m_VerificationRules));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Verification_Rules::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Verification_Rules::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_Set_Verification_Rules::Atomic_Set_Verification_Rules()
{
     m_nCmd = CMD_SET_VERIFICATION_RULES;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Set_Verification_Rules::~Atomic_Set_Verification_Rules()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Verification_Rules::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_VerificationRules);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_VerificationRules, sizeof(m_VerificationRules));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Verification_Rules::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_VerificationRules, pPtr, sizeof(m_VerificationRules));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Verification_Rules::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Atomic_Set_Verification_Rules::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Verification_Rules::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Verification_Rules::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Macro_Enroll_User::Macro_Enroll_User()
{
     m_nCmd = CMD_ENROLL_USER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_Enroll_User::~Macro_Enroll_User()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Macro_Enroll_User::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Sanity Checks...
    if( (m_UserRecord.nSizeMetaData > MAX_SIZE_META_DATA)) return false;
    // Increment Challenge Buffer Size.
    m_nChallengeBufferSize+=sizeof(m_UserRecord);
    // Generate Challenge Header
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack it.
    pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Enroll_User::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Enroll_User::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Enroll_User::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_Enroll_User::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Enroll_User::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
//

Macro_Verify_User::Macro_Verify_User()
{
     m_nCmd = CMD_VERIFY_USER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_Verify_User::~Macro_Verify_User()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Macro_Verify_User::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Sanity Checks...
    if( (m_UserRecord.nSizeMetaData > MAX_SIZE_META_DATA)) return false;
    // Increment Challenge Buffer Size.
    m_nChallengeBufferSize+=sizeof(m_UserRecord);
    // Generate Challenge Header
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack it.
    pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Verify_User::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Verify_User::PackResponse(uchar** pPacket, uint& nSize)
{
    UNUSED(nSize);
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
        // Fill in
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool Macro_Verify_User::UnpackResponse(const uchar* pPacket, uint nSize)
{
    UNUSED(nSize);

    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_Verify_User::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Verify_User::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

