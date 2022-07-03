#include "CmdExecutiveFS.h"
//#include <experimental/filesystem>

CExecStatus CmdExecutiveFS::Execute_File_Get_CWD(char* pFile, uint* pSz)
{
       return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Dir_Find_First(char* pFile, uint* pSz, _V100_FILE* pVF)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Dir_Find_Next(char* pFile, uint* pSz, _V100_FILE* pVF)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Cd(char* pFile, uint* pSz)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Delete(char* pFile, uint* pSz, _V100_FILE_ATTR attr)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Read(_V100_FILE* pVF, char* pFileName, char** pFileStream, uint* pFileStreamSize)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_File_Write(_V100_FILE* pVF, char* pFileName, char* pFileStream, uint pFileStreamSize)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveFS::Execute_Format_Volume()
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}
