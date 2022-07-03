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
#pragma once
#include "ICmd.h"
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "string.h"

/* ----------------------- Generic_Command -------------------------*/
class Generic_Command : public ICmd
{
public:
    Generic_Command();
    ~Generic_Command();
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

/* ----------------------- Atomic_Get_Config -------------------------*/
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
    };
    _V100_INTERFACE_CONFIGURATION_TYPE GetConfiguration()
    {
        return m_conf;
    };
private:
    // V100 Interface Configuration Type
    _V100_INTERFACE_CONFIGURATION_TYPE    m_conf;
    uint                                m_nSizeOfStruct;
};

/* ----------------------- Atomic_Arm_Trigger -------------------------*/
class Atomic_Arm_Trigger :  public ICmd
{
public:
    Atomic_Arm_Trigger();
    ~Atomic_Arm_Trigger();
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
    // Specialized Interfaces
    bool SetTriggerType(uint _Trigger) { m_nArg = (uint)_Trigger; return true;}
    bool GetTriggerType(uint& _Trigger);
private:
    // SPECIALIZED DATA GOES HERE
};

/* ----------------------- Macro_Vid_Stream -------------------------*/
class Macro_Vid_Stream :  public ICmd
{
public:
    Macro_Vid_Stream();
    ~Macro_Vid_Stream();
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
    void SetVidStreamMode(_V100_VID_STREAM_MODE mode);
    void GetVidStreamMode(_V100_VID_STREAM_MODE& mode);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_VID_STREAM_MODE m_nVidStreamMode;
};

/* ----------------------- Macro_Match -------------------------*/
class Macro_Match :  public ICmd
{
public:
    Macro_Match();
    ~Macro_Match();
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
    // SPECIALIZED DATA GOES HERE
    bool    SetGalleryTemplateData(uchar* pTemplate, uint nTemplateSize);
    bool    SetProbeTemplateData(uchar* pTemplate, uint nTemplateSize);
    uchar*  GetGalleryTemplate();
    uchar*  GetProbeTemplate();
    uint    GetGalleryTemplateSize();
    uint    GetProbeTemplateSize();
    bool    SetMatchScore(uint MatchScore) { m_nScore = MatchScore; return true; }
    int        GetMatchScore() { return m_nScore;}
private:
    // SPECIALIZED DATA GOES HERE
    uchar*    m_pGalleryTemplate;
    uint    m_nGalleryTemplateSize;
    uchar*    m_pProbeTemplate;
    uint    m_nProbeTemplateSize;
    uint    m_nScore;
};

/* ----------------------- Atomic_Error -------------------------*/
class Atomic_Error :  public ICmd
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
    virtual bool SetErrorCode(_V100_GENERAL_ERROR_CODES code) { m_nArg = (_V100_GENERAL_ERROR_CODES)code; return true;}
    // Get Error Code
    virtual _V100_GENERAL_ERROR_CODES  GetErrorCode() { return (_V100_GENERAL_ERROR_CODES)m_nArg;}
private:
};

/* ----------------------- Atomic_Get_Image -------------------------*/
class Atomic_Get_Image :  public ICmd
{
public:
    Atomic_Get_Image();
    ~Atomic_Get_Image();
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
    // Set/Get Image Type
    bool    SetImageType(_V100_IMAGE_TYPE type);
    bool    GetImageType(_V100_IMAGE_TYPE& type);
    // Set/Get Image Buffer
    bool    SetImageMetrics(uchar* pImage, uint nImageSize);
    bool    GetImageMetrics(uchar** pImage, uint& nImageSize);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_IMAGE_TYPE    m_nImage_Type;
    uchar*                m_pImage;
    uint                m_nImageSize;

};

/* ----------------------- Atomic_Set_Image -------------------------*/
class Atomic_Set_Image :  public ICmd
{
public:
    Atomic_Set_Image();
    ~Atomic_Set_Image();
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
    // Get Image Size
    uint GetImageSize() { return m_nImageSize; }
    // Set Image Size
    void SetImageSize(uint _ImageSize) { m_nImageSize = _ImageSize;}
    // Get Image Ptr
    uchar* GetImage() { return m_pImage;}
    // Set Image Ptr
    void   SetImage(uchar* _Image) { m_pImage = _Image;}    // Shallow Copy
private:
    // SPECIALIZED DATA GOES HERE
    uchar*        m_pImage;            // The Image
    uint        m_nImageSize;        // Size of m_pImage
};

/* ----------------------- Atomic_Get_Composite_Image -------------------------*/
class Atomic_Get_Composite_Image :  public ICmd
{
public:
    Atomic_Get_Composite_Image();
    ~Atomic_Get_Composite_Image();
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
    // Get Image Size
    uint GetImageSize() { return m_nImageSize; }
    // Get Spoof
    int  GetSpoofValue() { return m_nSpoofValue; }
    // Set Spoof
    void SetSpoofValue(int _SpoofValue) { m_nSpoofValue = _SpoofValue;}
    // Get Image
    uchar* GetImage() { return m_pImage;}
    // Set Image
    void   SetImage(uchar* _Image, uint _ImageSize);
private:
    // SPECIALIZED DATA GOES HERE
    uchar*        m_pImage;            // The Image
    uint        m_nImageSize;        // Size of m_pImage
    int            m_nSpoofValue;        // Spoof Value

};

/* ----------------------- Atomic_Set_Composite_Image -------------------------*/
class Atomic_Set_Composite_Image :  public ICmd
{
public:
    Atomic_Set_Composite_Image();
    ~Atomic_Set_Composite_Image();
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
    // Get Image Size
    uint GetImageSize() { return m_nImageSize; }
    // Set Image Size
    void SetImageSize(uint _ImageSize) { m_nImageSize = _ImageSize;}
    // Get Image Ptr
    uchar* GetImage() { return m_pImage;}
    // Set Image Ptr
    void   SetImage(uchar* _Image) { m_pImage = _Image;}    // Shallow Copy
private:
    // SPECIALIZED DATA GOES HERE
    uchar*        m_pImage;            // The Image
    uint        m_nImageSize;        // Size of m_pImage
};

/* ----------------------- Atomic_Get_Template -------------------------*/
class Atomic_Get_Template :  public ICmd
{
public:
    Atomic_Get_Template();
    ~Atomic_Get_Template();
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
    // Get/Set Template
    void    SetTemplate(uchar* pTemplate, uint nTemplateSize);
    uchar*    GetTemplate(uint& nTemplateSize);
private:
    // SPECIALIZED DATA GOES HERE
    uchar*    m_pTemplate;
    uint    m_nTemplateSize;

};

/* ----------------------- Atomic_Set_Template -------------------------*/
class Atomic_Set_Template :  public ICmd
{
public:
    Atomic_Set_Template();
    ~Atomic_Set_Template();
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
    void    SetTemplate(uchar* pTemplate, uint nTemplateSize);
    uchar*    GetTemplate(uint& nTemplateSize);
private:
    // SPECIALIZED DATA GOES HERE
    uchar*    m_pTemplate;
    uint    m_nTemplateSize;
};

/* ----------------------- Atomic_Get_Acq_Status -------------------------*/
class Atomic_Get_Acq_Status :  public ICmd
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
    void    SetAcqStatus(_V100_ACQ_STATUS_TYPE acq) { m_ACQStatus = acq;}
    _V100_ACQ_STATUS_TYPE GetAcqStatus() { return m_ACQStatus;}
private:
    // SPECIALIZED DATA GOES HERE
    _V100_ACQ_STATUS_TYPE    m_ACQStatus;

};

/* ----------------------- Atomic_Get_Status -------------------------*/
class Atomic_Get_Status :  public ICmd
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
    void SetInterfaceStatusType(_V100_INTERFACE_STATUS_TYPE st) {m_nIST = st;}
    _V100_INTERFACE_STATUS_TYPE  GetInterfaceStatusType() { return m_nIST;}
private:
    // SPECIALIZED DATA GOES HERE
    uint                        m_nStatusSize;
    _V100_INTERFACE_STATUS_TYPE m_nIST;
};

/* ----------------------- Atomic_Get_Cmd -------------------------*/
class Atomic_Get_Cmd :  public ICmd
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
    _V100_INTERFACE_COMMAND_TYPE    GetCmd()      { return m_nICT;};
    bool    SetCmd(_V100_INTERFACE_COMMAND_TYPE* _ICT) { m_nICT = *_ICT; return true;};
private:
    // SPECIALIZED DATA GOES HERE
    _V100_INTERFACE_COMMAND_TYPE    m_nICT;
    uint                            m_nSize;
};

/* ----------------------- Atomic_Set_Cmd -------------------------*/
class Atomic_Set_Cmd :  public ICmd
{
public:
    Atomic_Set_Cmd();
    ~Atomic_Set_Cmd();
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
    // Set/Get CMD
    _V100_INTERFACE_COMMAND_TYPE* GetCmd()
    {
        return &m_nICT;
    };
    bool    SetCmd(_V100_INTERFACE_COMMAND_TYPE    _ICT)
    {
        m_nICT = _ICT;
        m_nSize = sizeof(m_nICT);
        return true;
    };
private:
    // SPECIALIZED DATA GOES HERE
    _V100_INTERFACE_COMMAND_TYPE    m_nICT;
    uint                            m_nSize;
};

/* ----------------------- Atomic_Set_LED -------------------------*/
class Atomic_Set_LED :  public ICmd
{
public:
    Atomic_Set_LED();
    ~Atomic_Set_LED();
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
    void    SetLEDControl(_V100_LED_CONTROL cnt) {m_nV100_LED_CONTROL = cnt;}
    _V100_LED_CONTROL GetLEDControl(void) { return m_nV100_LED_CONTROL;};
private:
    // SPECIALIZED DATA GOES HERE
    _V100_LED_CONTROL m_nV100_LED_CONTROL;
    uint              m_nLEDControlSize;
};

/* ----------------------- Atomic_Set_Enrollment_Rules -------------------------*/
class Atomic_Set_Enrollment_Rules :  public ICmd
{
public:
    Atomic_Set_Enrollment_Rules();
    ~Atomic_Set_Enrollment_Rules();
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

/* ----------------------- Atomic_Find_UID -------------------------*/
class Atomic_Find_UID :  public ICmd
{
public:
    Atomic_Find_UID();
    ~Atomic_Find_UID();
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

/* ----------------------- Atomic_Delete_UID -------------------------*/
class Atomic_Delete_UID :  public ICmd
{
public:
    Atomic_Delete_UID();
    ~Atomic_Delete_UID();
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

/* ----------------------- Atomic_Update_UID -------------------------*/
class Atomic_Update_UID :  public ICmd
{
public:
    Atomic_Update_UID();
    ~Atomic_Update_UID();
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

/* ----------------------- Atomic_Config_Comport -------------------------*/
class Atomic_Config_Comport :  public ICmd
{
public:
    Atomic_Config_Comport();
    ~Atomic_Config_Comport();
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
    bool SetBaudRate(uint  nBaudRate);
    bool GetBaudRate(uint& nBaudRate);
private:
    // SPECIALIZED DATA GOES HERE
    uint    m_nBaudSize;
    uint    m_nBaudRate;

};

/* ----------------------- Atomic_Reset -------------------------*/
class Atomic_Reset :  public ICmd
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

/* ----------------------- Atomic_Set_Option -------------------------*/
class Atomic_Set_Option :  public ICmd
{
public:
    Atomic_Set_Option();
    ~Atomic_Set_Option();
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
    // Set Option
    bool SetOption(_V100_OPTION_TYPE OptType, uchar* pOptData, uint nSizeOptData);
    bool GetOption(_V100_OPTION_TYPE& OptType, uchar** pOptData, uint& nSizeOptData);
private:
    // SPECIALIZED DATA GOES HERE
    uchar*                m_pOptData;
    uint                m_nSizeOptData;
};

/* ----------------------- Atomic_Match_Ex -------------------------*/
class Atomic_Match_Ex :  public ICmd
{
public:
    Atomic_Match_Ex();
    ~Atomic_Match_Ex();
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
    // Client
    bool    SetProbeTemplate(uchar* pTpl, uint Size);
    bool    SetGalleryTemplate(uchar* pTpl, uint Size);
    // Host
    uchar*    GetProbeTemplate();
    uint    GetProbeTemplateSize() { return m_nSzProbeTemplate;}
    uchar*  GetGalleryTemplate();
    uint    GetGalleryTemplateSize() { return m_nSzGalleryTemplate;}
    void    GetSpoofScore(int& SpoofScore) { SpoofScore = m_nSpoofScore;}
    void    GetMatchScore(int& MatchScore) { MatchScore = m_nMatchScore;}
    bool    SetSpoofScore(int nScore) { m_nSpoofScore = nScore; return true; }
    bool    SetMatchScore(int nScore) { m_nMatchScore = nScore; return true; }
private:
    // SPECIALIZED DATA GOES HERE
    uchar* m_pProbeTemplate;
    uint   m_nSzProbeTemplate;
    uchar* m_pGalleryTemplate;
    uint   m_nSzGalleryTemplate;
    int       m_nSpoofScore;
    int    m_nMatchScore;
};

/* ----------------------- Atomic_Spoof_Get_Template -------------------------*/
class Atomic_Spoof_Get_Template :  public ICmd
{
public:
    Atomic_Spoof_Get_Template();
    ~Atomic_Spoof_Get_Template();
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
    // CHALLENGE
    // Set Data
    bool    SetData(uchar* pData, uint nDataSize);
    // Get Data
    uchar*  GetData();
    // Get Data Size
    uint    GetDataSize();
    // Get Spoof Template
    uchar*  GetSpoofTemplate();
    // Get Spoof Template size
    uint    GetSpoofTemplateSize();
    // Set Spoof Template
    bool    SetSpoofTemplate(uchar* pData, uint nDataSize);
private:
    // SPECIALIZED DATA GOES HERE
    uchar*    m_pData;
    uint    m_nDataSize;
    uchar*  m_pSpoofTemplate;
    uint    m_nSpoofTemplateSize;
};

/* ----------------------- Atomic_Get_DB_Metrics -------------------------*/
class Atomic_Get_DB_Metrics :  public ICmd
{
public:
    Atomic_Get_DB_Metrics();
    ~Atomic_Get_DB_Metrics();
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
    bool    GetDBMetrics(_V100_DB_METRICS* pRec) { memcpy(pRec, &m_DBMetrics, sizeof(m_DBMetrics)); return true;}
    bool    SetDBMetrics(_V100_DB_METRICS* pRec) { memcpy(&m_DBMetrics, pRec, sizeof(m_DBMetrics)); return true;}
private:
    _V100_DB_METRICS m_DBMetrics;

};

/* ----------------------- Atomic_Get_OP_Status -------------------------*/
class Atomic_Get_OP_Status :  public ICmd
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
    virtual void Exec();
    //
    bool    GetOPStatus(_V100_OP_STATUS* pRec) { memcpy(pRec, &m_OPStatus, sizeof(m_OPStatus)); return true;}
    bool    SetOPStatus(_V100_OP_STATUS* pRec) { memcpy(&m_OPStatus, pRec, sizeof(m_OPStatus)); return true;}
private:
    _V100_OP_STATUS m_OPStatus;
};

/* ----------------------- Atomic_Format_DB -------------------------*/
class Atomic_Format_DB :  public ICmd
{
public:
    Atomic_Format_DB();
    ~Atomic_Format_DB();
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

/* ----------------------- Atomic_Set_Tag -------------------------*/
class Atomic_Set_Tag :  public ICmd
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
    bool SetTag(char* pTag, ushort& nSize);
    bool GetTag(char** pTag, ushort& nSize);
private:
    char*    m_pTag;
    ushort    m_nTagSize;
};

/* ----------------------- Atomic_Get_Tag -------------------------*/
class Atomic_Get_Tag :  public ICmd
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

/* ----------------------- Atomic_Truncate_378 -------------------------*/
class Atomic_Truncate_378 :  public ICmd
{
public:
    Atomic_Truncate_378();
    ~Atomic_Truncate_378();
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
    bool GetMaxTemplateSize(uint& nSizeToTruncate);
    bool SetMaxTemplateSize(uint nSizeToTruncate);
    bool SetTemplate(const uchar* pTemplate, uint nTemplateSize);
    bool GetTemplate(uchar* pTemplate, uint& nTemplateSize);
private:
    uchar*    m_pTemplate;
    uint    m_nTemplateSize;
    uint    m_nTemplateMaxSize;
    uint    m_nTemplateActualSize;
};

/* ----------------------- Atomic_Set_GPIO -------------------------*/
class Atomic_Set_GPIO :  public ICmd
{
public:
    Atomic_Set_GPIO();
    ~Atomic_Set_GPIO();
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
    uchar    GetMask() { return m_Mask; }
    void    SetMask(uchar _mask) { m_Mask = _mask; }
private:
    uchar    m_Mask;

};

/* ----------------------- Atomic_Get_GPIO -------------------------*/
class Atomic_Get_GPIO :  public ICmd
{
public:
    Atomic_Get_GPIO();
    ~Atomic_Get_GPIO();
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
    uchar    GetMask() { return m_Mask; }
    void    SetMask(uchar _mask) { m_Mask = _mask; }
private:
    uchar    m_Mask;

};

/* ----------------------- Atomic_Cancel_Operation -------------------------*/
class Atomic_Cancel_Operation :  public ICmd
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
private:
    // SPECIALIZED DATA GOES HERE

};

/* ----------------------- Atomic_File_Delete  -------------------------*/
class Atomic_File_Delete :  public ICmd
{
public:
    Atomic_File_Delete();
    ~Atomic_File_Delete();
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
    bool            GetName(char* pObjectName, uint& nObjectLength, _V100_FILE_ATTR& attr);
    bool            SetName(char* pObjectName, uint nObjectLength, _V100_FILE_ATTR attr);
    char*            GetName();
    uint*            GetSize();
    _V100_FILE_ATTR GetAttr();
private:
    // SPECIALIZED DATA GOES HERE
    char                m_pFileName[MAX_FILE_NAME_LENGTH];
    uint                m_szFileLength;
     _V100_FILE_ATTR    Attribute;
};

/* ----------------------- Atomic_Get_FIR_Image -------------------------*/
class Atomic_Get_FIR_Image :  public ICmd
{
public:
    Atomic_Get_FIR_Image();
    ~Atomic_Get_FIR_Image();
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
    bool SetFingerType(_V100_FINGER_PALM_POSITION FingerType);
    bool SetFIRType(_V100_FIR_RECORD_TYPE FIRType);
    _V100_FIR_RECORD_TYPE GetFIRType() {return m_nFIRType;};
    _V100_FINGER_PALM_POSITION GetFingerType() {return m_nFingerType;};
    bool SetFIRImage(uchar* pFIRImage, uint nFIRImageSz);
    uchar* GetFIRImage(){ return m_FIRImage;};
    uint GetFIRImageSize(){ return m_nFIRImageSz; };

private:
    // SPECIALIZED DATA GOES HERE

    _V100_FIR_RECORD_TYPE m_nFIRType;
    _V100_FINGER_PALM_POSITION m_nFingerType;
    uint m_nFIRImageSz;
    uchar* m_FIRImage;

};

/* ----------------------- Atomic_Set_License_Key -------------------------*/
class Atomic_Set_License_Key :  public ICmd
{
public:
    Atomic_Set_License_Key();
    ~Atomic_Set_License_Key();
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
    // Set License Key
    bool SetLicenseKey(_V100_DEVICE_POLICY* pLK);
    // Get License Key
    bool GetLicenseKey(_V100_DEVICE_POLICY* ppLK);
private:
    // SPECIALIZED DATA GOES HERE
    uint                 m_nSizeLK;
    _V100_DEVICE_POLICY  m_LK;

};

/* ----------------------- Atomic_Set_Record -------------------------*/
class Atomic_Set_Record :  public ICmd
{
public:
    Atomic_Set_Record();
    ~Atomic_Set_Record();
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
    bool SetRecord(_V100_RECORD_TYPE type, uint m_nRecordSize, uchar* m_pRecordData);
    bool GetRecord(_V100_RECORD_TYPE& type, uint& m_nRecordSize, uchar** m_pRecordData);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_RECORD_TYPE    m_Record_Type;
    uint                m_nRecordSize;
    uchar*                m_pRecordData;
};

/* ----------------------- Atomic_Write_Flash -------------------------*/
class Atomic_Write_Flash :  public ICmd
{
public:
    Atomic_Write_Flash();
    ~Atomic_Write_Flash();
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
    // Get
    void SetFlashType(_V100_RECORD_TYPE ft);
    void GetFlashType(_V100_RECORD_TYPE& ft);
private:
    uint                m_nSizeRecordType;
    _V100_RECORD_TYPE    m_eWhat_To_Flash;
};

/* ----------------------- Atomic_Get_Serial -------------------------*/
class Atomic_Get_Serial :  public ICmd
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

/* ----------------------- Atomic_Clear -------------------------*/
class Atomic_Clear :  public ICmd
{
public:
    Atomic_Clear();
    ~Atomic_Clear();

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

/* ----------------------- Atomic_Log -------------------------*/
class Atomic_Log :  public ICmd
{
public:
    Atomic_Log();
    ~Atomic_Log();
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
    // Get/Set
    void SetLogType(_V100_LOG_TYPE LogType)
    {
        m_LogType = LogType;
    }
    _V100_LOG_TYPE GetLogType()
    {
        return m_LogType;
    }
    void SetLogOption(_V100_LOG_OPTION LogOption)
    {
        m_LogOption = LogOption;
    }
    _V100_LOG_OPTION GetLogOption()
    {
        return m_LogOption;
    }
    void SetLog(uchar* pLog, uint LogSize);
    void GetLog(uchar** hLog, uint* pLogSize);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_LOG_TYPE m_LogType;        // In        Out
    _V100_LOG_OPTION m_LogOption;    // In
    uint m_LogSize;                    //            Out
    uchar* m_pLog;                    //            Out
};

/* ----------------------- Atomic_Get_Template -------------------------*/
class Atomic_Get_Version :  public ICmd
{
public:
    Atomic_Get_Version();
    ~Atomic_Get_Version();
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
    // Get/Set Template
    void    SetVersion(char* pVersionString, uint nSzVersionString);
    char*    GetVersion(uint& nSzVersionString);
private:
    // SPECIALIZED DATA GOES HERE
    char*    m_pVersionString;
    uint    m_nSzVersionString;

};

/* ----------------------- Macro_Update_Firmware -------------------------*/
class Macro_Update_Firmware : public ICmd
{
public:
    Macro_Update_Firmware();
    ~Macro_Update_Firmware();
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
    bool SetData(uchar* pData, uint nDataSize);
    uchar* GetData();
    uint   GetDataSize();
private:
    // SPECIALIZED DATA GOES HERE
    uchar* m_pFWData;
    uint    m_nDataSize;
};

class Atomic_Delete_User :  public ICmd
{
public:
    Atomic_Delete_User();
    ~Atomic_Delete_User();
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
    // Get/Set User Record
    bool    GetUserRecord(_V100_USER_RECORD* pRec) { memcpy(pRec, &m_UserRecord, sizeof(m_UserRecord)); return true;}
    bool    SetUserRecord(_V100_USER_RECORD* pRec) { memcpy(&m_UserRecord, pRec, sizeof(m_UserRecord)); return true;}
private:
    _V100_USER_RECORD m_UserRecord;

};

class Atomic_Get_User :  public ICmd
{
public:
    Atomic_Get_User();
    ~Atomic_Get_User();
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
    // Get/Set User Record
    bool    GetUserRecord(_V100_USER_RECORD* pRec) { memcpy(pRec, &m_UserRecord, sizeof(m_UserRecord)); return true;}
    bool    SetUserRecord(_V100_USER_RECORD* pRec) { memcpy(&m_UserRecord, pRec, sizeof(m_UserRecord)); return true;}
    char*    GetRecordData() { return m_pRecordData; }
    bool    SetRecordData(char* pRecData);
    bool    SetUserIndexToGet(uint nI) { m_UID_Index = nI; return true;}
    uint    GetUserIndex() { return m_UID_Index;}
private:
    _V100_USER_RECORD m_UserRecord;
    char*              m_pRecordData;
    uint              m_UID_Index;
};

class Atomic_Add_User :  public ICmd
{
public:
    Atomic_Add_User();
    ~Atomic_Add_User();
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
    // Get/Set User Record
    bool    GetUserRecord(_V100_USER_RECORD* pRec) { memcpy(pRec, &m_UserRecord, sizeof(m_UserRecord)); return true;}
    bool    SetUserRecord(_V100_USER_RECORD* pRec) { memcpy(&m_UserRecord, pRec, sizeof(m_UserRecord)); return true;}
    bool    SetOpaqueData(char* pOpData);
    char*    GetOpaqueData() { return m_pOpaqueData;}
private:
    _V100_USER_RECORD m_UserRecord;
    char*              m_pOpaqueData;
};

class Atomic_Get_Verification_Rules :  public ICmd
{
public:
    Atomic_Get_Verification_Rules();
    ~Atomic_Get_Verification_Rules();
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
    _V100_VERIFICATION_RULES GetVerificationRules() { return m_VerificationRules; };
    bool SetVerificationRules(_V100_VERIFICATION_RULES rules) { m_VerificationRules = rules; return true;}
private:
    _V100_VERIFICATION_RULES m_VerificationRules;

};

class Macro_Verify_User :  public ICmd
{
public:
    Macro_Verify_User();
    ~Macro_Verify_User();
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
    // Get/Set User Record
    bool    GetUserRecord(_V100_USER_RECORD* pRec) { memcpy(pRec, &m_UserRecord, sizeof(m_UserRecord)); return true;}
    bool    SetUserRecord(_V100_USER_RECORD* pRec) { memcpy(&m_UserRecord, pRec, sizeof(m_UserRecord)); return true;}
private:
    _V100_USER_RECORD m_UserRecord;

};

class Atomic_Set_Verification_Rules :  public ICmd
{
public:
    Atomic_Set_Verification_Rules();
    ~Atomic_Set_Verification_Rules();
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
    _V100_VERIFICATION_RULES GetVerificationRules() { return m_VerificationRules; };
    bool SetVerificationRules(_V100_VERIFICATION_RULES rules) { m_VerificationRules = rules; return true;}
private:
    _V100_VERIFICATION_RULES m_VerificationRules;

};

