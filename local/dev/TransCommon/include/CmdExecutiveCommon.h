#pragma once
#include <unordered_map>

#include "CExecStatus.h"
#include "IMemMgr.h"
#include "V100_enc_types.h"
#include "V100_internal_types.h"
#include "V100_shared_types.h"
#include <vector>
#include <string>

#if defined(__linux__) && !defined(_YOCTO_LINUX_)
#if __cpp_template_template_args < 201611
#define INS_STL_HASH_FOR_ENUM(_TYPE)                     \
            namespace std                                        \
            {                                                    \
                template<>                                       \
                struct hash<_TYPE>                               \
                {                                                \
                    size_t operator()(const _TYPE& value) const  \
                    {                                            \
                        return hash<int>()((int)value);          \
                    }                                            \
                };                                               \
            };                                                   \

INS_STL_HASH_FOR_ENUM(PhysicalSensorTypes)
#endif
#endif
class CmdExecutiveCommon : MemoryBase
{
public:

    CExecStatus Execute_Set_Cmd(_V100_INTERFACE_COMMAND_TYPE* cmd);

    CExecStatus Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize);
    CExecStatus Execute_Get_Image(_V100_IMAGE_TYPE imageType, u8** pImageBuffer, uint& nImageSize, uint nAcqStep);
    CExecStatus Execute_Get_Db_Metrics(_V100_DB_METRICS* pDBMetrics);
    CExecStatus Execute_Arm_Trigger(_V100_TRIGGER_MODE eMode);
    CExecStatus Execute_Match(u8* pProbe, uint nProbeSize, u8* pGallery, uint nGallerySize, uint& nMatchScore);
    CExecStatus Execute_VID_Stream(_V100_VID_STREAM_MODE mode);
    CExecStatus Execute_Config_COM_Port(uint nBaudRate);
    CExecStatus Execute_Get_Spoof_Score(int& nSpoofScore);
    CExecStatus Execute_Get_Template(u8** pTemplate, uint& nTemplateSize);
    CExecStatus Execute_Set_Template(u8* pTemplate, uint nTemplateSize);
    CExecStatus Execute_Set_LED(_V100_LED_CONTROL led, bool bOverride = false);
    CExecStatus Execute_Get_FIR_Image(_V100_FIR_RECORD_TYPE FIRType, _V100_FINGER_PALM_POSITION FingerType,
                                      uchar** pFIRImage, uint* nFIRImageSz);
    CExecStatus Execute_Verify_378(u8* pTemplate, uint nTemplateSize);

    CExecStatus Execute_Truncate_378(uint nSizeRequested, uchar* pTplIn, uint nTplInSize, uchar* pTplOut,
                                     uint& nTplOutSize);

    CExecStatus Execute_Get_GPIO(uchar* pGPIOMask);
    CExecStatus Execute_Set_GPIO(uchar GPIOMask);

    bool Execute_Update_Crop_Level(_V100_CROP_LEVEL cropLevel);

    CExecStatus Execute_Reboot();
    CExecStatus Execute_Enc_Clear();
    CExecStatus Execute_Baud_Rate_Change();
    CExecStatus Execute_Get_EEPROM_M320(_MX00_EEPROM_DATA_M320** pED);
    CExecStatus Execute_Get_Interface_Cal(_V100_INTERFACE_CALIBRATION_TYPE** pCal);
    CExecStatus Execute_Perform_Process(uchar* pData, uint nDataSize); //used by CE_CMD_PROCESS

    _MX00_ID_RESULT* Execute_Get_Result();


    static CmdExecutiveCommon& GetInstance();

    //Manufacturing only (?)
    _VX00_DSM_EEPROM_DATA* GetDSM_EEPROM();


    CmdExecutiveCommon(const CmdExecutiveCommon&) = delete;
    CmdExecutiveCommon& operator=(const CmdExecutiveCommon&) = delete;


private:
    CmdExecutiveCommon() = default;
    ~CmdExecutiveCommon() = default;

    void        Tokenize(std::string const& str, const char delim, std::vector<std::string>& out);
    CExecStatus ExecuteShellCommand(std::string script_cmd_line);
    CExecStatus GetScriptOutputBin(uint8_t** p_results_data, uint32_t& size_results_data);

    CExecStatus GetSensorConfig(uint8_t** p_config_data, uint32_t& size_sensor_cfg);
    CExecStatus SetSensorConfig(u8* p_sensor_cfg, uint size_sensor_cfg);

    CExecStatus WriteFile(const char* p_file_path, u8* p_file_data, uint size_file_data);


    const std::unordered_map<PhysicalSensorTypes, bool> sensor_is_venus_map_{
        {PhysicalSensorTypes::VENUS_V30X, true},
        {PhysicalSensorTypes::VENUS_V31X, true},
        {PhysicalSensorTypes::VENUS_V371, true},
        {PhysicalSensorTypes::VENUS_V40X, true},
        {PhysicalSensorTypes::VENUS_V42X, true},
        {PhysicalSensorTypes::VENUS_V31X_10, true},
        {PhysicalSensorTypes::VENUS_V32X, true},
        {PhysicalSensorTypes::VENUS_V52X, true},
        {PhysicalSensorTypes::MERCURY_M30X, false},
        {PhysicalSensorTypes::MERCURY_M31X, false},
        {PhysicalSensorTypes::MERCURY_M32X, false},
        {PhysicalSensorTypes::MERCURY_M42X, false},
        {PhysicalSensorTypes::MERCURY_M21X, false}
    };
};
