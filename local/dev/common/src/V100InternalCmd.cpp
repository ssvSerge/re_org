#include "V100InternalCmd.h"
#include "string.h"


// CMD_BARCODE_GET_DETAILS*************************

Atomic_Barcode_Get_Data::Atomic_Barcode_Get_Data() : m_pData(NULL)
{
    m_nCmd = CMD_BARCODE_GET_TEXT;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Barcode_Get_Data::~Atomic_Barcode_Get_Data()
{
    if (m_pData)
    {
        FREE(m_pData);
        m_pData = NULL;
    }
}
// No details
bool Atomic_Barcode_Get_Data::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Barcode_Get_Data::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Barcode_Get_Data::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    m_nResponseBufferSize += sizeof(unsigned int); // Data size
    m_nResponseBufferSize += m_nDataSize;
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDataSize, sizeof(unsigned int));
    pPtr = Pack(pPtr, m_pData, m_nDataSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Barcode_Get_Data::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDataSize, pPtr, sizeof(unsigned int));
    m_pData = (unsigned char*)MALLOC(m_nDataSize);
    pPtr = Unpack(m_pData, pPtr, m_nDataSize);
    return true;
}
void Atomic_Barcode_Get_Data::SetBarcodeData(unsigned char* pData, const unsigned int nSz)
{
    m_nDataSize = nSz;
    if (m_pData) FREE(m_pData);
    m_pData = (uchar*)MALLOC(m_nDataSize);
    memcpy(m_pData, pData, m_nDataSize);
}
// How large is the Challenge packet?
int  Atomic_Barcode_Get_Data::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Barcode_Get_Data::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_BARCODE_GET_DETAILS*************************

Atomic_Barcode_Get_Details::Atomic_Barcode_Get_Details()
{
    m_nCmd = CMD_BARCODE_GET_DETAILS;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_pDetails = NULL;
    m_nDetailsSize = 0;
}
Atomic_Barcode_Get_Details::~Atomic_Barcode_Get_Details()
{
    if (m_pDetails)
    {
        FREE(m_pDetails);
        m_pDetails = NULL;
    }

}
// No details
bool Atomic_Barcode_Get_Details::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Barcode_Get_Details::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Barcode_Get_Details::PackResponse(uchar** pPacket, uint& nSize)
{
    m_nResponseBufferSize += sizeof(unsigned int); // Details size
    m_nResponseBufferSize += m_nDetailsSize; // Details
    // Check for error.
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr) return false;
    pPtr = Pack(pPtr, &m_nDetailsSize, sizeof(unsigned int));
    Pack(pPtr, m_pDetails, m_nDetailsSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Barcode_Get_Details::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nDetailsSize, pPtr, sizeof(unsigned int));
    m_pDetails = (unsigned char*)MALLOC(m_nDetailsSize);
    if (m_pDetails == NULL) return false;
    pPtr = Unpack(m_pDetails, pPtr, m_nDetailsSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Barcode_Get_Details::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Barcode_Get_Details::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

void Atomic_Barcode_Get_Details::SetBarcodeDetails(void* pDetails, uint nDetailsSize)
{
    if (m_pDetails)
    {
        FREE(m_pDetails);
        m_pDetails = NULL;
    }
    m_pDetails = (void*)MALLOC(nDetailsSize);
    memcpy(m_pDetails, pDetails, nDetailsSize);
    m_nDetailsSize = nDetailsSize;
};


// Atomic_Get_System_State *******************************

Atomic_Get_System_State::Atomic_Get_System_State()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_GET_SYSTEM_STATE;
    m_nArg = (short)0x9849;
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    //int x = sizeof(_V100_SYSTEM_DIAGNOSTICS);
    m_nResponseBufferSize  = ENVELOPE_INFO_SIZE + sizeof(_V100_SYSTEM_DIAGNOSTICS);
}
Atomic_Get_System_State::~Atomic_Get_System_State()
{

}
// ICmd

// Takes content of Command, and packs it into pPacket
bool Atomic_Get_System_State::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pOpaqueDataStart = */GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Opaque Data
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_System_State::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    if(pPacket == NULL) return false;
    // Lets see what this header contains....
    /*uchar* pOpaqueDataStart =*/ UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_System_State::PackResponse(uchar** pPacket, uint& nSize)
{
    uchar* pOpaqueDataStart = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Opaque Data
    memcpy(pOpaqueDataStart,&m_Diagnostics,sizeof(_V100_SYSTEM_DIAGNOSTICS));
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
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
/* ----------------------- Atomic_Get_License_Key -------------------------*/
Atomic_Get_License_Key::Atomic_Get_License_Key()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_LICENSE_KEY;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     //
      m_nSizeLK = sizeof(_V100_DEVICE_POLICY);
}
Atomic_Get_License_Key::~Atomic_Get_License_Key()
{

}
// Set License Key
bool Atomic_Get_License_Key::SetLicenseKey(_V100_DEVICE_POLICY* pLC)
{
    memcpy(&m_LK, pLC, sizeof(m_LK));
    return true;
}
// Get License Key
bool Atomic_Get_License_Key::GetLicenseKey(_V100_DEVICE_POLICY* pLC)
{
    memcpy(pLC,&m_LK,sizeof(m_LK));
    return true;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_License_Key::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_License_Key::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_License_Key::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize +=sizeof(m_nSizeLK);
    m_nResponseBufferSize +=m_nSizeLK;

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    pPtr = Pack(pPtr, &m_nSizeLK, sizeof(m_nSizeLK));
    pPtr = Pack(pPtr, &m_LK, m_nSizeLK);
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_License_Key::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSizeLK, pPtr, sizeof(m_nSizeLK));
    pPtr = Unpack(&m_LK, pPtr, m_nSizeLK);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_License_Key::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_License_Key::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// Get Cal
Atomic_Get_Cal::Atomic_Get_Cal()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_CAL;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     m_pCal = NULL;
     m_nCalSize = 0;
}
Atomic_Get_Cal::~Atomic_Get_Cal()
{
    if(m_pCal)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
}
void Atomic_Get_Cal::SetCal(uchar* pCalData, uint nCalSize)
{
    if(m_pCal)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
    m_pCal = (uchar*)MALLOC(nCalSize);
    memcpy(m_pCal, pCalData, nCalSize);
    m_nCalSize = nCalSize;
}
void Atomic_Get_Cal::GetCal(uchar** pCalData, uint& nCalSize)
{
    nCalSize = m_nCalSize;
    *pCalData = m_pCal;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Cal::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Cal::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Cal::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_nResponseBufferSize+= sizeof(m_nCalSize);
     m_nResponseBufferSize+= m_nCalSize;
    // Response
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nCalSize, sizeof(m_nCalSize));
    pPtr = Pack(pPtr, m_pCal, m_nCalSize);

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Cal::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nCalSize, pPtr, sizeof(m_nCalSize));
    if(m_pCal != NULL)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
    m_pCal = (uchar*)MALLOC(m_nCalSize);
    pPtr = Unpack(m_pCal, pPtr, m_nCalSize);

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Cal::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Cal::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Set_Cal::Atomic_Set_Cal()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_CAL;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     m_pCal = NULL;
     m_nCalSize = 0;
}
Atomic_Set_Cal::~Atomic_Set_Cal()
{
    if(m_pCal)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
}
void Atomic_Set_Cal::SetCal(uchar* pCalData, uint nCalSize)
{
    if(m_pCal)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
    m_pCal = (uchar*)MALLOC(nCalSize);
    memcpy(m_pCal, pCalData, nCalSize);
    m_nCalSize = nCalSize;
}
void Atomic_Set_Cal::GetCal(uchar** pCalData, uint& nCalSize)
{
    nCalSize = m_nCalSize;
    *pCalData = m_pCal;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Cal::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_nCalSize);
    m_nChallengeBufferSize += m_nCalSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    pPtr = Pack(pPtr, &m_nCalSize, sizeof(m_nCalSize));
    pPtr = Pack(pPtr, m_pCal, m_nCalSize);

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_Cal::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nCalSize, pPtr, sizeof(m_nCalSize));
    // Allocate
    if(m_pCal)
    {
        FREE(m_pCal);
        m_pCal = NULL;
    }
    m_pCal = (uchar*)MALLOC(m_nCalSize);
    pPtr = Unpack(m_pCal, pPtr, m_nCalSize);
    //
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Cal::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_Set_Cal::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Cal::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Cal::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_Get_Record::Atomic_Get_Record()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_Record::~Atomic_Get_Record()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return false;
}
// How large is the Challenge packet?
int  Atomic_Get_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_Loopback::Atomic_Loopback()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_LOOPBACK;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Loopback::~Atomic_Loopback()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Loopback::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Loopback::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Loopback::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Loopback::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return false;
}
// How large is the Challenge packet?
int  Atomic_Loopback::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Loopback::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_BIT::Atomic_BIT()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_BIT;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_BIT::~Atomic_BIT()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_BIT::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_BIT::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_BIT::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Atomic_BIT::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return false;
}
// How large is the Challenge packet?
int  Atomic_BIT::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_BIT::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Test::Atomic_Test()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_TEST;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Test::~Atomic_Test()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Test::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Test::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Test::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Test::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return false;
}
// How large is the Challenge packet?
int  Atomic_Test::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Test::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Start_Burnin::Atomic_Start_Burnin()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_START_BURNIN;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Start_Burnin::~Atomic_Start_Burnin()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Start_Burnin::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Start_Burnin::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Start_Burnin::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in

        // Fill in
    GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Start_Burnin::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Start_Burnin::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Start_Burnin::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
Atomic_Stop_Burnin::Atomic_Stop_Burnin()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_STOP_BURNIN;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Stop_Burnin::~Atomic_Stop_Burnin()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Stop_Burnin::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Stop_Burnin::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Stop_Burnin::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_Stop_Burnin::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Stop_Burnin::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Stop_Burnin::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// CMD_WRITE_FLASH *************************

Atomic_Write_File::Atomic_Write_File()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_WRITE_FILE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

      // Fill in Start of Header
     m_FileHeader.nSOH  = (uint)0x00C0FFEE;
     // Fill in Time
     m_pFileToWrite = NULL;
}
Atomic_Write_File::~Atomic_Write_File()
{

    if(m_pFileToWrite)
    {
         FREE(m_pFileToWrite);
         m_pFileToWrite = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Write_File::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize+=sizeof(_V100_FILE_HEADER);
    m_nChallengeBufferSize+=m_FileHeader.nFileSize;

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr,&m_FileHeader, sizeof(m_FileHeader));
    pPtr = Pack(pPtr,m_pFileToWrite, m_FileHeader.nFileSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Write_File::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileHeader, pPtr, sizeof(m_FileHeader));
    // Allocate
    m_pFileToWrite = (uchar*)MALLOC(m_FileHeader.nFileSize);
    pPtr = Unpack(m_pFileToWrite, pPtr, m_FileHeader.nFileSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Write_File::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_Write_File::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Fill in
    return true;
}
// How large is the Challenge packet?
int  Atomic_Write_File::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Write_File::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool  Atomic_Write_File::SetChallengeData(uchar* pFileToWrite, uint nSize, uint nFlashLocation, uint lTime)
{
    // Populate File header.
    m_FileHeader.lTime = lTime;
    m_FileHeader.nFlashAddress = nFlashLocation;
    m_FileHeader.nFileSize = nSize;
    m_pFileToWrite = (uchar*)MALLOC(nSize);
    memcpy(m_pFileToWrite, pFileToWrite, nSize);
    return true;
}
// Get Challenge Info (Host)
bool Atomic_Write_File::GetChallengeData(uchar** pFileToWrite, _V100_FILE_HEADER& FileHeader)
{
    FileHeader = m_FileHeader;
    *pFileToWrite = m_pFileToWrite;
    return true;
}

// CMD_READ_FLASH *************************

Atomic_Read_File::Atomic_Read_File()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_READ_FILE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_pFile = NULL;

}
Atomic_Read_File::~Atomic_Read_File()
{
    if(m_pFile)
    {
         FREE(m_pFile);
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Read_File::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_nFlashLocation);
    //
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_nFlashLocation, sizeof(m_nFlashLocation));
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Read_File::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nFlashLocation, pPtr, sizeof(m_nFlashLocation));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Read_File::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false)
    {
        return true;
    }
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize +=sizeof(_V100_FILE_HEADER);
    m_nResponseBufferSize +=m_FileHeader.nFileSize;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileHeader, sizeof(m_FileHeader));
    pPtr = Pack(pPtr, m_pFile, m_FileHeader.nFileSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Read_File::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    //
    pPtr = Unpack(&m_FileHeader, pPtr, sizeof(m_FileHeader));
    // Allocate
    if(m_pFile)
    {
        FREE(m_pFile);
        m_pFile = NULL;
    }

    m_pFile = (uchar*)MALLOC(m_FileHeader.nFileSize);
    memcpy(m_pFile, pPtr, m_FileHeader.nFileSize);

    return true;
}
// How large is the Challenge packet?
int  Atomic_Read_File::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Read_File::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// Challenge
bool Atomic_Read_File::SetChallengeData(uint nFlashLocation)
{
    m_nFlashLocation = nFlashLocation;
    return true;
}
// Get
bool Atomic_Read_File::GetChallengeData(uint& nFlashLocation)
{
    nFlashLocation = m_nFlashLocation;
    return true;
}
// Set
bool Atomic_Read_File::SetResponseData(uchar* pBytes, _V100_FILE_HEADER FileHeader)
{
    m_FileHeader = FileHeader;
    if(m_pFile)
    {
        FREE(m_pFile);
        m_pFile = NULL;
    }
    m_pFile = (uchar*) MALLOC(FileHeader.nFileSize);
    memcpy(m_pFile, pBytes, FileHeader.nFileSize);

    return true;
}
// Get
bool Atomic_Read_File::GetResponseData(uchar* pBytes, _V100_FILE_HEADER& FileHeader)
{
    FileHeader = m_FileHeader;
    memcpy(pBytes, m_pFile, FileHeader.nFileSize);

    return true;
}

// CMD_GET_SPOOF_DETAILS *************************

Atomic_Get_Spoof_Details::Atomic_Get_Spoof_Details()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_SPOOF_DETAILS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_Spoof_Details::~Atomic_Get_Spoof_Details()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Spoof_Details::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Spoof_Details::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Spoof_Details::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize +=sizeof(_V100_SPOOF_METRICS);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    pPtr = Pack(pPtr, &m_nSpoofMetrics, sizeof(_V100_SPOOF_METRICS));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Spoof_Details::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSpoofMetrics, pPtr, sizeof(_V100_SPOOF_METRICS));
    //

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Spoof_Details::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Spoof_Details::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool  Atomic_Get_Spoof_Details::SetSpoofMetrics(_V100_SPOOF_METRICS  metrics)
{
    memcpy(&m_nSpoofMetrics, &metrics, sizeof(metrics));
    //m_nSpoofMetrics = metrics;
    return true;
}
bool  Atomic_Get_Spoof_Details::GetSpoofMetrics(_V100_SPOOF_METRICS& metrics)
{
    memcpy(&metrics, &m_nSpoofMetrics, sizeof(metrics));
    return true;
}

// CMD_GET_SPOOF_DETAILS_V2 *************************

Atomic_Get_Spoof_Details_V2::Atomic_Get_Spoof_Details_V2()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_SPOOF_DETAILS_V2;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_Spoof_Details_V2::~Atomic_Get_Spoof_Details_V2()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Spoof_Details_V2::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Spoof_Details_V2::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Spoof_Details_V2::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize +=sizeof(_V100_SPOOF_METRICS_V2);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    pPtr = Pack(pPtr, &m_nSpoofMetrics, sizeof(_V100_SPOOF_METRICS_V2));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Spoof_Details_V2::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_nSpoofMetrics, pPtr, sizeof(_V100_SPOOF_METRICS_V2));
    //

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Spoof_Details_V2::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Spoof_Details_V2::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool  Atomic_Get_Spoof_Details_V2::SetSpoofMetrics(_V100_SPOOF_METRICS_V2* metrics)
{
    memcpy(&m_nSpoofMetrics, metrics, sizeof(_V100_SPOOF_METRICS_V2));
    //m_nSpoofMetrics = metrics;
    return true;
}
bool  Atomic_Get_Spoof_Details_V2::GetSpoofMetrics(_V100_SPOOF_METRICS_V2* metrics)
{
    memcpy(metrics, &m_nSpoofMetrics, sizeof(_V100_SPOOF_METRICS_V2));
    return true;
}

// CMD_FILE_READ

Atomic_File_Read::Atomic_File_Read()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_READ;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     memset(&m_FileAttr, 0, sizeof(m_FileAttr));
     m_pFileStream = NULL;
}
Atomic_File_Read::~Atomic_File_Read()
{
    if( m_pFileStream != NULL )
    {
        FREE(m_pFileStream);
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Read::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_szFileNameSize);
    m_nChallengeBufferSize += m_szFileNameSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Pack File Attribute
    pPtr = Pack(pPtr, &m_szFileNameSize, sizeof(m_szFileNameSize));
    pPtr = Pack(pPtr, m_pFileName, m_szFileNameSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Read::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_szFileNameSize, pPtr, sizeof(m_szFileNameSize));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileNameSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_Read::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize +=sizeof(m_FileAttr);
    m_nResponseBufferSize +=m_FileAttr.FileSize;

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileAttr, sizeof(m_FileAttr));
    pPtr = Pack(pPtr, m_pFileStream, m_FileAttr.FileSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Read::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileAttr, pPtr, sizeof(m_FileAttr));

    if( m_pFileStream == NULL )
    {
        m_pFileStream = (char*)MALLOC(m_FileAttr.FileSize);
    }
    pPtr = Unpack(m_pFileStream, pPtr, m_FileAttr.FileSize);

    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Read::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Read::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_File_Read::SetFileStream(char* pPtr, uint nSZ)
{
    if( nSZ > MAX_FILE_SIZE) return false;

    if(m_pFileStream)
    {
        FREE(m_pFileStream);
        m_pFileStream = NULL;
    }
    // SHALLOW COPY
    m_pFileStream = pPtr;
    m_FileAttr.FileSize = nSZ;

    return true;
}
bool Atomic_File_Read::SetFileName(char* pPtr, uint nSz)
{
    if( nSz > MAX_FILE_NAME_LENGTH ) return false;
    memcpy(m_pFileName, pPtr, nSz);
    m_szFileNameSize = nSz;
    return true;
}

// CMD_FILE_WRITE


Atomic_File_Write::Atomic_File_Write()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_WRITE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     memset(&m_FileAttr, 0, sizeof(m_FileAttr));
     m_pFileStream = NULL;
}
Atomic_File_Write::~Atomic_File_Write()
{
    if( m_pFileStream != NULL )
    {
        FREE(m_pFileStream);
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Write::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_szFileNameSize);
    m_nChallengeBufferSize += m_szFileNameSize;
    m_nChallengeBufferSize += sizeof(_V100_FILE);
    m_nChallengeBufferSize += m_FileAttr.FileSize;

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Pack File Attribute
    pPtr = Pack(pPtr, &m_FileAttr, sizeof(_V100_FILE));
    pPtr = Pack(pPtr, &m_szFileNameSize, sizeof(m_szFileNameSize));
    pPtr = Pack(pPtr, m_pFileName, m_szFileNameSize);
    pPtr = Pack(pPtr, m_pFileStream, m_FileAttr.FileSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Write::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileAttr, pPtr, sizeof(_V100_FILE));
    pPtr = Unpack(&m_szFileNameSize, pPtr, sizeof(m_szFileNameSize));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileNameSize);
    if(m_pFileStream != NULL )
    {
        FREE(m_pFileStream);
    }
    m_pFileStream = (char*)MALLOC(m_FileAttr.FileSize);
    pPtr = Unpack(m_pFileStream, pPtr, m_FileAttr.FileSize);

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_Write::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    /*uchar* pPtr =*/ GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Write::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Write::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Write::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_File_Write::SetFileStream(char* pPtr, uint nSZ)
{
    if( nSZ > MAX_FILE_SIZE) return false;

    if(m_pFileStream)
    {
        FREE(m_pFileStream);
        m_pFileStream = NULL;
    }
    m_pFileStream = (char*)MALLOC(nSZ);
    memcpy(m_pFileStream, pPtr, nSZ);
    m_FileAttr.FileSize = nSZ;

    return true;
}
bool Atomic_File_Write::SetFileName(const char* pPtr, uint nSz)
{
    if( nSz > MAX_FILE_NAME_LENGTH ) return false;
    memcpy(m_pFileName, pPtr, nSz);
    m_szFileNameSize = nSz;
    return true;
}


// CMD_FILE_CD
Atomic_File_CD::Atomic_File_CD()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_CD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_CD::~Atomic_File_CD()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_CD::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_szFileLength);
    m_nChallengeBufferSize += m_szFileLength;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_CD::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
    return (pPtr!=NULL)? true:false;
}

// Takes content of Command, and packs it into pPacket
bool Atomic_File_CD::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    /*uchar* pPtr =*/ GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_CD::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_CD::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_CD::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// SPEC
char* Atomic_File_CD::GetName()
{
    return (char*)m_pFileName;
}
uint* Atomic_File_CD::GetSize()
{
    return &m_szFileLength;
}
bool Atomic_File_CD::SetName(const char* pObjectName, uint nObjectLength)
{
    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
    memcpy(m_pFileName, pObjectName, nObjectLength);
    m_pFileName[nObjectLength] = '\0';
    m_szFileLength = nObjectLength;
    return true;
}


// CMD_FILE_GET_ATTRIBUTES
Atomic_File_Get_Attributes::Atomic_File_Get_Attributes()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_GET_ATTRIBUTES;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_Get_Attributes::~Atomic_File_Get_Attributes()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Get_Attributes::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Get_Attributes::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_Get_Attributes::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize += sizeof(_V100_FILE_SYSTEM_ATTRIBUTE);
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileSysAttr, sizeof(m_FileSysAttr));
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Get_Attributes::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileSysAttr, pPtr, sizeof(m_FileSysAttr));
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Get_Attributes::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Get_Attributes::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
//// CMD_FILE_CD
//Atomic_File_Delete::Atomic_File_Delete()
//{
//     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_DELETE;
//     // Challenge buffer size...
//     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
//     m_pChallengeBuffer = NULL;
//     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
//     m_pResponseBuffer = NULL;
//}
//Atomic_File_Delete::~Atomic_File_Delete()
//{
//
//}
//// ICmd
// // Takes content of Command, and packs it into pPacket
//bool Atomic_File_Delete::PackChallenge(uchar** pPacket, uint& nSize)
//{
//    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
//    m_nChallengeBufferSize += sizeof(m_szFileLength);
//    m_nChallengeBufferSize += m_szFileLength;
//    m_nChallengeBufferSize += sizeof(Attribute);
//    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
//    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
//    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
//    pPtr = Pack(pPtr, &Attribute, sizeof(Attribute));
//    // Fill in some stuff
//    *pPacket = m_pChallengeBuffer;
//    nSize = m_nChallengeBufferSize;
//    return true;
//}
//// Unpacks packet passed in into interal data structure
//bool Atomic_File_Delete::UnpackChallenge(const uchar* pPacket, uint nSize)
//{
//    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
//    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
//    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
//    pPtr = Unpack(&Attribute, pPtr, sizeof(Attribute));
//    return (pPtr!=NULL)? true:false;
//}
//
//// Takes content of Command, and packs it into pPacket
//bool Atomic_File_Delete::PackResponse(uchar** pPacket, uint& nSize)
//{
//    // Check for error.
//    if(CheckErrorCode(pPacket,nSize) == false) return true;
//    // Fill in
//    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
//    // Response
//    *pPacket = m_pResponseBuffer;
//    nSize = m_nResponseBufferSize;
//    return true;
//}
//// Unpacks packet passed in into interal data structure
//bool Atomic_File_Delete::UnpackResponse(const uchar* pPacket, uint nSize)
//{
//    // Remember to allocate memory for composite image, and template, if requested.
//    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
//    return true;
//}
//// How large is the Challenge packet?
//int  Atomic_File_Delete::GetChallengeBufferSize()
//{
//    return m_nChallengeBufferSize;
//}
//// How large is the Response packet?
//int  Atomic_File_Delete::GetResponseBufferSize()
//{
//    return m_nResponseBufferSize;
//}
//// SPEC
//char* Atomic_File_Delete::GetName()
//{
//    return (char*)m_pFileName;
//}
//uint* Atomic_File_Delete::GetSize()
//{
//    return &m_szFileLength;
//}
//_V100_FILE_ATTR Atomic_File_Delete::GetAttr()
//{
//    return Attribute;
//}
//bool Atomic_File_Delete::SetName(char* pObjectName, uint nObjectLength, _V100_FILE_ATTR attr)
//{
//    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
//    memcpy(m_pFileName, pObjectName, nObjectLength);
//    m_pFileName[nObjectLength] = NULL;
//    m_szFileLength = nObjectLength;
//    Attribute = attr;
//    return true;
//}

// CMD_FORMAT_VOLUME
Atomic_Format_Volume::Atomic_Format_Volume()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FORMAT_VOLUME;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Format_Volume::~Atomic_Format_Volume()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Format_Volume::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Format_Volume::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Format_Volume::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    /*uchar* pPtr =*/ GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Format_Volume::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Format_Volume::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Format_Volume::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// Atomic_File_GetCwd

// CMD_FORMAT_VOLUME
Atomic_File_GetCwd::Atomic_File_GetCwd()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_GETCWD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_GetCwd::~Atomic_File_GetCwd()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_GetCwd::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_GetCwd::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr =  UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_GetCwd::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_nResponseBufferSize+= sizeof(m_FileAttr);
     m_nResponseBufferSize+= sizeof(m_szFileLength);
     m_nResponseBufferSize+= m_szFileLength;
    // Response
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileAttr, sizeof(m_FileAttr));
    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_GetCwd::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileAttr, pPtr, sizeof(m_FileAttr));
    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
    if(m_szFileLength > MAX_FILE_NAME_LENGTH) return false;
    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_GetCwd::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_GetCwd::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// SPEC
bool Atomic_File_GetCwd::GetName(char* pObjectName, uint& nObjectLength)
{
    memcpy(pObjectName, (const char*)m_pFileName, m_szFileLength);
    nObjectLength = m_szFileLength;
    return true;
}
char* Atomic_File_GetCwd::GetName()
{
    return (char*)m_pFileName;
}
uint* Atomic_File_GetCwd::GetSize()
{
    return &m_szFileLength;
}
bool Atomic_File_GetCwd::SetName(char* pObjectName, uint nObjectLength)
{
    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
    memcpy(m_pFileName, pObjectName, nObjectLength);
    m_pFileName[nObjectLength] = '\0';
    m_szFileLength = nObjectLength;
    return true;
}


// CMD_FORMAT_VOLUME
Atomic_File_Dir_FindFirst::Atomic_File_Dir_FindFirst()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_DIR_FINDFIRST;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_Dir_FindFirst::~Atomic_File_Dir_FindFirst()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Dir_FindFirst::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Dir_FindFirst::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr =  UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_Dir_FindFirst::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_nResponseBufferSize+= sizeof(m_FileAttr);
     m_nResponseBufferSize+= sizeof(m_szFileLength);
     m_nResponseBufferSize+= m_szFileLength;
    // Response
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileAttr, sizeof(m_FileAttr));
    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Dir_FindFirst::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileAttr, pPtr, sizeof(m_FileAttr));
    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Dir_FindFirst::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Dir_FindFirst::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// SPEC
bool Atomic_File_Dir_FindFirst::GetName(char* pObjectName, uint& nObjectLength)
{
    memcpy(pObjectName, (const char*)m_pFileName, m_szFileLength);
    nObjectLength = m_szFileLength;
    return true;
}
char* Atomic_File_Dir_FindFirst::GetName()
{
    return (char*)m_pFileName;
}
uint* Atomic_File_Dir_FindFirst::GetSize()
{
    return &m_szFileLength;
}
bool Atomic_File_Dir_FindFirst::SetName(char* pObjectName, uint nObjectLength)
{
    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
    memcpy(m_pFileName, pObjectName, nObjectLength);
    m_pFileName[nObjectLength] = '\0';
    m_szFileLength = nObjectLength;
    return true;
}

// CMD_FORMAT_VOLUME
Atomic_File_Dir_FindNext::Atomic_File_Dir_FindNext()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_FILE_DIR_FINDNEXT;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_File_Dir_FindNext::~Atomic_File_Dir_FindNext()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_File_Dir_FindNext::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Dir_FindNext::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr =  UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_File_Dir_FindNext::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_nResponseBufferSize+= sizeof(m_FileAttr);
     m_nResponseBufferSize+= sizeof(m_szFileLength);
     m_nResponseBufferSize+= m_szFileLength;
    // Response
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, &m_FileAttr, sizeof(m_FileAttr));
    pPtr = Pack(pPtr, &m_szFileLength, sizeof(m_szFileLength));
    pPtr = Pack(pPtr, m_pFileName, m_szFileLength);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_File_Dir_FindNext::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    pPtr = Unpack(&m_FileAttr, pPtr, sizeof(m_FileAttr));
    pPtr = Unpack(&m_szFileLength, pPtr, sizeof(m_szFileLength));
    pPtr = Unpack(m_pFileName, pPtr, m_szFileLength);
    return true;
}
// How large is the Challenge packet?
int  Atomic_File_Dir_FindNext::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_File_Dir_FindNext::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// SPEC
bool Atomic_File_Dir_FindNext::GetName(char* pObjectName, uint& nObjectLength)
{
    memcpy(pObjectName, (const char*)m_pFileName, m_szFileLength);
    nObjectLength = m_szFileLength;
    return true;
}
char* Atomic_File_Dir_FindNext::GetName()
{
    return (char*)m_pFileName;
}
uint* Atomic_File_Dir_FindNext::GetSize()
{
    return &m_szFileLength;
}
bool Atomic_File_Dir_FindNext::SetName(char* pObjectName, uint nObjectLength)
{
    if(nObjectLength > MAX_FILE_NAME_LENGTH) return false;
    memcpy(m_pFileName, pObjectName, nObjectLength);
    m_pFileName[nObjectLength] = '\0';
    m_szFileLength = nObjectLength;
    return true;
}

// CMD_GET_EEPROM
Atomic_Get_EEPROM::Atomic_Get_EEPROM()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_EEPROM;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_EEPROM::~Atomic_Get_EEPROM()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_EEPROM::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_EEPROM::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_EEPROM::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    // Size field
    m_nSize = sizeof(m_nEEP);
    //
    m_nResponseBufferSize += sizeof(m_nSize);
    // The Structure
    m_nResponseBufferSize += sizeof(m_nEEP);
    //
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Pack size of structure
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_nEEP, m_nSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_EEPROM::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    if( m_nSize != sizeof(m_nEEP) ) return false;
    pPtr = Unpack(&m_nEEP, pPtr, m_nSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_EEPROM::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_EEPROM::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_SET_EEPROM
Atomic_Set_EEPROM::Atomic_Set_EEPROM()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_EEPROM;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_nChallengeBufferSize += sizeof(m_nSize);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Set_EEPROM::~Atomic_Set_EEPROM()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_EEPROM::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += m_nSize;
    uchar* pPtr= GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Pack Size
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_nEEPROM, m_nSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_EEPROM::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack Size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    // Unpack Structure
    pPtr = Unpack(&m_nEEPROM, pPtr, m_nSize);
    //
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_EEPROM::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    /*uchar* pPtr =*/ GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_EEPROM::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_EEPROM::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_EEPROM::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_GET_EEPROM_M320
Atomic_Get_EEPROM_M320::Atomic_Get_EEPROM_M320()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_GET_EEPROM_M320;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Get_EEPROM_M320::~Atomic_Get_EEPROM_M320()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_EEPROM_M320::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_EEPROM_M320::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_EEPROM_M320::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    // Size field
    m_nSize = sizeof(m_nEEP);
    //
    m_nResponseBufferSize += sizeof(m_nSize);
    // The Structure
    m_nResponseBufferSize += sizeof(m_nEEP);
    //
    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    // Pack size of structure
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_nEEP, m_nSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_EEPROM_M320::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    if (m_nSize != sizeof(m_nEEP)) return false;
    pPtr = Unpack(&m_nEEP, pPtr, m_nSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_EEPROM_M320::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_EEPROM_M320::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_SET_EEPROM_M320
Atomic_Set_EEPROM_M320::Atomic_Set_EEPROM_M320()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_SET_EEPROM_M320;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(m_nSize);
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Atomic_Set_EEPROM_M320::~Atomic_Set_EEPROM_M320()
{

}
// ICmd
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_EEPROM_M320::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += m_nSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    // Pack Size
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_nEEPROM, m_nSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_EEPROM_M320::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack Size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    // Unpack Structure
    pPtr = Unpack(&m_nEEPROM, pPtr, m_nSize);
    //
    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_EEPROM_M320::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;
    // Fill in
    /*uchar* pPtr =*/ GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_EEPROM_M320::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_EEPROM_M320::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_EEPROM_M320::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_GET_DSM_EEPROM *************************

Atomic_Get_DSM_EEPROM::Atomic_Get_DSM_EEPROM()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_DSM_EEPROM;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Get_DSM_EEPROM::~Atomic_Get_DSM_EEPROM()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_DSM_EEPROM::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr)
        return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_DSM_EEPROM::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_DSM_EEPROM::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    // Size field
    m_nSize = sizeof(m_EEPROMData);
    //
    m_nResponseBufferSize += sizeof(m_nSize);
    // The Structure
    m_nResponseBufferSize += sizeof(m_EEPROMData);
    //
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Pack size of structure
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_EEPROMData, m_nSize);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_DSM_EEPROM::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    if( m_nSize != sizeof(m_EEPROMData) ) return false;
    pPtr = Unpack(&m_EEPROMData, pPtr, m_nSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_DSM_EEPROM::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_DSM_EEPROM::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_SET_DSM_EEPROM *************************

Atomic_Set_DSM_EEPROM::Atomic_Set_DSM_EEPROM()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_DSM_EEPROM;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_nChallengeBufferSize += sizeof(m_nSize);
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_Set_DSM_EEPROM::~Atomic_Set_DSM_EEPROM()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_DSM_EEPROM::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += m_nSize;
    uchar* pPtr= GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr)
        return false;
    // Pack Size
    pPtr = Pack(pPtr, &m_nSize, sizeof(m_nSize));
    // Pack Structure
    pPtr = Pack(pPtr, &m_EEPROMData, m_nSize);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_DSM_EEPROM::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Unpack Size
    pPtr = Unpack(&m_nSize, pPtr, sizeof(m_nSize));
    // Unpack Structure
    pPtr = Unpack(&m_EEPROMData, pPtr, m_nSize);
    //
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_DSM_EEPROM::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    /*uchar* pPtr =*/ GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_DSM_EEPROM::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_DSM_EEPROM::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_DSM_EEPROM::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}



// CMD_PROCESS *************************

Atomic_Process::Atomic_Process()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_PROCESS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pData = nullptr;
}
Atomic_Process::~Atomic_Process()
{
    if(m_pChallengeBuffer == NULL)
    {
        if(m_pData)
        {
            FREE(m_pData);
        }
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Process::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Data Size
    m_nChallengeBufferSize += sizeof(m_nDataSize);
    // Data Data
    m_nChallengeBufferSize += m_nDataSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Generate Response Header
    // Pack Image Size
    pPtr = Pack(pPtr, &m_nDataSize, sizeof(m_nDataSize));
        if(NULL == pPtr)
           return false;

    // Pack Image
    pPtr = Pack(pPtr, m_pData, m_nDataSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Process::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nDataSize, pPtr, sizeof(m_nDataSize));
        if(NULL == pPtr)
           return false;

    // Allocate space, and Unpack Image
    m_pData = (uchar*)MALLOC(m_nDataSize);
    pPtr = Unpack(m_pData, pPtr, m_nDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Process::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_Process::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Process::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Process::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


Atomic_Diagnostic_Test::Atomic_Diagnostic_Test()
{
    m_nCmd = (_V100_COMMAND_SET)CMD_DIAGNOSTIC_TEST;
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;

    m_nComponent = 0;
    m_nOperation = 0;
    m_nOperationParameter = 0;
}
Atomic_Diagnostic_Test::~Atomic_Diagnostic_Test()
{
}
bool Atomic_Diagnostic_Test::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nComponent);
    m_nChallengeBufferSize += sizeof(m_nOperation);
    m_nChallengeBufferSize += sizeof(m_nOperationParameter);

    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nComponent, sizeof(m_nComponent));
    if (NULL == pPtr)
        return false;
    pPtr = Pack(pPtr, &m_nOperation, sizeof(m_nOperation));
    if (NULL == pPtr)
        return false;
    pPtr = Pack(pPtr, &m_nOperationParameter, sizeof(m_nOperation));
    if (NULL == pPtr)
        return false;

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Diagnostic_Test::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nComponent, pPtr, sizeof(m_nComponent));
    if (NULL == pPtr)
        return false;
    pPtr = Unpack(&m_nOperation, pPtr, sizeof(m_nOperation));
    if (NULL == pPtr)
        return false;
    pPtr = Unpack(&m_nOperationParameter, pPtr, sizeof(m_nOperationParameter));
    if (NULL == pPtr)
        return false;

    return true;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Diagnostic_Test::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if (CheckErrorCode(pPacket, nSize) == false) return true;

    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize += sizeof(m_nResult);
    m_nResponseBufferSize += sizeof(m_nError);

    uchar* pPtr = GenerateResponseHeader(0, m_nResponseBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nResult, sizeof(m_nResult));
    if (NULL == pPtr)
        return false;

    pPtr = Pack(pPtr, &m_nError, sizeof(m_nError));
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Diagnostic_Test::UnpackResponse(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    pPtr = Unpack(&m_nResult, pPtr, sizeof(m_nResult));
    if (NULL == pPtr)
        return false;
    pPtr = Unpack(&m_nError, pPtr, sizeof(m_nError));
    if (NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_Diagnostic_Test::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Diagnostic_Test::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// Get Mfg State
Atomic_Get_Mfg_State::Atomic_Get_Mfg_State()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_GET_MFG_STATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     m_pMfgState = NULL;
}
Atomic_Get_Mfg_State::~Atomic_Get_Mfg_State()
{
    if(m_pMfgState)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
}
void Atomic_Get_Mfg_State::SetMfgState(_V100_MFG_STATE* pMfgState)
{
    if(m_pMfgState)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
    m_pMfgState = (_V100_MFG_STATE*)MALLOC(sizeof(_V100_MFG_STATE));
    memcpy(m_pMfgState, pMfgState, sizeof(_V100_MFG_STATE));
}
void Atomic_Get_Mfg_State::GetMfgState(_V100_MFG_STATE** pMfgState)
{
    *pMfgState = m_pMfgState;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_Get_Mfg_State::PackChallenge(uchar** pPacket, uint& nSize)
{
    /*uchar* pPtr =*/ GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Mfg_State::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Get_Mfg_State::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_nResponseBufferSize+= sizeof(_V100_MFG_STATE);
    // Response
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    pPtr = Pack(pPtr, m_pMfgState, sizeof(_V100_MFG_STATE));

    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Get_Mfg_State::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(m_pMfgState != NULL)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
    m_pMfgState = (_V100_MFG_STATE*)MALLOC(sizeof(_V100_MFG_STATE));
    pPtr = Unpack(m_pMfgState, pPtr, sizeof(_V100_MFG_STATE));

    return true;
}
// How large is the Challenge packet?
int  Atomic_Get_Mfg_State::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Get_Mfg_State::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}




Atomic_Set_Mfg_State::Atomic_Set_Mfg_State()
{
     m_nCmd = (_V100_COMMAND_SET)CMD_SET_MFG_STATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     // SPEC
     m_pMfgState = NULL;
}
Atomic_Set_Mfg_State::~Atomic_Set_Mfg_State()
{
    if(m_pMfgState)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
}
void Atomic_Set_Mfg_State::SetMfgState(_V100_MFG_STATE* pMfgState)
{
    if(m_pMfgState)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
    m_pMfgState = (_V100_MFG_STATE*)MALLOC(sizeof(_V100_MFG_STATE));
    memcpy(m_pMfgState, pMfgState, sizeof(_V100_MFG_STATE));
}
void Atomic_Set_Mfg_State::GetMfgState(_V100_MFG_STATE** pMfgState)
{
    *pMfgState = m_pMfgState;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_Set_Mfg_State::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += sizeof(_V100_MFG_STATE);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    // Fill in some stuff
    pPtr = Pack(pPtr, m_pMfgState, sizeof(_V100_MFG_STATE));

    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_Set_Mfg_State::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    // Allocate
    if(m_pMfgState)
    {
        FREE(m_pMfgState);
        m_pMfgState = NULL;
    }
    m_pMfgState = (_V100_MFG_STATE*)MALLOC(sizeof(_V100_MFG_STATE));
    pPtr = Unpack(m_pMfgState, pPtr, sizeof(_V100_MFG_STATE));
    //
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_Set_Mfg_State::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_Set_Mfg_State::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    /*uchar* pPtr =*/ UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return true;
}
// How large is the Challenge packet?
int  Atomic_Set_Mfg_State::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_Set_Mfg_State::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

