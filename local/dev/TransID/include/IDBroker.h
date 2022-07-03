#pragma once
#include "BaseTransBroker.h"

class IDBroker : public BaseTransBroker
{
public:
    IDBroker();
    ~IDBroker();
    bool cb_CMD_ID_COMMIT                  (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_DELETE_DB               (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_CREATE_DB               (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_DELETE_USER_RECORD      (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_ENROLL_USER_RECORD      (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_DB_METRICS          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_IMAGE               (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_PARAMETERS          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_SPOOF_SCORE         (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_SYSTEM_METRICS      (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_TEMPLATE            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_USER_RECORD         (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_GET_USER_RECORD_HEADER  (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_IDENTIFY_378            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_PURGE_DB_ALL            (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_SET_API_KEY             (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_IDENTIFY                (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_SET_PARAMETERS          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_SET_USER_RECORD         (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_SET_WORKING_DB          (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_VERIFY_378              (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_VERIFY_MANY             (const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ID_VERIFY_USER_RECORD      (const std::shared_ptr<ICmd>& pCmd);
};