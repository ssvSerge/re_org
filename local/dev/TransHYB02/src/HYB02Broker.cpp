#include "V100EncCmdBase.h"
#include "V100EncCmdProvision.h"
#include "V100EncCmdHYB02.h"
#include "HYB02Broker.h"
#include "CmdExecutiveHYB02.h"
#include "ICmd.h"
#include "V100Cmd.h"
#include "ISELogger.h"
#include "ISensorInstance.h"
#include "IXServiceProvider.h"

using namespace HYB02;
using namespace Provision;
using namespace Base;

#define __CLASSNAME__ HYB02Broker

#define CMD_EXECUTIVE CmdExecutiveHYB02::GetInstance()

HYB02Broker::HYB02Broker()
{
    // Common Commands
    REG_EXT_CMD(CMD_SET_OPTION,                 Atomic_Set_Option,                 false, false, false, true);

    // Provisioning Commands
    REG_EXT_CMD(CMD_ENC_GET_RND_NUMBER,         Atomic_Enc_Get_Rnd_Number,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_KEY,             Atomic_Enc_Get_Key,             false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_KEYVERSION,         Atomic_Enc_Get_KeyVersion,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_KEY,             Atomic_Enc_Set_Key,             false, false, false, true);

    // HYB02 Commands
    REG_EXT_CMD(CMD_ENC_MATCH,                 Atomic_Enc_Match,                 false, false, false, true);
    REG_EXT_CMD(CMD_ENC_VERIFY,                 Atomic_Enc_Verify,                 false, false, false, true);
    REG_EXT_CMD(CMD_ENC_ENROLL,                 Atomic_Enc_Enroll,                 false, false, false, true);
    REG_EXT_CMD(CMD_ENC_CAPTURE,             Atomic_Enc_Capture,             false, false, false, true);
    REG_EXT_CMD(CMD_ENC_CLEAR,                 Atomic_Enc_Clear,                 false, false, false, false);
    REG_EXT_CMD(CMD_ENC_GET_IMAGE,             Atomic_Enc_Get_Image,             false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_SPOOF_SCORE,     Atomic_Enc_Get_Spoof_Score,     false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_TEMPLATE,         Atomic_Enc_Get_Template,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_PARAMETERS,         Atomic_Enc_Set_Parameters,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_PARAMETERS,         Atomic_Enc_Get_Parameters,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GENERATE_SESSIONKEY, Atomic_Enc_Generate_SessionKey, false, false, false, false);
    REG_EXT_CMD(CMD_ENC_SET_ACTIVE_KEY,         Atomic_Enc_Set_Active_Key,         false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_CAPTURE_STATS,     Atomic_Enc_Get_Capture_Stats,     false, false, false, true);
}


bool HYB02Broker::cb_CMD_ENC_MATCH(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Match);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Match");

    uint nMatchResult = 0;

    CExecStatus stat = CMD_EXECUTIVE.Execute_Match(object->GetCGTpl1(),    object->GetCGTpl1Size(), object->GetTpl1DigSig(),
                                                   object->GetCGTpl2(),    object->GetCGTpl2Size(), object->GetTpl2DigSig(),
                                                   nMatchResult);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetMatchResult((u32)nMatchResult);
    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_VERIFY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Verify);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Verify");

    // Fill in
    uchar* pOutCG     = nullptr;
    uint   nOutCGSize = 0;
    u256   pOutDigSig;

    CExecStatus stat = CMD_EXECUTIVE.Execute_Verify(object->GetDataBuffer(), object->GetDataBufferSize(), object->GetDigSig(),
                                                    &pOutCG, &nOutCGSize, pOutDigSig);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        pCmd->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetDataBuffer(pOutCG, nOutCGSize);
    object->SetDigSig(pOutDigSig);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;

}


bool HYB02Broker::cb_CMD_ENC_ENROLL(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Enroll);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Enroll");

    _V100_CAPTURE_TYPE nBestInsertion = (_V100_CAPTURE_TYPE)0;

    CExecStatus stat = CMD_EXECUTIVE.Execute_Enroll(nBestInsertion);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        pCmd->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetCaptureType(nBestInsertion);
    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_CAPTURE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Capture);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Capture");

    CExecStatus stat = CMD_EXECUTIVE.Execute_Capture(object->GetCaptureType());

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Clear);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Clear");

    CExecStatus stat = CMD_EXECUTIVE.Execute_Clear();
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_SET_OPTION(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Set_Option);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Set_Option");

    // have to give this a valid value, but it gets changed in 'GetOption' below...
    _V100_OPTION_TYPE Option_To_Set = OPTION_SET_ONE;
    uchar* pOptData     = nullptr;
    uint   nOptDataSize = 0;

    if (false == object->GetOption(Option_To_Set, &pOptData, nOptDataSize))
    {
        object->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }

    CExecStatus stat = CMD_EXECUTIVE.Execute_Set_Option(Option_To_Set, pOptData, nOptDataSize);
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Image);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Image");

    u8*                pInCG        = object->GetCG();
    uint               nInCGSize    = object->GetCGSize();
    u8*                pInDS        = object->GetDigSig();
    _V100_CAPTURE_TYPE capture_type = object->GetCaptureType();

    u8*  pOutCG     = nullptr;
    uint nOutCGSize = 0;
    u256 pOutDS     = {};

    CExecStatus stat = CMD_EXECUTIVE.Execute_Get_Image(capture_type, pInCG, nInCGSize, pInDS, &pOutCG, &nOutCGSize, pOutDS);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetCG(pOutCG, nOutCGSize);
    object->SetDigSig(pOutDS);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Spoof_Score);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Spoof_Score");

    u8*                pInCG        = object->GetCG();
    uint               nInCGSize    = object->GetCGSize();
    u8*                pInDS        = object->GetDigSig();
    _V100_CAPTURE_TYPE capture_type = object->GetCaptureType();

    u8*  pOutCG     = nullptr;
    uint nOutCGSize = 0;
    u256 pOutDS     = {};

    CExecStatus stat = CMD_EXECUTIVE.Execute_Get_Spoof_Score(capture_type, pInCG, nInCGSize, pInDS, &pOutCG, &nOutCGSize, pOutDS);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }
    object->SetCG(pOutCG, nOutCGSize);
    object->SetDigSig(pOutDS);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_ENC_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Template);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Template");

    u8*  pOutCG     = nullptr;
    uint nOutCGSize = 0;
    u256 pDigSig    = {};

    CExecStatus    stat = CMD_EXECUTIVE.Execute_Get_Template(object->GetCaptureType(),    object->GetCG(), object->GetCGSize(), object->GetDigSig(),
                                                          &pOutCG, &nOutCGSize, pDigSig);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetCG(pOutCG, nOutCGSize);
    object->SetDigSig(pDigSig);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Set_Parameters);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Parameters");

    CExecStatus stat = CMD_EXECUTIVE.Execute_Set_Parameters(object->GetCG(), object->GetCGSize(), object->GetDigSig());

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_ENC_GET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Parameters);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Parameters");

    u8*  pOutCG     = nullptr;
    uint nOutCGSize = 0;
    u256 pOutDS     = {};

    CExecStatus stat = CMD_EXECUTIVE.Execute_Get_Parameters(object->GetCG(), object->GetCGSize(), object->GetDigSig(),
                                                            &pOutCG, &nOutCGSize, pOutDS);

    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetCG(pOutCG, nOutCGSize);
    object->SetDigSig(pOutDS);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}

bool HYB02Broker::cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Rnd_Number);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Rnd_Number");

    // Fill in
    short nArg = 0;
    object->GetArguement(nArg);
    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Get_Rnd_Number(object->GetRandomNumber(), nArg))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_GENERATE_SESSIONKEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Generate_SessionKey);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Generate_SessionKey");

    object->SetReturnCode(GEN_NOT_SUPPORTED);// Device generated SK is not supported
    return false;
}


bool HYB02Broker::cb_CMD_ENC_GET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Key");

    short nArg = 0;
    object->GetArguement(nArg);

    _V100_ENC_KEY_TYPE nKeyType    = (_V100_ENC_KEY_TYPE)nArg;
    u32                nKeySize    = 0;
    u16                nKeyVersion = 0;
    _V100_ENC_KEY_MODE nKeyMode;

    u8* pKCV = object->GetKCV();
    u8* pKey = object->GetKey();

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Get_Key(nKeyType, pKey, nKeySize, pKCV, nKeyVersion, nKeyMode);
    if (CExecStatus::CMD_EXEC_OK != nCEErr)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    object->SetKeyParameters(nKeySize, nKeyVersion, nKeyMode);

    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_KeyVersion);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_KeyVersion");

    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)object->GetKeySlot();
    u16                nKeyVer  = 0;
    u8                 pKCV[4]  = {};
    _V100_ENC_KEY_MODE nKeyMode;

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Get_Key_Version(nKeyType, nKeyVer, nKeyMode, pKCV);

    if (nCEErr != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    object->SetKeyVersion(nKeyVer);
    object->SetKCV(pKCV);
    object->SetKeyMode(nKeyMode);

    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_SET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Set_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Key");

    u8*                pOutCG     = nullptr;
    uint               nOutCGSize = 0;
    u8*                pCGKey     = object->GetBuffer();
    uint               nKeySize   = object->GetBufferSize();
    _V100_ENC_KEY_TYPE nKeyType = object->GetKeyType();

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Set_Key(nKeyType, pCGKey, nKeySize, &pOutCG, nOutCGSize);

    if (CExecStatus::CMD_EXEC_OK != nCEErr)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    object->SetBuffer(pOutCG, nOutCGSize);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}



bool HYB02Broker::cb_CMD_ENC_SET_ACTIVE_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Set_Active_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Active_Key");

    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)object->GetKeySlot();

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Set_Active_Key(nKeyType);
    if (CExecStatus::CMD_EXEC_OK != nCEErr)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    RETURN_SUCCESS;
}


bool HYB02Broker::cb_CMD_ENC_GET_CAPTURE_STATS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Capture_Stats);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Capture_Stats");

    _V100_CAPTURE_STATS capture_stats = {};
    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Get_Capture_Stats((_V100_CAPTURE_TYPE)object->GetCaptureType(), &capture_stats);
    if (CExecStatus::CMD_EXEC_OK != nCEErr)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(nCEErr));
        return false;
    }

    object->SetCaptureStats(&capture_stats);
    RETURN_SUCCESS;
}