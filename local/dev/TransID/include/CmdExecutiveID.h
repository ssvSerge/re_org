#pragma once
#include <vector>

#include "IMemMgr.h"
#include "CExecStatus.h"
#include "V100_enc_types.h"
#include "V100_shared_types.h"

class CmdExecutiveID : public MemoryBase
{
public:
    _V100_GENERAL_ERROR_CODES Execute_Delete_User_Record(_MX00_ID_USER_RECORD record_to_delete, bool delete_all);
    _V100_GENERAL_ERROR_CODES Execute_Get_Db_Metrics(_MX00_DB_METRICS* data_to_get, bool get_current);
    _V100_GENERAL_ERROR_CODES Execute_Get_Image(_V100_CAPTURE_TYPE capture_type, uchar **out_image, uint &out_image_size);
    _V100_GENERAL_ERROR_CODES Execute_Get_Spoof_Score(_V100_CAPTURE_TYPE capture_type, int& spoof_score);
    _V100_GENERAL_ERROR_CODES Execute_Get_System_Metrics(_MX00_DB_METRICS** db_metrics, uint& metrics_sent_back);
    _V100_GENERAL_ERROR_CODES Execute_Get_Template(_V100_CAPTURE_TYPE capture_type, std::vector<uchar>&  tpl, uint& tpl_size);
    _V100_GENERAL_ERROR_CODES Execute_Get_User_Record(_MX00_ID_USER_RECORD* user_record_header,  _MX00_TEMPLATE_INSTANCE* buffer, short index_of_record_to_retrieve);
    _V100_GENERAL_ERROR_CODES GetNumberOfInstancesPerUserRecord(uint group_id, uint &num_instances, uint &size_per_template);
    _V100_GENERAL_ERROR_CODES Execute_Get_User_Record_Header(short index_of_record_to_get, _MX00_ID_USER_RECORD* header_to_change);
    _V100_GENERAL_ERROR_CODES Execute_Adapt_to_378(uchar* tpl, uint tpl_size, uchar** out_tpl, uint& out_tpl_size);
    _V100_GENERAL_ERROR_CODES Execute_Set_Parameters(_MX00_ID_PARAMETERS* params);
    _V100_GENERAL_ERROR_CODES Execute_Set_User_Record(_MX00_ID_USER_RECORD* header, _MX00_TEMPLATE_INSTANCE* finger_instance);
    CExecStatus Execute_Verify_Many(uchar* buffer, uint total_size);

    CExecStatus     Execute_ID_Verify_378(u8* pTemplate, uint nTemplateSize, _MX00_ID_USER_RECORD* pUserRecord);

    void Execute_Get_Parameters(_MX00_ID_PARAMETERS* params);

    static CmdExecutiveID& GetInstance()
    {
        static CmdExecutiveID instance;
        return instance;
    }

    CmdExecutiveID(const CmdExecutiveID&) = delete;
    CmdExecutiveID& operator=(const CmdExecutiveID&) = delete;

private:
    CmdExecutiveID()
    {
    }
    ~CmdExecutiveID()
    {
    }
};
