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

#include "V100Cmd.h"
#include "V100IDCmd.h"
#include "string.h"

// CMD_XXX_XXXX *************************

Macro_ID_Create_DB::Macro_ID_Create_DB()
{
     m_nCmd = CMD_ID_CREATE_DB;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_ID_Create_DB::~Macro_ID_Create_DB()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Create_DB::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_DB_INIT_STRUCT);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_DbCreateParms, sizeof(_MX00_DB_INIT_STRUCT));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Create_DB::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_DbCreateParms, pPtr, sizeof(_MX00_DB_INIT_STRUCT));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Create_DB::PackResponse(uchar** pPacket, uint& nSize)
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
bool Macro_ID_Create_DB::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Create_DB::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Create_DB::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_XXX_XXXX *************************

Macro_ID_Set_Working_DB::Macro_ID_Set_Working_DB()
{
     m_nCmd = CMD_ID_SET_WORKING_DB;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_nGroupID = 0;
}
Macro_ID_Set_Working_DB::~Macro_ID_Set_Working_DB()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Set_Working_DB::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_nGroupID);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nGroupID, sizeof(m_nGroupID));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Set_Working_DB::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nGroupID, pPtr, sizeof(m_nGroupID));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Set_Working_DB::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Set_Working_DB::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Set_Working_DB::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Set_Working_DB::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_XXX_XXXX *************************

Atomic_ID_Get_User_Record::Atomic_ID_Get_User_Record()
{
     m_nCmd = CMD_ID_GET_USER_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pTemplateInstances = NULL;
}
Atomic_ID_Get_User_Record::~Atomic_ID_Get_User_Record()
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_User_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecordHeader, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_User_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecordHeader, pPtr, sizeof(m_UserRecordHeader));

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_User_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    m_nResponseBufferSize+=sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecordHeader, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplateInstances, sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_User_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecordHeader, pPtr, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;


    // Create Template Instances
    m_pTemplateInstances = (_MX00_TEMPLATE_INSTANCE*)MALLOC(m_UserRecordHeader.nInstances*sizeof(_MX00_TEMPLATE_INSTANCE));
    pPtr = Unpack(m_pTemplateInstances, pPtr, m_UserRecordHeader.nInstances*sizeof(_MX00_TEMPLATE_INSTANCE));
        if(NULL == pPtr)
           return false;


    return true;
}

// How large is the Challenge packet?
int  Atomic_ID_Get_User_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_User_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Atomic_ID_Get_User_Record::GetTemplates(_MX00_TEMPLATE_INSTANCE instanceRecords[])
{
    memcpy(instanceRecords, m_pTemplateInstances, sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances);
    return true;
}
_MX00_TEMPLATE_INSTANCE* Atomic_ID_Get_User_Record::GetNewTemplateBuffer(uint nNumInstances, uint nSizePerTemplate)
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
    m_pTemplateInstances = (_MX00_TEMPLATE_INSTANCE*)MALLOC(nSizePerTemplate*nNumInstances);
    return m_pTemplateInstances;
}

void Atomic_ID_Get_User_Record::SetUserRecords(const _MX00_ID_USER_RECORD& rec, const _MX00_TEMPLATE_INSTANCE instanceRecords[])
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
    m_UserRecordHeader = rec;
    m_pTemplateInstances = (_MX00_TEMPLATE_INSTANCE*)MALLOC(sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances);
    _MX00_TEMPLATE_INSTANCE* pHdPtr = m_pTemplateInstances;
    //
    for( uint ii = 0 ; ii < m_UserRecordHeader.nInstances ; ii++)
    {
        memcpy(pHdPtr, &instanceRecords[ii], sizeof(_MX00_TEMPLATE_INSTANCE));
        pHdPtr++;
    }
}

// CMD_XXX_XXXX *************************

Atomic_ID_Get_User_Record_Header::Atomic_ID_Get_User_Record_Header()
{
     m_nCmd = CMD_ID_GET_USER_RECORD_HEADER;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Get_User_Record_Header::~Atomic_ID_Get_User_Record_Header()
{
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_User_Record_Header::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr) return false;
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_User_Record_Header::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(NULL == pPtr) return false;
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_User_Record_Header::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize+=sizeof(_MX00_ID_USER_RECORD);

    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecordHeader, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_User_Record_Header::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecordHeader, pPtr, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;
    // Create Template Instances
    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_User_Record_Header::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_User_Record_Header::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
// CMD_XXX_XXXX *************************

Atomic_ID_Set_User_Record::Atomic_ID_Set_User_Record()
{
     m_nCmd = CMD_ID_SET_USER_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     //
     m_pTemplateInstances = NULL;
}
Atomic_ID_Set_User_Record::~Atomic_ID_Set_User_Record()
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Set_User_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    m_nChallengeBufferSize+=sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecordHeader, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplateInstances, sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances);
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Set_User_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecordHeader, pPtr, sizeof(m_UserRecordHeader));
        if(NULL == pPtr)
           return false;


    // Create Template Instances
    m_pTemplateInstances = (_MX00_TEMPLATE_INSTANCE*)MALLOC(m_UserRecordHeader.nInstances*sizeof(_MX00_TEMPLATE_INSTANCE));
    pPtr = Unpack(m_pTemplateInstances, pPtr, m_UserRecordHeader.nInstances*sizeof(_MX00_TEMPLATE_INSTANCE));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Set_User_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Set_User_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Set_User_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Set_User_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
void Atomic_ID_Set_User_Record::SetUserRecords(const _MX00_ID_USER_RECORD& rec, const _MX00_TEMPLATE_INSTANCE instanceRecords[])
{
    if(m_pTemplateInstances)
    {
        FREE(m_pTemplateInstances);
        m_pTemplateInstances = NULL;
    }
    m_UserRecordHeader = rec;
    m_pTemplateInstances = (_MX00_TEMPLATE_INSTANCE*)MALLOC(sizeof(_MX00_TEMPLATE_INSTANCE)*m_UserRecordHeader.nInstances);
    _MX00_TEMPLATE_INSTANCE* pHdPtr = m_pTemplateInstances;
    //
    for( uint ii = 0 ; ii < m_UserRecordHeader.nInstances ; ii++)
    {
        memcpy(pHdPtr, &instanceRecords[ii], sizeof(_MX00_TEMPLATE_INSTANCE));
        pHdPtr++;
    }
}
_MX00_TEMPLATE_INSTANCE* Atomic_ID_Set_User_Record::GetFingerInstance(uint nInstance)
{
    if( nInstance > m_UserRecordHeader.nInstances) return NULL;
    if(m_pTemplateInstances == NULL) return NULL;
    return &m_pTemplateInstances[nInstance];
}


// CMD_XXX_XXXX *************************

Macro_ID_Enroll_User_Record::Macro_ID_Enroll_User_Record()
{
     m_nCmd = CMD_ID_ENROLL_USER_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_ID_Enroll_User_Record::~Macro_ID_Enroll_User_Record()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Enroll_User_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nUserRecord, sizeof(_MX00_ID_USER_RECORD));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Enroll_User_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nUserRecord, pPtr, sizeof(_MX00_ID_USER_RECORD));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Enroll_User_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Enroll_User_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Enroll_User_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Enroll_User_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_XXX_XXXX *************************

Macro_ID_Verify_User_Record::Macro_ID_Verify_User_Record()
{
     m_nCmd = CMD_ID_VERIFY_USER_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_ID_Verify_User_Record::~Macro_ID_Verify_User_Record()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_User_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecord, sizeof(_MX00_ID_USER_RECORD));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_User_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(_MX00_ID_USER_RECORD));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_User_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_User_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Verify_User_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Verify_User_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_XXX_XXXX *************************

Macro_ID_Identify_378::Macro_ID_Identify_378()
{
     m_nCmd = CMD_ID_IDENTIFY_378;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

    m_pTemplate    = NULL;
    m_nTemplateSize = 0;
}
Macro_ID_Identify_378::~Macro_ID_Identify_378()
{
    if(m_pTemplate) FREE(m_pTemplate); m_pTemplate = NULL;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Identify_378::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=m_nTemplateSize;
    m_nChallengeBufferSize+=sizeof(m_nTemplateSize);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
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
// Unpacks packet passed in into interal data structure
bool Macro_ID_Identify_378::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(m_nTemplateSize));
        if(NULL == pPtr)
           return false;

    if( m_nTemplateSize > MAX_378_TEMPLATE_SIZE ) return false;
    if( m_pTemplate ) FREE(m_pTemplate);
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Identify_378::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Identify_378::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Identify_378::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Identify_378::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Macro_ID_Identify_378::SetTemplate(uchar* p378Template, uint nSizeTemplate)
{
    if( m_pTemplate ) FREE(m_pTemplate);
    m_pTemplate = (uchar*)MALLOC(nSizeTemplate);
    memcpy(m_pTemplate, p378Template, nSizeTemplate);
    m_nTemplateSize = nSizeTemplate;
    return true;
}

/****** CMD_ID_IDENTIFY ********/

// CMD_XXX_XXXX *************************

Macro_ID_Identify::Macro_ID_Identify()
{
     m_nCmd = CMD_ID_IDENTIFY;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_ID_Identify::~Macro_ID_Identify()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Identify::PackChallenge(uchar** pPacket, uint& nSize)
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
bool Macro_ID_Identify::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Identify::PackResponse(uchar** pPacket, uint& nSize)
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
bool Macro_ID_Identify::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Identify::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Identify::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

//

// CMD_XXX_XXXX *************************

Macro_ID_Delete_DB::Macro_ID_Delete_DB()
{
     m_nCmd = CMD_ID_DELETE_DB;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_ID_Delete_DB::~Macro_ID_Delete_DB()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Delete_DB::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(m_nDBToDelete);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    //
    pPtr = Pack(pPtr, &m_nDBToDelete, sizeof(m_nDBToDelete));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Delete_DB::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nDBToDelete, pPtr, sizeof(m_nDBToDelete));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Delete_DB::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;



    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Delete_DB::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Delete_DB::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Delete_DB::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_XXX_XXXX *************************

Atomic_ID_Get_DB_Metrics::Atomic_ID_Get_DB_Metrics()
{
     m_nCmd = CMD_ID_GET_DB_METRICS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Get_DB_Metrics::~Atomic_ID_Get_DB_Metrics()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_DB_Metrics::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_DB_METRICS);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_dbMetrics, sizeof(_MX00_DB_METRICS));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_DB_Metrics::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_dbMetrics, pPtr, sizeof(m_dbMetrics));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_DB_Metrics::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize+=sizeof(_MX00_DB_METRICS);
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_dbMetrics, sizeof(m_dbMetrics));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_DB_Metrics::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_dbMetrics, pPtr, sizeof(_MX00_DB_METRICS));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_DB_Metrics::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_DB_Metrics::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

// CMD_XXX_XXXX *************************

Atomic_ID_Get_System_Metrics::Atomic_ID_Get_System_Metrics()
{
     m_nCmd = CMD_ID_GET_SYSTEM_METRICS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pDBMetrics = NULL;
}
Atomic_ID_Get_System_Metrics::~Atomic_ID_Get_System_Metrics()
{
    if(m_pDBMetrics) FREE(m_pDBMetrics);
    m_pDBMetrics = NULL;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_System_Metrics::PackChallenge(uchar** pPacket, uint& nSize)
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
bool Atomic_ID_Get_System_Metrics::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_System_Metrics::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize+=sizeof(_MX00_DB_METRICS)*m_nMetricsTracked;
    SetArguement(m_nMetricsTracked);
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Pack structures
    for(uint ii = 0; ii < m_nMetricsTracked ; ii++)
    {
        pPtr = Pack(pPtr, &m_pDBMetrics[ii], sizeof(_MX00_DB_METRICS));
                if(NULL == pPtr)
                    return false;

    }
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_System_Metrics::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    // IMPLEMENT
    m_nMetricsTracked = (uint)m_nArg;
    if(m_pDBMetrics) FREE(m_pDBMetrics);
    m_pDBMetrics = (_MX00_DB_METRICS*)MALLOC(m_nMetricsTracked*sizeof(_MX00_DB_METRICS));
    // Pack structures
    for(uint ii = 0; ii < m_nMetricsTracked ; ii++)
    {
        pPtr = Unpack(&m_pDBMetrics[ii], pPtr, sizeof(_MX00_DB_METRICS));
                if(NULL == pPtr)
                   return false;

    }
    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_System_Metrics::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;

}
// How large is the Response packet?
int  Atomic_ID_Get_System_Metrics::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
void Atomic_ID_Get_System_Metrics::SetNumMetrics(uint nMetrics)
{
    if(m_pDBMetrics)
    {
        FREE(m_pDBMetrics);
        m_pDBMetrics = NULL;
    }
    m_nMetricsTracked = nMetrics;
    if(nMetrics == 0) return;
    m_pDBMetrics = (_MX00_DB_METRICS*)MALLOC(sizeof(_MX00_DB_METRICS)*nMetrics);
    m_nMetricsTracked = nMetrics;
}
void Atomic_ID_Get_System_Metrics::AddMetric(_MX00_DB_METRICS* pMetric, uint nMetricToSet)
{
    if( nMetricToSet > (m_nMetricsTracked-1)) return;
    memcpy(&m_pDBMetrics[nMetricToSet], pMetric, sizeof(_MX00_DB_METRICS));
}

// CMD_XXX_XXXX *************************

Atomic_ID_Get_Result::Atomic_ID_Get_Result()
{
     m_nCmd = CMD_ID_GET_RESULT;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Get_Result::~Atomic_ID_Get_Result()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Result::PackChallenge(uchar** pPacket, uint& nSize)
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
bool Atomic_ID_Get_Result::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Result::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;

    m_nResponseBufferSize+=sizeof(_MX00_ID_RESULT);
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nID_Result, sizeof(_MX00_ID_RESULT));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Result::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nID_Result, pPtr, sizeof(_MX00_ID_RESULT));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_Result::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_Result::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

Atomic_ID_Delete_User_Record::Atomic_ID_Delete_User_Record()
{
     m_nCmd = CMD_ID_DELETE_USER_RECORD;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Delete_User_Record::~Atomic_ID_Delete_User_Record()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Delete_User_Record::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_USER_RECORD);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Delete_User_Record::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Delete_User_Record::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Delete_User_Record::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Delete_User_Record::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Delete_User_Record::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/* Set parameters */

Atomic_ID_Set_Parameters::Atomic_ID_Set_Parameters()
{
     m_nCmd = CMD_ID_SET_PARAMETERS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Set_Parameters::~Atomic_ID_Set_Parameters()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Set_Parameters::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=sizeof(_MX00_ID_PARAMETERS);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_Parameters, sizeof(m_Parameters));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Set_Parameters::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_Parameters, pPtr, sizeof(m_Parameters));
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Set_Parameters::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Set_Parameters::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Set_Parameters::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Set_Parameters::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/* Get Parameters */

Atomic_ID_Get_Parameters::Atomic_ID_Get_Parameters()
{
     m_nCmd = CMD_ID_GET_PARAMETERS;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Atomic_ID_Get_Parameters::~Atomic_ID_Get_Parameters()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Parameters::PackChallenge(uchar** pPacket, uint& nSize)
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
bool Atomic_ID_Get_Parameters::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Parameters::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize+=sizeof(m_Parameters);
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_Parameters, sizeof(m_Parameters));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Parameters::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_Parameters, pPtr, sizeof(m_Parameters));
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_Parameters::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_Parameters::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_XXX_XXXX *************************

Macro_ID_Verify_378::Macro_ID_Verify_378()
{
     m_nCmd = CMD_ID_VERIFY_378;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pTemplate = NULL;
}
Macro_ID_Verify_378::~Macro_ID_Verify_378()
{
    if(m_pTemplate) FREE(m_pTemplate);
}
// ICmd
void Macro_ID_Verify_378::SetTemplate(uchar* pTpl, uint nTplSize)
{
    if(m_pTemplate) FREE(m_pTemplate);
    m_pTemplate = (uchar*)MALLOC(MAX_TEMPLATE_SIZE);
    if(nTplSize > MAX_TEMPLATE_SIZE) return;
    memcpy(m_pTemplate, pTpl, nTplSize);
    m_nTemplateSize = nTplSize;
    return;
}
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_378::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+= sizeof(_MX00_ID_USER_RECORD);
    m_nChallengeBufferSize+= sizeof(uint);
    m_nChallengeBufferSize+= m_nTemplateSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_UserRecord, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nTemplateSize, sizeof(uint));
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
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_378::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_UserRecord, pPtr, sizeof(m_UserRecord));
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nTemplateSize, pPtr, sizeof(uint));
        if(NULL == pPtr)
           return false;

    if(m_pTemplate) FREE(m_pTemplate);
    m_pTemplate = (uchar*)MALLOC(m_nTemplateSize);
    pPtr = Unpack(m_pTemplate, pPtr, m_nTemplateSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_378::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_378::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Verify_378::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Verify_378::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_VERIFY_378 *************************

Macro_Verify_378::Macro_Verify_378()
{
     m_nCmd = CMD_VERIFY_378;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pGalleryTemplate = NULL;
     m_nGalleryTemplateSize = 0;
}
Macro_Verify_378::~Macro_Verify_378()
{
    if(m_pGalleryTemplate)
    {
        FREE(m_pGalleryTemplate);
        m_pGalleryTemplate = NULL;
    }
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_Verify_378::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Calculate Challenge Buffer Size. Envelope + Template + sizeof(TemplateSize)
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_nChallengeBufferSize += m_nGalleryTemplateSize + sizeof(uint);
    // Create Opaque Data
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Copy size
    memcpy(pPtr, &m_nGalleryTemplateSize, sizeof(m_nGalleryTemplateSize));
    pPtr+=sizeof(m_nGalleryTemplateSize);
    memcpy(pPtr,m_pGalleryTemplate, m_nGalleryTemplateSize);
    pPtr+=m_nGalleryTemplateSize;

    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_Verify_378::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    memcpy(&m_nGalleryTemplateSize, pPtr, sizeof(m_nGalleryTemplateSize));
    pPtr+=sizeof(m_nGalleryTemplateSize);
    if(m_nGalleryTemplateSize > nSize) return false; // simple sanity check
    m_pGalleryTemplate = (uchar*)MALLOC(m_nGalleryTemplateSize);
    memcpy(m_pGalleryTemplate, pPtr, m_nGalleryTemplateSize);
    pPtr+=m_nGalleryTemplateSize;

    return true;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Verify_378::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    m_nResponseBufferSize = 0;
    m_nResponseBufferSize += ENVELOPE_INFO_SIZE;                    // Static Envelope size
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_Verify_378::UnpackResponse(const uchar* pPacket, uint nSize)
{
    return true;
}
// How large is the Challenge packet?
int  Macro_Verify_378::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Verify_378::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}
bool Macro_Verify_378::SetGalleryTemplateData(uchar* pTemplate, uint nTemplateSize)
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

// CMD_XXX_XXXX *************************

Macro_Save_Last_Capture::Macro_Save_Last_Capture()
{
     m_nCmd = CMD_SAVE_LAST_CAPTURE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
}
Macro_Save_Last_Capture::~Macro_Save_Last_Capture()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_Save_Last_Capture::PackChallenge(uchar** pPacket, uint& nSize)
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
bool Macro_Save_Last_Capture::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_Save_Last_Capture::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;


    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return false;
}
// Unpacks packet passed in into interal data structure
bool Macro_Save_Last_Capture::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    return false;
}
// How large is the Challenge packet?
int  Macro_Save_Last_Capture::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_Save_Last_Capture::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// CMD_ID_VERIFY_MANY *************************

Macro_ID_Verify_Many::Macro_ID_Verify_Many()
{
     m_nCmd = CMD_ID_VERIFY_MANY;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_pTemplates = NULL;
     m_pSzTemplates = NULL;
     m_nNumTemplates = 0;
     m_nSzAllTemplates = 0;
}
Macro_ID_Verify_Many::~Macro_ID_Verify_Many()
{

    if(m_pSzTemplates)
    {
        FREE(m_pSzTemplates);
        m_pSzTemplates = NULL;
    }

    if(m_pTemplates)
    {
        FREE(m_pTemplates);
        m_pTemplates = NULL;
    }

    m_nNumTemplates = 0;
    m_nSzAllTemplates = 0;
}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_Many::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(uint);
    m_nChallengeBufferSize += sizeof(uint)* m_nNumTemplates;
    m_nChallengeBufferSize += m_nSzAllTemplates;

    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);

    pPtr = Pack(pPtr, &m_nNumTemplates, sizeof(unsigned int));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pSzTemplates, sizeof(uint)*m_nNumTemplates);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pTemplates, m_nSzAllTemplates);
    if(NULL == pPtr)
       return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_Many::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    if(m_pSzTemplates)
    {
        FREE(m_pSzTemplates);
    }
    if(m_pTemplates)
    {
         FREE(m_pTemplates);
         m_pTemplates = NULL;

    }

    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nNumTemplates, pPtr, sizeof(uint));
     if(NULL == pPtr)
           return false;

    m_pSzTemplates = (uint*)MALLOC(m_nNumTemplates*sizeof(uint));
    pPtr = Unpack(m_pSzTemplates, pPtr, m_nNumTemplates*sizeof(uint));
    if(NULL == pPtr)
           return false;

    m_nSzAllTemplates = 0;
    for(uint ii=0; ii< m_nNumTemplates; ii++)
    {
        m_nSzAllTemplates += m_pSzTemplates[ii];
    }

    m_pTemplates = (uchar*)MALLOC(m_nSzAllTemplates);
    pPtr = Unpack(m_pTemplates, pPtr, m_nSzAllTemplates);
    if(NULL == pPtr)
           return false;

    return true;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Verify_Many::PackResponse(uchar** pPacket, uint& nSize)
{
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr)
    {
       return false;
    }
    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Verify_Many::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(NULL == pPtr)
    {
       return false;
    }
    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Verify_Many::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Verify_Many::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

bool Macro_ID_Verify_Many::SetTemplates(uchar** pTemplates, uint* pSizeTemplates, uint nNumTemplates)
{
    if(m_pSzTemplates)
    {
        FREE(m_pSzTemplates);
        m_pSzTemplates = NULL;
    }

    if(m_pTemplates)
    {
        FREE(m_pTemplates);
        m_pTemplates = NULL;
    }

    if(nNumTemplates == 0 || nNumTemplates > 30) return false;

    m_nNumTemplates = nNumTemplates;
    m_pSzTemplates = (uint*)MALLOC(nNumTemplates* sizeof(uint));
    memcpy(m_pSzTemplates, pSizeTemplates, sizeof(uint)*nNumTemplates);

    m_nSzAllTemplates = 0;
    for(uint ii=0; ii< nNumTemplates; ii++)
    {
        if(m_pSzTemplates[ii] > MAX_TEMPLATE_SIZE) return false;
        m_nSzAllTemplates += m_pSzTemplates[ii];
    }

    m_pTemplates = (uchar*)MALLOC(m_nSzAllTemplates);
    uchar* pTplsTemp = m_pTemplates;
    for(uint jj=0; jj< nNumTemplates; jj++)
    {
        memcpy(pTplsTemp, pTemplates[jj], m_pSzTemplates[jj]);
        pTplsTemp += m_pSzTemplates[jj];
    }
    return true;
}

// Atomic_ID_Get_Template *************************
Atomic_ID_Get_Template::Atomic_ID_Get_Template()
{
     m_nCmd = CMD_ID_GET_TEMPLATE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pTemplate = NULL;
     m_nTemplateSize = 0;
     m_nCaptureType = CAPTURE_IMAGE;
}
Atomic_ID_Get_Template::~Atomic_ID_Get_Template()
{
    if(m_pTemplate)
    {
        FREE(m_pTemplate);
        m_pTemplate = NULL;
    }
}
// ICmd
void Atomic_ID_Get_Template::SetTemplate(uchar* pTemplate, uint nTemplateSize)
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
uchar*    Atomic_ID_Get_Template::GetTemplate(uint& nTemplateSize)
{
    nTemplateSize = m_nTemplateSize;
    return m_pTemplate;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Template::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nCaptureType);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nCaptureType, sizeof(m_nCaptureType));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Template::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(pPtr ==NULL)
    {
        return false;
    }

    // Unpack Image Size
    pPtr = Unpack(&m_nCaptureType, pPtr, sizeof(m_nCaptureType));

    return (pPtr!=NULL)? true:false;

}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Template::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Template::UnpackResponse(const uchar* pPacket, uint nSize)
{
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
int  Atomic_ID_Get_Template::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_Template::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// Atomic_ID_Get_Image *************************
Atomic_ID_Get_Image::Atomic_ID_Get_Image()
{
     m_nCmd = CMD_ID_GET_IMAGE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_pImage = NULL;
     m_nImageSize = 0;
     m_nCaptureType = CAPTURE_IMAGE;
}
Atomic_ID_Get_Image::~Atomic_ID_Get_Image()
{
    if(m_pImage)
    {
        FREE(m_pImage);
        m_pImage = NULL;
    }
}
// ICmd
void Atomic_ID_Get_Image::SetImage(uchar* pImage, uint nImageSize)
{
    if(m_pImage)
    {
        FREE(m_pImage);
        m_pImage = NULL;
    }
    m_pImage = (uchar*)MALLOC(nImageSize);
    memcpy(m_pImage, pImage, nImageSize);
    m_nImageSize = nImageSize;
}
uchar*    Atomic_ID_Get_Image::GetImage(uint& nImageSize)
{
    nImageSize = m_nImageSize;
    return m_pImage;
}
 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Image::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nCaptureType);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nCaptureType, sizeof(m_nCaptureType));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Image::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(pPtr ==NULL)
    {
        return false;
    }

    // Unpack Image Size
    pPtr = Unpack(&m_nCaptureType, pPtr, sizeof(m_nCaptureType));

    return (pPtr!=NULL)? true:false;

}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Image::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Calculate Response Buffer Size.
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nImageSize);
    m_nResponseBufferSize+=m_nImageSize;
    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Populate Response Buffer
    pPtr = Pack(pPtr, &m_nImageSize, sizeof(m_nImageSize));
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, m_pImage, m_nImageSize);
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Image::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nImageSize, pPtr, sizeof(m_nImageSize));
    if(NULL == pPtr)
        return false;

    if(m_pImage)
    {
        FREE(m_pImage);
        m_pImage = NULL;
    }
    m_pImage = (uchar*)MALLOC(m_nImageSize);
    pPtr = Unpack(m_pImage, pPtr, m_nImageSize);
        if(NULL == pPtr)
           return false;


    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_Image::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_Image::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}


// Atomic_ID_Get_Spoof_Score *************************
Atomic_ID_Get_Spoof_Score::Atomic_ID_Get_Spoof_Score()
{
    m_nCmd = CMD_ID_GET_SPOOF_SCORE;
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;
     m_nSpoofScore = 0;
     m_nCaptureType = CAPTURE_IMAGE;
}
Atomic_ID_Get_Spoof_Score::~Atomic_ID_Get_Spoof_Score()
{

}
// ICmd

 // Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Spoof_Score::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize += sizeof(m_nCaptureType);
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    pPtr = Pack(pPtr, &m_nCaptureType, sizeof(m_nCaptureType));
        if(NULL == pPtr)
           return false;

    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Spoof_Score::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(pPtr ==NULL)
    {
        return false;
    }

    // Unpack Image Size
    pPtr = Unpack(&m_nCaptureType, pPtr, sizeof(m_nCaptureType));

    return (pPtr!=NULL)? true:false;

}
// Takes content of Command, and packs it into pPacket
bool Atomic_ID_Get_Spoof_Score::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Calculate Response Buffer Size.
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_nResponseBufferSize+=sizeof(m_nSpoofScore);

    uchar* pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        if(NULL == pPtr)
           return false;

    // Populate Response Buffer
    pPtr = Pack(pPtr, &m_nSpoofScore, sizeof(m_nSpoofScore));
        if(NULL == pPtr)
           return false;

    // Response
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Atomic_ID_Get_Spoof_Score::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
        if(NULL == pPtr)
           return false;

    pPtr = Unpack(&m_nSpoofScore, pPtr, sizeof(m_nSpoofScore));
    if(NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Atomic_ID_Get_Spoof_Score::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Atomic_ID_Get_Spoof_Score::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/*
**
**    Macro_ID_Set_API_Key
**
*/

Macro_ID_Set_API_Key::Macro_ID_Set_API_Key()
{
    m_nCmd = CMD_ID_SET_API_KEY;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
    m_pData = NULL;
    m_nDataLen = 0;
}
Macro_ID_Set_API_Key::~Macro_ID_Set_API_Key()
{
    if (m_pData) {
        FREE(m_pData);
        m_pData = NULL;
    }
    m_nDataLen = 0;
}
// ICmd
bool Macro_ID_Set_API_Key::SetAPIKey(const char* pAPIKeyString, uint nStrlenKey)
{
    if (pAPIKeyString == NULL || nStrlenKey == 0) {
        SetArguement(1);
        m_pData = NULL;
        m_nDataLen = 0;
        return true;
    }
    m_pData = (uchar*)MALLOC(nStrlenKey);
    if (m_pData == NULL) {
        return false;
    }
    memcpy(m_pData, pAPIKeyString, nStrlenKey);
    m_nDataLen = nStrlenKey;
    return true;
}

bool Macro_ID_Set_API_Key::GetAPIKey(uchar** pAPIKeyString, uint& nStrlenKey)
{
    *pAPIKeyString = (uchar*)MALLOC(m_nDataLen);
    if (m_pData == NULL) {
        *pAPIKeyString = NULL;
        nStrlenKey = 0;
        return true;
    }
    if (*pAPIKeyString == NULL) {
        return false;
    }
    nStrlenKey = m_nDataLen;
    memcpy(*pAPIKeyString, m_pData, m_nDataLen);
    return true;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Set_API_Key::PackChallenge(uchar** pPacket, uint& nSize)
{
    // Image Size
    m_nChallengeBufferSize += sizeof(m_nDataLen);
    // Image Data
    m_nChallengeBufferSize += m_nDataLen;
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;

    // Generate Response Header
    // Pack Image Size
    pPtr = Pack(pPtr, &m_nDataLen, sizeof(m_nDataLen));
    if (NULL == pPtr)
        return false;

    // Pack Image
    pPtr = Pack(pPtr, m_pData, m_nDataLen);
    if (NULL == pPtr)
        return false;

    // Response
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    // Fill in some stuff
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Set_API_Key::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    // Unpack Image Size
    pPtr = Unpack(&m_nDataLen, pPtr, sizeof(m_nDataLen));
    if (NULL == pPtr)
        return false;

    // Allocate space, and Unpack Image
    m_pData = (uchar*)MALLOC(m_nDataLen);
    pPtr = Unpack(m_pData, pPtr, m_nDataLen);
    if (NULL == pPtr)
        return false;


    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Set_API_Key::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Macro_ID_Set_API_Key::UnpackResponse(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Set_API_Key::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Set_API_Key::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/*
**
**    Macro_ID_Commit
**
*/
Macro_ID_Commit::Macro_ID_Commit()
{
    m_nCmd = CMD_ID_COMMIT;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Macro_ID_Commit::~Macro_ID_Commit()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Commit::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Commit::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Commit::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Macro_ID_Commit::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Commit::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Commit::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

/*
**
**    Macro_ID_Purge_DB_All
**
*/
Macro_ID_Purge_DB_All::Macro_ID_Purge_DB_All()
{
    m_nCmd = CMD_ID_PURGE_DB_ALL;
    // Challenge buffer size...
    m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
    m_pChallengeBuffer = NULL;
    m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
    m_pResponseBuffer = NULL;
}
Macro_ID_Purge_DB_All::~Macro_ID_Purge_DB_All()
{

}
// ICmd
 // Takes content of Command, and packs it into pPacket
bool Macro_ID_Purge_DB_All::PackChallenge(uchar** pPacket, uint& nSize)
{
    uchar* pPtr = GenerateChallengeHeader(m_nArg, m_nChallengeBufferSize - ENVELOPE_INFO_SIZE);
    if (NULL == pPtr)
        return false;


    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into interal data structure
bool Macro_ID_Purge_DB_All::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);

    return (pPtr != NULL) ? true : false;
}
// Takes content of Command, and packs it into pPacket
bool Macro_ID_Purge_DB_All::PackResponse(uchar** pPacket, uint& nSize)
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
// Unpacks packet passed in into interal data structure
bool Macro_ID_Purge_DB_All::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if (NULL == pPtr)
        return false;

    return true;
}
// How large is the Challenge packet?
int  Macro_ID_Purge_DB_All::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  Macro_ID_Purge_DB_All::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}

