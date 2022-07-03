#pragma once
#include "BaseTransBroker.h"

class CommonBroker : public BaseTransBroker
{
public:
    CommonBroker();
    ~CommonBroker();

    bool cb_CMD_MATCH                   (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_VID_STREAM              (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_IMAGE               (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_COMPOSITE_IMAGE     (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_TEMPLATE            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_TEMPLATE            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ARM_TRIGGER             (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_CMD                 (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_LED                 (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_CONFIG_COMPORT          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_TRUNCATE_378            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_MATCH_EX                (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_GPIO                (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_GPIO                (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_FIR_IMAGE           (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_VERIFY_378              (const std::shared_ptr<ICmd>& pCmd);
   // bool cb_CMD_ID_VERIFY_378           (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_SET_OPTION              (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_GET_DB_METRICS          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_RESULT           (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_CLEAR               (const std::shared_ptr<ICmd>& pCmd);
};
