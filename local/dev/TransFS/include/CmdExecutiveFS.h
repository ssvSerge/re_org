#pragma once
#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "V100_internal_types.h"
#include "CExecStatus.h"

class CmdExecutiveFS : public MemoryBase
{
public:

    CExecStatus Execute_File_Get_CWD(char* pFile, uint* pSz);
    CExecStatus Execute_File_Dir_Find_First(char* pFile, uint* pSz, _V100_FILE* pVF);
    CExecStatus Execute_File_Dir_Find_Next(char* pFile, uint* pSz, _V100_FILE* pVF);
    CExecStatus Execute_File_Cd(char* pFile, uint* pSz);
    CExecStatus Execute_File_Delete(char* pFile, uint* pSz, _V100_FILE_ATTR attr);
    CExecStatus Execute_File_Read(_V100_FILE* pVF, char* pFileName, char** pFileStream, uint* pFileStreamSize);
    CExecStatus Execute_File_Write(_V100_FILE* pVF, char* pFileName, char* pFileStream, uint pFileStreamSize);
    CExecStatus Execute_Format_Volume();

    static CmdExecutiveFS& GetInstance()
    {
        static CmdExecutiveFS instance;
        return instance;
    }

    CmdExecutiveFS(const CmdExecutiveFS&) = delete;
    CmdExecutiveFS& operator=(const CmdExecutiveFS&) = delete;
private:
    CmdExecutiveFS() = default;
    ~CmdExecutiveFS() = default;

};