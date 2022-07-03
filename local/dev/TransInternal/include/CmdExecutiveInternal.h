#pragma once
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_internal_types.h"
#include "CExecStatus.h"
#include "manufacturing_state.h"

class CmdExecutiveInternal : public MemoryBase
{
public:

    CExecStatus     Execute_Set_Image(_V100_IMAGE_TYPE type, uchar* pImage, uint imageSize);
    CExecStatus     Execute_Get_Cal(_V100_INTERFACE_CALIBRATION_TYPE** pCal);
    CExecStatus     Execute_Set_Mfg_State(_V100_MFG_STATE* pMfgState);
    CExecStatus        Execute_Get_EEPROM_M320(_MX00_EEPROM_DATA_M320**);
    CExecStatus     Execute_Process(uchar* pData, uint nDataSize);
    static CmdExecutiveInternal& GetInstance()
    {
        static CmdExecutiveInternal instance;
        return instance;
    }

    CmdExecutiveInternal(const CmdExecutiveInternal&) = delete;
    CmdExecutiveInternal& operator=(const CmdExecutiveInternal&) = delete;
private:
    CmdExecutiveInternal() = default;
    ~CmdExecutiveInternal() = default;
};