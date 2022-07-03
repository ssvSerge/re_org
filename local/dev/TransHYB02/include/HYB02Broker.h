#pragma once
#include "BaseTransBroker.h"


class HYB02Broker : public BaseTransBroker
{
public:
    HYB02Broker();
    ~HYB02Broker() = default;

    bool cb_CMD_ENC_MATCH(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_VERIFY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_ENROLL(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_CAPTURE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_OPTION(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_IMAGE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_TEMPLATE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GENERATE_SESSIONKEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_ACTIVE_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_CAPTURE_STATS(const std::shared_ptr<ICmd>& pCmd);
};
