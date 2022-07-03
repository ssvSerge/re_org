#pragma once
#include "BaseTransBroker.h"

/// <summary>
/// You should not use any of these functions!!!
/// </summary>
class FSBroker : public BaseTransBroker
{
public:
    FSBroker() = default;
    ~FSBroker() = default;
    bool cb_CMD_FILE_CD(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_DIR_FINDFIRST(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_DIR_FINDNEXT(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_GETCWD(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_DELETE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_READ(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FILE_WRITE(const std::shared_ptr<ICmd>& pCmd);
    bool cb_CMD_FORMAT_VOLUME(const std::shared_ptr<ICmd>& pCmd);
};