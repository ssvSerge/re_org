#pragma once
#include "BaseTransBroker.h"

class ISELogger;

class InternalBroker :public BaseTransBroker
{
public:
    InternalBroker();
    bool cb_CMD_PROCESS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_COMPOSITE_IMAGE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_IMAGE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_CAL(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_SPOOF_DETAILS_V2(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_LICENSE_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_LICENSE_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_MFG_STATE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_EEPROM_M320(const std::shared_ptr<ICmd>& pCmd);
};
