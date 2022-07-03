/******************************<%BEGIN LICENSE%>****************************/
// (c) Copyright 2013 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
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
/******************************<%END LICENSE%>******************************/
#pragma once

#include <vector>
#include <string>

#include "ICmd.h"
#include "IEncCmd.h"
#include "IHidEncCmd.h"
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_enc_types.h"
#include "string.h"
#include <HFTypes.h>


typedef std::vector<std::string>  string_list_t;


// CMD_UPDATE_FIRMWARE *****************************************************/

#ifdef _VCOM_CRYPTO_BBX
class Macro_Update_Firmware : public ICmd
{
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
    bool SetData(uchar* pData, uint nDataSize);
    uchar* GetData();
    uint   GetDataSize();
private:
    // SPECIALIZED DATA GOES HERE
    uchar*  m_pFWData;
    uint    m_nDataSize;
};
#endif

// **************************************  CMD_ENC_IDENTIFYMATCH  **********/

class Atomic_Enc_IdentifyMatch : public ICmd {
public:
    Atomic_Enc_IdentifyMatch();
    ~Atomic_Enc_IdentifyMatch();
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
    // Set Data Buffer
    bool SetDataBuffer(uchar* pData, uint nSize) {
        if (m_pDataBuffer) {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }
    uchar* GetDataBuffer() {
        return m_pDataBuffer;
    }
    int GetDataBufferSize() {
        return m_nDataBufferSize;
    }
    void  SetDigSig(u256 digSig) {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }
    void SetANSOLSK(u256 pAleatoryNumberBB) {
        memcpy(puszAleatoryNumberBB, pAleatoryNumberBB, sizeof(u256));
    }
    u8* GetANSOLSK() {
        return puszAleatoryNumberBB;
    }
    u8* GetDigSig()
    {
        return (u8*)&m_DigSig;
    }
    void SetFMRRequested(int FMRRequested)
    {
        m_nFMR = FMRRequested;
    }
    int GetFMRRequested() { return m_nFMR; }
    void    SetResult(int _res) { m_Result = _res; }
    int        GetResult() { return m_Result; }
    // Get Data Buffer
private:
    // SPECIALIZED DATA GOES HERE
    uchar*     m_pDataBuffer;
    int        m_nDataBufferSize;
    u256       m_DigSig;
    int        m_nFMR;
    int        m_Result;
    u256       puszAleatoryNumberBB;
};

// **************************************  CMD_ENC_RETURNCAPTUREDBIR  ******/

class Atomic_Enc_ReturnCapturedBIR : public ICmd
{
public:
    Atomic_Enc_ReturnCapturedBIR();
    ~Atomic_Enc_ReturnCapturedBIR();
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
    // Set Data Buffer
    bool SetBIR(ST_BBXBIO_BIR_HEADER* inHdr)
    {
        memcpy(&m_BIRHdr, inHdr, sizeof(ST_BBXBIO_BIR_HEADER));
        return true;
    }
    ST_BBXBIO_BIR_HEADER* GetBIR()
    {
        return &m_BIRHdr;
    }
    bool SetCG(uchar* pData, uint nSize)
    {
        if (m_pDataBuffer)
        {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }
    uchar* GetCG()
    {
        return m_pDataBuffer;
    }
    int GetCGSize()
    {
        return m_nDataBufferSize;
    }
    void  SetDigSig(u256 digSig)
    {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }
    u8* GetDigSig()
    {
        return (u8*)&m_DigSig;
    }

    // Get Data Buffer
protected:
    // SPECIALIZED DATA GOES HERE
    uchar*                  m_pDataBuffer;
    int                     m_nDataBufferSize;
    u256                    m_DigSig;
    ST_BBXBIO_BIR_HEADER    m_BIRHdr;
};


// **************************************  CMD_ENC_UNLOCK_KEY  *************/

class Atomic_Enc_Unlock_Key : public IEncCmd {
public:
    Atomic_Enc_Unlock_Key() {
        m_nCmd = CMD_ENC_UNLOCK_KEY;
    }
    virtual ~Atomic_Enc_Unlock_Key() {
    };
    // Execute command
    virtual void Exec();
};

// **************************************  CMD_ENC_GENERATE_RSA_KEYS  ******/

class Macro_Enc_Generate_RSA_Keys : public IEncCmd {
public:
    Macro_Enc_Generate_RSA_Keys() {
        m_nCmd = CMD_ENC_GENERATE_RSA_KEYS;
    };

    virtual ~Macro_Enc_Generate_RSA_Keys() {};
    // Execute command
    virtual void Exec();
};

// **************************************  CMD_ENC_GET_KEY  ****************/

class Atomic_Enc_Get_Key : public ICmd {
public:
    Atomic_Enc_Get_Key();
    ~Atomic_Enc_Get_Key();
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
    // Execute command
    virtual void Exec();
    //
    u8* GetKey() { return (u8*)m_pKey; }
    u32 GetKeyLength() { return m_nKeyLength; }
    u8* GetKCV() { return m_KCV; }
    u16 GetKeyVersion() { return m_nKeyVersion; }
    u16 GetKeyMode() { return m_nKeyMode; }
    //
    void SetKeyParameters(u32 keyLength, u16 nKeyVersion, u16 nKeyMode) {
        m_nKeyVersion = nKeyVersion;
        m_nKeyLength = keyLength;
        m_nKeyMode = nKeyMode;
    };

private:
    u2048   m_pKey;
    u32     m_nKeyLength;
    u8      m_KCV[4];
    u16     m_nKeyVersion;
    u16     m_nKeyMode;
};


// **************************************  CMD_ENC_GET_KEYVERSION  *********/

class Atomic_Enc_Get_KeyVersion : public ICmd {
    public:
        // 
        Atomic_Enc_Get_KeyVersion();
        ~Atomic_Enc_Get_KeyVersion();

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
        // Execute command
        virtual void Exec();

        // 
        _V100_ENC_KEY_TYPE GetKeySlot() {
            return m_nKeySlot;
        }

        u16 GetKeyVersion() {
            return m_nKeyVersion;
        }

        u16 GetKeyMode() {
            return (int)m_nKeyMode;
        }

        u8* GetKCV() {
            return &m_pKCV[0];
        }

        void SetKeySlot(_V100_ENC_KEY_TYPE nKeySlot) {
            m_nKeySlot = nKeySlot;
        }

        void SetKeyVersion(u16 nKeyVersion) {
            m_nKeyVersion = nKeyVersion;
        }

        void SetKeyMode(u16 nKeyMode) {
            m_nKeyMode = (_V100_ENC_KEY_MODE)nKeyMode;
        }

        void SetKCV(u8* pKCV) {
            memcpy(&m_pKCV, pKCV, 4);
        }

    private:
        _V100_ENC_KEY_TYPE      m_nKeySlot;         // +
        _V100_ENC_KEY_MODE      m_nKeyMode;         // +
        u16                     m_nKeyVersion;      // 
        u8                      m_pKCV[4];          // 
};

// **************************************  CMD_ENC_CLEAR  ******************/

class Atomic_Enc_Clear : public IEncCmd {
    public:
        Atomic_Enc_Clear() { m_nCmd = CMD_ENC_CLEAR; };
        virtual ~Atomic_Enc_Clear() {};
};

class Atomic_Enc_Factory_Set_Key : public ICmd {
public:
    Atomic_Enc_Factory_Set_Key();
    ~Atomic_Enc_Factory_Set_Key();
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
    // Execute command
    virtual void Exec();
    //
    u256* GetKey() {
        return &m_Key;
    }

    u256* GetDigSig() {
        return &m_DigSig;
    }

    void SetKey(u256 _n) {
        memcpy(m_Key, _n, sizeof(u256));
    }

    void SetDigSig(u256 _n) {
        memcpy(m_DigSig, _n, sizeof(u256));
    }

private:
    // SPECIALIZED DATA GOES HERE
    u256 m_Key;
    u256 m_DigSig;

};

class Atomic_Enc_Get_Serial_Number : public ICmd
{
public:
    Atomic_Enc_Get_Serial_Number();
    ~Atomic_Enc_Get_Serial_Number();
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
    u64 GetSerialNum() { return m_SerialNum; }
    void SetSerialNum(u64 _n) { m_SerialNum = _n; }
private:
    // SPECIALIZED DATA GOES HERE
    u64        m_SerialNum;

};

class Atomic_Enc_Get_Rnd_Number : public ICmd {
public:
    Atomic_Enc_Get_Rnd_Number();
    ~Atomic_Enc_Get_Rnd_Number();
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
    // Execute command
    virtual void Exec();
    //
    u256* GetRandomNumber() { return &m_Rand; }
    //
    void SetRandomNumber(u256 rand) { memcpy(m_Rand, rand, sizeof(u256)); }
private:
    // ANBIO is only 16 bytes, but as it is encrypted, 32 bytes
    u256    m_Rand;
};

class Atomic_Enc_Set_Parameters : public ICmd
{
public:
    Atomic_Enc_Set_Parameters();
    ~Atomic_Enc_Set_Parameters();
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
    // Set Parameters
    virtual bool SetParameters(uchar* pSK, int szSK, uchar* pANSOL, int szANSOL, _V400_SPOOF_PROTECTION_LEVEL protLevel, int nTimeoutSeconds);
    virtual uchar* GetSK() { return m_pSKCG; }
    virtual int    GetSKSize() { return m_nSKCGSize; }
    virtual uchar* GetANSOL() { return m_pANSOL; }
    virtual int    GetANSOLSize() { return m_nANSOLSize; }
    virtual _V400_SPOOF_PROTECTION_LEVEL   GetSpoofProtectionLevel() { return m_nSPL; }
    virtual int    GetTimeout() { return m_nTimeoutSeconds; }
private:
    // SPECIALIZED DATA GOES HERE
    uchar*                              m_pSKCG;
    int                                 m_nSKCGSize;
    uchar*                              m_pANSOL;
    int                                 m_nANSOLSize;
    _V400_SPOOF_PROTECTION_LEVEL        m_nSPL;
    int                                 m_nTimeoutSeconds;
};

class Atomic_Enc_Decrypt : public ICmd
{
public:
    Atomic_Enc_Decrypt();
    ~Atomic_Enc_Decrypt();
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
    // Set Data Buffer
    bool SetDataBuffer(uchar* pData, uint nSize)
    {
        if (m_pDataBuffer)
        {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }
    uchar* GetDataBuffer()
    {
        return m_pDataBuffer;
    }
    int GetDataBufferSize()
    {
        return m_nDataBufferSize;
    }
    void  SetDigSig(u256 digSig)
    {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }
    u8* GetDigSig()
    {
        return (u8*)&m_DigSig;
    }

    // Get Data Buffer
private:
    // SPECIALIZED DATA GOES HERE
    uchar*  m_pDataBuffer;
    int     m_nDataBufferSize;
    u256    m_DigSig;
};

class Atomic_Enc_VerifyMatch : public ICmd
{
public:
    Atomic_Enc_VerifyMatch();
    ~Atomic_Enc_VerifyMatch();
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
    // Set Data Buffer
    bool SetDataBuffer(uchar* pData, uint nSize)
    {
        if (m_pDataBuffer)
        {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }
    uchar* GetDataBuffer()
    {
        return m_pDataBuffer;
    }
    int GetDataBufferSize()
    {
        return m_nDataBufferSize;
    }
    void  SetDigSig(u256 digSig)
    {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }
    void SetANSOLSK(u256 pAleatoryNumberBB)
    {
        memcpy(puszAleatoryNumberBB, pAleatoryNumberBB, sizeof(u256));
    }
    u8* GetANSOLSK()
    {
        return puszAleatoryNumberBB;
    }
    u8* GetDigSig()
    {
        return (u8*)&m_DigSig;
    }
    void    SetFMRRequested(long int FMRRequested)
    {
        m_nFMR = FMRRequested;
    }
    long int GetFMRRequested() { return m_nFMR; }
    void    SetResult(int _res) { m_Result = _res; }
    int        GetResult() { return m_Result; }
    // Get Data Buffer
private:
    // SPECIALIZED DATA GOES HERE
    uchar*     m_pDataBuffer;
    int        m_nDataBufferSize;
    u256       m_DigSig;
    long int   m_nFMR;
    int        m_Result;
    u256       puszAleatoryNumberBB;
};

class Atomic_Enc_Enroll : public ICmd {
public:
    Atomic_Enc_Enroll();
    ~Atomic_Enc_Enroll();
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
private:
};

class Atomic_Enc_Capture : public ICmd
{
public:
    Atomic_Enc_Capture();
    ~Atomic_Enc_Capture();
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
    void                  SetCaptureType(_V100_CAPTURE_TYPE type) { m_Type = type; }
    _V100_CAPTURE_TYPE    GetCaptureType() { return m_Type; }
    //
private:
    // SPECIALIZED DATA GOES HERE
    _V100_CAPTURE_TYPE    m_Type;
};

// **************************************  HID Commands  *******************/

class Atomic_Hid_Init : public IHidEncCmd {
    public:
        Atomic_Hid_Init();
        ~Atomic_Hid_Init();
        // ICmd
        void SetParam(const char* const param);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();
};

class Atomic_Hid_Enum_Cams : public IHidEncCmd {
    public:
        Atomic_Hid_Enum_Cams();
        ~Atomic_Hid_Enum_Cams();
        // ICmd
        void GetCamsList (string_list_t& str_list);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);
    private:
        virtual void do_cleanup();
    private:
        std::vector<std::string> m_enum_list_;
};

class Atomic_Hid_Terminate : public IHidEncCmd {
    public:
        Atomic_Hid_Terminate();
        ~Atomic_Hid_Terminate();
        // ICmd
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();
};

class Atomic_Hid_Set_Param_Int : public IHidEncCmd {
    public:
        Atomic_Hid_Set_Param_Int();
        ~Atomic_Hid_Set_Param_Int();
        // ICmd
        void SetParam(int32_t ctx, uint32_t id, int32_t val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        int32_t     m_val;
};

class Atomic_Hid_Get_Param_Int : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Param_Int();
        ~Atomic_Hid_Get_Param_Int();
        // ICmd
        void SetId(int32_t ctx, uint32_t id);
        void GetValue (int32_t& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        int32_t     m_val;
};

class Atomic_Hid_Set_Param_Str : public IHidEncCmd {
    public:
        Atomic_Hid_Set_Param_Str();
        ~Atomic_Hid_Set_Param_Str();
        // ICmd
        void SetParam(int32_t ctx, uint32_t id, std::string val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        std::string m_val;
};

class Atomic_Hid_Get_Param_Str : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Param_Str();
        ~Atomic_Hid_Get_Param_Str();
        // ICmd
        void SetId(int32_t ctx, uint32_t id);
        void GetValue(array_str_t& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        array_str_t m_val;
};

class Atomic_Hid_Set_Param_Bin : public IHidEncCmd {
    public:
        Atomic_Hid_Set_Param_Bin();
        ~Atomic_Hid_Set_Param_Bin();
        // ICmd
        void SetParam(int32_t ctx, uint32_t id, const uint8_t* const val, uint32_t val_len);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t         m_ctx;
        uint32_t        m_id;
        array_bin_t     m_val;
};

class Atomic_Hid_Get_Param_Bin : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Param_Bin();
        ~Atomic_Hid_Get_Param_Bin();
        // ICmd
        void SetId(int32_t ctx, uint32_t id);
        void GetValue(array_bin_t& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();
    
    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        array_bin_t m_val;
};

class Atomic_Hid_Set_Param_Long : public IHidEncCmd {
    public:
        Atomic_Hid_Set_Param_Long();
        ~Atomic_Hid_Set_Param_Long();
        // ICmd
        void SetParam(int32_t ctx, uint32_t id, double val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        double      m_val;
};

class Atomic_Hid_Get_Param_Long : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Param_Long();
        ~Atomic_Hid_Get_Param_Long();
        // ICmd
        void SetId(int32_t ctx, uint32_t id);
        void GetValue(double& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t     m_ctx;
        uint32_t    m_id;
        double      m_val;
};

class Atomic_Hid_Capture_Img : public IHidEncCmd {
    public:
        Atomic_Hid_Capture_Img();
        ~Atomic_Hid_Capture_Img();
        // ICmd
        void SetParam(HFContext ctx, int32_t timeout, double minimalQuality, double minimalLivenessScore, uint64_t intermediateResultFlags, uint64_t finalResultFlags);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext   m_ctx;
        int32_t     m_timeout;
        double      m_minQuality;
        double      m_minLiveness;
        uint64_t    m_intFlags;
        uint64_t    m_finalFlags;
        HFOperation m_operation;
};

class Atomic_Hid_Open_Context : public IHidEncCmd {
    public:
        Atomic_Hid_Open_Context();
        ~Atomic_Hid_Open_Context();
        // ICmd
        void SetParam (int32_t cam_id, HFAlgorithmType algo_type);
        void GetValue (HFContext& ctx);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext           m_ctx;
        int32_t             m_camId;
        HFAlgorithmType     m_algoType;
};

class Atomic_Hid_Close_Context : public IHidEncCmd {
    public:
        Atomic_Hid_Close_Context();
        ~Atomic_Hid_Close_Context();
        // ICmd
        void SetId(HFContext id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext  m_ctx;
};

class Atomic_Hid_Async_Stop_Operation : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Stop_Operation();
        ~Atomic_Hid_Async_Stop_Operation();
        // ICmd
        void SetId(HFOperation operation);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        int32_t m_operation;
};

class Atomic_Hid_Async_Extract_Template : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Extract_Template();
        ~Atomic_Hid_Async_Extract_Template();
        // ICmd
        void SetParam(HFContext ctx, HFImageEncoding imageEncoding, const void* const img_ptr, uint32_t img_len, uint64_t flags );
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFImageEncoding m_encoding;
        HFContext       m_ctx;
        uint64_t        m_finalFlags;
        array_bin_t     m_img;
        HFOperation     m_res;
};

class Atomic_Hid_Async_Match_With_Template : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Match_With_Template();
        ~Atomic_Hid_Async_Match_With_Template();
        // ICmd
        void SetParam(HFContext ctx, const void* const templA_ptr, uint32_t len_A, const void* const templB_ptr, uint32_t len_B);
        void GetValue(HFOperation& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext       m_ctx;
        array_bin_t     m_templA;
        array_bin_t     m_templB;
        HFOperation     m_operation;
};

class Atomic_Hid_Async_Verify_With_Captured : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Verify_With_Captured();
        ~Atomic_Hid_Async_Verify_With_Captured();
        // ICmd
        void SetParam(HFOperation op, const char* const galery, const char* const id, double minScore);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation     m_op;
        double          m_minScore;
        array_str_t     m_gallery;
        array_str_t     m_id;
};

class Atomic_Hid_Async_Identify_With_Captured : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Identify_With_Captured();
        ~Atomic_Hid_Async_Identify_With_Captured();
        // ICmd
        void SetParam(HFOperation op, const char* const gallery, double minScore);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation     m_op;
        array_str_t     m_gallery;
        double          m_minScore;
};

class Atomic_Hid_Async_Match_With_Captured : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Match_With_Captured();
        ~Atomic_Hid_Async_Match_With_Captured();
        // ICmd
        void SetParam(HFOperation op, const uint8_t* const bin, uint32_t len);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation     m_op;
        array_bin_t     m_data;
};

class Atomic_Hid_Async_Identify_With_Template : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Identify_With_Template();
        ~Atomic_Hid_Async_Identify_With_Template();
        // ICmd
        void SetParams (HFContext ctx, const char* const gal_ptr, double minScore, const uint8_t* const templ_ptr, uint32_t teml_len);
        void GetValue(HFOperation& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext           m_ctx;
        array_str_t         m_gal;
        double              m_minScore;
        array_bin_t         m_templ;
        HFOperation         m_operation;
};

class Atomic_Hid_Async_Verify_With_Template : public IHidEncCmd {
    public:
        Atomic_Hid_Async_Verify_With_Template();
        ~Atomic_Hid_Async_Verify_With_Template();
        // ICmd
        void SetParams(HFContext ctx, double minScore, const char* const gal_ptr, const char* const id_ptr, const uint8_t* const templ_ptr, uint32_t templ_len );
        void GetValue(HFOperation& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext           m_ctx;
        double              m_minScore;
        array_str_t         m_gal;
        array_str_t         m_id;
        array_bin_t         m_templ;
        HFOperation         m_operation;
};

class Atomic_Hid_Parse_Res_Int : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Res_Int();
        ~Atomic_Hid_Parse_Res_Int();
        // ICmd
        void SetId (uint32_t id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t    m_id;
        int32_t     m_val;
};

class Atomic_Hid_Parse_Res_Double : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Res_Double();
        ~Atomic_Hid_Parse_Res_Double();
        // ICmd
        void SetId(uint32_t id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t    m_id;
        double      m_val;
};

class Atomic_Hid_Parse_Res_Data : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Res_Data();
        ~Atomic_Hid_Parse_Res_Data();
        // ICmd
        void SetId(int id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t    m_id;
        array_bin_t m_val;
};

class Atomic_Hid_Parse_Res_Point : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Res_Point();
        ~Atomic_Hid_Parse_Res_Point();
        // ICmd
        void SetId(int id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t    m_id;
        int32_t     m_x;
        int32_t     m_y;
};

class Atomic_Hid_Parse_Res_Image : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Res_Image();
        ~Atomic_Hid_Parse_Res_Image();
        // ICmd
        void SetId(int id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t            m_id;
        HFImageEncoding     m_encoding;
        array_bin_t         m_image;
};

class Atomic_Hid_Parse_Match_Gallery : public IHidEncCmd {
    public:
        Atomic_Hid_Parse_Match_Gallery();
        ~Atomic_Hid_Parse_Match_Gallery();
        // ICmd
        void SetId(int id);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        uint32_t             m_id;
        array_match_rec_t    m_match_list;
};

class Atomic_Hid_Get_Video_Frame : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Video_Frame();
        ~Atomic_Hid_Get_Video_Frame();
        // ICmd
        void SetParam(HFContext ctx, int64_t sec);
        void GetValue(int64_t& seq, HFImageEncoding& encoding, array_bin_t& data_bin);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFContext           m_ctx;
        int64_t             m_seqIn;
        int64_t             m_seqOut;
        HFImageEncoding     m_encoding;
        array_bin_t         m_image;
};

class Atomic_Hid_Get_Intermediate_Res : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Intermediate_Res();
        ~Atomic_Hid_Get_Intermediate_Res();
        // ICmd
        void SetParam(HFOperation operation, uint64_t resultFlags, int32_t lastSequenceNumber);
        void GetValue(array_bin_t& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation     m_op;
        uint64_t        m_flags;
        int32_t         m_sequence;
        array_bin_t     m_data;
};

class Atomic_Hid_Get_Final_Res : public IHidEncCmd {
    public:
        Atomic_Hid_Get_Final_Res();
        ~Atomic_Hid_Get_Final_Res();
        // ICmd
        void SetParam(HFOperation operation, uint64_t resultFlags);
        void GetValue(array_bin_t& val);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation     m_op;
        uint64_t        m_flags;
        array_bin_t     m_data;
};

class Atomic_Hid_Db_Add_Record_With_Captured : public IHidEncCmd {
    public:
        Atomic_Hid_Db_Add_Record_With_Captured();
        ~Atomic_Hid_Db_Add_Record_With_Captured();
        // ICmd
        void SetParam(HFOperation op, bool replace, const char* const id, const char* const gallery, const char* const custom);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        HFOperation         m_op;
        bool                m_replace;
        array_str_t         m_id;
        array_str_t         m_gallery;
        array_str_t         m_custom;
};

class Atomic_Hid_Db_Add_Record_With_Template : public IHidEncCmd {
    public:
        Atomic_Hid_Db_Add_Record_With_Template();
        ~Atomic_Hid_Db_Add_Record_With_Template();
        // ICmd
        void SetParam(bool replace, const char* const id, const char* const gallery, const char* const custom, const uint8_t* const data_ptr, uint32_t data_len);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        bool                m_replace;
        array_str_t         m_id;
        array_str_t         m_gallery;
        array_str_t         m_custom;
        array_bin_t         m_image;
};

class Atomic_Hid_Db_Get_Record : public IHidEncCmd {
    public:
        Atomic_Hid_Db_Get_Record();
        ~Atomic_Hid_Db_Get_Record();
        // ICmd
        void SetParam(const char * const id, const char * const gallery);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        array_str_t         m_id;
        array_str_t         m_gallery;
        array_str_t         m_custom;
        array_bin_t         m_data;
};

class Atomic_Hid_Db_List_Records : public IHidEncCmd {
    public:
        Atomic_Hid_Db_List_Records();
        ~Atomic_Hid_Db_List_Records();
        // ICmd
        void SetId( const char* const gallery);
        void GetParams(string_list_t& list);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        array_str_t                 m_gallery;
        std::vector<array_str_t>    m_list;
};

class Atomic_Hid_Db_Del_Record : public IHidEncCmd {
    public:
        Atomic_Hid_Db_Del_Record();
        ~Atomic_Hid_Db_Del_Record();
        // ICmd
        void SetId(const char * const id, const char * const gallery);
        // Takes content of Command, and packs it into pPacket
        virtual bool PackChallengeHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackChallengeHid();
        // Takes content of Command, and packs it into pPacket
        virtual bool PackResponseHid();
        // Unpacks packet passed in into internal data structure
        virtual bool UnpackResponseHid();
        // Execute command
        virtual void Exec();
        // Dump data to console
        virtual void dump(std::stringstream& msg);

    private:
        virtual void do_cleanup();

    private:
        array_str_t   m_id;
        array_str_t   m_gallery;
};

// **************************************  CMD_ENC_SET_KEY  *****************/

class Atomic_Enc_Set_Key : public ICmd {
    public:
        Atomic_Enc_Set_Key();
        ~Atomic_Enc_Set_Key();
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
        // Set Key
        bool SetBuffer(u8* m_pBuffer, uint m_nBufferSize);
        // Execute command
        virtual void Exec();

        u8* GetBuffer() {
            return m_pBuffer;
        }

        uint GetBufferSize() {
            return m_nBufferSize;
        }

        _V100_ENC_KEY_TYPE GetKeyType() {
            return m_nKeyType;
        }

        void SetKeyType(_V100_ENC_KEY_TYPE nKeyType) {
            m_nKeyType = nKeyType;
        }

    private:
        // SPECIALIZED DATA GOES HERE
        u8*                   m_pBuffer;
        uint                  m_nBufferSize;
        _V100_ENC_KEY_TYPE    m_nKeyType;
};

// **************************************  CMD_ENC_RETURNCAPTUREDBIR_IM  ****/

class Atomic_Enc_ReturnCapturedBIR_IM : public Atomic_Enc_ReturnCapturedBIR
{
public:
    Atomic_Enc_ReturnCapturedBIR_IM();
    ~Atomic_Enc_ReturnCapturedBIR_IM();

    // Cmd
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into internal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into internal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);

public:
    void SetImageType(_V100_IMAGE_TYPE type) { m_nImage_Type = type; }
    _V100_IMAGE_TYPE GetImageType() { return m_nImage_Type; }
    uint GetImageSize() { return m_nImageCGBufferSize; }
    u8* GetImageDigSig() { return (u8*)&m_ImageCGDigSig; }
    u8* GetImageCG() { return (u8*)m_pImageCGBuffer; }
    bool SetImageCG(uchar* pImageCG, uint nSize, u256 digSig);

protected:
    _V100_IMAGE_TYPE    m_nImage_Type;
    uchar*              m_pImageCGBuffer;
    uint                m_nImageCGBufferSize;
    u256                m_ImageCGDigSig;
};

class Atomic_Enc_Generate_SessionKey : public ICmd
{
public:
    Atomic_Enc_Generate_SessionKey();
    ~Atomic_Enc_Generate_SessionKey();
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

private:
    // SPECIALIZED DATA GOES HERE

};

class Atomic_Enc_Get_Spoof_Score : public ICmd
{
public:
    Atomic_Enc_Get_Spoof_Score();
    ~Atomic_Enc_Get_Spoof_Score();
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
    u128* GetANSOL() { return &m_pANSOL; }
    void SetANSOL(u128 pANSOL) { memcpy(m_pANSOL, pANSOL, sizeof(u128)); }
    u8* GetSpoofResult() { return m_CGSpoofResult; }
    uint GetSpoofResultSize() { return m_CGSpoofResultsz; }
    bool SetSpoofResult(u8* pCGSpoofResult, uint nCGSpoofResultsz);
private:

    u128    m_pANSOL;
    u8*     m_CGSpoofResult;
    uint    m_CGSpoofResultsz;
};

class Atomic_Enc_Set_ActiveKey : public ICmd
{
public:
    Atomic_Enc_Set_ActiveKey();
    ~Atomic_Enc_Set_ActiveKey();
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
    u16 GetKeySlot() { return m_nKeySlot; }
    void SetKeySlot(u16 _nKeySlot) { m_nKeySlot = _nKeySlot; }
private:
    // SPECIALIZED DATA GOES HERE
    u16 m_nKeySlot;
};

class Atomic_Enc_VerifyMatch_Result : public ICmd
{
public:
    Atomic_Enc_VerifyMatch_Result();
    ~Atomic_Enc_VerifyMatch_Result();
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
    // Set Data Buffer
    bool SetDataBuffer(uchar* pData, uint nSize)
    {
        if (m_pDataBuffer)
        {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }
    uchar* GetDataBuffer()
    {
        return m_pDataBuffer;
    }
    int GetDataBufferSize()
    {
        return m_nDataBufferSize;
    }
    void  SetDigSig(u256 digSig)
    {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }
    u8* GetDigSig()
    {
        return (u8*)&m_DigSig;
    }
private:
    // SPECIALIZED DATA GOES HERE
    uchar*  m_pDataBuffer;
    int     m_nDataBufferSize;
    u256    m_DigSig;
};

class Macro_Enc_VerifyMatch_Many : public ICmd {
public:
    Macro_Enc_VerifyMatch_Many();
    ~Macro_Enc_VerifyMatch_Many();
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

    // Set Data Buffer
    bool SetDataBuffer(uchar* pData, uint nSize) {
        if (m_pDataBuffer) {
            FREE(m_pDataBuffer);
            m_pDataBuffer = NULL;
        }
        m_pDataBuffer = (uchar*)MALLOC(nSize);
        memcpy(m_pDataBuffer, pData, nSize);
        m_nDataBufferSize = nSize;
        return true;
    }

    uchar* GetDataBuffer() {
        return m_pDataBuffer;
    }

    int GetDataBufferSize() {
        return m_nDataBufferSize;
    }

    void SetDigSig(u256 digSig) {
        memcpy(m_DigSig, digSig, sizeof(u256));
    }

    u8* GetDigSig() {
        return (u8*)&m_DigSig;
    }

    // Get Data Buffer

private:
    // SPECIALIZED DATA GOES HERE
    uchar*    m_pDataBuffer;
    int       m_nDataBufferSize;
    u256      m_DigSig;
};

