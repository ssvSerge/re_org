#include <sstream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <CExecStatus.h>
#include <CmdExecutiveCommon.h>

CExecStatus CmdExecutiveCommon::Execute_Get_Db_Metrics(_V100_DB_METRICS* pDBMetrics) {
    // pDBMetrics->nLastSpoofScore = ISensorInstance::GetInstance()->GetDataMgr()->GetLastSpoofScore();
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Arm_Trigger(_V100_TRIGGER_MODE eMode) {

    // Currently barcode not supported. If supported remove this condition

    if ( (int) eMode == (int) TRIGGER_BARCODE_ON ) {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Match(u8* pProbe, uint nProbeSize, u8* pGallery, uint nGallerySize, uint& nMatchScore) {

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_VID_Stream(_V100_VID_STREAM_MODE mode) {

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Cmd(_V100_INTERFACE_COMMAND_TYPE* cmd) {

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_LED(_V100_LED_CONTROL led_to_set, bool bOverride) {
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize) {

    CExecStatus status = CExecStatus::CMD_EXEC_OK;

    switch (OptionType) {
        case OPTION_SET_ONE:
        case OPTION_SET_TWO:
        case OPTION_SET_THREE:
        case OPTION_SET_FORCE_FINGER_LIFT_MODE:
        case OPTION_SET_WSQ_COMPRESSION_RATIO:
        case OPTION_SET_LATENT_DETECTION:
        case OPTION_SET_TEMPLATE_MODE:
        case OPTION_SET_RESTORE_FILESYSTEM:
            break;
    }

    return status;
}

CExecStatus CmdExecutiveCommon::Execute_Get_GPIO(uchar* pGPIOMask) {

    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Set_GPIO(uchar GPIOMask) {

    // if (ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType() == PhysicalSensorTypes::MERCURY_M42X) {
    //     const auto GPIO = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>()->GetGPIO();
    //     GPIO->Write(BSP_GPIO_0, (GPIOMask & GPIO_0) ? true : false);
    //     GPIO->Write(BSP_GPIO_1, (GPIOMask & GPIO_1) ? true : false);
    //     return CExecStatus::CMD_EXEC_OK;
    // }
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

bool CmdExecutiveCommon::Execute_Update_Crop_Level(_V100_CROP_LEVEL cropLevel) {
    return true;
}

CExecStatus CmdExecutiveCommon::Execute_Get_EEPROM_M320(_MX00_EEPROM_DATA_M320** pED) {

    // const auto bsp = ISensorInstance::GetInstance()->GetBSP();
    //
    // if (!bsp->IsStreamingBSP() || sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType())) {
    //     return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    // }
    // auto* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    // auto* eeprom = bsp->GetBSP<IStreamBSP>()->GetEEPROM();
    // auto* eeprom_m320 = data_mgr->GetEEPROM_M320();
    // M210_EEPROM_Util_GetEEPROM_M320(*eeprom_m320, *eeprom);
    // *pED = eeprom_m320;

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Reboot() {

    //m_pSI->GetLogger()->Log_String(SEL_DEBUG, __FILE__, std::string("Atomic_Reset")); TODO: implement logging. (Like Legacy)
    // if (false == CmdExecutiveBase::CreateAndPostMacroMessage(CMD_RESET, nullptr, /*NULL*/ 0, App_Busy_Macro)) {
    //     return CExecStatus::CMD_EXEC_OK;
    // }
    // TODO: Intercept RESET command in higher level
    //if (sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType()) && ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP() == false)
    //{
    //    ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>()->RebootDevice(); // UFW implementation
    //    return CExecStatus::CMD_EXEC_OK;
    //}
    //if (!ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
    //{
    //    return CExecStatus::CMD_EXEC_OK; // SEngine (old) compatible.
    //}
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Perform_Process(uchar* pData, uint nDataSize) {

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Image(_V100_IMAGE_TYPE imageType, u8** pImageBuffer, uint& nImageSize, uint nAcqStep) {
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Spoof_Score(int& nSpoofScore) {
    // nSpoofScore = ISensorInstance::GetInstance()->GetDataMgr()->GetLastSpoofScore();
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Template(u8** pTemplate, uint& nTemplateSize) {

    // *pTemplate = nullptr;
    // DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    // uint template_size = 0;
    //
    // if (pDM->GetProbeTemplate() == nullptr || pDM->GetProbeTemplateSize() == 0)
    // {
    //     return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    // }
    //
    //
    // BioTemplate Tpl(pDM->GetProbeTemplate(), pDM->GetProbeTemplateSize());
    // u8* p_template = Tpl.GetOutModeTemplate(template_size);
    // if (!p_template) return CExecStatus::CMD_ERR_BAD_DATA;
    //
    // // when BioTemplate goes out of scope, the memory it allocates will be
    // // automatically freed.  must allocate another buffer and copy the
    // // data over.  the caller has the responsibility of freeing the buffer.
    // //
    // *pTemplate = static_cast<u8*>(MALLOC(template_size));
    // if (*pTemplate == nullptr)
    // {
    //     return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    // }
    // memcpy(*pTemplate, p_template, template_size);
    // nTemplateSize = template_size;

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Template(u8* pTemplate, uint nTemplateSize) {

    // DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    //
    // BioTemplate gallery(pTemplate, nTemplateSize, BioParameters::GetInstance().GetTemplateMode());
    // uint nDevModeGalleryTplSz = 0;
    // u8* pDevModeGalleryTpl = gallery.GetDevModeTemplate(nDevModeGalleryTplSz);
    // if (!pDevModeGalleryTpl) return CExecStatus::CMD_ERR_BAD_DATA;
    //
    //
    // if (false == pDM->SetGalleryTemplate(pDevModeGalleryTpl, nDevModeGalleryTplSz))
    // {
    //     return CExecStatus::CMD_ERR_BAD_SIZE;
    // }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Config_COM_Port(uint nBaudRate) {
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Get_FIR_Image(_V100_FIR_RECORD_TYPE FIRType, _V100_FINGER_PALM_POSITION FingerType, uchar** pFIRImage, uint* nFIRImageSz) {

    // auto* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    // auto* image = data_mgr->GetCompositeImage();
    // uint X, Y, P;
    //
    // uint out_FIR_size;
    // uint BPP = 8;
    // uint impression_type = 0;
    //
    // data_mgr->GetScaledDims(X, Y, P);
    // if (FIRecordAPI::GetFIRecord(image, X, Y, BPP, FIRType, FingerType, impression_type, pFIRImage, out_FIR_size) != FORMAT_CONVERSION_OK) {
    //     return CExecStatus::CMD_ERR_BAD_PARAMETER;
    // }
    // *nFIRImageSz = out_FIR_size;
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Verify_378(u8* pTemplate, uint nTemplateSize) {

    // DataMgr* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    // TAdapter adapter;
    // uchar* dest_tpl = nullptr;
    // uint dest_tpl_size = 0;
    //
    // if (adapter.AdaptTo378(BioParameters::GetInstance().GetTemplateMode(), pTemplate, nTemplateSize, &dest_tpl, &dest_tpl_size) == false) {
    //     return CExecStatus::CMD_ERR_BAD_DATA;
    // }
    //
    // if (CmdExecutiveBase::CreateAndPostMacroMessage(CMD_VERIFY_378, dest_tpl, dest_tpl_size, App_Busy_Macro) == false) {
    //     return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    // }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Truncate_378(uint nSizeRequested, uchar* pTplIn, uint nTplInSize, uchar* pTplOut, uint& nTplOutSize) {

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Enc_Clear() {

    // ISensorInstance::GetInstance()->GetDataMgr()->ClearAllData();
    // if (ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc() != nullptr) {
    //     ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc()->ClearEnroll();
    // }
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Baud_Rate_Change() {
    // TODO: N/A in SEngine.
    //if (ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
    //{
    //    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    //    uint nBaudRate = pDM->GetBaudRate();
    //    IUART* pUART = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>()->GetUART();
    //    pUART->SetBaudRate(nBaudRate);
    //    return CExecStatus::CMD_EXEC_OK;
    //}
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

_MX00_ID_RESULT* CmdExecutiveCommon::Execute_Get_Result() {
    // return ISensorInstance::GetInstance()->GetDataMgr()->GetIDResult();
    return nullptr;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Interface_Cal(_V100_INTERFACE_CALIBRATION_TYPE** pCal) {
    // if (sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType())) {
    //     *pCal = ISensorInstance::GetInstance()->GetDataMgr()->GetCAL();
    //     return CExecStatus::CMD_EXEC_OK;
    // }
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CmdExecutiveCommon& CmdExecutiveCommon::GetInstance() {
    static CmdExecutiveCommon instance;
    return instance;
}

_VX00_DSM_EEPROM_DATA* CmdExecutiveCommon::GetDSM_EEPROM() {

    // const auto is_streaming = ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP();
    // auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    // if (is_streaming)
    // {
    //     auto bsp = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>();
    //     _VX00_DSM_EEPROM_DATA* pDSM_EEPROM = data_mgr->GetDSM_EEPROM();
    //     // Refresh DataMgr copy of eeprom
    //     if (ISensorInstance::GetInstance()->GetConfiguration()->GetSensorType() == V310_10_Type)
    //     {
    //         EEPROM_Format_Type const* pEEPROM = bsp->GetEEPROMData();
    //
    //         // Manually copy over all struct members due to potential alignment issues
    //         pDSM_EEPROM->type = pEEPROM->type;
    //         pDSM_EEPROM->VID_LSB = pEEPROM->VID_LSB;
    //         pDSM_EEPROM->VID_MSB = pEEPROM->VID_MSB;
    //         pDSM_EEPROM->PID_LSB = pEEPROM->PID_LSB;
    //         pDSM_EEPROM->PID_MSB = pEEPROM->PID_MSB;
    //         pDSM_EEPROM->DEVID_LSB = pEEPROM->DEVID_LSB;
    //         pDSM_EEPROM->DEVID_MSB = pEEPROM->DEVID_MSB;
    //         pDSM_EEPROM->CONFIG = pEEPROM->CONFIG;
    //         pDSM_EEPROM->Serial_Number = pEEPROM->Serial_Number;
    //
    //         pDSM_EEPROM->CPLD_Firmware_Revision = pEEPROM->CPLD_Firmware_Revision;
    //
    //         pDSM_EEPROM->ManDateCode = pEEPROM->ManDateCode;
    //         pDSM_EEPROM->Product_ID = pEEPROM->Product_ID;
    //         pDSM_EEPROM->Platform_Type = pEEPROM->Platform_Type;
    //
    //         pDSM_EEPROM->Bx_Row = pEEPROM->Bx_Row;
    //         pDSM_EEPROM->Bx_Col = pEEPROM->Bx_Col;
    //         pDSM_EEPROM->PD_Row = pEEPROM->PD_Row;
    //         pDSM_EEPROM->PD_Col = pEEPROM->PD_Row;
    //         pDSM_EEPROM->DPI = pEEPROM->DPI;
    //
    //         pDSM_EEPROM->MfgStateFlag = pEEPROM->MfgStateFlag;
    //
    //         memcpy(pDSM_EEPROM->pCalData, pEEPROM->pCalData, sizeof(pEEPROM->pCalData));
    //         memcpy(&pDSM_EEPROM->pTagData, pEEPROM->pTagData, sizeof(pEEPROM->pTagData));
    //     }
    //     else
    //     {
    //         EEPROM_Unified_Format_Type const* pEEPROM = bsp->GetEEPROM();
    //
    //         // Manually copy over all struct members due to potential alignment issues
    //         pDSM_EEPROM->type = pEEPROM->cypress.type;
    //         pDSM_EEPROM->VID_LSB = pEEPROM->cypress.VID_LSB;
    //         pDSM_EEPROM->VID_MSB = pEEPROM->cypress.VID_MSB;
    //         pDSM_EEPROM->PID_LSB = pEEPROM->cypress.PID_LSB;
    //         pDSM_EEPROM->PID_MSB = pEEPROM->cypress.PID_MSB;
    //         pDSM_EEPROM->DEVID_LSB = pEEPROM->cypress.DEVID_LSB;
    //         pDSM_EEPROM->DEVID_MSB = pEEPROM->cypress.DEVID_MSB;
    //         pDSM_EEPROM->CONFIG = pEEPROM->cypress.CONFIG;
    //         pDSM_EEPROM->Serial_Number = pEEPROM->platform.Serial_Number;
    //
    //         // Not implemented in the M210 EEPROM
    //         pDSM_EEPROM->CPLD_Firmware_Revision = 0;
    //
    //         pDSM_EEPROM->ManDateCode = pEEPROM->platform.ManDateCode;
    //         pDSM_EEPROM->Product_ID = pEEPROM->platform.Product_ID;
    //         pDSM_EEPROM->Platform_Type = pEEPROM->platform.Platform_Type;
    //
    //         // Not implemented in the M210 EEPROM ... or is it?
    //         pDSM_EEPROM->Bx_Row = 0;
    //         pDSM_EEPROM->Bx_Col = 0;
    //         pDSM_EEPROM->PD_Row = 0;
    //         pDSM_EEPROM->PD_Col = 0;
    //         pDSM_EEPROM->DPI = 0;
    //
    //         pDSM_EEPROM->MfgStateFlag = pEEPROM->platform.MfgStateFlag;
    //
    //         // Not implemented in the M210 EEPROM
    //         memset(pDSM_EEPROM->pCalData, 0, sizeof(pDSM_EEPROM->pCalData));
    //
    //         pDSM_EEPROM->pTagData[0] = pEEPROM->platform.nTagDataSize;
    //         memcpy(&pDSM_EEPROM->pTagData[1], pEEPROM->platform.pTagData, pEEPROM->platform.nTagDataSize);
    //     }
    //     return pDSM_EEPROM;
    // }
    // auto bsp = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>();
    // // Grab the EEPROM
    // IEEPROM* pEEPROM = bsp->GetEEPROM();
    // // Grab the EEPROM Data from BSP
    // DSM_EEPROM_Calibration* pBSPEEPROM = pEEPROM->GetDSMSettings();
    // // Grab the Data Managers EEPROM structure
    // _VX00_DSM_EEPROM_DATA* pEEPROMSt = data_mgr->GetDSM_EEPROM();
    //
    // *pEEPROMSt = *((_VX00_DSM_EEPROM_DATA*)(pBSPEEPROM));

    return nullptr;
}

void CmdExecutiveCommon::Tokenize(std::string const& str, const char delim, std::vector<std::string>& out) {

    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

//////////////////////////////////////////////////////////////////////////////////////
/// Private Helper Functions                                                        //
//////////////////////////////////////////////////////////////////////////////////////

CExecStatus CmdExecutiveCommon::ExecuteShellCommand(std::string script_cmd_line) {

    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::GetScriptOutputBin(uint8_t** p_results_data, uint32_t& size_results_data)
{
#ifdef _YOCTO_LINUX_
    return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
#if 0
    std::vector<std::string> strVec;

    RemoteExec remoteExec;



    while (remoteExec.get_status() == 0)
    {
        std::string strLine;
        remoteExec.get_output(strLine);
        strVec.push_back(strLine);
    }

    // Convert the string vector into a string stream
    std::stringstream scriptoutput;
    for (auto& i : strVec)
    {
        scriptoutput << i.c_str() << "\n";
    }

    // Convert the string stream into an unsigned char buffer
    std::string fullScriptOutput = scriptoutput.str();
    size_results_data = fullScriptOutput.length() + 1;
    *p_results_data = new uint8_t[size_results_data];
    memset(*p_results_data, 0, size_results_data);
    memcpy(*p_results_data, fullScriptOutput.c_str(), fullScriptOutput.length());

    return CExecStatus::CMD_EXEC_OK;
#endif
#endif
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::GetSensorConfig(uint8_t** p_config_data, uint32_t& size_sensor_cfg) {
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::SetSensorConfig(u8* p_sensor_cfg, uint size_sensor_cfg) {
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::WriteFile(const char* p_file_path, u8* p_file_data, uint size_file_data) {
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}
