#pragma once

#include "IMemMgr.h"
#include "lumi_stdint.h"
#include "IBSP.h"
#include "ISecureElement.h"
#ifdef _WIN32
#include "VirtualSecureElement.h"
#endif // _WIN32

// #define SECURE_ELEMENT ISecureElement::GetInstance()

class CryptoMgr : public MemoryBase
{
public:
    CryptoMgr() {};
    virtual ~CryptoMgr() {};

    static bool CheckSEStatus();
    static bool CheckHWSupportsSE();
    static bool GetSESerialNumber(u64* pSN);
    static bool GetSEVersion(u32* pVer);

    // Revisit these functions to obtain appropriate diagnostic info from OpTee and provide to CmdExecutiveBase::Execute_Get_System_State(), perform diagnostics
    //virtual void GetSEDiagnosticInfo(XmtDiagType &atsha_diag) = 0;
    //virtual bool PerformSelfDiagnostic(bool bVerbose) = 0;

    // IsKeyLoaded: Returns true if key is found in filesystem.
    bool IsKeyLoaded(u16 nKeySlot);

    bool IsZeroVecKey(u8* pKey, u32 nKeySize);

    int GetKCVType(u16 nKeyType, u16 nKeyMode);
    int GetKCVTypeForVariableMode(u16 nKeyMode);
    u32 GetKeySize(u16 nKeyType, u16 nKeyMode);
    u32 GetKeySizeForVariableMode(u16 nKeyMode);
    bool ValidateKeyMode(u16 nKeyType, u16 nKeyMode);

    bool CreateKCV(u8* pKey, uint nKeySize, uint nZeros, uint nVals, u8* pKCV, int nKCVType);

private:

protected:

    // ReleaseSEBuff: Releases memory allocated by secure element
    void ReleaseSEBuff(u8** pIn);

    // GetKeySlot: Helper method to retrieve key slot for requested key type. Returns
    //    false if key not found in map. This method should be used to avoid exceptions
    //    caused by key map not initialized correctly or trying to get slot for an
    //    unsupported key
    static bool GetKeySlot(u16 nKeyType, u16& nKeySlot);

    bool PullKeyInfo(u16 nKeySlot, u16& nKeyVer, u16& nKeyMode, u8* pKCV32);
    // EncryptBuffer: Note that this call expects pIn to be aligned to BLOCK_SIZE
    //***   NOTE: This function will not support (Host)RSA or DUKPT key encryption,
    //***    instead user should call SE->Select_<DUKPT/RSA>_Key then call SE->Encrypt
    bool EncryptBuffer(u16 nKeySlot, u8* pIn, uint nInSize, u8** pOutCG, uint& nOutCGSize);
    // DecryptBuffer: Note that this call expects pInCG to be aligned to BLOCK_SIZE
    //***   NOTE: This function will not support (Host)RSA or DUKPT key decryption,
    //***    instead user should call SE->Select_<DUKPT/RSA>_Key then call SE->Decrypt
    bool DecryptBuffer(u16 nKeySlot, u8* pInCG, uint nInCGSize, u8** pOut, uint& nOutSize);
};

