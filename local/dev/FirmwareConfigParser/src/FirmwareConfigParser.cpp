#include "FirmwareConfigParser.h"
#include <fstream>
#include <iostream>
#include <utility>
#include "logging.h"


FirmwareConfigParser::FirmwareConfigParser(const bool load_data)
{
    //m_pJSON.clear();
    available_sensor_types_.clear();
    available_trans_models_.clear();
    allowed_fw_cfgs_.clear();
    key_mappings_.clear();

    m_bUseJSONData = false;
    if (load_data)
    {
        LoadData();
    }
}

FirmwareConfigParser::~FirmwareConfigParser()
{
    try {
        UnloadData();
    }catch(...)
    {
        // Ignore exception while destruction.
    }
}

// Reads JSON file into memory
int FirmwareConfigParser::LoadData()
{
    using namespace std;

    UnloadData();

    ifstream cfg_in("/var/local/bin/FirmwareConfig.json", ios::binary);
    nlohmann::json json_document;
    if (!cfg_in) {
        cfg_in.open("/usr/local/HID/Lumidigm/dat/FirmwareConfig.json", ios::in | ios::binary);
        if (!cfg_in)
        {
            SetDefaultValues();
            return -1;
        }
    }
    try {
        json_document = json::parse(cfg_in);

    } catch (...) {
        SetDefaultValues();
        cfg_in.close();
        return -2;
    }
    cfg_in.close();

    try {
        available_sensor_types_ = json_document["AvailableSensorTypes"].get<StrToUIntMap>();
        available_trans_models_ = json_document["AvailableTransModels"].get<StrToUIntMap>();

        auto strAllowedCfgs = json_document["AllowedFwCfgs"];
        for (auto& it : strAllowedCfgs.items())
        {
            auto value_str_vect = it.value().get<std::vector<std::string>>();
            std::vector<unsigned int> value_vect;
            for (auto vect : value_str_vect)
            {
                value_vect.emplace_back(available_trans_models_.at(vect));
            }

            allowed_fw_cfgs_.insert(std::pair<unsigned, std::vector<unsigned>>{ available_sensor_types_.at(it.key()), value_vect });
        }
        auto mapping_str_items = json_document["KeyMappings"];
        for (auto& it : mapping_str_items.items())
        {
            auto cfg_id = available_trans_models_.at(it.key());
            auto keys = it.value()["Keys"];
            std::vector<KeyItem> key_items;
            for (auto& key : keys)
            {
                key_items.emplace_back(KeyItem{ key["EncKeyType"].get<unsigned int>(), key["SecureElemSlot"].get<unsigned int>() });
            }
            auto props = it.value()["Properties"].get<std::vector<std::string>>();
            key_mappings_.insert({
                cfg_id, { key_items , props }
                });
        }
    }
    catch (...) {
        // Exception occurred while parsing data from json object
        SetDefaultValues();
        return -3;
    }

    m_bUseJSONData = true;
    return 0;
}

// Unloads JSON data from memory
int FirmwareConfigParser::UnloadData()
{
    available_sensor_types_.clear();
    available_trans_models_.clear();
    allowed_fw_cfgs_.clear();
    key_mappings_.clear();
    m_bUseJSONData = false;

    return 0;
}

// Returns vector<sensor enums>
std::vector<unsigned int> FirmwareConfigParser::GetAvailableSensorEnums()
{
    std::vector<unsigned int> sensor_enums;

    for (auto it : available_sensor_types_)
    {
        sensor_enums.push_back(it.second);
    }

    return sensor_enums;
}

// Returns enum val for the sensor name provided
unsigned int FirmwareConfigParser::GetSensorEnumByName(const char* sensor_name)
{
    try {
        return available_sensor_types_.at(sensor_name);
    }
    catch (...)
    {
        return 9999;
    }
}

// Returns vector<trans model enums>
std::vector<unsigned int> FirmwareConfigParser::GetAvailableFwConfEnums()
{
    std::vector<unsigned int> vModelEnums;

    for (auto it : available_trans_models_)
    {
        vModelEnums.push_back(it.second);
    }

    return vModelEnums;
}

// Returns enum val for the model name provided
unsigned int FirmwareConfigParser::GetFwConfByName(const char* fw_conf_name)
{
    try {
        return available_trans_models_.at(fw_conf_name);
    }
    catch (...)
    {
        return 9999;
    }
}

// Returns map<sensor enum val, vector<trans model enums>>
std::unordered_map<unsigned int, std::vector<unsigned int>> FirmwareConfigParser::GetAllowedFwConf()
{
    return allowed_fw_cfgs_;
}

// Determines if sensor and transaction model combination are supported
FwUpdateStates FirmwareConfigParser::IsFwConfAllowed(const unsigned int sensor, const unsigned int current_config, const unsigned int new_config)
{
    std::unordered_map<unsigned int, std::vector<unsigned int>> allowedConfig = GetAllowedFwConf();

    auto it = allowedConfig.find(sensor);
    if (it == allowedConfig.end())
    {
        return FwUpdateStates::kNotAllowed;
    }
    const auto base_cfg = GetFwConfByName("BASE");

    const auto it_conf = find(it->second.begin(), it->second.end(), new_config);
    if (it_conf == it->second.end())
    {
        return FwUpdateStates::kNotAllowed;
    }

    if (current_config == base_cfg && new_config != base_cfg)
    {
        return FwUpdateStates::kToNonBase;
    }

    if (current_config != base_cfg && new_config  == base_cfg)
    {
        return FwUpdateStates::kToBase;
    }

    return FwUpdateStates::kNotAllowed;
}

// Returns map<Enc Key Type, Secure Elem Slot>
std::map<unsigned int, unsigned int> FirmwareConfigParser::GetFwConfKeySlotsMap(unsigned int fw_conf_id)
{
    std::map<unsigned int, unsigned int> mTransKeys;

    try {
        for (auto it : key_mappings_.at(fw_conf_id).keys)
        {
            mTransKeys.insert({ it.EncKeyType, it.SecElemSlot });
        }
    }
    catch (...)
    {
        return std::map<unsigned int, unsigned int>();
    }

    return mTransKeys;
}

std::vector<KeyItem> FirmwareConfigParser::GetFwConfKeySlots(unsigned int fw_conf_id)
{
    try
    {
        return key_mappings_.at(fw_conf_id).keys;
    }catch (...)
    {
        return std::vector<KeyItem>();
    }
}

// Returns vector<Properties strings> for the modelName provided
std::vector<std::string> FirmwareConfigParser::GetFwConfProperties(unsigned int fw_conf_id)
{
    try
    {
        return key_mappings_.at(fw_conf_id).properties;
    }
    catch (...)
    {
        return std::vector<std::string>();
    }
}


void FirmwareConfigParser::SetDefaultValues()
{
    available_sensor_types_ = StrToUIntMap({
        {"UNKNOWN_LUMI_DEVICE",     0 },
        {"VENUS_V30X",              1 },
        {"MERCURY_M30X",            2 },
        {"MERCURY_M31X",            3 },
        {"VENUS_V31X",              4 },
        {"VENUS_V371",              5 },
        {"VENUS_V40X",              6 },
        {"VENUS_V42X",              7 },
        {"MERCURY_M32X",            8 },
        {"MERCURY_M42X",            9 },
        {"MERCURY_M21X",            10},
        {"VENUS_V31X_10",           11},
        {"VENUS_V32X",              12},
        {"VENUS_V52X",              13},
        {"UNSUPPORTED_LUMI_DEVICE", 99}
    });

    available_trans_models_ = StrToUIntMap({
        {"MSK00",        0x0100},
        {"PKI",          0x0200},
        {"VEX",          0x0300},
        {"BASE",         0x0400},
        {"UNLOCKED",     0x0500},
        {"PKI_UNLOCKED", 0x0600},
        {"TECBAN",       0x0700},
        {"MSK01",        0x0800},
        {"INTERM",       0x0900},
        {"ALL",          0x1000},
        {"HYB01",        0x1100},
        {"RECOVERY",     0x1200},
        {"HYB02",        0x1300},
        {"CP001",        0x1400}
    });

    allowed_fw_cfgs_ = std::unordered_map<unsigned int, std::vector<unsigned int>>({
        {GetSensorEnumByName("MERCURY_M21X"), { GetFwConfByName("VEX")} },
        {GetSensorEnumByName("MERCURY_M31X"), { GetFwConfByName("VEX") } },
        {GetSensorEnumByName("VENUS_V31X"),   { GetFwConfByName("VEX") }},
        {GetSensorEnumByName("VENUS_V31X_10"),{ GetFwConfByName("VEX") }},
        {GetSensorEnumByName("VENUS_V32X"),   { GetFwConfByName("VEX"),
                                                GetFwConfByName("BASE"),
                                                GetFwConfByName("MSK00"),
                                                GetFwConfByName("HYB02") }},
        {GetSensorEnumByName("VENUS_V52X"),   { GetFwConfByName("VEX"),
                                                GetFwConfByName("BASE"),
                                                GetFwConfByName("MSK00"),
                                                GetFwConfByName("HYB02") }}
    });

    key_mappings_ = std::unordered_map<unsigned int, KeyMappingItem>({
        {GetFwConfByName("BASE"),
            KeyMappingItem(std::vector<KeyItem>({{1, 1},        /*KT_EXTKEY_CTK - Slot 1*/
                                                {17, 17},        /*KT_EXTKEY_DEVICE_PUBLIC - Slot 17*/
                                                {18, 18},        /*KT_EXTKEY_DEVICE_PRIVATE - Slot 18*/
                                                {19, 19},        /*KT_EXTKEY_DEVICE_P - Slot 19*/
                                                {20, 20},        /*KT_EXTKEY_DEVICE_Q - Slot 20*/
                                                {22, 22},        /*KT_EXTKEY_PRIVATE_EXP - Slot 22*/
                                                {65535, 99}}),    /*KT_KEY_TMP - Slot 99*/
                           std::vector<std::string>({})) },
        {GetFwConfByName("HYB02"),
            KeyMappingItem(std::vector<KeyItem>({{2,101},        /*KT_EXTKEY_BTK - Slot 101*/
                                                {3, 102},        /*KT_EXTKEY_AES0 - Slot 102*/
                                                {4, 103},        /*KT_EXTKEY_AES1 - Slot 103*/
                                                {5, 104},        /*KT_EXTKEY_AES2 - Slot 104*/
                                                {6, 105},        /*KT_EXTKEY_AES3 - Slot 105*/
                                                {9, 106},        /*KT_EXTKEY_TDES2 - Slot 106*/
                                                {10, 107},        /*KT_EXTKEY_TDES3 - Slot 107*/
                                                {11, 108},        /*KT_EXTKEY_AES_VEND - Slot 108*/
                                                {4097, 109},    /*KT_EXT_BSK - Slot 109*/
                                                {65535, 99}}),    /*KT_KEY_TMP - Slot 99*/
                            std::vector<std::string>({std::string("FKL_LOCK_STATUS")}))},
        {GetFwConfByName("MSK00"),
            KeyMappingItem(std::vector<KeyItem>({{8193,101},    /*KT_MSK_MKD - Slot 101*/
                                                {8195, 102},    /*KT_MSK_SK - Slot 102*/
                                                {65535, 99}}),    /*KT_KEY_TMP - Slot 99*/
                            std::vector<std::string>({std::string("PARTIAL_MKD_FOR_TRANSPORT")}))},
    });
}