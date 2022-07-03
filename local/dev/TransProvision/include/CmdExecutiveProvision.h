#pragma once
#include "IMemMgr.h"
#include "CommonIBSP.h"
#include "V100_enc_types.h"
#include "CExecStatus.h"

class CmdExecutiveProvision : MemoryBase
{
public:

    CExecStatus        Execute_Enc_Get_Rnd_Number(u256* pRN);
    CExecStatus        Execute_Enc_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pKeyCG, uint nKeyCGSize, u8** pOutCG, uint& nOutCGSize);
    CExecStatus        Execute_Enc_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV, u2048 pKey, u32& nKeySize);
    CExecStatus        Execute_Enc_Generate_RSA_Keys();
    CExecStatus        Execute_Enc_Unlock_Key(u8* pInCG, uint nInCGSize);
    void            Release_Buffer(u8** pBuffer);


    static CmdExecutiveProvision& GetInstance()
    {
        static CmdExecutiveProvision instance;
        return instance;
    }
    CmdExecutiveProvision(const CmdExecutiveProvision&) = delete;
    CmdExecutiveProvision& operator=(const CmdExecutiveProvision&) = delete;

private:
    CmdExecutiveProvision()
    {
    }
    ~CmdExecutiveProvision()
    {

    }

};