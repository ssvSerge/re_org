#include "V100EncCmdProvision.h"
#include "ProvisionBroker.h"
#include "CmdExecutiveProvision.h"
#include "ISELogger.h"
using namespace Provision;

#define __CLASSNAME__ ProvisionBroker
#define CMD_EXECUTIVE CmdExecutiveProvision::GetInstance()

ProvisionBroker::ProvisionBroker()
{
    REG_EXT_CMD(CMD_ENC_GET_RND_NUMBER, Atomic_Enc_Get_Rnd_Number, false, false, false, true);
    REG_EXT_CMD(CMD_ENC_SET_KEY, Atomic_Enc_Set_Key, false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GET_KEY, Atomic_Enc_Get_Key, false, false, false, true);
    REG_EXT_CMD(CMD_ENC_GENERATE_RSA_KEYS, Macro_Enc_Generate_RSA_Keys, false, false, false, false);
    REG_EXT_CMD(CMD_ENC_UNLOCK_KEY, Atomic_Enc_Unlock_Key, false, false,false, true);
}

bool ProvisionBroker::cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Rnd_Number);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Rnd_Number");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_Rnd_Number(object->GetRandomNumber()))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    RETURN_SUCCESS;
}

bool ProvisionBroker::cb_CMD_ENC_SET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Set_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Set_Key");

    u8* pKeyCG = object->GetBuffer();
    uint nKeyCGSize = object->GetBufferSize();
    _V100_ENC_KEY_TYPE nKeyType = object->GetKeyType();
    u8* pOutCG = nullptr;
    uint nOutCGSize = 0;

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Set_Key(nKeyType, pKeyCG, nKeyCGSize, &pOutCG, nOutCGSize))
    {
        pCmd->SetReturnCode(GEN_DECRYPTION_FAIL);
        return false;
    }
    object->SetBuffer(pOutCG, nOutCGSize);
    CMD_EXECUTIVE.Release_Buffer(&pOutCG);
    RETURN_SUCCESS;
}

bool ProvisionBroker::cb_CMD_ENC_GET_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Get_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Get_Key");

    short nArg = 0;
    object->GetArguement(nArg);
    _V100_ENC_KEY_TYPE nKeyType = (_V100_ENC_KEY_TYPE)nArg;
    u32 nKeySize = 0;
    u16 nKeyVer = 0;
    u8* pKCV = object->GetKCV();
    u8* pKey = object->GetKey();
    _V100_ENC_KEY_MODE nKeyMode;


    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Get_Key(nKeyType, nKeyVer, nKeyMode, pKCV, pKey, nKeySize))
    {
        pCmd->SetReturnCode(GEN_ERROR_CRYPTO_FAIL);
        return false;
    }

    object->SetKeyParameters(nKeySize, nKeyVer, nKeyMode);
    RETURN_SUCCESS;
}

bool ProvisionBroker::cb_CMD_ENC_GENERATE_RSA_KEYS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Macro_Enc_Generate_RSA_Keys);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Macro_Enc_Generate_RSA_Keys");

    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_Enc_Generate_RSA_Keys())
    {
        pCmd->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    RETURN_SUCCESS;
}

bool ProvisionBroker::cb_CMD_ENC_UNLOCK_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Enc_Unlock_Key);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Atomic_Enc_Unlock_Key");

    const auto err = CMD_EXECUTIVE.Execute_Enc_Unlock_Key(object->GetCryptogram(), object->GetCryptogramSize());
    if (CExecStatus::CMD_EXEC_OK != err)
    {
        RETURN_MARSHAL_FAILURE;
    }
    RETURN_SUCCESS;
}