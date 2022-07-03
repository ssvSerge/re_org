#pragma once
#include "V100_internal_types.h"
#include "manufacturing_state.h"
#include "ICmd.h"



/* ----------------------- Atomic_Barcode_Get_Data -------------------------*/
class Atomic_Barcode_Get_Data : public ICmd
{
public:
    Atomic_Barcode_Get_Data();
    ~Atomic_Barcode_Get_Data();
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
    void    SetBarcodeData(unsigned char* pData, const unsigned int nSz);
    uchar* GetBarcodeData();
    uint    GetBarcodeDataSize();
private:
    uchar* m_pData;
    uint    m_nDataSize;
};

/* ----------------------- Atomic_Barcode_Get_Details -------------------------*/
class Atomic_Barcode_Get_Details : public ICmd
{
public:
    Atomic_Barcode_Get_Details();
    ~Atomic_Barcode_Get_Details();
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
    void  SetBarcodeDetails(void* pDetails, uint nDetailsSize);
    void* GetBarcodeDetails() { return m_pDetails; }
    uint  GetBarcodeDetailsSize() { return m_nDetailsSize; }
private:
    void* m_pDetails;
    uint m_nDetailsSize;
};


/* ----------------------- Atomic_Get_System_State -------------------------*/
class Atomic_Get_System_State :  public ICmd
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
    virtual _V100_SYSTEM_DIAGNOSTICS* GetSystemMetricsStruct() { return &m_Diagnostics;}
    // Set System Metrics
    virtual void SetSystemMetricsStruct(_V100_SYSTEM_DIAGNOSTICS* met) { m_Diagnostics = *met;}
private:
    // SPECIALIZED DATA GOES HERE
    _V100_SYSTEM_DIAGNOSTICS    m_Diagnostics;
};
/* ----------------------- Atomic_Get_License_Key -------------------------*/
class Atomic_Get_License_Key :  public ICmd
{
public:
    Atomic_Get_License_Key();
    ~Atomic_Get_License_Key();
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
/* ----------------------- Atomic_Get_Cal -------------------------*/
class Atomic_Get_Cal :  public ICmd
{
public:
    Atomic_Get_Cal();
    ~Atomic_Get_Cal();
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
    void SetCal(uchar* pCalData, uint nCalSize);
    void GetCal(uchar** pCalData, uint& nCalSize);
private:
    // SPECIALIZED DATA GOES HERE
    uint    m_nCalSize;
    uchar*  m_pCal;

};
/* ----------------------- Atomic_Set_Cal -------------------------*/
class Atomic_Set_Cal :  public ICmd
{
public:
    Atomic_Set_Cal();
    ~Atomic_Set_Cal();
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
    void SetCal(uchar* pCalData, uint nCalSize);
    void GetCal(uchar** pCalData, uint& nCalSize);
private:
    // SPECIALIZED DATA GOES HERE
    uint    m_nCalSize;
    uchar*  m_pCal;
};

/* ----------------------- Atomic_Get_Record -------------------------*/
class Atomic_Get_Record :  public ICmd
{
public:
    Atomic_Get_Record();
    ~Atomic_Get_Record();
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
/* ----------------------- Atomic_Loopback -------------------------*/
class Atomic_Loopback :  public ICmd
{
public:
    Atomic_Loopback();
    ~Atomic_Loopback();
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
/* ----------------------- Atomic_BIT -------------------------*/
class Atomic_BIT :  public ICmd
{
public:
    Atomic_BIT();
    ~Atomic_BIT();
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
/* ----------------------- Atomic_Test -------------------------*/
class Atomic_Test :  public ICmd
{
public:
    Atomic_Test();
    ~Atomic_Test();
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
/* ----------------------- Atomic_Start_Burnin -------------------------*/
class Atomic_Start_Burnin :  public ICmd
{
public:
    Atomic_Start_Burnin();
    ~Atomic_Start_Burnin();
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
/* ----------------------- Atomic_Stop_Burnin -------------------------*/
class Atomic_Stop_Burnin :  public ICmd
{
public:
    Atomic_Stop_Burnin();
    ~Atomic_Stop_Burnin();
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

/* ----------------------- CMD_WRITE_FILE -------------------------*/

class Atomic_Write_File :  public ICmd
{
public:
    Atomic_Write_File();
    ~Atomic_Write_File();
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
    // Set Challenge Info (Client)
    virtual bool SetChallengeData(uchar* pFileToWrite, uint nSize, uint nFlashLocation, uint lTime);
    // Get Challenge Info (Host)
    virtual bool GetChallengeData(uchar** pFileToWrite, _V100_FILE_HEADER& FileHeader);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE_HEADER    m_FileHeader;
    uchar*                m_pFileToWrite;
};

/* ----------------------- CMD_READ_FILE -------------------------*/

class Atomic_Read_File :  public ICmd
{
public:
    Atomic_Read_File();
    ~Atomic_Read_File();
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
    // Challenge
    virtual bool SetChallengeData(uint nFlashLocation);
    // Get
    virtual bool GetChallengeData(uint& nFlashLocation);
    // Set
    virtual bool SetResponseData(uchar* pBytes, _V100_FILE_HEADER FileHeader);
    // Get
    virtual bool GetResponseData(uchar* pBytes, _V100_FILE_HEADER& FileHeader);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE_HEADER    m_FileHeader;
    uchar*                m_pFile;
    uint                m_nFlashLocation;
};

/* ----------------------- CMD_GET_SPOOF_DETAILS -------------------------*/

class Atomic_Get_Spoof_Details :  public ICmd
{
public:
    Atomic_Get_Spoof_Details();
    ~Atomic_Get_Spoof_Details();
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
    bool    SetSpoofMetrics(_V100_SPOOF_METRICS  metrics);
    bool    GetSpoofMetrics(_V100_SPOOF_METRICS& metrics);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_SPOOF_METRICS        m_nSpoofMetrics;

};

/* ----------------------- CMD_GET_SPOOF_DETAILS_V2 -------------------------*/

class Atomic_Get_Spoof_Details_V2 :  public ICmd
{
public:
    Atomic_Get_Spoof_Details_V2();
    ~Atomic_Get_Spoof_Details_V2();
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
    bool    SetSpoofMetrics(_V100_SPOOF_METRICS_V2* metrics);
    bool    GetSpoofMetrics(_V100_SPOOF_METRICS_V2* metrics);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_SPOOF_METRICS_V2        m_nSpoofMetrics;

};

/*
** FILE MANAGEMENT COMMANDS
*/

/* ----------------------- Atomic_File_Read  -------------------------*/

class Atomic_File_Read :  public ICmd
{
public:
    Atomic_File_Read();
    ~Atomic_File_Read();
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
    bool         SetFileAttr(_V100_FILE att) { m_FileAttr = att; return true; }
    _V100_FILE*     GetFileAttr() { return &m_FileAttr; }
    bool         SetFileStream(char* pPtr, uint nSZ);
    char*         GetFileStream(){ return m_pFileStream;}
    uint*         GetFileStreamSize() { return &m_FileAttr.FileSize;}
    char*         GetFileName() { return m_pFileName; }
    uint         GetFileNameSize()  { return m_szFileNameSize; };
    bool         SetFileName(char* pPtr, uint nSz);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE     m_FileAttr;
    char         m_pFileName[MAX_FILE_NAME_LENGTH];
    uint         m_szFileNameSize;
    char*         m_pFileStream;
};

/* ----------------------- Atomic_File_Write  -------------------------*/

class Atomic_File_Write :  public ICmd
{
public:
    Atomic_File_Write();
    ~Atomic_File_Write();
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
    bool         SetFileAttr(_V100_FILE att) { m_FileAttr = att; return true; }
    _V100_FILE*     GetFileAttr() { return &m_FileAttr; }
    bool         SetFileStream(char* pPtr, uint nSZ);
    char*         GetFileStream(){ return m_pFileStream;}
    uint*         GetFileStreamSize() { return &m_FileAttr.FileSize;}
    char*         GetFileName() { return m_pFileName; }
    uint         GetFileNameSize()  { return m_szFileNameSize; };
    bool         SetFileName(const char* pPtr, uint nSz);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE     m_FileAttr;
    char         m_pFileName[MAX_FILE_NAME_LENGTH];
    uint         m_szFileNameSize;
    char*         m_pFileStream;
};



/* ----------------------- Atomic_File_Get_Attributes  -------------------------*/

class Atomic_File_Get_Attributes :  public ICmd
{
public:
    Atomic_File_Get_Attributes();
    ~Atomic_File_Get_Attributes();
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
    bool         SetFileSysAttr(_V100_FILE_SYSTEM_ATTRIBUTE att) { m_FileSysAttr = att; return true; }
    _V100_FILE_SYSTEM_ATTRIBUTE     GetFileSysAttr() { return m_FileSysAttr; }
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE_SYSTEM_ATTRIBUTE m_FileSysAttr;
};

class Atomic_Format_Volume :  public ICmd
{
public:
    Atomic_Format_Volume();
    ~Atomic_Format_Volume();
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
};

/* ----------------------- Atomic_File_CD  -------------------------*/

class Atomic_File_CD :  public ICmd
{
public:
    Atomic_File_CD();
    ~Atomic_File_CD();
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
    bool GetName(char* pObjectName, uint& nObjectLength);
    bool SetName(const char* pObjectName, uint nObjectLength);
    char* GetName();
    uint* GetSize();
private:
    // SPECIALIZED DATA GOES HERE
    char       m_pFileName[MAX_FILE_NAME_LENGTH];
    uint       m_szFileLength;
};


class Atomic_File_GetCwd :  public ICmd
{
public:
    Atomic_File_GetCwd();
    ~Atomic_File_GetCwd();
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
    bool SetFileAttr(_V100_FILE file)                        { m_FileAttr = file; return true; }
    bool GetFileAttr(_V100_FILE& file)                        { file = m_FileAttr; return true; }
    bool GetName(char* pObjectName, uint& nObjectLength);
    bool SetName(char* pObjectName, uint nObjectLength);
    char* GetName();
    uint* GetSize();
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE m_FileAttr;
    char       m_pFileName[MAX_FILE_NAME_LENGTH];
    uint       m_szFileLength;
};

class Atomic_File_Dir_FindFirst :  public ICmd
{
public:
    Atomic_File_Dir_FindFirst();
    ~Atomic_File_Dir_FindFirst();
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
    bool SetFileAttr(_V100_FILE file)                        { m_FileAttr = file; return true; }
    bool GetFileAttr(_V100_FILE& file)                        { file = m_FileAttr; return true; }
    _V100_FILE* GetFileAttr() { return &m_FileAttr;}
    bool GetName(char* pObjectName, uint& nObjectLength);
    bool SetName(char* pObjectName, uint nObjectLength);
    char* GetName();
    uint* GetSize();
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE m_FileAttr;
    char       m_pFileName[MAX_FILE_NAME_LENGTH];
    uint       m_szFileLength;
};

class Atomic_File_Dir_FindNext :  public ICmd
{
public:
    Atomic_File_Dir_FindNext();
    ~Atomic_File_Dir_FindNext();
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
    bool SetFileAttr(_V100_FILE file)                        { m_FileAttr = file; return true; }
    bool GetFileAttr(_V100_FILE& file)                        { file = m_FileAttr; return true; }
   _V100_FILE* GetFileAttr() { return &m_FileAttr;}
    bool GetName(char* pObjectName, uint& nObjectLength);
    bool SetName(char* pObjectName, uint nObjectLength);
    char* GetName();
    uint* GetSize();
private:
    // SPECIALIZED DATA GOES HERE
    _V100_FILE m_FileAttr;
    char       m_pFileName[MAX_FILE_NAME_LENGTH];
    uint       m_szFileLength;
};

/* ----------------------- Atomic_Get_EEPROM -------------------------*/
class Atomic_Get_EEPROM :  public ICmd
{
public:
    Atomic_Get_EEPROM();
    ~Atomic_Get_EEPROM();
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
    _MX00_EEPROM_DATA    GetEEPROM()      { return m_nEEP;};
    bool    SetEEPROM(_MX00_EEPROM_DATA* _nEEP) { m_nEEP = *_nEEP; return true;};
private:
    // SPECIALIZED DATA GOES HERE
    _MX00_EEPROM_DATA                m_nEEP;
    uint                            m_nSize;

};

/* ----------------------- Atomic_Get_EEPROM_M320 -------------------------*/
class Atomic_Get_EEPROM_M320 : public ICmd
{
public:
    Atomic_Get_EEPROM_M320();
    ~Atomic_Get_EEPROM_M320();
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
    _MX00_EEPROM_DATA_M320    GetEEPROM()      { return m_nEEP; };
    bool    SetEEPROM(_MX00_EEPROM_DATA_M320* _nEEP) { m_nEEP = *_nEEP; return true; };
private:
    // SPECIALIZED DATA GOES HERE
    _MX00_EEPROM_DATA_M320            m_nEEP;
    uint                            m_nSize;

};
/* ----------------------- Atomic_Set_EEPROM -------------------------*/
class Atomic_Set_EEPROM :  public ICmd
{
public:
    Atomic_Set_EEPROM();
    ~Atomic_Set_EEPROM();
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
    _MX00_EEPROM_DATA* GetEEPROM()
    {
        return &m_nEEPROM;
    };
    bool    SetEEPROM(_MX00_EEPROM_DATA    _nEEPROM)
    {
        m_nEEPROM = _nEEPROM;
        m_nSize = sizeof(m_nEEPROM);
        return true;
    };
private:
    // SPECIALIZED DATA GOES HERE
    _MX00_EEPROM_DATA                m_nEEPROM;
    uint                            m_nSize;
};
/* ----------------------- Atomic_Set_EEPROM_M320 -------------------------*/
class Atomic_Set_EEPROM_M320 : public ICmd
{
public:
    Atomic_Set_EEPROM_M320();
    ~Atomic_Set_EEPROM_M320();
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
    _MX00_EEPROM_DATA_M320* GetEEPROM()
    {
        return &m_nEEPROM;
    };
    bool    SetEEPROM(_MX00_EEPROM_DATA_M320    _nEEPROM)
    {
        m_nEEPROM = _nEEPROM;
        m_nSize = sizeof(m_nEEPROM);
        return true;
    };
private:
    // SPECIALIZED DATA GOES HERE
    _MX00_EEPROM_DATA_M320            m_nEEPROM;
    uint                            m_nSize;
};
/* ----------------------- Atomic_Get_DSM_EEPROM -------------------------*/
class Atomic_Get_DSM_EEPROM :  public ICmd
{
public:
    Atomic_Get_DSM_EEPROM();
    ~Atomic_Get_DSM_EEPROM();
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
    // Set/Get EEPROM
    _VX00_DSM_EEPROM_DATA    GetEEPROM()      { return m_EEPROMData;};
    bool    SetEEPROM(_VX00_DSM_EEPROM_DATA* _nEEP) { m_EEPROMData = *_nEEP; return true;};

private:
    // SPECIALIZED DATA GOES HERE
    _VX00_DSM_EEPROM_DATA        m_EEPROMData;
    uint                        m_nSize;

};
/* ----------------------- Atomic_Set_DSM_EEPROM -------------------------*/
class Atomic_Set_DSM_EEPROM :  public ICmd
{
public:
    Atomic_Set_DSM_EEPROM();
    ~Atomic_Set_DSM_EEPROM();
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
    // Set/Get EEPROM
    _VX00_DSM_EEPROM_DATA* GetEEPROM()
    {
        return &m_EEPROMData;
    };
    bool    SetEEPROM(_VX00_DSM_EEPROM_DATA    _nEEPROM)
    {
        m_EEPROMData = _nEEPROM;
        m_nSize = sizeof(m_EEPROMData);
        return true;
    };
private:
    // SPECIALIZED DATA GOES HERE
    _VX00_DSM_EEPROM_DATA        m_EEPROMData;
    uint                        m_nSize;
};

// CMD_PROCESS *************************

class Atomic_Process :  public ICmd
{
public:
    Atomic_Process();
    ~Atomic_Process();
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
    uint GetDataSize() { return m_nDataSize; }
    // Set Data Size
    void SetDataSize(uint _DataSize) { m_nDataSize = _DataSize;}
    // Get Data Ptr
    uchar* GetData() { return m_pData;}
    // Set Data Ptr
    void   SetData(uchar* _Data) { m_pData = _Data;}    // Shallow Copy
private:
    // SPECIALIZED DATA GOES HERE
    uchar*        m_pData;            // The Image
    uint        m_nDataSize;        // Size of m_pImage
};


/* ----------------------- Atomic_Diagnostic_Test -------------------------*/
class Atomic_Diagnostic_Test : public ICmd
{
public:
    Atomic_Diagnostic_Test();
    ~Atomic_Diagnostic_Test();

    // ICmd functions
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    virtual int  GetChallengeBufferSize();
    virtual int  GetResponseBufferSize();

    // Class Getters/Setters
    uint GetResult() { return m_nResult; }
    void SetResult(uint result) { m_nResult = result; }
    uint GetError() { return m_nError; }
    void SetError(uint error) { m_nError = error; }
    uint GetComponent() { return m_nComponent; }
    void SetSubsystem(uint component) { m_nComponent = component; }
    uint GetOperation() { return m_nOperation; }
    void SetOperation(uint operation) { m_nOperation = operation; }
    uint GetOperationParameter() { return m_nOperationParameter; }
    void SetOperationParameter(uint parameter) { m_nOperationParameter = parameter; }

private:
    // SPECIALIZED DATA GOES HERE
    uint m_nComponent;
    uint m_nOperation;
    uint m_nOperationParameter;
    uint m_nResult;
    uint m_nError;
};

/* ----------------------- Atomic_Get_Mfg_State -------------------------*/
class Atomic_Get_Mfg_State :  public ICmd
{
public:
    Atomic_Get_Mfg_State();
    ~Atomic_Get_Mfg_State();
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
    void SetMfgState(_V100_MFG_STATE* pMfgState);
    void GetMfgState(_V100_MFG_STATE** pMfgState);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_MFG_STATE*  m_pMfgState;
};

/* ----------------------- Atomic_Set_Mfg_State -------------------------*/
class Atomic_Set_Mfg_State :  public ICmd
{
public:
    Atomic_Set_Mfg_State();
    ~Atomic_Set_Mfg_State();
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
    void SetMfgState(_V100_MFG_STATE* pMfgState);
    void GetMfgState(_V100_MFG_STATE** pMfgState);
private:
    // SPECIALIZED DATA GOES HERE
    _V100_MFG_STATE*  m_pMfgState;
};

