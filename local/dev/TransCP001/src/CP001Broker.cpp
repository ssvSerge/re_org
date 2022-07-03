#include "V100EncCmdCP001.h"
#include "CP001Broker.h"
#include "CmdExecutiveCommon.h"
#include "CmdExecutiveCP001.h"
#include "CmdExecutiveDefs.h"
#include "ICmd.h"
#include "V100Cmd.h"
#include "ISensorInstance.h"
#include "IXServiceProvider.h"
#include "ISELogger.h"

using namespace CP001;

#define __CLASSNAME__ CP001Broker
#define CMD_EXECUTIVE CmdExecutiveCP001::GetInstance()
#define CMD_EXECUTIVE_COMMON CmdExecutiveCommon::GetInstance()

CP001Broker::CP001Broker()
{
    REG_EXT_CMD(CMD_ARM_TRIGGER,                Atomic_Arm_Trigger,                        true, true, false, true);
    REG_EXT_CMD(CMD_GET_IMAGE,                    CP001::Atomic_Get_Image,                false, false, false, true);
    REG_EXT_CMD(CMD_GET_COMPOSITE_IMAGE,        Atomic_Get_Composite_Image,                false, false, false, true);
    REG_EXT_CMD(CMD_GET_TEMPLATE,                Atomic_Get_Template,                    false, false, false, true);
    REG_EXT_CMD(CMD_SET_OPTION,                    Atomic_Set_Option,                        false, false, false, true);

    REG_EXT_CMD(CMD_ENC_CLEAR,                    CP001::Atomic_Enc_Clear,                false, false, false, false);
    REG_EXT_CMD(CMD_ENC_GET_SPOOF_SCORE,        CP001::Atomic_Enc_Get_Spoof_Score,        false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_PARAMETERS,            CP001::Atomic_Enc_Set_Parameters,        false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_RND_NUMBER,            CP001::Atomic_Enc_Get_Rnd_Number,        false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GENERATE_SESSIONKEY,    CP001::Atomic_Enc_Generate_SessionKey,    false, false, false, false);
    REG_EXT_CMD(CMD_ENC_GET_KEY,                CP001::Atomic_Enc_Get_Key,                false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_KEYVERSION,            CP001::Atomic_Enc_Get_KeyVersion,        false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_KEY,                CP001::Atomic_Enc_Set_Key,                false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_ACTIVEKEY,            CP001::Atomic_Enc_Set_ActiveKey,        false, false, false, true);
}

bool CP001Broker::cb_CMD_ARM_TRIGGER(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Arm_Trigger);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Arm_Trigger");

    uint nTriggerMode = 0;
    object->GetTriggerType(nTriggerMode);

    CExecStatus stat = CMD_EXECUTIVE_COMMON.Execute_Arm_Trigger(_V100_TRIGGER_MODE(nTriggerMode));
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Get_Image");

    // This command is allowed in most configs, but for CP001 - in order to
    // be consistent with legacy devices - it is not allowed.
    if (false == CheckAppStatus())
    {
        pCmd->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }

    DOWNCAST_TO(CP001::Atomic_Get_Image);

    _V100_IMAGE_TYPE imageType;
    object->GetImageType(imageType);
    uchar* pCG = nullptr;
    uint nCGSize = 0;

    CExecStatus stat = CMD_EXECUTIVE.Execute_Get_Image(imageType, &pCG, &nCGSize);
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(stat));
        return false;
    }

    object->SetImageMetrics(pCG, nCGSize);

    CMD_EXECUTIVE.Release_Buffer(&pCG);

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_GET_COMPOSITE_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_Composite_Image);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Get_Composite_Image");

    uchar* pCG = NULL;
    uint nCGSize = 0;

    CExecStatus status = CMD_EXECUTIVE.Execute_Get_Image(IMAGE_COMPOSITE, &pCG, &nCGSize);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(status));
        return false;
    }

    object->SetImage(pCG, nCGSize);
    int nSpoof = -1;
    object->SetSpoofValue(nSpoof);

    CMD_EXECUTIVE.Release_Buffer(&pCG);

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_Template);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Get_Template");

    u8* pDestTemplate = NULL;
    uint nTemplateSize = 0;

    CExecStatus status = CMD_EXECUTIVE.Execute_Get_Template(&pDestTemplate, &nTemplateSize);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        // Free allocation, set error code
        CMD_EXECUTIVE.Release_Buffer(&pDestTemplate);
        object->SetReturnCode(MarshalCEErrToGENErr(status));
        return false;
    }

    // Marshal it to packet
    object->SetTemplate(pDestTemplate, nTemplateSize);
    CMD_EXECUTIVE.Release_Buffer(&pDestTemplate);

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_SET_OPTION(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Set_Option);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Set_Option");

    _V100_OPTION_TYPE Option_To_Set;
    uchar* pOptData;
    uint              nOptDataSize;

    if (false == object->GetOption(Option_To_Set, &pOptData, nOptDataSize))
    {
        object->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }
    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Set_Option(Option_To_Set, pOptData, nOptDataSize))
    {
        // normally, we would parse the return code from the command exec
        // and translate to an appropriate GEN_ERR_* code.  But the legacy
        // firmware treats all failures the same, so we do that here as well.
        pCmd->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Clear);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Clear");

    CExecStatus stat = CMD_EXECUTIVE.Execute_Enc_Clear();
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}

bool CP001Broker::cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Get_Spoof_Score);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Spoof_Score");

    u128* pANSOL = object->GetANSOL();
    u8* pOutCG = NULL;
    uint nOutCGSize = 0;

    CExecStatus status = CMD_EXECUTIVE.Execute_Enc_Get_Spoof_Score(pANSOL, &pOutCG, &nOutCGSize);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(status));
        return false;
    }

    object->SetSpoofResult(pOutCG, nOutCGSize);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Set_Parameters);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Parameters");

    CExecStatus status = CMD_EXECUTIVE.Execute_Enc_Set_Parameters(
        object->GetSK(),
        object->GetSKSize(),
        object->GetANSOL(),
        object->GetANSOLSize(),
        object->GetCropLevel(),
        object->GetTimeout());

    if (CExecStatus::CMD_EXEC_OK != status)
    {
        // normally, we would parse the return code from the command exec
        // and translate to an appropriate GEN_ERR_* code.  But the legacy
        // firmware treats all failures the same, so we do that here as well.
        object->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Get_Rnd_Number);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Rnd_Number");

    // Fill in
    short nArg = 0;
    object->GetArguement(nArg);

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_Rnd_Number(object->GetRandomNumber(), nArg))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_GENERATE_SESSIONKEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Generate_SessionKey);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Generate_SessionKey");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Generate_SessionKey())
    {
        object->SetReturnCode(GEN_ERROR_CRYPTO_FAIL);
        return false;
    }

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_GET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Get_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Key");

    short nArg = 0;
    object->GetArguement(nArg);
    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)nArg;
    u32 nKeySize = 0;
    u16 nKeyVersion = 0;
    _V100_ENC_KEY_MODE nKeyMode;
    u8* pKCV = object->GetKCV();
    u8* pKey = object->GetKey();

    CExecStatus nCEErr = CMD_EXECUTIVE.Execute_Enc_Get_Key(nKeyType, pKey, nKeySize, pKCV, nKeyVersion, nKeyMode);
    if (CExecStatus::CMD_EXEC_OK != nCEErr)
    {
        object->SetReturnCode(GEN_ERROR_CRYPTO_FAIL);
        return false;
    }

    object->SetKeyParameters(nKeySize, nKeyVersion);

    RETURN_SUCCESS;
}



bool CP001Broker::cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Get_KeyVersion);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_KeyVersion");

    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)object->GetKeySlot();
    u16 nKeyVer = 0;
    u8  pKCV[4] = {};
    _V100_ENC_KEY_MODE nKeyMode;

    CExecStatus status = CMD_EXECUTIVE.Execute_Enc_Get_KeyVersion(nKeyType, nKeyVer, nKeyMode, pKCV);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        pCmd->SetReturnCode(GEN_ERROR_CRYPTO_FAIL);
        return false;
    }

    object->SetKeyVersion(nKeyVer);
    object->SetKCV(pKCV);

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_SET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Set_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Key");

    u8* pCGKey = object->GetKey();
    uint nKeySize = object->GetKeySize();
    _V100_ENC_KEY_TYPE keyType = object->GetKeyType();

    CExecStatus status = CMD_EXECUTIVE.Execute_Enc_Set_Key(keyType, pCGKey, nKeySize);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        object->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }

    RETURN_SUCCESS;
}


bool CP001Broker::cb_CMD_ENC_SET_ACTIVEKEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(CP001::Atomic_Enc_Set_ActiveKey);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_ActiveKey");

    _V100_ENC_KEY_TYPE nKeySlot = (_V100_ENC_KEY_TYPE)object->GetKeySlot();

    CExecStatus status = CMD_EXECUTIVE.Execute_Enc_Set_ActiveKey(nKeySlot);
    if (CExecStatus::CMD_EXEC_OK != status)
    {
        object->SetReturnCode(MarshalCEErrToGENErr(status));
        return false;
    }

    RETURN_SUCCESS;
}
