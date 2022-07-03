#include "V100EncCmdBase.h"
#include "V100EncCmdMSK00.h"
#include "MSK00Broker.h"
#include "CmdExecutiveMSK00.h"
#include "ISensorInstance.h"
#include "IXServiceProvider.h"
#include "ISELogger.h"

using namespace MSK00;

#define __CLASSNAME__ MSK00Broker

#define CMD_EXECUTIVE CmdExecutiveMSK00::GetInstance()

#define MKD_CHECK_ON    1
#if MKD_CHECK_ON
#define CHECK_MKD(A)    if( false == CMD_EXECUTIVE.IsMKDLoaded() ){ A->SetReturnCode(GEN_NOT_SUPPORTED); return false; }
#else
#define CHECK_MKD(A)
#endif



MSK00Broker::MSK00Broker()
{
    REG_EXT_CMD(CMD_ENC_GET_RND_NUMBER, Atomic_Enc_Get_Rnd_Number,                false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_PARAMETERS, Atomic_Enc_Set_Parameters,                false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_KCV, Atomic_Enc_Get_KCV,                            false, false, false, true);
    REG_EXT_CMD(CMD_ENC_VERIFYMATCH, Atomic_Enc_VerifyMatch,                    false, false, false, true);
    REG_EXT_CMD(CMD_ENC_IDENTIFYMATCH, Atomic_Enc_IdentifyMatch,                false, false, false, true);
    REG_EXT_CMD(CMD_ENC_ENROLL, Atomic_Enc_Enroll,                                false, false, false, false);
    REG_EXT_CMD(CMD_ENC_CAPTURE, Atomic_Enc_Capture,                            false, false, false, true);
    REG_EXT_CMD(CMD_ENC_RETURNCAPTUREDBIR, Atomic_Enc_ReturnCapturedBIR,        false, false, false, true);
    REG_EXT_CMD(CMD_ENC_FACTORY_SET_KEY, Atomic_Enc_Factory_Set_Key,            false, false, false, true);
    REG_EXT_CMD(CMD_ENC_RETURNCAPTUREDBIR_IM, Atomic_Enc_ReturnCapturedBIR_IM,    false, false, false, true);
    REG_EXT_CMD(CMD_ENC_CLEAR, Atomic_Enc_Clear,                                false, false, false, false);
    REG_EXT_CMD(CMD_ENC_GET_SPOOF_SCORE, Atomic_Enc_Get_Spoof_Score,            false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_DIAG_STATUS, Atomic_Enc_Get_Diagnostic_Status,        false, true , true , false);
    REG_EXT_CMD(CMD_ENC_GET_KEYVERSION, Base::Atomic_Enc_Get_KeyVersion,        false, false, false, true);

    // TECBAN
    REG_EXT_CMD(CMD_ENC_VERIFYMATCH_MANY, Macro_Enc_VerifyMatch_Many,            false, false, false, true);
    REG_EXT_CMD(CMD_ENC_VERIFYMATCH_RESULT, Atomic_Enc_VerifyMatch_Result,        false, false, false, true);
}


bool MSK00Broker::cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_Get_Rnd_Number);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Rnd_Number");

    short nArg = 0;    // nArg represents 1 for EncryptedWithSK, 0 for MKD
    object->GetArguement(nArg);

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_Rnd_Number(object->GetRandomNumber(), nArg))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_Set_Parameters);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Parameters");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Set_Parameters(object->GetSK(),
                                                    (uint)object->GetSKSize(),
                                                    object->GetANSOL(),
                                                    (uint)object->GetANSOLSize(),
                                                    object->GetSpoofProtectionLevel(),
                                                    object->GetTimeout()))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_GET_KCV(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_Get_KCV);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_KCV");

    uchar* pKCV = nullptr;
    uint nOutKCVSize = object->GetKCVSize();

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_KCV(&pKCV, nOutKCVSize))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    object->SetKCV(pKCV, nOutKCVSize);

    FREE_MEM(pKCV);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_VERIFYMATCH(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_VerifyMatch);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_VerifyMatch");

    // Fill in
    uchar* pOutCG = nullptr;
    uint nOutCGSize = 0;
    u256 outDigSig;
    uint nResult;
    uint nFMRRequested = (uint)object->GetFMRRequested();
    uint nFMRScore;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_VerifyMatch(nFMRRequested,
        object->GetDataBuffer(),
        object->GetDataBufferSize(),
        object->GetDigSig(),
        object->GetANSOLSK(),
        &pOutCG,
        nOutCGSize,
        &nFMRScore,
        &nResult))
    {
        object->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }

    object->SetDataBuffer(pOutCG, nOutCGSize);
    object->SetDigSig(outDigSig);
    object->SetResult(nResult);
    object->SetFMRRequested((int)nFMRScore);

    FREE_MEM(pOutCG);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_IDENTIFYMATCH(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_IdentifyMatch);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_IdentifyMatch");

    // Fill in
    uchar*        pOutCG = NULL;
    uint         nOutCGSize = 0;
    u256        outDigSig;
    uint        nResult;
    uint         nFMRRequested = (uint)object->GetFMRRequested();
    uint         nFMRScore = 0;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_IdentifyMatch(nFMRRequested,
        object->GetDataBuffer(),
        object->GetDataBufferSize(),
        object->GetDigSig(),
        object->GetANSOLSK(),
        &pOutCG,
        &nOutCGSize,
        &nFMRScore,
        &nResult))
    {
        object->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }

    object->SetDataBuffer(pOutCG, nOutCGSize);
    object->SetDigSig(pOutCG);
    object->SetResult(nResult);
    object->SetFMRRequested((int)nFMRScore);

    FREE_MEM(pOutCG);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_ENROLL(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_Enroll);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Enroll");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Enroll())
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_CAPTURE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Capture);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Capture");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Capture((_V100_CAPTURE_TYPE)object->GetCaptureType()))
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_RETURNCAPTUREDBIR(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_ReturnCapturedBIR);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_ReturnCapturedBIR");

    // Fill in
    uchar* pOutTplCG = nullptr;
    uint     nOutTplCGSize = 0;
    u256    outTplDigSig;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_ReturnCapturedBIR(object->GetBIR(),
        object->GetCG(),
        object->GetCGSize(),
        &pOutTplCG,
        nOutTplCGSize,
        outTplDigSig))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    // Set Out variables
    object->SetCG(pOutTplCG, nOutTplCGSize);
    object->SetDigSig(outTplDigSig);

    FREE_MEM(pOutTplCG);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_FACTORY_SET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Factory_Set_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Factory_Set_Key");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Factory_Set_Key((u8*)object->GetKey(), sizeof(u256), (u8*)object->GetDigSig()))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_RETURNCAPTUREDBIR_IM(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_ReturnCapturedBIR_IM);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_ReturnCapturedBIR_IM");

    // Get tpl, image, encrypt and return
    uchar* pOutTplCG = NULL, * pOutImgCG = NULL;
    uint     nOutTplCGSize = 0, nOutImgCGSize = 0;
    u256    outTplDigSig, outImgDigSig;
    CExecStatus CERet;

    //TODO should we optimize image buffers ??
    CERet = CMD_EXECUTIVE.Execute_Enc_ReturnCapturedBIR_IM(object->GetImageType(),
        object->GetBIR(),
        object->GetCG(),
        object->GetCGSize(),
        &pOutTplCG,
        nOutTplCGSize,
        outTplDigSig,
        &pOutImgCG,
        nOutImgCGSize,
        outImgDigSig);
    if (CERet == CExecStatus::CMD_ERR_NOT_SUPPORTED)
    {

        pCmd->SetReturnCode(GEN_NOT_SUPPORTED);
    }
    else if (CERet != CExecStatus::CMD_EXEC_OK)
    {
        pCmd->SetReturnCode(GEN_ERROR_INTERNAL);
    }
    else
    {
        object->SetCG(pOutTplCG, nOutTplCGSize);
        object->SetDigSig(outTplDigSig);
        object->SetImageCG(pOutImgCG, nOutImgCGSize, outImgDigSig);
        object->SetReturnCode(GEN_OK);
    }

    FREE_MEM(pOutTplCG);
    FREE_MEM(pOutImgCG);

    return (CERet == CExecStatus::CMD_EXEC_OK) ? true : false;
}


bool MSK00Broker::cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Clear");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Clear())
    {
        pCmd->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    pCmd->SetReturnCode(GEN_OK);
    return true;
}


bool MSK00Broker::cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(IEncCmd);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Spoof_Score");

    uchar* pOutCG = NULL;
    uint   nOutCGSize = 0;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_Spoof_Score(object->GetCryptogram(), object->GetCryptogramSize(), &pOutCG, &nOutCGSize))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    object->SetCryptogram(pOutCG, nOutCGSize);
    FREE_MEM(pOutCG);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_GET_DIAG_STATUS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Diagnostic_Status);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Diagnostic_Status");

    _V100_ENC_DIAG_STATUS diagStatus;

    CExecStatus stat = CMD_EXECUTIVE.Execute_Enc_Get_Diag_Status(diagStatus);
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        pCmd->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }
    object->SetStatus(diagStatus);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Base::Atomic_Enc_Get_KeyVersion);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_KeyVersion");

    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)object->GetKeySlot();
    u16 nKeyVer = 0;
    u8  pKCV[4] = {};
    _V100_ENC_KEY_MODE nKeyMode;

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Enc_Get_KeyVersion(nKeyType, nKeyVer, nKeyMode, pKCV);
    if (nCEErr != CExecStatus::CMD_EXEC_OK)
    {
        pCmd->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    object->SetKeyVersion(nKeyVer);
    object->SetKCV(pKCV);
    object->SetKeyMode(nKeyMode);

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_VERIFYMATCH_MANY(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Macro_Enc_VerifyMatch_Many);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Macro_Enc_VerifyMatch_Many");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_VerifyMatch_Many(object->GetDataBuffer(), object->GetDataBufferSize(), object->GetDigSig()))
    {
        object->SetReturnCode(GEN_ENCRYPTION_FAIL);
        return false;
    }

    RETURN_SUCCESS;
}


bool MSK00Broker::cb_CMD_ENC_VERIFYMATCH_RESULT(const std::shared_ptr<ICmd>& pCmd)
{
    CHECK_MKD(pCmd);
    DOWNCAST_TO(Atomic_Enc_VerifyMatch_Result);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_VerifyMatch_Result");

    uchar* pOutCG = nullptr;
    uint nOutCGSize = 0;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_VerifyMatch_Result(&pOutCG, nOutCGSize, object->GetDigSig()))
    {
        object->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }

    object->SetDataBuffer(pOutCG, nOutCGSize);

    FREE_MEM(pOutCG);

    RETURN_SUCCESS;
}
