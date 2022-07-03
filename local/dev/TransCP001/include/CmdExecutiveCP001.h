#pragma once

#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_internal_types.h"
#include "V100_enc_types.h"
#include "CmdExecutiveDefs.h"

class CmdExecutiveCP001 : MemoryBase
{
public:
    CExecStatus    Execute_Get_Image(_V100_IMAGE_TYPE type, uchar** pOutCG, uint* nOutCGSize);
    CExecStatus    Execute_Get_Template(uchar** pRecord, uint* nRecordSize);
    CExecStatus    Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize);

    CExecStatus    Execute_Enc_Clear();
    CExecStatus    Execute_Enc_Get_Spoof_Score(u128* pANSOL, u8** pOutCG, uint* nOutCGSize);
    CExecStatus    Execute_Enc_Set_Parameters(u8* iSPLCG, int nSPLCGSize, u8* ANSOL, int ANSOLSize, _V100_CROP_LEVEL nCropLevel, int nTimeout);
    CExecStatus    Execute_Enc_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk);
    CExecStatus    Execute_Enc_Generate_SessionKey();
    CExecStatus    Execute_Enc_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u32& nKeySize, u8* pKCV, u16& nVerNum, _V100_ENC_KEY_MODE& nKeyMode);
    CExecStatus    Execute_Enc_Get_KeyVersion(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV);
    CExecStatus    Execute_Enc_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGKeySize);
    CExecStatus    Execute_Enc_Set_ActiveKey(_V100_ENC_KEY_TYPE nKeyType);
    void           Release_Buffer(u8** pBuffer);

    static CmdExecutiveCP001& GetInstance() {
        static CmdExecutiveCP001 instance;
        return instance;
    }
    CmdExecutiveCP001(const CmdExecutiveCP001&) = delete;
    CmdExecutiveCP001& operator=(const CmdExecutiveCP001&) = delete;

private:
    CmdExecutiveCP001() {
    }

    ~CmdExecutiveCP001() {
    }

};



