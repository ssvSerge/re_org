#include "CmdExecutiveInternal.h"
#include "ISensorInstance.h"
#include "DataMgr.h"

CExecStatus CmdExecutiveInternal::Execute_Set_Image(_V100_IMAGE_TYPE type, uchar* pImage, uint imageSize)
{
    uint nX, nY, nP;

    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    pDM->GetRawDims(nX, nY, nP);
    uint nCalImageSize = nX * nY;
    switch (type)
    {
        case IMAGE_NATIVE_1:
        case IMAGE_NATIVE_2:
        case IMAGE_NATIVE_3:
        case IMAGE_NATIVE_4:
        case IMAGE_NATIVE_MASK:
        case IMAGE_PD:
        case IMAGE_DARK_FRAME:
        case IMAGE_DARK_FRAME_STATE_1:
        {
        } break;
        case IMAGE_SCALED_1:
        case IMAGE_SCALED_2:
        case IMAGE_SCALED_3:
        case IMAGE_SCALED_4:
        case IMAGE_COMPOSITE:
        case IMAGE_SCALED_MASK:
        {
            pDM->GetScaledDims(nX, nY, nP);
            nCalImageSize = nX * nY;
        } break;
        default:
        {
            return CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }


    if (imageSize > nCalImageSize)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    // Let DataMgr handle this.
    if (!pDM->SetImage(type, pImage, imageSize))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    /* TODO - extract template
    if (type == IMAGE_COMPOSITE)
    {
        if (false == IProcess::ExtractTemplate(pDM->GetCompositeImage(), nX, nY, NULL, pDM->GetProbeTemplate(), pDM->GetProbeTemplateSize()))
        {
            return CMD_ERR_BAD_DATA;
        }
    }*/

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveInternal::Execute_Get_Cal(_V100_INTERFACE_CALIBRATION_TYPE** pCal)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveInternal::Execute_Set_Mfg_State(_V100_MFG_STATE* pMfgState)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveInternal::Execute_Get_EEPROM_M320(_MX00_EEPROM_DATA_M320** pEEPROM)
{
    auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    *pEEPROM = data_mgr->GetEEPROM_M320();
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveInternal::Execute_Process(uchar* pData, uint nDataSize)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}
