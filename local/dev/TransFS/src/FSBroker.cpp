#include "FSBroker.h"
#define __CLASSNAME__  FSBroker
#define CMD_EXECUTIVE CmdExecutiveFS::GetInstance()

bool FSBroker::cb_CMD_FILE_CD(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_DIR_FINDFIRST(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_DIR_FINDNEXT(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_GETCWD(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_DELETE(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_READ(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FILE_WRITE(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}

bool FSBroker::cb_CMD_FORMAT_VOLUME(const std::shared_ptr<ICmd>& pCmd)
{
    return false;
}
