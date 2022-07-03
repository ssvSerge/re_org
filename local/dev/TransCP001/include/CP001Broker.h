#pragma once

#include "BaseTransBroker.h"

class CP001Broker : public BaseTransBroker
{
public:
    CP001Broker();
    ~CP001Broker() = default;

    bool cb_CMD_ARM_TRIGGER(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_COMPOSITE_IMAGE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_OPTION(const std::shared_ptr<ICmd>& pCmd);

    bool cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GENERATE_SESSIONKEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_ACTIVEKEY(const std::shared_ptr<ICmd>& pCmd);

};
