#include "CmdExecutiveProvision.h"
#include "CryptoMgrProvision.h"
#include "CriticalErrorLog.h"
#include "CmdExecutiveBase.h"
#include "V100_internal_types.h"
#include "CfgMgr.h"

#define CRYPTO_MGR CryptoMgrProvision::GetInstance()

CExecStatus        CmdExecutiveProvision::Execute_Enc_Get_Rnd_Number(u256* pRN)
{

    memset(pRN, 0, sizeof(u256));
    if (false == CRYPTO_MGR.GetRandomNumber(pRN))
    {
        LOGMSG("Execute_Enc_Get_Rnd_Number:Getting random number returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}
CExecStatus        CmdExecutiveProvision::Execute_Enc_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pKeyCG, uint nKeyCGSize, u8** pOutCG, uint& nOutCGSize)
{
    if (false == CRYPTO_MGR.SetKey(nKeyType, pKeyCG, nKeyCGSize, pOutCG, nOutCGSize))
    {
        LOGMSG("Execute_Enc_Set_Key:Setting Key returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}
CExecStatus        CmdExecutiveProvision::Execute_Enc_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV, u2048 pKey, u32& nKeySize)
{
    if (false == CRYPTO_MGR.GetKey(nKeyType, nKeyVer, nKeyMode, pKCV, pKey, nKeySize))
    {
        LOGMSG("Execute_Enc_Get_Key:Getting Key returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}
CExecStatus        CmdExecutiveProvision::Execute_Enc_Generate_RSA_Keys()
{
    _V100_FIRMWARE_CONFIG fwconfig = ISensorInstance::GetInstance()->GetCfgMgr()->GetCurrentCfg();
    if (CmdExecutiveBase::CreateAndPostMacroMessage(CMD_ENC_GENERATE_RSA_KEYS, nullptr,0, App_Busy_Macro, (short)fwconfig) == false)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    return CExecStatus::CMD_EXEC_OK;
}
CExecStatus        CmdExecutiveProvision::Execute_Enc_Unlock_Key(u8* pInCG, uint nInCGSize)
{
    if (false == CRYPTO_MGR.Decommission(pInCG, nInCGSize))
    {
        LOGMSG("Execute_Enc_Unlock_Key:Decommissioning returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}
void CmdExecutiveProvision::Release_Buffer(u8** pBuffer)
{
    if (pBuffer && *pBuffer)
    {
        FREE(*pBuffer);
        *pBuffer = nullptr;
    }

    //CRYPTO_MGR.ReleaseSEBuff(pBuffer);
}