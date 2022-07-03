#pragma once
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_enc_types.h"
#include "CmdExecutiveDefs.h"
#include "CExecStatus.h"


class CmdExecutiveMSK00 : MemoryBase
{
public:

    CExecStatus        Execute_Enc_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk);
    CExecStatus        Execute_Enc_Factory_Set_Key(u8* pInCG, uint nInCGSize, u256 pDigSig);
    CExecStatus        Execute_Enc_Capture(_V100_CAPTURE_TYPE nCapType);
    CExecStatus        Execute_Enc_Enroll();
    CExecStatus        Execute_Enc_ReturnCapturedBIR(_V100_ENC_BIR_HEADER_PROPRIETARY* pBIRHdr, u8* pInCG, uint  nInCGSize, u8** pOutCG, uint& nOutCGSize, u256 pOutDigSig);
    CExecStatus        Execute_Enc_ReturnCapturedBIR_IM(_V100_IMAGE_TYPE imageType, _V100_ENC_BIR_HEADER_PROPRIETARY* pBIRHdr, u8* pInCG, uint  nInCGSize, u8** pOutTplCG,
        uint& nOutTplCGSize, u256 pOutTplDigSig, u8** pOutImgCG, uint& nOutImgCGSize, u256 pOutImgDigSig);
    CExecStatus        Execute_Enc_Set_Parameters(u8* pInCG, uint nInCGSize, u8* pANSOL, uint nANSOLSize, _V100_ENC_SPOOF_PROTECTION_LEVEL nSpoofProtection, int nTimeout);
    CExecStatus        Execute_Enc_Get_KCV(u8** pKCV, uint nKCVSize);
    CExecStatus        Execute_Enc_VerifyMatch(uint nFMRRequested, u8* pInCG, uint  nInCGSize, u256  pInDigSig, u256  pInANSOLCG, u8** pOutCG,
        uint& nOutCGSize, uint* nFMRScore, uint* nResult);
    CExecStatus        Execute_Enc_IdentifyMatch(uint nFMRRequested, u8* pInCG, uint  nInCGSize, u256  pInDigSig, u256  pInANSOLCG, u8** pOutCG,
        uint* nOutCGSize, uint* nFMRScore, uint* nResult);
    CExecStatus        Execute_Enc_Get_Spoof_Score(u8* pInCG, uint nInCGSize, u8** pOutCG, uint* nOutCGSize);
    CExecStatus     Execute_Enc_Clear();
    CExecStatus       Execute_Enc_Get_Diag_Status(_V100_ENC_DIAG_STATUS& diagStatus);
    CExecStatus        Execute_Enc_Get_KeyVersion(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV);

    // TECBAN
    CExecStatus     Execute_Enc_VerifyMatch_Many(u8* pInCG, uint nInCGSize, u256 pInDigSig);
    CExecStatus     Execute_Enc_VerifyMatch_Result(u8** pOutCG, uint& nOutCGSize, u256 pOutDigSig);

    // Helper method for MSK00Broker
    bool IsMKDLoaded();


    static CmdExecutiveMSK00& GetInstance()
    {
        static CmdExecutiveMSK00 instance;
        return instance;
    }
    CmdExecutiveMSK00(const CmdExecutiveMSK00&) = delete;
    CmdExecutiveMSK00& operator=(const CmdExecutiveMSK00&) = delete;

private:
    CmdExecutiveMSK00()
    {

    }
    ~CmdExecutiveMSK00()
    {

    }

    // Helper function(s)
    CExecStatus VerifyBIRAgainstCaptured(u8* pTpl, u32 nFMRRequested, uint* nResult, uint* nFMRScore, _V100_ENC_VERIFY_MATCH_RESULT&);

};
