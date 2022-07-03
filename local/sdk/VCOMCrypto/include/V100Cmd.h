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
#pragma once
#include "ICmd.h"
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "string.h"


// **************************************  CMD_CANCEL_OPERATION  ***********************/
class Atomic_Cancel_Operation : public ICmd
{
public:
    Atomic_Cancel_Operation();
    ~Atomic_Cancel_Operation();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    // Execute command.
private:
    // SPECIALIZED DATA GOES HERE
};


// **************************************  CMD_ERROR  **********************************/
class Atomic_Error : public ICmd
{
public:
    Atomic_Error();
    ~Atomic_Error();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    // Set Error Code
    virtual bool SetErrorCode(_V100_GENERAL_ERROR_CODES code) { m_nArg = (_V100_GENERAL_ERROR_CODES)code; return true; }
    // Get Error Code
    virtual _V100_GENERAL_ERROR_CODES  GetErrorCode() { return (_V100_GENERAL_ERROR_CODES)m_nArg; }
private:
};


// **************************************  CMD_GET_ACQ_STATUS  *************************/
class Atomic_Get_Acq_Status : public ICmd
{
public:
    Atomic_Get_Acq_Status();
    ~Atomic_Get_Acq_Status();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    void    SetAcqStatus(_V100_ACQ_STATUS_TYPE acq) { m_ACQStatus = acq; }
    _V100_ACQ_STATUS_TYPE GetAcqStatus() { return m_ACQStatus; }
private:
    // SPECIALIZED DATA GOES HERE
    _V100_ACQ_STATUS_TYPE    m_ACQStatus;
};


// **************************************  CMD_GET_CONFIG  *****************************/
class Atomic_Get_Config :  public ICmd
{
public:
    Atomic_Get_Config();
    ~Atomic_Get_Config();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    // SPECIFIC
    void SetConfiguration(volatile _V100_INTERFACE_CONFIGURATION_TYPE* conf)
    {
        memcpy(&m_conf,(void*)conf,sizeof(_V100_INTERFACE_CONFIGURATION_TYPE));
        m_nSizeOfStruct = sizeof(_V100_INTERFACE_CONFIGURATION_TYPE);

    };
    _V100_INTERFACE_CONFIGURATION_TYPE GetConfiguration()
    {
        return m_conf;
    };
private:
    // V100 Interface Configuration Type
    _V100_INTERFACE_CONFIGURATION_TYPE  m_conf;
    uint                                m_nSizeOfStruct;
};


// **************************************  CMD_GET_CMD  ********************************/
class Atomic_Get_Cmd : public ICmd
{
public:
    Atomic_Get_Cmd();
    ~Atomic_Get_Cmd();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    //
    _V100_INTERFACE_COMMAND_TYPE    GetCmd()      { return m_nICT; };
    bool    SetCmd(_V100_INTERFACE_COMMAND_TYPE* _ICT) { m_nICT = *_ICT; return true; };
private:
    // SPECIALIZED DATA GOES HERE
    _V100_INTERFACE_COMMAND_TYPE    m_nICT;
    uint                            m_nSize;
};


// **************************************  CMD_GET_OP_STATUS  **************************/
class Atomic_Get_OP_Status : public ICmd
{
public:
    Atomic_Get_OP_Status();
    ~Atomic_Get_OP_Status();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    //
    virtual void Exec(void);
    //
    bool    GetOPStatus(_V100_OP_STATUS* pRec) { memcpy(pRec, &m_OPStatus, sizeof(m_OPStatus)); return true; }
    bool    SetOPStatus(_V100_OP_STATUS* pRec) { memcpy(&m_OPStatus, pRec, sizeof(m_OPStatus)); return true; }
private:
    _V100_OP_STATUS m_OPStatus;
};


// **************************************  CMD_GET_SERIAL_NUMBER  **********************/
class Atomic_Get_Serial : public ICmd
{
public:
    Atomic_Get_Serial();
    ~Atomic_Get_Serial();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    void SetSerialNumber(uint serial) { m_nSerialNumber = serial; }
    uint GetSerialNumber() { return m_nSerialNumber; }
private:
    // SPECIALIZED DATA GOES HERE
    uint m_nSerialNumber;
    uint m_nSize;
};


// **************************************  CMD_GET_STATUS  *****************************/
class Atomic_Get_Status : public ICmd
{
public:
    Atomic_Get_Status();
    ~Atomic_Get_Status();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    // SPEC
    void SetInterfaceStatusType(_V100_INTERFACE_STATUS_TYPE st) { m_nIST = st; }
    _V100_INTERFACE_STATUS_TYPE  GetInterfaceStatusType() { return m_nIST; }
private:
    // SPECIALIZED DATA GOES HERE
    uint                        m_nStatusSize;
    _V100_INTERFACE_STATUS_TYPE m_nIST;
};


// **************************************  CMD_GET_SYSTEM_STATE  ***********************/
class Atomic_Get_System_State : public ICmd
{
public:
    Atomic_Get_System_State();
    ~Atomic_Get_System_State();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    // Get System Metrics
    virtual _V100_SYSTEM_DIAGNOSTICS* GetSystemMetricsStruct() { return &m_Diagnostics; }
    // Set System Metrics
    virtual void SetSystemMetricsStruct(_V100_SYSTEM_DIAGNOSTICS* met) { m_Diagnostics = *met; }
private:
    // SPECIALIZED DATA GOES HERE
    _V100_SYSTEM_DIAGNOSTICS    m_Diagnostics;
};


// **************************************  CMD_GET_TAG  ********************************/
class Atomic_Get_Tag : public ICmd
{
public:
    Atomic_Get_Tag();
    ~Atomic_Get_Tag();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    //
    bool SetTag(char* pTag, ushort& nSize);
    bool GetTag(char** pTag, ushort& nSize);
private:
    char*    m_pTag;
    ushort    m_nTagSize;
};


// **************************************  CMD_RESET  **********************************/
class Atomic_Reset : public ICmd
{
public:
    Atomic_Reset();
    ~Atomic_Reset();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
private:
    // SPECIALIZED DATA GOES HERE
};


// **************************************  CMD_SET_TAG  ********************************/
class Atomic_Set_Tag : public ICmd
{
public:
    Atomic_Set_Tag();
    ~Atomic_Set_Tag();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    //
    bool SetTag(char* pTag, ushort nSize);
    bool GetTag(char** pTag, ushort& nSize);
private:
    char*    m_pTag;
    ushort     m_nTagSize;
};


// **************************************  CMD_UPDATE_FIRMWARE  ************************/

class Macro_Update_Firmware : public ICmd {
public:
    Macro_Update_Firmware();
    ~Macro_Update_Firmware();
    // ICmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into internal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into internal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
    //
    bool   SetData(const uchar* pData, uint nDataSize);
    uchar* GetData();
    uint   GetDataSize();
private:
    // SPECIALIZED DATA GOES HERE
    uchar*  m_pFWData;
    uint    m_nDataSize;
};

// **************************************  CMD_UPDATE_FIRMWARE  ************************/
