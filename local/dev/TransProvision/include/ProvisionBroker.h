#pragma once
#include "BaseTransBroker.h"

class ProvisionBroker : public BaseTransBroker
{
public:
    ProvisionBroker();
    ~ProvisionBroker() = default;
    bool cb_CMD_ENC_GET_RND_NUMBER          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_KEY                 (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEY                 (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GENERATE_RSA_KEYS       (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_UNLOCK_KEY              (const std::shared_ptr<ICmd>& pCmd);

};