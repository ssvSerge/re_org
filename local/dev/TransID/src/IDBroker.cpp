#include "IDBroker.h"
#include "ICmd.h"
#include "V100IDCmd.h"
#include "CmdExecutiveID.h"
#include "ISELogger.h"
#include "SensorInstance.h"

#define __CLASSNAME__ IDBroker

#define CMD_EXECUTIVE CmdExecutiveID::GetInstance()

#define FILE_AND_LINE std::string(__FILE__) + ":" + std::to_string(__LINE__)
#define LOG_DEBUG(STR_CONTENT) ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, FILE_AND_LINE,STR_CONTENT)



#define CHECK_APP_STATUS_AND_ERROR_OUT  \
    if (CheckAppStatus() == false) \
    { \
        object->SetReturnCode(GEN_ERROR_APP_BUSY); \
        return false; \
    }

IDBroker::IDBroker()
{
    REG_EXT_CMD(CMD_ID_COMMIT,                  Macro_ID_Commit,                    false);
    REG_EXT_CMD(CMD_ID_CREATE_DB,               Macro_ID_Create_DB,                 false);
    REG_EXT_CMD(CMD_ID_ENROLL_USER_RECORD,      Macro_ID_Enroll_User_Record,        false);
    REG_EXT_CMD(CMD_ID_PURGE_DB_ALL,            Macro_ID_Purge_DB_All,              false);
    REG_EXT_CMD(CMD_ID_IDENTIFY_378,            Macro_ID_Identify_378,              false);
    REG_EXT_CMD(CMD_ID_DELETE_DB,               Macro_ID_Delete_DB,                 false);
    REG_EXT_CMD(CMD_ID_IDENTIFY,                Macro_ID_Identify,                  false);
    REG_EXT_CMD(CMD_ID_SET_WORKING_DB,          Macro_ID_Set_Working_DB,            false);
    REG_EXT_CMD(CMD_ID_VERIFY_378,              Macro_ID_Verify_378,                false);
    REG_EXT_CMD(CMD_ID_VERIFY_MANY,             Macro_ID_Verify_Many,               false);
    REG_EXT_CMD(CMD_ID_VERIFY_USER_RECORD,      Macro_ID_Verify_User_Record,        false);
    REG_EXT_CMD(CMD_ID_SET_API_KEY,             Macro_ID_Set_API_Key,               false);
    REG_EXT_CMD(CMD_ID_DELETE_USER_RECORD,      Atomic_ID_Delete_User_Record,       false);
    REG_EXT_CMD(CMD_ID_GET_DB_METRICS,          Atomic_ID_Get_DB_Metrics,           false);
    REG_EXT_CMD(CMD_ID_GET_IMAGE,               Atomic_ID_Get_Image,                false, true, false, false);
    REG_EXT_CMD(CMD_ID_GET_PARAMETERS,          Atomic_ID_Get_Parameters,           false);
    REG_EXT_CMD(CMD_ID_GET_SPOOF_SCORE,         Atomic_ID_Get_Spoof_Score,          false, true, false, false);
    REG_EXT_CMD(CMD_ID_GET_SYSTEM_METRICS,      Atomic_ID_Get_System_Metrics,       false);
    REG_EXT_CMD(CMD_ID_GET_TEMPLATE,            Atomic_ID_Get_Template,             false, true, false, false);
    REG_EXT_CMD(CMD_ID_GET_USER_RECORD,         Atomic_ID_Get_User_Record,          false);
    REG_EXT_CMD(CMD_ID_GET_USER_RECORD_HEADER,  Atomic_ID_Get_User_Record_Header,   false);
    REG_EXT_CMD(CMD_ID_SET_PARAMETERS,          Atomic_ID_Set_Parameters,           false);
    REG_EXT_CMD(CMD_ID_SET_USER_RECORD,         Atomic_ID_Set_User_Record,          false);
}

IDBroker::~IDBroker()
{
}

bool IDBroker::cb_CMD_ID_COMMIT(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Commit");
    DOWNCAST_TO(Macro_ID_Commit);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    if (CreateAndPostMacroMessage(CMD_ID_COMMIT, nullptr, 0, App_Busy_Macro) == false)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_CREATE_DB(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Create_DB");
    DOWNCAST_TO(Macro_ID_Create_DB);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    auto* init_params = object->GetDBInitParms();
    if (!CreateAndPostMacroMessage(CMD_ID_CREATE_DB, init_params, sizeof(_MX00_DB_INIT_STRUCT), App_Busy_Macro))
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_DELETE_USER_RECORD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Delete_User_Record");
    DOWNCAST_TO(Atomic_ID_Delete_User_Record);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    short arg = 0;
    object->GetArguement(arg);

    const auto err = CMD_EXECUTIVE.Execute_Delete_User_Record(object->GetUserRecord(), arg);
    object->SetReturnCode(err);
    return err == GEN_OK ? true : false;
}

bool IDBroker::cb_CMD_ID_ENROLL_USER_RECORD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Enroll_User_Record");
    DOWNCAST_TO(Macro_ID_Enroll_User_Record);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    if (false == CreateAndPostMacroMessage(CMD_ID_ENROLL_USER_RECORD, object->GetUserRecord(),
                                           sizeof(_MX00_ID_USER_RECORD), App_Busy_Macro))
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_GET_DB_METRICS(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_DB_Metrics");
    DOWNCAST_TO(Atomic_ID_Get_DB_Metrics);
    CHECK_APP_STATUS_AND_ERROR_OUT;

    short nArg = 0;
    object->GetArguement(nArg);
    const auto err = CMD_EXECUTIVE.Execute_Get_Db_Metrics(object->GetDBMetrics(), nArg);
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_Image");
    DOWNCAST_TO(Atomic_ID_Get_Image);
    uchar* image = nullptr;
    uint image_size;
    const auto err = CMD_EXECUTIVE.Execute_Get_Image(object->GetCaptureType(), &image, image_size);
    if (err == GEN_OK)
    {
        object->SetImage(image, image_size);
    }
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_GET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_Parameters");
    DOWNCAST_TO(Atomic_ID_Get_Parameters);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    short arg = 0;
    object->GetArguement(arg);
    CMD_EXECUTIVE.Execute_Get_Parameters(object->GetParameters());
    return true;
}

bool IDBroker::cb_CMD_ID_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_Spoof_Score");
    DOWNCAST_TO(Atomic_ID_Get_Spoof_Score);
    int spoof_score = -1;
    const auto err = CMD_EXECUTIVE.Execute_Get_Spoof_Score(object->GetCaptureType(), spoof_score);
    object->SetReturnCode(err);
    if (err == GEN_OK)
    {
        object->SetSpoofScore(spoof_score);
    }
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_GET_SYSTEM_METRICS(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_System_Metrics");
    DOWNCAST_TO(Atomic_ID_Get_System_Metrics);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    _MX00_DB_METRICS* metrics = nullptr;
    uint metrics_sent_back = 0;
    const auto err = CMD_EXECUTIVE.Execute_Get_System_Metrics(&metrics, metrics_sent_back);
    object->SetReturnCode(err);
    object->SetNumMetrics(metrics_sent_back);
    for (uint index = 0; index < metrics_sent_back; index++)
    {
        object->AddMetric(&metrics[index], index);
    }
    delete[] metrics;
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_Template");
    DOWNCAST_TO(Atomic_ID_Get_Template);
    std::vector<uchar> result_template;
    uint result_template_size = 0;
    const auto err = CMD_EXECUTIVE.
        Execute_Get_Template(object->GetCaptureType(), result_template, result_template_size);
    object->SetReturnCode(err);
    if (err == GEN_OK)
    {
        object->SetTemplate(result_template.data(), result_template_size);
    }
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_GET_USER_RECORD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_User_Record");
    DOWNCAST_TO(Atomic_ID_Get_User_Record);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    short index_of_record_to_retrieve = 0;
    uint size_per_tpl = 0;
    uint num_instances = 0;
    object->GetArguement(index_of_record_to_retrieve);
    auto err = CMD_EXECUTIVE.GetNumberOfInstancesPerUserRecord(object->GetUserRecordHeader()->nGroupID, num_instances,
                                                               size_per_tpl);
    if (err != GEN_OK)
    {
        object->SetReturnCode(err);
        return false;
    }
    if (num_instances == 0 || size_per_tpl == 0)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }

    err = CMD_EXECUTIVE.Execute_Get_User_Record(object->GetUserRecordHeader(),
                                                object->GetNewTemplateBuffer(
                                                    num_instances, sizeof(_MX00_TEMPLATE_INSTANCE)),
                                                index_of_record_to_retrieve);
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_GET_USER_RECORD_HEADER(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_User_Record_Header");
    DOWNCAST_TO(Atomic_ID_Get_User_Record_Header);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    short index_of_record_to_get = 0;
    object->GetArguement(index_of_record_to_get);
    const auto err = CMD_EXECUTIVE.
        Execute_Get_User_Record_Header(index_of_record_to_get, object->GetUserRecordHeader());
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_IDENTIFY_378(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Identify_378");
    DOWNCAST_TO(Macro_ID_Identify_378);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    uchar* msg_tpl = nullptr;
    uint msg_tpl_size = 0;
    auto err = CMD_EXECUTIVE.Execute_Adapt_to_378(object->GetTemplate(), object->GetTemplateSize(), &msg_tpl,
                                                  msg_tpl_size);
    object->SetReturnCode(err);
    if (err != GEN_OK)
    {
        return false;
    }
    if (CreateAndPostMacroMessage(CMD_ID_IDENTIFY_378, msg_tpl, msg_tpl_size, App_Busy_Macro) == false)
    {
        err = GEN_ERROR_MEMORY;
        return false;
    }
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_PURGE_DB_ALL(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Purge_DB_All");
    DOWNCAST_TO(Macro_ID_Purge_DB_All);
    ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(
        SEL_DEBUG, __FILE__, "Macro_ID_Purge_DB_All Start");
    CHECK_APP_STATUS_AND_ERROR_OUT;
    if (CreateAndPostMacroMessage(CMD_ID_PURGE_DB_ALL, nullptr, 0, App_Busy_Macro) == false)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_SET_API_KEY(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Set_API_Key");
    DOWNCAST_TO(Macro_ID_Set_API_Key);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    if (CreateAndPostMacroMessage(CMD_ID_SET_API_KEY, (void*)object->GetAPIKey(), object->GetAPIKeyLength(),
                                  App_Busy_Macro) == false)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_DELETE_DB(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Delete_DB");
    DOWNCAST_TO(Macro_ID_Delete_DB);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    uint group_to_del = object->GetDBToDelete();
    short arg = 0;
    object->GetArguement(arg);
    if (CreateAndPostMacroMessage(CMD_ID_DELETE_DB, static_cast<void*>(&group_to_del), sizeof(uint), App_Busy_Macro,
                                  arg) == false)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_IDENTIFY(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Identify");
    DOWNCAST_TO(Macro_ID_Identify);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    if (false == CreateAndPostMacroMessage(CMD_ID_IDENTIFY, nullptr, /*NULL*/ 0, App_Busy_Macro))
    {
        pCmd->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    return false;
}

bool IDBroker::cb_CMD_ID_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Set_Parameters");
    DOWNCAST_TO(Atomic_ID_Set_Parameters);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    const auto err = CMD_EXECUTIVE.Execute_Set_Parameters(object->GetParameters());
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_SET_USER_RECORD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Set_User_Record");
    DOWNCAST_TO(Atomic_ID_Set_User_Record);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    const auto err = CMD_EXECUTIVE.Execute_Set_User_Record(object->GetUserRecordHeader(), object->GetFingerInstance(0));
    object->SetReturnCode(err);
    return err == GEN_OK;
}

bool IDBroker::cb_CMD_ID_SET_WORKING_DB(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Set_Working_DB");
    DOWNCAST_TO(Macro_ID_Set_Working_DB);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    uint group_id_to_set = object->GetGroupID();
    if (CreateAndPostMacroMessage(CMD_ID_SET_WORKING_DB, &group_id_to_set, sizeof(uint), App_Busy_Macro) == false)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_VERIFY_378(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Verify_378");
    DOWNCAST_TO(Macro_ID_Verify_378);
    if (CheckAppStatus() == false)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    short nArg = -1;
    object->GetArguement(nArg);
    if (nArg == 0)
    {
        object->GetUserRecord()->nFinger = 0xFFFFFFFF;
    }
    auto err = CMD_EXECUTIVE.Execute_ID_Verify_378(object->GetTemplate(), object->GetTemplateSize(),
                                                   object->GetUserRecord());
    if (err == CExecStatus::CMD_ERR_BAD_DATA)
    {
        object->SetReturnCode(GEN_VER_INVALID_RECORD_FORMAT);
        return false;
    }
    if (err == CExecStatus::CMD_ERR_OUT_OF_MEMORY)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_VERIFY_MANY(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Verify_Many");
    DOWNCAST_TO(Macro_ID_Verify_Many);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    const uint num_templates = object->GetNumTemplates();
    uchar* templates = object->GetTemplates();
    uint* size_templates = object->GetTemplatesSz();
    const uint size_all_templates = object->GetAllTemplatesSz();
    uint total_size = sizeof(uint) + sizeof(uint) * num_templates + size_all_templates;
    uchar* buffer = new uchar[total_size];
    uchar* tmp_ptr = buffer;
    memcpy(tmp_ptr, &num_templates, sizeof(uint));
    tmp_ptr += sizeof(uint);
    memcpy(tmp_ptr, size_templates, sizeof(uint) * num_templates);
    tmp_ptr += sizeof(uint) * num_templates;
    memcpy(tmp_ptr, templates, size_all_templates);
    const auto err = CMD_EXECUTIVE.Execute_Verify_Many(buffer, total_size);
    delete[] buffer;
    buffer = nullptr;
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_MEMORY);
        return false;
    }
    RETURN_SUCCESS;
}

bool IDBroker::cb_CMD_ID_VERIFY_USER_RECORD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_ID_Verify_User_Record");
    DOWNCAST_TO(Macro_ID_Verify_User_Record);
    CHECK_APP_STATUS_AND_ERROR_OUT;
    short arg = -1;
    object->GetArguement(arg);
    if (arg == 0)
    {
        object->GetUserRecord()->nFinger = 0xFFFFFFFF;
    }
    if (CreateAndPostMacroMessage(CMD_ID_VERIFY_USER_RECORD, static_cast<void*>(object->GetUserRecord()),
                                  sizeof(_MX00_ID_USER_RECORD), App_Busy_Macro) == false)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    RETURN_SUCCESS;
}
