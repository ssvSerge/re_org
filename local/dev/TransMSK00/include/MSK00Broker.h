#pragma once
#include "BaseTransBroker.h"


class MSK00Broker : public BaseTransBroker
{
public:
    MSK00Broker();
    ~MSK00Broker() = default;

    bool cb_CMD_ENC_GET_RND_NUMBER(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_SET_PARAMETERS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KCV(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_VERIFYMATCH(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_IDENTIFYMATCH(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_ENROLL(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_CAPTURE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_RETURNCAPTUREDBIR(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_FACTORY_SET_KEY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_RETURNCAPTUREDBIR_IM(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_CLEAR(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_SPOOF_SCORE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_DIAG_STATUS(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_GET_KEYVERSION(const std::shared_ptr<ICmd>& pCmd);

    // TECBAN
    bool cb_CMD_ENC_VERIFYMATCH_MANY(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_ENC_VERIFYMATCH_RESULT(const std::shared_ptr<ICmd>& pCmd);
};
