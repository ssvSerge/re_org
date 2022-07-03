#include "CommonBroker.h"
#include "CmdExecutiveCommon.h"

#include "CfgMgr.h"
#include "CmdExecutiveBase.h"
#include "DataMgr.h"
#include "TAdapter.h"
#include "V100Cmd.h"
#include "V100IDCmd.h"
#include "ISELogger.h"

class ICmd;

#define __CLASSNAME__  CommonBroker
#define CMD_EXECUTIVE CmdExecutiveCommon::GetInstance()

#define FILE_AND_LINE std::string(__FILE__) + ":" + std::to_string(__LINE__)
#define LOG_DEBUG(STR_CONTENT) ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, FILE_AND_LINE,STR_CONTENT)


// For any other macro definition, please see BaseTransBroker.h.

CommonBroker::CommonBroker()
{
    REG_EXT_CMD(CMD_MATCH,                      Macro_Match,                false);
    REG_EXT_CMD(CMD_VID_STREAM,                 Macro_Vid_Stream,           false, true,  false,  false);
    REG_EXT_CMD(CMD_VERIFY_378,                 Macro_Verify_378,           false);
    REG_EXT_CMD(CMD_GET_IMAGE,                  Atomic_Get_Image,           false, true,false, false);
    REG_EXT_CMD(CMD_GET_COMPOSITE_IMAGE,        Atomic_Get_Composite_Image, false);
    REG_EXT_CMD(CMD_GET_TEMPLATE,               Atomic_Get_Template,        false);
    REG_EXT_CMD(CMD_SET_TEMPLATE,               Atomic_Set_Template,        false);
    REG_EXT_CMD(CMD_ARM_TRIGGER,                Atomic_Arm_Trigger,         true,  true,  false, false);
    REG_EXT_CMD(CMD_SET_CMD,                    Atomic_Set_Cmd,             false, false, true,  false);
    REG_EXT_CMD(CMD_SET_LED,                    Atomic_Set_LED,             false, true,  false, false);
    REG_EXT_CMD(CMD_CONFIG_COMPORT,             Atomic_Config_Comport,      false);
    REG_EXT_CMD(CMD_TRUNCATE_378,               Atomic_Truncate_378,        false);
    REG_EXT_CMD(CMD_MATCH_EX,                   Atomic_Match_Ex,            false);
    REG_EXT_CMD(CMD_SET_GPIO,                   Atomic_Set_GPIO,            false);
    REG_EXT_CMD(CMD_GET_GPIO,                   Atomic_Get_GPIO,            false);
    REG_EXT_CMD(CMD_GET_FIR_IMAGE,              Atomic_Get_FIR_Image,       false);
    REG_EXT_CMD(CMD_SET_OPTION,                 Atomic_Set_Option,          false, true, false, false);
    REG_EXT_CMD(CMD_GET_DB_METRICS,             Atomic_Get_DB_Metrics,      false);
    REG_EXT_CMD(CMD_ENC_CLEAR,                  Atomic_Clear,               false);
    REG_EXT_CMD(CMD_ID_GET_RESULT,              Atomic_ID_Get_Result,       false );
}

CommonBroker::~CommonBroker()
{
}

bool CommonBroker::cb_CMD_MATCH(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_Match");
    DOWNCAST_TO(Macro_Match);
    uint nScore = 0;

    auto err = CMD_EXECUTIVE.Execute_Match(object->GetProbeTemplate(), object->GetProbeTemplateSize(),
                                           object->GetGalleryTemplate(), object->GetGalleryTemplateSize(), nScore);
    if (err == CExecStatus::CMD_ERR_BAD_DATA)
    {
        pCmd->SetReturnCode(GEN_VER_INVALID_RECORD_FORMAT);
        return false;
    }
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        pCmd->SetReturnCode(GEN_ERROR_MATCH);
        return false;
    }
    object->SetMatchScore(nScore);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_VID_STREAM(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_Vid_Stream");
    DOWNCAST_TO(Macro_Vid_Stream);
    _V100_VID_STREAM_MODE _vidStream;
    object->GetVidStreamMode(_vidStream);
    if (CExecStatus::CMD_EXEC_OK != CMD_EXECUTIVE.Execute_VID_Stream(_vidStream))
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_Image");
    DOWNCAST_TO(Atomic_Get_Image);
    uchar* image = nullptr;
    uint image_size = 0;
    _V100_IMAGE_TYPE image_type{};
    short index = 0;

    object->GetImageType(image_type);
    object->GetArguement(index);
    CExecStatus RC = CMD_EXECUTIVE.Execute_Get_Image(image_type, &image, image_size, index);
    if (RC != CExecStatus::CMD_EXEC_OK)
    {
        switch (RC) {
        case CExecStatus::CMD_ERR_BAD_PARAMETER:
            object->SetReturnCode(GEN_ERROR_PARAMETER);
            break;
        case CExecStatus::CMD_ERR_NOT_SUPPORTED:
            object->SetReturnCode(GEN_NOT_SUPPORTED);
            break;
        case CExecStatus::CMD_ERR_DATA_UNAVAILABLE:
            object->SetReturnCode(GEN_ERROR_DATA_UNAVAILABLE);
            break;
        default:
            object->SetReturnCode(GEN_ERROR_APP_BUSY);
            break;
        }
        return false;
    }

    object->SetImageMetrics(image, image_size);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_COMPOSITE_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_Composite_Image");
    DOWNCAST_TO(Atomic_Get_Composite_Image);

    uint image_size = 0;
    uchar* CIP = nullptr;
    int spoof = 0;
    if (CMD_EXECUTIVE.Execute_Get_Image(IMAGE_COMPOSITE, &CIP, image_size, 0) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    if (CMD_EXECUTIVE.Execute_Get_Spoof_Score(spoof) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    object->SetImage(CIP, image_size);
    object->SetSpoofValue(spoof);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_Template");
    DOWNCAST_TO(Atomic_Get_Template);
    u8* dest_template = nullptr;
    uint template_size = 0;
    const auto err = CMD_EXECUTIVE.Execute_Get_Template(&dest_template, template_size);
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    object->SetTemplate(dest_template, template_size);
    FREE(dest_template);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_SET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Set_Template");
    DOWNCAST_TO(Atomic_Set_Template);
    uint template_size = 0;
    auto* const my_template = object->GetTemplate(template_size);
    const auto err = CMD_EXECUTIVE.Execute_Set_Template(my_template, template_size);
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_ARM_TRIGGER(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Arm_Trigger");
    DOWNCAST_TO(Atomic_Arm_Trigger);
    uint trigger_mode = 0;
    object->GetTriggerType(trigger_mode);
    const auto err = CMD_EXECUTIVE.Execute_Arm_Trigger(static_cast<_V100_TRIGGER_MODE>(trigger_mode));
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_SET_CMD(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Set_Cmd");
    DOWNCAST_TO(Atomic_Set_Cmd);
    if (CMD_EXECUTIVE.Execute_Set_Cmd(object->GetCmd()) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_SET_LED(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Set_LED");
    DOWNCAST_TO(Atomic_Set_LED);
    if (CMD_EXECUTIVE.Execute_Set_LED(object->GetLEDControl()) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_NOT_SUPPORTED);
        return false;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_CONFIG_COMPORT(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Config_Comport");
    DOWNCAST_TO(Atomic_Config_Comport);
    uint baud_rate = 0;
    object->GetBaudRate(baud_rate);
    if (CMD_EXECUTIVE.Execute_Config_COM_Port(baud_rate) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_NOT_SUPPORTED);
        return false;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_TRUNCATE_378(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Truncate_378");
    DOWNCAST_TO(Atomic_Truncate_378);
    uchar* template_in = new uchar[4096];
    uchar* template_out = new uchar[4096];

    uint template_size_in = 0, template_size_out = 0, template_size_requested = 0;

    object->GetTemplate(template_in, template_size_in);
    object->GetMaxTemplateSize(template_size_requested);

    const auto err = CMD_EXECUTIVE.Execute_Truncate_378(template_size_requested, template_in, template_size_in,
                                                        template_out, template_size_out);
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        delete[] template_in;
        delete[] template_out;
        RETURN_MARSHAL_FAILURE;
    }
    object->SetTemplate(template_out, template_size_out);
    delete[] template_in;
    delete[] template_out;
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_MATCH_EX(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Match_Ex");
    DOWNCAST_TO(Atomic_Match_Ex);
    uint score = 0;
    const auto err = CMD_EXECUTIVE.Execute_Match(object->GetProbeTemplate(), object->GetProbeTemplateSize(),
                                                 object->GetGalleryTemplate(), object->GetGalleryTemplateSize(), score);
    if (err == CExecStatus::CMD_ERR_BAD_DATA)
    {
        object->SetReturnCode(GEN_VER_INVALID_RECORD_FORMAT);
        return false;
    }
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_MATCH);
        return false;
    }
    object->SetMatchScore(score);
    object->SetSpoofScore(-1);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_SET_GPIO(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Set_GPIO");
    DOWNCAST_TO(Atomic_Set_GPIO);
    const auto err = CMD_EXECUTIVE.Execute_Set_GPIO(object->GetMask());
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_GPIO(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_GPIO");
    DOWNCAST_TO(Atomic_Get_GPIO);
    uchar GPIO_mask = 0;
    const auto err = CMD_EXECUTIVE.Execute_Get_GPIO(&GPIO_mask);
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        RETURN_MARSHAL_FAILURE;
    }
    object->SetMask(GPIO_mask);
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_FIR_IMAGE(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_FIR_Image");
    DOWNCAST_TO(Atomic_Get_FIR_Image);
    uint FIR_image_size = 0;
    uchar* FIR_image = nullptr;
    if (CMD_EXECUTIVE.Execute_Get_FIR_Image(object->GetFIRType(), object->GetFingerType(), &FIR_image, &FIR_image_size)
        != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    object->SetFIRImage(FIR_image, FIR_image_size);
    FREE(FIR_image);
    FIR_image = nullptr;
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_VERIFY_378(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Macro_Verify_378");
    DOWNCAST_TO(Macro_Verify_378);
    if (CheckAppStatus() == false)
    {
        object->SetReturnCode(GEN_ERROR_APP_BUSY);
        return false;
    }
    auto err = CMD_EXECUTIVE.Execute_Verify_378(object->GetGalleryTemplate(), object->GetGalleryTemplateSize());
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

    RETURN_SUCCESS;
}

//
//bool CommonBroker::cb_CMD_ID_VERIFY_378(const std::shared_ptr<ICmd>& pCmd)
//{
//    DOWNCAST_TO(Macro_ID_Verify_378);
//    if (CheckAppStatus() == false)
//    {
//        object->SetReturnCode(GEN_ERROR_APP_BUSY);
//        return false;
//    }
//    short nArg = -1;
//    object->GetArguement(nArg);
//    if (nArg == 0)
//    {
//        object->GetUserRecord()->nFinger = 0xFFFFFFFF;
//    }
//    auto err = CMD_EXECUTIVE.Execute_ID_Verify_378(object->GetTemplate(), object->GetTemplateSize(), object->GetUserRecord());
//    if ( err == CExecStatus::CMD_ERR_BAD_DATA)
//    {
//        object->SetReturnCode(GEN_VER_INVALID_RECORD_FORMAT);
//        return false;
//    }
//    if (err == CExecStatus::CMD_ERR_OUT_OF_MEMORY)
//    {
//        object->SetReturnCode(GEN_ERROR_MEMORY);
//        return false;
//    }
//    if (err != CExecStatus::CMD_EXEC_OK)
//    {
//        RETURN_MARSHAL_FAILURE;
//    }
//    RETURN_SUCCESS;
//}


bool CommonBroker::cb_CMD_SET_OPTION(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Set_Option");
    DOWNCAST_TO(Atomic_Set_Option);
    _V100_OPTION_TYPE option_to_set;
    uchar* opt_data;
    uint opt_data_size;
    if (object->GetOption(option_to_set, &opt_data, opt_data_size) == false)
    {
        object->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }
    if (CMD_EXECUTIVE.Execute_Set_Option(option_to_set, opt_data, opt_data_size) != CExecStatus::CMD_EXEC_OK)
    {
        object->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }
    RETURN_SUCCESS;
}

bool CommonBroker::cb_CMD_GET_DB_METRICS(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Get_DB_Metrics");
    DOWNCAST_TO(Atomic_Get_DB_Metrics);
    _V100_DB_METRICS dbMetrics;
    memset(&dbMetrics, 0, sizeof(_V100_DB_METRICS));
    dbMetrics.nLastSpoofScore = ISensorInstance::GetInstance()->GetDataMgr()->GetLastSpoofScore();
    object->SetDBMetrics(&dbMetrics);
    RETURN_SUCCESS;
}

//bool CommonBroker::cb_CMD_ID_GET_RESULT(const std::shared_ptr<ICmd>& pCmd)
//{
//    // TODO: NOT IMPLEMENTED
//    pCmd->SetReturnCode(GEN_NOT_SUPPORTED);
//    return false;
//}


bool CommonBroker::cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_Enc_Clear");
    auto err = CMD_EXECUTIVE.Execute_Enc_Clear();
    if (err != CExecStatus::CMD_EXEC_OK)
    {
        pCmd->SetReturnCode(GEN_ERROR_INTERNAL);
        return false;
    }
    pCmd->SetReturnCode(GEN_OK);
    return true;
}

bool CommonBroker::cb_CMD_ID_GET_RESULT(const std::shared_ptr<ICmd>& pCmd)
{
    LOG_DEBUG("Atomic_ID_Get_Result");
    DOWNCAST_TO(Atomic_ID_Get_Result);

    memcpy(object->GetResult(), CMD_EXECUTIVE.Execute_Get_Result(), sizeof(_MX00_ID_RESULT));
    return true;
}
