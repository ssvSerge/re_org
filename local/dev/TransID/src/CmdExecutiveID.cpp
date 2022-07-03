#include "CmdExecutiveID.h"

#include <string>

#include "ApplicationStates.h"
#include "BaseThread.h"
#include "BioParameters.h"
#include "CmdExecutiveBase.h"
#include "DataMgr.h"
#include "IdentificationMgr.h"
#include "ISELogger.h"
#include "ISensorInstance.h"
#include "TAdapter.h"

#define FILE_AND_LINE std::string(__FILE__) + ":" + std::to_string(__LINE__)
#define LOG_DEBUG(STR_CONTENT) ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, FILE_AND_LINE,STR_CONTENT)


_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Delete_User_Record(_MX00_ID_USER_RECORD record_to_delete, bool delete_all)
{
    return ISensorInstance::GetInstance()->GetIdentificationMgr()->DeleteUserFingerRecord(record_to_delete, delete_all);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_Db_Metrics(_MX00_DB_METRICS* data_to_get, bool get_current)
{
    return ISensorInstance::GetInstance()->GetIdentificationMgr()->GetDBMetrics(data_to_get, get_current);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_Image(_V100_CAPTURE_TYPE capture_type, uchar** out_image,
    uint &out_image_size)
{
    auto instance = ISensorInstance::GetInstance();
    auto data_mgr = instance->GetDataMgr();
    _V100_INTERFACE_CONFIGURATION_TYPE* pICT = data_mgr->GetInterfaceConfiguration();
    out_image_size = pICT->Composite_Image_Size_X * pICT->Composite_Image_Size_Y;
    *out_image = new uchar[out_image_size]{};
    if (*out_image == nullptr)
    {
        LOG_DEBUG("ID_Get_Image Error: Memory");
        return GEN_ERROR_MEMORY;
    }

    if (false == data_mgr->GetCapturedImageData(capture_type, *out_image, out_image_size))
    {
        if (*out_image)
        {
            delete[] *out_image; *out_image = nullptr;
        }
        LOG_DEBUG("ID_Get_Image Error: data_mgr->GetCapturedImageData() returned false");
        return GEN_ERROR_PARAMETER;
    }

    if (out_image_size == 0)
    {
        delete[] *out_image; *out_image = nullptr;
        LOG_DEBUG("ID_Get_Image Error: out_image_size = 0");
        return GEN_ERROR_DATA_UNAVAILABLE;
    }
    return GEN_OK;
}

void CmdExecutiveID::Execute_Get_Parameters(_MX00_ID_PARAMETERS* params)
{
    ISensorInstance::GetInstance()->GetIdentificationMgr()->GetIDParameters(params);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_Template(_V100_CAPTURE_TYPE capture_type, std::vector<uchar>& dest_tpl, uint& tpl_size)
{
    auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    auto *tpl = new uchar[MAX_TEMPLATE_SIZE_CONST]{};
    uint raw_tpl_size = MAX_TEMPLATE_SIZE_CONST;
    if (tpl == nullptr)
        return GEN_ERROR_MEMORY;
    if (data_mgr->GetCapturedTemplateData(capture_type, tpl, raw_tpl_size) == false)
    {
        delete[] tpl;
        tpl = nullptr;
        LOG_DEBUG("ID_Get_Template Error: GetCapturedTemplateData() returned false");
        return GEN_ERROR_PARAMETER;
    }
    if (raw_tpl_size == 0)
    {
        delete[] tpl;
        tpl = nullptr;
        LOG_DEBUG("ID_Get_Template Error: raw_tpl_size = 0");
        return GEN_ERROR_DATA_UNAVAILABLE;
    }
    TAdapter adapter;
    uchar* result_arr = nullptr;
    if (adapter.ConvertFrom378(BioParameters::GetInstance().GetTemplateMode(), tpl, raw_tpl_size, &result_arr, &tpl_size) == false)
    {
        delete[] tpl;
        tpl = nullptr;
        LOG_DEBUG("ID_Get_Template Error: TAdapter::ConvertFrom378() returned false");
        return GEN_VER_INVALID_RECORD_FORMAT;
    }
    dest_tpl.resize(tpl_size);
    memcpy(dest_tpl.data(), result_arr, tpl_size);
    return GEN_OK;
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_User_Record(_MX00_ID_USER_RECORD* user_record_header,
    _MX00_TEMPLATE_INSTANCE* buffer, short index_of_record_to_retrieve)
{
    auto id_mgr = ISensorInstance::GetInstance()->GetIdentificationMgr();
    if (index_of_record_to_retrieve == -1)
        return id_mgr->GetUserRecord(user_record_header, buffer);
    else
    {
        ushort nIndex = (ushort)index_of_record_to_retrieve;//First convert to ushort then convert to uint.. this is needed so users can pull indices 32767(short max value)
        return id_mgr->GetUserRecordByIndex((uint)nIndex, user_record_header, buffer);
    }
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::GetNumberOfInstancesPerUserRecord(uint group_id, uint& num_instances,
                                                                            uint& size_per_template)
{
        return ISensorInstance::GetInstance()->GetIdentificationMgr()->GetNumberOfInstancesPerUserRecord(group_id, num_instances, size_per_template);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_User_Record_Header(short index_of_record_to_get,
    _MX00_ID_USER_RECORD* header_to_change)
{
    ushort nIndex = (ushort)index_of_record_to_get;//First convert to ushort then convert to uint.. this is needed so users can pull indices 32767(short max value)
    return ISensorInstance::GetInstance()->GetIdentificationMgr()->GetUserRecordByIndex((uint)nIndex, header_to_change, nullptr);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Adapt_to_378(uchar* tpl, uint tpl_size, uchar** out_tpl, uint& out_tpl_size)
{
    TAdapter adapter;
    if (adapter.AdaptTo378(BioParameters::GetInstance().GetTemplateMode(), tpl, tpl_size, out_tpl, &out_tpl_size) == false)
    {
        return GEN_VER_INVALID_RECORD_FORMAT;
    }
    return GEN_OK;
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Set_Parameters(_MX00_ID_PARAMETERS* params)
{
    return ISensorInstance::GetInstance()->GetIdentificationMgr()->SetIDParameters(params);
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Set_User_Record(_MX00_ID_USER_RECORD* header,
    _MX00_TEMPLATE_INSTANCE* finger_instance)
{
    auto* pTM = new ThreadMessage();
    pTM->CreateThreadMessage(CMD_ID_SET_USER_RECORD, nullptr, 0);

    const auto err = ISensorInstance::GetInstance()->GetIdentificationMgr()->AddUserFingerRecord(pTM, header, finger_instance);
    delete pTM;
    pTM = nullptr;
    return err;
}

CExecStatus CmdExecutiveID::Execute_Verify_Many(uchar* buffer, uint total_size)
{
    if (CmdExecutiveBase::CreateAndPostMacroMessage(CMD_ID_VERIFY_MANY, buffer, total_size, App_Busy_Macro) == false)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveID::Execute_ID_Verify_378(u8* pTemplate, uint nTemplateSize,
                                                  _MX00_ID_USER_RECORD* pUserRecord)
{
    DataMgr* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    TAdapter adapter;
    uchar* dest_tpl = nullptr;
    uint dest_tpl_size = 0;
    if (adapter.AdaptTo378(BioParameters::GetInstance().GetTemplateMode(), pTemplate, nTemplateSize, &dest_tpl, &dest_tpl_size) == false)
    {
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    uint total_size = sizeof(_MX00_ID_USER_RECORD) + sizeof(uint) + dest_tpl_size;
    uchar* buffer = new uchar[total_size]{};
    uint offset = 0;
    memcpy(buffer, pUserRecord, sizeof(_MX00_ID_USER_RECORD));
    offset += sizeof(_MX00_ID_USER_RECORD);
    memcpy(buffer + offset, &dest_tpl_size, sizeof(uint));
    offset += sizeof(uint);
    memcpy(buffer + offset, dest_tpl, dest_tpl_size);
    if (CmdExecutiveBase::GetInstance().CreateAndPostMacroMessage(CMD_ID_VERIFY_378, buffer, total_size, App_Busy_Macro) == false)
    {
        delete[] buffer; buffer = nullptr;
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    delete[] buffer; buffer = nullptr;
    return CExecStatus::CMD_EXEC_OK;
}



_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_Spoof_Score(_V100_CAPTURE_TYPE capture_type, int& spoof_score)
{
    const auto result = ISensorInstance::GetInstance()->GetDataMgr()->GetCapturedSpoofScore(capture_type, spoof_score);
    if (!result)
    {
        LOG_DEBUG("ID_Get_Spoof_Score Error: GetCapturedSpoofScore() returned false!");
        return GEN_ERROR_PARAMETER;
    }
    if (spoof_score == -1)
    {
        LOG_DEBUG("ID_Get_Spoof_Score Error: Data Not Available!");
        return GEN_ERROR_DATA_UNAVAILABLE;
    }
    return GEN_OK;
}

_V100_GENERAL_ERROR_CODES CmdExecutiveID::Execute_Get_System_Metrics(_MX00_DB_METRICS** db_metrics,
    uint& metrics_sent_back)
{
    return ISensorInstance::GetInstance()->GetIdentificationMgr()->GetSystemMetrics(db_metrics, metrics_sent_back);
}


