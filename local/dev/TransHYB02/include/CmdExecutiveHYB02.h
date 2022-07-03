#pragma once

#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_enc_types.h"
#include "CmdExecutiveDefs.h"


class CmdExecutiveHYB02 : public MemoryBase
{
public:

    CExecStatus        Execute_Enroll(_V100_CAPTURE_TYPE& nBestInsertion);
    CExecStatus        Execute_Capture(_V100_CAPTURE_TYPE nCapType);
    CExecStatus        Execute_Clear();
    CExecStatus     Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize);
    CExecStatus        Execute_Verify(u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS);
    CExecStatus        Execute_Match(u8* pInTpl1CG, uint nInTpl1CGSize, u256 pInTpl1DS, u8* pInTpl2CG, uint nInTpl2CGSize, u256 pInTpl2DS, uint& nMatchResult);


    CExecStatus        Execute_Get_Image(_V100_CAPTURE_TYPE capture_type, u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS);
    CExecStatus        Execute_Get_Spoof_Score(_V100_CAPTURE_TYPE capture_type, u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS);
    CExecStatus        Execute_Get_Template(_V100_CAPTURE_TYPE capture_type, u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS);

    CExecStatus        Execute_Set_Parameters(u8* pInCG, uint nInCGSize, u256 pInDS);
    CExecStatus        Execute_Get_Parameters(u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS);

    CExecStatus        Execute_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk);
    CExecStatus     Execute_Generate_Session_Key();
    CExecStatus     Execute_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u32& nKeySize, u8* pKCV, u16& nVerNum, _V100_ENC_KEY_MODE& nKeyMode);
    CExecStatus        Execute_Get_Key_Version(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV);
    CExecStatus     Execute_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGKeySize, u8** pOutCG, uint& nOutCGSize);
    CExecStatus     Execute_Set_Active_Key(_V100_ENC_KEY_TYPE nKeyType);
    CExecStatus     Execute_Get_Capture_Stats(_V100_CAPTURE_TYPE capture_type, _V100_CAPTURE_STATS* capture_stats);

    void            Release_Buffer(u8** pBuffer);

    static CmdExecutiveHYB02& GetInstance()
    {
        static CmdExecutiveHYB02 instance;
        return instance;
    }
    CmdExecutiveHYB02(const CmdExecutiveHYB02&) = delete;
    CmdExecutiveHYB02& operator=(const CmdExecutiveHYB02&) = delete;

private:
    CmdExecutiveHYB02()
    {
    }
    ~CmdExecutiveHYB02()
    {

    }

    CExecStatus Verify(u8* pInData, _V100_ENC_VERIFY_RESULT_HDR* pVerifyResultHdr, uint* pVerifyScores);

};

