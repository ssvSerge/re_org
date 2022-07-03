#include "InternalBroker.h"

#include "V100Cmd.h"
#include "V100InternalCmd.h"

#include "CfgMgr.h"
#include "CmdExecutiveCommon.h"
#include "CmdExecutiveInternal.h"
#include "DataMgr.h"
#include "ISELogger.h"
#include "ISensorInstance.h"

#define __CLASSNAME__  InternalBroker
#define CMD_EXECUTIVE CmdExecutiveInternal::GetInstance()


InternalBroker::InternalBroker()
{
    REG_EXT_CMD(CMD_SET_IMAGE, Atomic_Set_Image, false);
    REG_INT_CMD(CMD_GET_CAL, Atomic_Get_Cal, false);
    REG_INT_CMD(CMD_GET_LICENSE_KEY, Atomic_Get_License_Key, false);
    REG_INT_CMD(CMD_GET_SPOOF_DETAILS_V2, Atomic_Get_Spoof_Details_V2, false);
    REG_INT_CMD(CMD_PROCESS, Atomic_Process, false);
    REG_EXT_CMD(CMD_SET_COMPOSITE_IMAGE, Atomic_Set_Composite_Image, false);//Not supported but implemented in UFW. TODO?
    REG_EXT_CMD(CMD_SET_LICENSE_KEY, Atomic_Set_License_Key, false);
    REG_INT_CMD(CMD_SET_MFG_STATE, Atomic_Set_Mfg_State, false, false, true, false);
    REG_INT_CMD(CMD_GET_EEPROM_M320, Atomic_Get_EEPROM_M320, false);
}

bool InternalBroker::cb_CMD_PROCESS(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Process);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Atomic_Process Start");
    if (CmdExecutiveCommon::GetInstance().Execute_Perform_Process(object->GetData(), object->GetDataSize())  != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Atomic_Process End");
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_SET_COMPOSITE_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    pCmd->SetReturnCode(GEN_NOT_SUPPORTED);
    return false;
}

bool InternalBroker::cb_CMD_SET_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Set_Image);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Atomic_Set_Image Start");
    short image_type;
    object->GetArguement(image_type);
    if (CMD_EXECUTIVE.Execute_Set_Image(static_cast<_V100_IMAGE_TYPE>(image_type), object->GetImage(), object->GetImageSize()) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
    }
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Atomic_Set_Image End");
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_GET_CAL(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_Cal);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Get_Cal Start");
    auto* cal_type = ISensorInstance::GetInstance()->GetDataMgr()->GetCAL();
    object->SetCal(reinterpret_cast<uchar*>(cal_type), sizeof (_V100_INTERFACE_CALIBRATION_TYPE));
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Get_Cal End");
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_GET_SPOOF_DETAILS_V2(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_Spoof_Details_V2);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Get_Spoof_Details_V2 Start");
    if (CheckAppStatus() == false)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    object->SetSpoofMetrics(ISensorInstance::GetInstance()->GetDataMgr()->GetSpoofMetrics());
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, "Get_Spoof_Details_V2 End");
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_GET_LICENSE_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_License_Key);
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_SET_LICENSE_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Set_License_Key);
    RETURN_SUCCESS;
}

bool InternalBroker::cb_CMD_SET_MFG_STATE(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool InternalBroker::cb_CMD_GET_EEPROM_M320(const std::shared_ptr<ICmd>& pCmd)
{
    DOWNCAST_TO(Atomic_Get_EEPROM_M320);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, std::string("Atomic_Get_EEPROM_M320 Start"));
    if (CheckAppStatus() == false)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    _MX00_EEPROM_DATA_M320* data = nullptr;
    auto err = CmdExecutiveCommon::GetInstance().Execute_Get_EEPROM_M320(&data);
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    if (data == nullptr)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    object->SetEEPROM(data);
    RETURN_SUCCESS;
}
