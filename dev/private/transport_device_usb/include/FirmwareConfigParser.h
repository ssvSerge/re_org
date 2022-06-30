#pragma once

#include "json.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;

enum class FwUpdateStates
{
    kToBase = -1,
    kNotAllowed = 0,
    kToNonBase = 1,
};

struct KeyItem
{
    KeyItem(unsigned int t, unsigned int s) : EncKeyType(t), SecElemSlot(s) {};
    unsigned int EncKeyType;
    unsigned int SecElemSlot;
};

struct KeyMappingItem
{
    KeyMappingItem(std::vector<KeyItem> k, std::vector<std::string> p) : keys(k), properties(p) {};
    std::vector<KeyItem> keys;
    std::vector<std::string> properties;
};

typedef std::unordered_map<std::string, unsigned int> StrToUIntMap;

class FirmwareConfigParser
{
    public:
        explicit FirmwareConfigParser(bool load_data = true);
        ~FirmwareConfigParser();

        int LoadData();
        int UnloadData();

        // Returns vector<sensor enums>
        std::vector<unsigned int> GetAvailableSensorEnums();

        // Returns enum val for the sensor name provided
        unsigned int GetSensorEnumByName(const char* sensor_name);

        // Returns vector<fwConf enums>
        std::vector<unsigned int> GetAvailableFwConfEnums();

        // Returns enum val for the fwConf provided
        unsigned int GetFwConfByName(const char* fw_conf_name);

        // Returns map<sensor enum val, vector<fwConf enums>>
        std::unordered_map<unsigned int, std::vector<unsigned int>> GetAllowedFwConf();

        // Determines if sensor and new_conf combination are supported
        FwUpdateStates IsFwConfAllowed(unsigned int sensor, unsigned int current_config, unsigned int new_conf);

        // Returns map<Enc Key Type, Secure Elem Slot> for the fwConf provided
        std::map<unsigned int, unsigned int> GetFwConfKeySlotsMap(unsigned int fw_conf_id);

        // Returns SecureElemSlot vector
        std::vector<KeyItem> GetFwConfKeySlots(unsigned int fw_conf_id);

        // Returns vector<Properties strings> for the fwConf provided
        std::vector<std::string> GetFwConfProperties(unsigned int fw_conf_id);

        FirmwareConfigParser(FirmwareConfigParser&& old_elem) = default;
        FirmwareConfigParser(const FirmwareConfigParser& old_elem) = default;
        FirmwareConfigParser& operator=(FirmwareConfigParser&& old_elem) = default;
        FirmwareConfigParser& operator=(const FirmwareConfigParser& old_elem) = default;

    private:

        void SetDefaultValues();
        //json m_pJSON;
        bool m_bUseJSONData;
        StrToUIntMap available_sensor_types_, available_trans_models_;
        std::unordered_map<unsigned int, std::vector<unsigned int>> allowed_fw_cfgs_;
        std::unordered_map<unsigned int, KeyMappingItem> key_mappings_;
};