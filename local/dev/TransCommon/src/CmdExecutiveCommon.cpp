#include "CmdExecutiveCommon.h"
#include "AppStateObserver.h"
#include "BioParameters.h"
#include "BioTemplate.h"
#include "CExecSharedFuncs.h"
#include "CfgMgr.h"
#include "CmdExecutiveBase.h"
#include "DataMgr.h"
#include "FIRecordAPI.h"
#include "ICallback.h"
#include "IdentificationMgr.h"
#include "IEngineV2.h"
#include  "M210_EEPROM_Util.h"
#include "TruncateANSITemplate.h"
#include "wsqWrapper.h"
#include "image_metadata.h"
#include <sstream>
#if __linux__
#ifdef _YOCTO_LINUX_

#include "RemoteExec.h"
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <functional>
#endif
#ifdef _YOCTO_LINUX_
#include "Performance.h"
#endif


CExecStatus CmdExecutiveCommon::Execute_Get_Db_Metrics(_V100_DB_METRICS* pDBMetrics)
{
    pDBMetrics->nLastSpoofScore = ISensorInstance::GetInstance()->GetDataMgr()->GetLastSpoofScore();

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Arm_Trigger(_V100_TRIGGER_MODE eMode)
{
    // Currently barcode not supported. If supported remove this condition
    if (eMode == TRIGGER_BARCODE_ON)
    {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }
    if (eMode == TRIGGER_OFF)
    {
        uint nTimeOut = 10;
        while (nTimeOut)
        {
            nTimeOut--;
            const auto acq_stat = CmdExecutiveBase::GetInstance().Execute_Get_Acq_Status();
            const auto cmd_stat = ISensorInstance::GetInstance()->GetAppStateObserver()->GetMacroCmdBrokerState();

            switch (acq_stat)
            {
            case ACQ_BUSY:
            case ACQ_NO_FINGER_PRESENT:
                // This can only happen during finger lift mode. In venus also happens during PD.
            case ACQ_FINGER_PRESENT: // This can only happen during finger lift mode
            case ACQ_MOVE_FINGER_LEFT: // Happens during PD
            case ACQ_MOVE_FINGER_RIGHT: // Happens during PD
            case ACQ_MOVE_FINGER_DOWN: // Happens during PD
            case ACQ_MOVE_FINGER_UP: // Happens during PD
            case ACQ_FINGER_POSITION_OK: // Happens during PD
                {
                    goto SET_TRIGGER_OFF; // perform cancel
                }
                //break;
            case ACQ_PROCESSING:
                {
                    return CExecStatus::CMD_ERR_BUSY;
                }
            case ACQ_DONE:
                {
                    if (cmd_stat)
                    {
                        return CExecStatus::CMD_ERR_BUSY;
                    }
                    return CExecStatus::CMD_EXEC_OK;
                }
            case ACQ_TIMEOUT:
            case ACQ_LATENT_DETECTED:
            case ACQ_SPOOF_DETECTED:
            case ACQ_ERROR_MATCH:
            case ACQ_ERROR_IMAGE:
            case ACQ_ERROR_SYSTEM:
            case ACQ_ERROR_PARAMETER:
            case ACQ_CANCELLED_BY_USER:
                {
                    if (cmd_stat)
                    {
                        GetServiceProvider()->Sleep(10);
                        continue;
                    }
                    return CExecStatus::CMD_EXEC_OK;
                }
            default:
                {

                }
                break;
            } // switch end
        } // while end

        if (nTimeOut == 0)
        {
            return CExecStatus::CMD_ERR_BUSY;
        }
    }
    // Commented out code from UFW branch, changes SEngine Behavior so action is pending.
    if (eMode != TRIGGER_OFF)
    {
        switch (CmdExecutiveBase::GetInstance().Execute_Get_Acq_Status())
        {
            // do not allow the command
            case ACQ_PROCESSING:
            case ACQ_BUSY:
            case ACQ_NO_FINGER_PRESENT:
            case ACQ_MOVE_FINGER_UP:
            case ACQ_MOVE_FINGER_DOWN:
            case ACQ_MOVE_FINGER_LEFT:
            case ACQ_MOVE_FINGER_RIGHT:
            case ACQ_FINGER_POSITION_OK:
            case ACQ_FINGER_PRESENT:
                return CExecStatus::CMD_ERR_BUSY;
                // allow the command...
            case ACQ_DONE:
            case ACQ_TIMEOUT:
            case ACQ_CANCELLED_BY_USER:
            case ACQ_LATENT_DETECTED:
            case ACQ_SPOOF_DETECTED:
            case ACQ_ERROR_MATCH:
            case ACQ_ERROR_IMAGE:
            case ACQ_ERROR_SYSTEM:
            case ACQ_ERROR_PARAMETER:
            default:
                break;
        }

        ISensorInstance::GetInstance()->GetAppStateObserver()->NotifyEvent(App_Busy_Acq);                // Notify Listeners
    }

    //auto ret = CEPostMessage(ImageServer, CMD_ARM_TRIGGER, (void*)eMode, sizeof(eMode));

SET_TRIGGER_OFF:
#ifdef _YOCTO_LINUX_
    //
    // Clear performance timers
    //
    Performance::Reset();
    Performance::MarkTime("ARM");
#endif

    //if (false == ISensorInstance::GetInstance()->GetAppStateObserver()->PushMessage())
    //{
    //    return CExecStatus::CMD_ERR_BUSY;
    //}

    /*
    **    Send a message to the image server.
    */

    POST_MESSAGE(ImageServer, CMD_ARM_TRIGGER, eMode, sizeof (eMode));

    //auto thread_msg = CreateImageServerMessage(CMD_ARM_TRIGGER, reinterpret_cast<void*>(eMode), sizeof eMode);
    //if (IThreadObserver::GetThreadObserver()->SendThreadMessageBlock(ImageServer, thread_msg.get(), 1000) == false)
    //{
    //    return CExecStatus::CMD_ERR_BUSY;
    //}
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Match(u8* pProbe, uint nProbeSize, u8* pGallery, uint nGallerySize,
                                              uint& nMatchScore)
{
    DataMgr* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    BioTemplate probe(pProbe, nProbeSize, BioParameters::GetInstance().GetTemplateMode());
    BioTemplate gallery(pGallery, nGallerySize, BioParameters::GetInstance().GetTemplateMode());

    // dev mode means ANSI-378 mode.
    uint dev_mode_probe_tpl_size = 0, dev_mode_gallery_template_size = 0;
    u8* dev_mode_probe_tpl = probe.GetDevModeTemplate(dev_mode_probe_tpl_size);
    u8* dev_mode_gallery_tpl = gallery.GetDevModeTemplate(dev_mode_gallery_template_size);

    if (!dev_mode_probe_tpl || !dev_mode_gallery_tpl)
        return CExecStatus::CMD_ERR_BAD_DATA;

    if (data_mgr->SetProbeTemplate(dev_mode_probe_tpl, dev_mode_probe_tpl_size) == false)
        return CExecStatus::CMD_ERR_BAD_SIZE;
    if (data_mgr->SetGalleryTemplate(dev_mode_gallery_tpl, dev_mode_gallery_template_size) == false)
        return CExecStatus::CMD_ERR_BAD_SIZE;

    data_mgr->SetLastMatchScore(0);

    if (false == ISensorInstance::GetInstance()->GetSEProc()->Match(dev_mode_probe_tpl, dev_mode_probe_tpl_size, dev_mode_gallery_tpl, dev_mode_gallery_template_size, &nMatchScore))
    {
        return CExecStatus::CMD_ERR_BAD_DATA;
    }
    data_mgr->SetLastMatchScore(nMatchScore);

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_VID_Stream(_V100_VID_STREAM_MODE mode)
{
    POST_MESSAGE(ImageServer, CMD_VID_STREAM, reinterpret_cast<void*>(mode), sizeof mode);

    if (mode == VID_STREAM_OFF)
    {
        // Poll for completion
        uint nAttempts = 300;
        while (nAttempts-- > 0)
        {
            if (CmdExecutiveBase::GetInstance().Execute_Get_Acq_Status() != ACQ_BUSY)
            {
                break;
            }
            GetServiceProvider()->Sleep(5);
        }

        if (nAttempts == 0)
        {
            return CExecStatus::CMD_ERR_TIMEOUT;
        }
    }
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Cmd(_V100_INTERFACE_COMMAND_TYPE* cmd)
{
    auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();

    if (cmd->Select_Crop_Level != CROP_NONE)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    data_mgr->SetCMD(cmd);
    BioParameters::ReconcileCmdStruct(true);
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_LED(_V100_LED_CONTROL led_to_set, bool bOverride)
{
    const auto sensor_type = ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType();
    if (!sensor_is_venus_map_.at(sensor_type))
    {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }
    // Streaming BSP LED Set.
    if (ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
    {
        auto pBSP = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>();

        if (sensor_type == PhysicalSensorTypes::VENUS_V32X || sensor_type == PhysicalSensorTypes::VENUS_V52X)
        {
            // this function expects one of two things:
            //   _V100_LED_CONTROL                      "user" leds -- green and red
            //   _V100_INTERNAL_VID_STREAM_MODE         camera leds for Venus (4096+)

            // the BSP expects one thing:
            //   _MCAPI_LED_TYPE

            if (led_to_set <= 0x000F)
            {
                if (led_to_set & GREEN_ON)
                    pBSP->SetLED(LED_VENUS_GREEN_ON);

                if (led_to_set & GREEN_OFF)
                    pBSP->SetLED(LED_VENUS_GREEN_OFF);

                if (led_to_set & RED_ON)
                    pBSP->SetLED(LED_VENUS_RED_ON);

                if (led_to_set & RED_OFF)
                    pBSP->SetLED(LED_VENUS_RED_OFF);

                if (led_to_set == ALL_OFF)
                {
                    pBSP->SetLED(LED_VENUS_ALL_OFF);
                }
            }
            else
            {
                int converted_value = 0;
                switch (led_to_set)
                {
                case VID_LED_1_ON:
                    converted_value = LED_VENUS_STATE1;
                    break;
                case VID_LED_2_ON:
                    converted_value = LED_VENUS_STATE2;
                    break;
                case VID_LED_3_ON:
                    converted_value = LED_VENUS_STATE3;
                    break;
                case VID_TIR_ON:
                    converted_value = LED_VENUS_TIR;
                    break;
                case VID_DARK_ON:
                    converted_value = LED_OFF;
                    break;
                case VID_PD_ON:
                    converted_value = LED_PD;
                    break;
                default:
                    return CExecStatus::CMD_ERR_BAD_PARAMETER;
                }

                pBSP->SetLED(converted_value);
            }

            return CExecStatus::CMD_EXEC_OK;
        }
        else if (sensor_type == PhysicalSensorTypes::VENUS_V31X_10)
        {
            switch (led_to_set)
            {
            case ALL_OFF:
                {
                    pBSP->SetLED(LED_VENUS_GREEN_OFF);
                    pBSP->SetLED(LED_VENUS_RED_OFF);
                    return CExecStatus::CMD_EXEC_OK;
                }
            case GREEN_ON:
                {
                    pBSP->SetLED(LED_VENUS_GREEN_ON);
                    return CExecStatus::CMD_EXEC_OK;
                }
            case GREEN_OFF:
                {
                    pBSP->SetLED(LED_VENUS_GREEN_OFF);

                    return CExecStatus::CMD_EXEC_OK;
                }
            case RED_ON:
                {
                    pBSP->SetLED(LED_VENUS_RED_ON);
                    return CExecStatus::CMD_EXEC_OK;
                }
            case RED_OFF:
                {
                    pBSP->SetLED(LED_VENUS_RED_OFF);

                    return CExecStatus::CMD_EXEC_OK;
                }
            case GREEN_CYCLE_ON:
            case GREEN_CYCLE_OFF:
            case RED_CYCLE_ON:
            case RED_CYCLE_OFF:
            case PD_ON:
            case PD_OFF:
            default:
                {
                    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
                }
            }
        }
        switch (static_cast<_V100_INTERNAL_LED_TYPE>(led_to_set))
        {
        case MS_LED_PD_IR:
        case MS_LED_NONP_1_GREEN:
        case MS_LED_NONP_1_BLUE:
        case MS_LED_NONP_2_GREEN:
        case MS_LED_NONP_2_BLUE:
        case MS_LED_VIDEO_DISPLAY:
        case MS_LED_EXT_NONE:
            return CExecStatus::CMD_ERR_NOT_SUPPORTED;
        case MS_LED_PD:
        case MS_LED_OFF:
        case MS_MERC_STATE_1:
        case MS_MERC_STATE_2:
            {
                POST_MESSAGE(ImageServer, CMD_SET_LED, led_to_set, sizeof(led_to_set));
                return CExecStatus::CMD_EXEC_OK;
            }
        default:
            return CExecStatus::CMD_ERR_NOT_SUPPORTED;
        }
    }
    // Legacy BSP LED set
    auto pBSP = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>();
    IEEPROM* pEEPROM = pBSP->GetEEPROM();
    switch (led_to_set)
    {
    case ALL_OFF:
        {
            DSM_EEPROM_Calibration* cal = pEEPROM->GetDSMSettings();
            if (cal->DSMEEMM.Product_ID == 33) // only for V31X
            {
                ILED* pLED = pBSP->GetLED();
                pLED->User_Green(false);
                pLED->User_Red(false);
            }
            return CExecStatus::CMD_EXEC_OK;
        }
    case GREEN_ON:
        {
            DSM_EEPROM_Calibration* cal = pEEPROM->GetDSMSettings();
            if (cal->DSMEEMM.Product_ID != 33) // only for V31X
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
            ILED* pLED = pBSP->GetLED();
            pLED->User_Green(true);
            return CExecStatus::CMD_EXEC_OK;
        }
    case GREEN_OFF:
        {
            DSM_EEPROM_Calibration* cal = pEEPROM->GetDSMSettings();
            if (cal->DSMEEMM.Product_ID != 33) // only for V31X
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
            ILED* pLED = pBSP->GetLED();
            pLED->User_Green(false);
            return CExecStatus::CMD_EXEC_OK;
        }
    case RED_ON:
        {
            DSM_EEPROM_Calibration* cal = pEEPROM->GetDSMSettings();
            if (cal->DSMEEMM.Product_ID != 33) // only for V31X
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
            ILED* pLED = pBSP->GetLED();
            pLED->User_Red(true);
            return CExecStatus::CMD_EXEC_OK;
        }
    case RED_OFF:
        {
            DSM_EEPROM_Calibration* cal = pEEPROM->GetDSMSettings();
            if (cal->DSMEEMM.Product_ID != 33) // only for V31X
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
            ILED* pLED = pBSP->GetLED();
            pLED->User_Red(false);
            return CExecStatus::CMD_EXEC_OK;
        }
    case GREEN_CYCLE_ON:
    case GREEN_CYCLE_OFF:
    case RED_CYCLE_ON:
    case RED_CYCLE_OFF:
    case PD_ON:
    case PD_OFF:
        {
            return CExecStatus::CMD_ERR_NOT_SUPPORTED;
        }
    }

    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize)
{
    CExecStatus status = CExecStatus::CMD_EXEC_OK;
    auto bsp = ISensorInstance::GetInstance()->GetBSP();
    auto sensor_type = ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType();

    switch (OptionType)
    {
    case OPTION_SET_ONE: // Alias: OPTION_PD_LEVEL
        {
            if (!bsp->IsStreamingBSP())
            {
                // Support any older client software that was using BSP Tester to do a capture and spoof predict
                if (sensor_type == PhysicalSensorTypes::VENUS_V32X || sensor_type == PhysicalSensorTypes::VENUS_V52X)
                {
                    status = Execute_Arm_Trigger(TRIGGER_ON);
                }
            }
            else
            {
                // This option is supported for compatibility with manufacturing tools
                status = CExecStatus::CMD_EXEC_OK;
            }
        }
        break;
    case OPTION_SET_TWO:
        {
            status = ExecuteShellCommand(std::string((char*)pOptData));
        } break;
        /*
        **    Normal Operation AGC Window Selection
        */
    case OPTION_SET_AGC_DEFAULT:
        {
            // This option is supported for compatibility with manufacturing tools
            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_SET_EXTRACTOR_OPTIONS:
        {
            if (nOptDataSize != 4)
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            else
            {
                InnovV2_SetExtractorOptions(*(uint*)pOptData);
                status = CExecStatus::CMD_EXEC_OK;
            }
        }
        break;
        /*
        **    Expanded AGC Window for Manufacturing
        */
    case OPTION_SET_AGC_MANUFACTURING:
        {
            u16 gain = 0;
            u16 exposure = 0;

            // Copy
            memcpy(static_cast<void*>(&gain),     pOptData, sizeof(u16));
            memcpy(static_cast<void*>(&exposure), pOptData + sizeof(u16), sizeof(u16));

            status = CExecStatus::CMD_ERR_BAD_PARAMETER;

            if (!bsp->IsStreamingBSP())
            {
                if (sensor_type == PhysicalSensorTypes::VENUS_V31X)
                {
                    auto pCam = bsp->GetBSP<IBSP>()->GetCamera();
                    pCam->Manual_Control(exposure, gain);
                    status = CExecStatus::CMD_EXEC_OK;
                }
            }
        }
        break;
    case OPTION_SET_BORESITE:
        {
            short n_BoreX = 0;
            short n_BoreY = 0;

            status = CExecStatus::CMD_ERR_BAD_PARAMETER;

            if (bsp->IsStreamingBSP())
            {
                //memcpy(static_cast<void*>(&n_BoreX), pOptData, sizeof(short));
                //memcpy(static_cast<void*>(&n_BoreY), pOptData + 2, sizeof(short));

                uint16_t x = *(uint16_t*)(pOptData + 0);
                uint16_t y = *(uint16_t*)(pOptData + 2);

                fprintf(stdout, "\n\nSet Camera boresight values: (%d, %d)\n", x, y);

                if (sensor_type == PhysicalSensorTypes::VENUS_V32X || sensor_type == PhysicalSensorTypes::VENUS_V52X)
                {
                    if (bsp->GetBSP<IStreamBSP>()->SetBoresight(x, y) != 0)
                    {
                        status = CExecStatus::CMD_ERR_BAD_PARAMETER;
                    }
                    else
                    {
                        status = CExecStatus::CMD_EXEC_OK;
                    }
                }
                else if (sensor_type == PhysicalSensorTypes::VENUS_V31X)
                {
                    auto pCam = bsp->GetBSP<IBSP>()->GetCamera();
                    pCam->Set_Boresight(static_cast<u16>(n_BoreX), static_cast<u16>(n_BoreY));
                    ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration()->Boresight_X = n_BoreX;
                    ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration()->Boresight_Y = n_BoreY;
                    // Store values in RAM EEPROM Struct
                    EEPROM_Calibration* pCal = bsp->GetBSP<IBSP>()->GetEEPROM()->GetPDSettings();
                    pCal->EEMM.Optical_Boresight_Row = n_BoreX;
                    pCal->EEMM.Optical_Boresight_Col = n_BoreY;
                    status = CExecStatus::CMD_EXEC_OK;
                }
            }
        }
        break;
    case OPTION_SET_CAL_DATA:
        {
            if (bsp->IsStreamingBSP())
            {
                if (sensor_type == PhysicalSensorTypes::VENUS_V32X || sensor_type == PhysicalSensorTypes::VENUS_V52X)
                {
                    const char* pFileName = "/etc/HID/CICal.bin";
                    WriteFile(pFileName, pOptData, nOptDataSize);
                    status = CExecStatus::CMD_EXEC_OK;
                }
                else
                {
                    return CExecStatus::CMD_ERR_BAD_PARAMETER;
                }
            }
            else if (sensor_type == PhysicalSensorTypes::VENUS_V31X)
            {
                bsp->GetBSP<IBSP>()->GetEEPROM()->Set_Cal(reinterpret_cast<char*>(pOptData), nOptDataSize);
                status = CExecStatus::CMD_EXEC_OK;
            }
        }
        break;
    case OPTION_SET_DPI:
        {
            if (bsp->IsStreamingBSP())
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            auto eeprom = bsp->GetBSP<IBSP>()->GetEEPROM();
            ushort DPI = 0;
            memcpy(&DPI, pOptData, sizeof(ushort));
            eeprom->Set_DPI(DPI);
            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_SET_MFG_STATE:
        {
            if (bsp->IsStreamingBSP())
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            auto eeprom = bsp->GetBSP<IBSP>()->GetEEPROM();
            ushort state = 0;
            memcpy(&state, pOptData, sizeof(ushort));
            eeprom->Set_MfgState(state);
        }
        break;
    case OPTION_SAVE_BORESITE:
        {
            if (bsp->IsStreamingBSP())
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            ushort n_BoreX = 0;
            ushort n_BoreY = 0;

            auto eeprom = bsp->GetBSP<IBSP>()->GetEEPROM();

            memcpy(&n_BoreX, pOptData, sizeof(unsigned short));
            memcpy(&n_BoreY, pOptData + 2, sizeof(unsigned short));

            eeprom->Set_Boresight(n_BoreX, n_BoreY);

            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_SET_FINGER_LIFT_MODE:
        {
            _V100_FINGER_LIFT_MODE FingerLift;
            memcpy(&FingerLift, pOptData, 1);
            bool bRet = false;
            bool FLMode = true;
            if (FORCE_FINGER_LIFT_ON & FingerLift)
            {
                FLMode = true;
                bRet = true;
            }
            else if (FORCE_FINGER_LIFT_OFF & FingerLift)
            {
                FLMode = false;
                bRet = true;
            }
            else
            {
                bRet = false;
            }

            if (bRet == true)
            {
                BioParameters::GetInstance().SetFingerLiftMode(FLMode);
                status = CExecStatus::CMD_EXEC_OK;
            }
            else
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
        }
        break;
    case OPTION_SET_FORCE_FINGER_LIFT_MODE:
        {
            status = CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
        break;
    case OPTION_SET_WSQ_COMPRESSION_RATIO:
        {
            const uint nCompressionRatio = *(int*)pOptData;
            if (false == BioParameters::GetInstance().SetCompressionRatio(static_cast<u16>(nCompressionRatio)))
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            else
            {
                status = CExecStatus::CMD_EXEC_OK;
            }
        }
        break;
    case OPTION_SET_LATENT_DETECTION:
        {
            uint CalcLatent = *(uint*)pOptData;
            BioParameters::GetInstance().SetLatentMode(static_cast<_V100_LATENT_DETECTION_MODE>(CalcLatent));
            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_SET_TEMPLATE_MODE:
        {
            uchar TemplateMode = 0;
            memcpy(&TemplateMode, pOptData, 1);
            const _V100_TEMPLATE_MODE mode = static_cast<_V100_TEMPLATE_MODE>(TemplateMode);
            if (false == BioParameters::GetInstance().SetTemplateMode(mode))
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            else
            {
                status = CExecStatus::CMD_EXEC_OK;
            }
        }
        break;
    case OPTION_SET_SA_STEP:
        {
            // TODO: This is not needed in SEngine for now.
            //            uint nAcqStep = *(int*)pOptData;
            //            auto pDM = ISensorInstance::GetInstance()->GetDataMgr();
            //            SA_Acq_State* pSAState = pDM->GetSAState();
            //
            //#if 1
            //            //Get the mode before changing and after changes(not using currently). Set the resolution with mode changed
            //            //bool bModeChanged = false;
            //            //M32X_SA_Acq_Step * pPrevStep = &(pDM->GetSASteps()[pSAState->Cur_Acq_Step]);
            //
            //            pSAState->Cur_Acq_Step = (uint8_t)nAcqStep;
            //
            //            M32X_SA_Acq_Step* pCurStep = &(pDM->GetSASteps()[pSAState->Cur_Acq_Step]);
            //
            //            //if(    pPrevStep->Mode_Res !=     pCurStep->Mode_Res)
            //            //{
            //            //    bModeChanged = true;
            //            //}
            //
            //            _V100_ACQ_STATUS_TYPE AcqState = CE_CMD_GET_ACQ_STATUS();
            //            if (AcqState != ACQ_BUSY && AcqState != ACQ_PROCESSING)
            //            {
            //
            //                IBSP::GetInstance()->GetCAM()->Set_Resolution(true, (Cam_Res_Mode)pCurStep->Mode_Res);
            //            }
            //#else
            //            pSAState->Cur_Acq_Step = (uint8_t)nAcqStep;
            //#endif
            //            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_SET_SA_TABLE:
        {
            // TODO: This is not needed in SEngine for now.
            /*int number_of_steps = *(int*)pOptData;
            if (nOptDataSize != sizeof(int) + number_of_steps * sizeof(M32X_SA_Acq_Step))
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            else
            {
                ISensorInstance::GetInstance()->GetDataMgr()->InitialiseSA((char*)pOptData);
                status = CExecStatus::CMD_EXEC_OK;
            }*/
        }
        break;
        /* case OPTION_SET_CAM_REG:
         {
               if(nOptDataSize <3)
               {
                   status = CMD_ERR_BAD_PARAMETER;
               }
               else
               {
                   IBSP* pBSP = IBSP::GetInstance();
                   ICAM* pCam = pBSP->GetCAM();
                   u8 Addr = *pOptData;
                   u16 Value;
                   memcpy(&Value, pOptData +1, sizeof(u16));
                   if(false == pCam->Set_Camera_Register(Addr, Value))
                   {
                       status = CMD_ERR_BAD_PARAMETER;
                   }
                   else
                   {
                       status = CMD_EXEC_OK;
                   }
               }

         }break;*/
    case OPTION_DUMP_LOG:
        {
            // Write latency timing data to a file (does nothing if not enabled --
            // see "lumi_time.h" for details).
            // TODO: NOT Supported in SEngine for now.
            //DumpLog(true);
            status = CExecStatus::CMD_EXEC_OK;
        }
        break;
    case OPTION_CLEAR_LOG:
        {
            // TODO: This is not needed in SEngine for now.
            //ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>()->GetLogger()->ClearLog();
        }
        break;
    case OPTION_SAVE_DEBUG_IMAGES:
        {
            uchar bDImages = 0;
            memcpy(&bDImages, pOptData, sizeof(uchar));
            status = CEPostMessage(VDMA, IMAGE_SAVE_DEBUG_IMAGES, &bDImages, sizeof(uchar));
        }
        break;

    case OPTION_SET_CONFIG:
    {
        return SetSensorConfig(pOptData, nOptDataSize);
    } break;
    case OPTION_SET_LUMINANCE_TARGET:
    {
        if (sensor_type == PhysicalSensorTypes::VENUS_V32X || sensor_type == PhysicalSensorTypes::VENUS_V52X)
        {
            // Only works on V52//V32x for now
            uint8_t luminance = 0;
            memcpy(&luminance, pOptData, sizeof(uint8_t));
            bsp->GetBSP<IStreamBSP>()->SetLuminanceTarget(luminance);
            status = CExecStatus::CMD_EXEC_OK;
        }
        else
        {
            return CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }
    break;
    case 255:
        {
            status = CEPostMessage(ImageServer, CMD_SET_OPTION, nullptr, 0);
        }
        break;
    default:
        {
            status = CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }
    return status;
}

CExecStatus CmdExecutiveCommon::Execute_Get_GPIO(uchar* pGPIOMask)
{
    const auto bsp = ISensorInstance::GetInstance()->GetBSP();
    if (bsp->IsStreamingBSP())
    {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }
    *pGPIOMask = 0;

#if _IMPLEMENT_
    auto pGPIO = bsp->GetBSP<IBSP>()->GetGPIO();
    u32 val = 0;
    pGPIO->Read(BSP_GPIO_0, &val);
    *pGPIOMask = val;
    pGPIO->Read(BSP_GPIO_1, &val);
    *pGPIOMask |= val << 1;
    pGPIO->Read(BSP_GPIO_2, &val);
    *pGPIOMask |= val << 2;
    pGPIO->Read(BSP_GPIO_3, &val);
    *pGPIOMask |= val << 3;
#else
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
#endif
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_GPIO(uchar GPIOMask)
{
    if (ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType() == PhysicalSensorTypes::MERCURY_M42X)
    {
        const auto GPIO = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>()->GetGPIO();
        GPIO->Write(BSP_GPIO_0, (GPIOMask & GPIO_0) ? true : false);
        GPIO->Write(BSP_GPIO_1, (GPIOMask & GPIO_1) ? true : false);
        return CExecStatus::CMD_EXEC_OK;
    }
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

bool CmdExecutiveCommon::Execute_Update_Crop_Level(_V100_CROP_LEVEL cropLevel)
{
    if (sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType()))
    {
        if (cropLevel != CROP_NONE) //  Crop is not supported in SEngine.
            return false;
        //if (ISensorInstance::GetInstance()->GetDataMgr()->AdjustImageBuffersToCropLevel(cropLevel) == false)
        //    return false;
        // Don't need policymgr in SEngine.
        return true;
    }
    else
    {
        if (cropLevel != CROP_NONE)
            return false;
        return true;
    }
}


CExecStatus CmdExecutiveCommon::Execute_Get_EEPROM_M320(_MX00_EEPROM_DATA_M320** pED)
{
    const auto bsp = ISensorInstance::GetInstance()->GetBSP();
    if (!bsp->IsStreamingBSP() || sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType()))
    {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }
    auto* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    auto* eeprom = bsp->GetBSP<IStreamBSP>()->GetEEPROM();
    auto* eeprom_m320 = data_mgr->GetEEPROM_M320();
    M210_EEPROM_Util_GetEEPROM_M320(*eeprom_m320, *eeprom);
    *pED = eeprom_m320;
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Reboot()
{
    //m_pSI->GetLogger()->Log_String(SEL_DEBUG, __FILE__, std::string("Atomic_Reset")); TODO: implement logging. (Like Legacy)
    if (false == CmdExecutiveBase::CreateAndPostMacroMessage(CMD_RESET, nullptr, /*NULL*/ 0, App_Busy_Macro))
    {
        return CExecStatus::CMD_EXEC_OK;
    }
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

CExecStatus CmdExecutiveCommon::Execute_Perform_Process(uchar* pData, uint nDataSize)
{
    DataMgr* pDMgr = ISensorInstance::GetInstance()->GetDataMgr();

    _V100_PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING* pPI = (_V100_PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING*)pData;
    if (nDataSize == sizeof(_V100_PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING) && pPI->nIdentifier ==
        PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING)
    {
        pDMgr->SetProcessData((_V100_PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING*)pData);
    }
    else if (nDataSize == sizeof(_V100_PROCESS_IMAGES_AND_SPOOF) && pPI->nIdentifier == PROCESS_IMAGES_AND_SPOOF)
    {
        pDMgr->SetProcessVSAData((_V100_PROCESS_IMAGES_AND_SPOOF*)pData);
    }
    else if (nDataSize == sizeof(_V100_PROCESS_IMAGES_AND_SPOOF_M320) && pPI->nIdentifier ==
        PROCESS_IMAGES_AND_SPOOF_M320)
    {
        pDMgr->SetProcessMSAData((_V100_PROCESS_IMAGES_AND_SPOOF_M320*)pData);
    }
    else
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    pDMgr->SetLastMatchScore(0);

    /*
    **    Send a message to the image server.
    */
    POST_MESSAGE(ImageServer, CMD_PROCESS, nullptr, 0);

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus CmdExecutiveCommon::Execute_Get_Image(_V100_IMAGE_TYPE imageType, u8** pImageBuffer, uint& nImageSize,
                                                  uint nAcqStep)
{
    using namespace MEM_NAMESPACE;
GET_IMG_NOW:
    uint nX, nY, nP;

    uchar* pPtrToImage = nullptr;
    *pImageBuffer = nullptr;

    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    pDM->GetRawDims(nX, nY, nP);
    nImageSize = nX * nY;

    switch (imageType)
    {
    case IMAGE_NATIVE_1:
    case IMAGE_NATIVE_2:
    case IMAGE_NATIVE_3:
    case IMAGE_NATIVE_4:
    case IMAGE_NATIVE_MASK:
    case IMAGE_PD:
    case IMAGE_DARK_FRAME:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetImage(imageType));
        }
        break;
    case IMAGE_SCALED_1:
    case IMAGE_SCALED_2:
    case IMAGE_SCALED_3:
    case IMAGE_SCALED_4:
    case IMAGE_COMPOSITE:
    case IMAGE_SCALED_MASK:
        {
            pDM->GetScaledDims(nX, nY, nP);
            pPtrToImage = static_cast<uchar*>(pDM->GetImage(imageType));
            // Encrypt
            nImageSize = nX * nY;
        }
        break;
    case IMAGE_WSQ:
        {
            // Compress it now
            pPtrToImage = static_cast<uchar*>(pDM->GetImage(imageType)); // Get buffer to store compressed image
            _V100_INTERFACE_CONFIGURATION_TYPE* pICT = pDM->GetInterfaceConfiguration();
            uchar* pCompositeImg = static_cast<uchar*>(pDM->GetImage(IMAGE_COMPOSITE));
            // Get composite image to compress
            uchar* pCompressedImg = nullptr;
            int nCompressedImgSz = 0;
            const uint nCompressionRatio = BioParameters::GetInstance().GetCompressionRatio(); // Get compression ratio
            const float r_bitrate = (1 / static_cast<float>(nCompressionRatio)) * 8;
            if (Lumi_Encode_WSQ(pCompositeImg, r_bitrate, static_cast<int>(pICT->Composite_Image_Size_X),
                                static_cast<int>(pICT->Composite_Image_Size_Y), 8,
                                static_cast<int>(pICT->Composite_DPI), &pCompressedImg, &nCompressedImgSz))
            {
                pPtrToImage = nullptr;
            }
            else
            {
                // Copy it if successfull
                memcpy(pPtrToImage, pCompressedImg, nCompressedImgSz);
                Lumi_Release(pCompressedImg);
                pCompressedImg = nullptr;
                nImageSize = nCompressedImgSz;
            }
        }
        break;
    case IMAGE_SA:
        {
            if (ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
            {
                const auto* SIA_table = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>()->GetSIAStack();
                const auto* image = SIA_table->Get_Acq_Frame(nAcqStep);
                if (image == nullptr)
                {
                    pPtrToImage = nullptr;
                    nImageSize = 0;
                    return CExecStatus::CMD_ERR_BAD_PARAMETER;
                }
                pPtrToImage = image->GetImage();
                nImageSize = image->GetImageSize();
            }
            else
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
        }
        break;
    case LOGGER_XML:
        {
            //pPtrToImage = (uchar*)DataMgr::GetInstance()->GetRecordData(nImageSize);
            //Logger::GetInstance()->GenerateXMLLog(pPtrToImage, nImageSize);
        }
        break;
    case MEMORY_MAP_1:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetRecordData());
            IMemMgr* pMemMgr = IMemMgr::GetMemoryManager(0);
            nImageSize = 1024 * 128;
            pMemMgr->ExportMemoryMetrics((char*)pPtrToImage, nImageSize);
            // Shouldnt be Hardcoded.
        }
        break;
    case MEMORY_MAP_2:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetRecordData());
            IMemMgr* pMemMgr = IMemMgr::GetMemoryManager(1);
            nImageSize = 1024 * 128;
            pMemMgr->ExportMemoryMetrics((char*)pPtrToImage, nImageSize);
            // Shouldnt be Hardcoded.
        }
        break;
    case MEMORY_MAP_3:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetRecordData());
            IMemMgr* pMemMgr = IMemMgr::GetMemoryManager(2);
            nImageSize = 1024 * 128;
            pMemMgr->ExportMemoryMetrics((char*)pPtrToImage, nImageSize);
            // Shouldnt be Hardcoded.
        }
        break;
    case MEMORY_MAP_4:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetRecordData());
            IMemMgr* pMemMgr = IMemMgr::GetMemoryManager(3);
            nImageSize = 1024 * 128;
            pMemMgr->ExportMemoryMetrics((char*)pPtrToImage, nImageSize);
            // Shouldnt be Hardcoded.
        }
        break;
    case PD_HISTORY:
        {
            pPtrToImage = static_cast<uchar*>(pDM->GetRecordData());
            IMemMgr* pMemMgr = IMemMgr::GetMemoryManager(2);
            nImageSize = 1024 * 128;
            pMemMgr->ExportMemoryMetrics((char*)pPtrToImage, nImageSize);
            // Shouldnt be Hardcoded.
        }
        break;
        //case 1000:
        //{
        //    pPtrToImage = (uchar*)pDM->GetLastDoneBuffer();

        //} break;
    case SCALE_IMAGE_1:
        {
            pPtrToImage = (uchar*)pDM->GetCALImages();
            nImageSize = nImageSize * sizeof(short);
        }
        break;
    case SCALE_IMAGE_2:
        {
            pPtrToImage = (uchar*)pDM->GetCALImages() + nImageSize * sizeof(short);
            nImageSize = nImageSize * sizeof(short);
        }
        break;
    case SCALE_IMAGE_3:
        {
            pPtrToImage = (uchar*)pDM->GetCALImages() + nImageSize * sizeof(short) * 2;
            nImageSize = nImageSize * sizeof(short);
        }
        break;
    case FEATURE_DATA:
        {
            // 887 features
            // For DL, feats only contain spoof metric
            pPtrToImage = static_cast<uchar*>(pDM->GetSpoofFeatureVectors());
            nImageSize = (pDM->GetSpoofFeatureVectorsSize()) * sizeof(float);
        }
        break;
    case TIMING_DATA:
        {
            //TODO: Not implemented
            /*int32_t const* p_times = NULL;
            uint32_t num_times = Performance::GetTimes(&p_times);
            pPtrToImage = (uchar*)(p_times);
            nImageSize = num_times * sizeof(int32_t);*/
        }
        break;
    case IMAGE_SA_METADATA:
        {
            if (ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
            {
                const auto* SIA_table = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>()->GetSIAStack();
                const auto* image = SIA_table->Get_Acq_Frame(nAcqStep);
                _V100_IMAGE_METADATA metadata{};
                if (image)
                {
                    metadata.nWidth     = image->GetWidth();
                    metadata.nHeight    = image->GetHeight();
                    metadata.nImageSz   = image->GetImageSize();
                    metadata.nLedState  = image->GetLEDState();
                    metadata.nAcqIdx    = image->GetAcqIndex();
                    metadata.nFrameNum  = image->GetFrameNo();
                    metadata.nExp       = image->GetExp();
                    metadata.nGain      = image->GetGain();
                    metadata.nLuminance = image->GetLuminance();
                }
                if (SIA_table)
                {
                    metadata.nAcqSteps = SIA_table->GetNumberOfSteps();
                }
                pPtrToImage = (uchar*)pDM->GetImage(IMAGE_DARK_FRAME);
                memset(pPtrToImage, 0, sizeof(_V100_IMAGE_METADATA));
                memcpy(pPtrToImage, &metadata, sizeof(_V100_IMAGE_METADATA));
                nImageSize = sizeof(_V100_IMAGE_METADATA);
            }
            else
            {
                return CExecStatus::CMD_ERR_NOT_SUPPORTED;
            }
        }
        break;
    case IMAGE_GET_TEST_RESULTS:
        {
            uint8_t* pResultsData = {};
            unsigned int nResultsData = 0;
            fprintf(stdout, "Getting Script output data...\n");
            if (GetScriptOutputBin(&pResultsData, nResultsData) != CExecStatus::CMD_EXEC_OK)
            {
                fprintf(stdout, "No Output Data Available!\n");
                return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
            }
            fprintf(stdout, "Grabbing output data out of dark frame buffer...\n");
            pPtrToImage = (uchar*)pDM->GetImage(IMAGE_DARK_FRAME);
            memset(pPtrToImage, 0, nResultsData);
            memcpy(pPtrToImage, pResultsData, nResultsData);
            nImageSize = nResultsData;

        }
        break;
    case IMAGE_GET_SENSOR_CONFIG:
        {
            uint8_t* pConfigData = {};
            unsigned int nConfigData = 0;
            if (GetSensorConfig(&pConfigData, nConfigData) != CExecStatus::CMD_EXEC_OK)
            {
                return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
            }
            pPtrToImage = (uchar*)pDM->GetImage(IMAGE_DARK_FRAME);
            memset(pPtrToImage, 0, nConfigData);
            memcpy(pPtrToImage, pConfigData, nConfigData);
            nImageSize = nConfigData;
        }
        break;
    case IMAGE_QUALITY:
    case IMAGE_DARK_FRAME_STATE_1:
    case IMAGE_DARK_FRAME_STATE_2:
    default:
        {
            pPtrToImage = nullptr;
            nImageSize = 0;
        }break;
    }
    if (pPtrToImage != nullptr)
    {
        *pImageBuffer = pPtrToImage;
        //nImageSizeOut = nImageSize;
    }
    else
    {
        nImageSize = 0;// make sure we return 0 incase of no buffer.
    }
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Spoof_Score(int& nSpoofScore)
{
    nSpoofScore = ISensorInstance::GetInstance()->GetDataMgr()->GetLastSpoofScore();
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Get_Template(u8** pTemplate, uint& nTemplateSize)
{
    *pTemplate = nullptr;
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    uint template_size = 0;

    if (pDM->GetProbeTemplate() == nullptr || pDM->GetProbeTemplateSize() == 0)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }


    BioTemplate Tpl(pDM->GetProbeTemplate(), pDM->GetProbeTemplateSize());
    u8* p_template = Tpl.GetOutModeTemplate(template_size);
    if (!p_template) return CExecStatus::CMD_ERR_BAD_DATA;

    // when BioTemplate goes out of scope, the memory it allocates will be
    // automatically freed.  must allocate another buffer and copy the
    // data over.  the caller has the responsibility of freeing the buffer.
    //
    *pTemplate = static_cast<u8*>(MALLOC(template_size));
    if (*pTemplate == nullptr)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    memcpy(*pTemplate, p_template, template_size);
    nTemplateSize = template_size;

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Set_Template(u8* pTemplate, uint nTemplateSize)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    BioTemplate gallery(pTemplate, nTemplateSize, BioParameters::GetInstance().GetTemplateMode());
    uint nDevModeGalleryTplSz = 0;
    u8* pDevModeGalleryTpl = gallery.GetDevModeTemplate(nDevModeGalleryTplSz);
    if (!pDevModeGalleryTpl) return CExecStatus::CMD_ERR_BAD_DATA;


    if (false == pDM->SetGalleryTemplate(pDevModeGalleryTpl, nDevModeGalleryTplSz))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Config_COM_Port(uint nBaudRate)
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::Execute_Get_FIR_Image(_V100_FIR_RECORD_TYPE FIRType,
                                                      _V100_FINGER_PALM_POSITION FingerType, uchar** pFIRImage,
                                                      uint* nFIRImageSz)
{
    auto* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    auto* image = data_mgr->GetCompositeImage();
    uint X, Y, P;

    uint out_FIR_size;
    uint BPP = 8;
    uint impression_type = 0;


    data_mgr->GetScaledDims(X, Y, P);
    if (FIRecordAPI::GetFIRecord(image, X, Y, BPP, FIRType, FingerType, impression_type, pFIRImage, out_FIR_size) !=
        FORMAT_CONVERSION_OK)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    *nFIRImageSz = out_FIR_size;
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Verify_378(u8* pTemplate, uint nTemplateSize)
{
    DataMgr* data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    TAdapter adapter;
    uchar* dest_tpl = nullptr;
    uint dest_tpl_size = 0;
    if (adapter.AdaptTo378(BioParameters::GetInstance().GetTemplateMode(), pTemplate, nTemplateSize, &dest_tpl,
                           &dest_tpl_size) == false)
    {
        return CExecStatus::CMD_ERR_BAD_DATA;
    }
    if (CmdExecutiveBase::CreateAndPostMacroMessage(CMD_VERIFY_378, dest_tpl, dest_tpl_size, App_Busy_Macro) == false)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus CmdExecutiveCommon::Execute_Truncate_378(uint nSizeRequested, uchar* pTplIn, uint nTplInSize,
                                                     uchar* pTplOut,
                                                     uint& nTplOutSize)
{
    TAdapter adapter;
    uchar *dest_tpl = nullptr, *out_tpl = nullptr, *return_tpl_buffer = nullptr;
    uint dest_tpl_size = 0, out_tpl_size = 0;
    auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();

    // Convert pTplIn to 378. (out: dest_tpl)
    if (false == adapter.AdaptTo378(BioParameters::GetInstance().GetTemplateMode(), pTplIn, nTplInSize, &dest_tpl,
                                    &dest_tpl_size))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    dest_tpl_size = dest_tpl_size - 4;

    auto pStrippedTemplate = new uchar[2048]{};

    // Convert dest_tpl to pStrippedTemplate
    if (FORMAT_CONVERSION_OK != StripANSITemplate(dest_tpl, pStrippedTemplate, dest_tpl_size))
    {
        delete[] pStrippedTemplate;
        pStrippedTemplate = nullptr;
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    out_tpl = new uchar[4096]{};

    // Truncate pStrippedTemplate to out_tpl (ANSI 378) with nSizeRequested.
    if (FORMAT_CONVERSION_OK != TruncateANSITemplate(pStrippedTemplate, out_tpl, nSizeRequested, 1))
    {
        delete[] pStrippedTemplate;
        pStrippedTemplate = nullptr;
        delete[] out_tpl;
        out_tpl = nullptr;
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    out_tpl_size = nSizeRequested;
    return_tpl_buffer = new uchar[4096]{};

    // Adapt back from out_tpl to return_tpl_buffer, but fill the nTplOutSize. This is due to TAdapter will *AUTOMATICALLY* delete the buffer.
    if (false == adapter.ConvertFrom378(BioParameters::GetInstance().GetTemplateMode(), out_tpl, out_tpl_size,
                                        &return_tpl_buffer, &nTplOutSize))
    {
        delete[] pStrippedTemplate;
        pStrippedTemplate = nullptr;
        delete[] out_tpl;
        out_tpl = nullptr;
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    // Avoid TAdapter auto delete.
    memcpy(pTplOut, return_tpl_buffer, nTplOutSize);

    delete[] pStrippedTemplate;
    delete[] out_tpl;
    pStrippedTemplate = nullptr;
    out_tpl = nullptr;
    nTplOutSize = nSizeRequested;
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Enc_Clear()
{
    ISensorInstance::GetInstance()->GetDataMgr()->ClearAllData();
    if (ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc() != nullptr)
    {
        ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc()->ClearEnroll();
    }
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCommon::Execute_Baud_Rate_Change()
{
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    // TODO: N/A in SEngine.
    //if (ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP())
    //{
    //    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    //    uint nBaudRate = pDM->GetBaudRate();
    //    IUART* pUART = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>()->GetUART();
    //    pUART->SetBaudRate(nBaudRate);
    //    return CExecStatus::CMD_EXEC_OK;
    //}
}


_MX00_ID_RESULT* CmdExecutiveCommon::Execute_Get_Result()
{
    return ISensorInstance::GetInstance()->GetDataMgr()->GetIDResult();
}

CExecStatus CmdExecutiveCommon::Execute_Get_Interface_Cal(_V100_INTERFACE_CALIBRATION_TYPE** pCal)
{
    if (sensor_is_venus_map_.at(ISensorInstance::GetInstance()->GetCfgMgr()->GetSensorType()))
    {
        *pCal = ISensorInstance::GetInstance()->GetDataMgr()->GetCAL();
        return CExecStatus::CMD_EXEC_OK;
    }
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CmdExecutiveCommon& CmdExecutiveCommon::GetInstance()
{
    static CmdExecutiveCommon instance;
    return instance;
}

_VX00_DSM_EEPROM_DATA* CmdExecutiveCommon::GetDSM_EEPROM()
{
    const auto is_streaming = ISensorInstance::GetInstance()->GetBSP()->IsStreamingBSP();
    auto data_mgr = ISensorInstance::GetInstance()->GetDataMgr();
    if (is_streaming)
    {
        auto bsp = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IStreamBSP>();
        _VX00_DSM_EEPROM_DATA* pDSM_EEPROM = data_mgr->GetDSM_EEPROM();
        // Refresh DataMgr copy of eeprom
        if (ISensorInstance::GetInstance()->GetConfiguration()->GetSensorType() == V310_10_Type)
        {
            EEPROM_Format_Type const* pEEPROM = bsp->GetEEPROMData();

            // Manually copy over all struct members due to potential alignment issues
            pDSM_EEPROM->type = pEEPROM->type;
            pDSM_EEPROM->VID_LSB = pEEPROM->VID_LSB;
            pDSM_EEPROM->VID_MSB = pEEPROM->VID_MSB;
            pDSM_EEPROM->PID_LSB = pEEPROM->PID_LSB;
            pDSM_EEPROM->PID_MSB = pEEPROM->PID_MSB;
            pDSM_EEPROM->DEVID_LSB = pEEPROM->DEVID_LSB;
            pDSM_EEPROM->DEVID_MSB = pEEPROM->DEVID_MSB;
            pDSM_EEPROM->CONFIG = pEEPROM->CONFIG;
            pDSM_EEPROM->Serial_Number = pEEPROM->Serial_Number;

            pDSM_EEPROM->CPLD_Firmware_Revision = pEEPROM->CPLD_Firmware_Revision;

            pDSM_EEPROM->ManDateCode = pEEPROM->ManDateCode;
            pDSM_EEPROM->Product_ID = pEEPROM->Product_ID;
            pDSM_EEPROM->Platform_Type = pEEPROM->Platform_Type;

            pDSM_EEPROM->Bx_Row = pEEPROM->Bx_Row;
            pDSM_EEPROM->Bx_Col = pEEPROM->Bx_Col;
            pDSM_EEPROM->PD_Row = pEEPROM->PD_Row;
            pDSM_EEPROM->PD_Col = pEEPROM->PD_Row;
            pDSM_EEPROM->DPI = pEEPROM->DPI;

            pDSM_EEPROM->MfgStateFlag = pEEPROM->MfgStateFlag;

            memcpy(pDSM_EEPROM->pCalData, pEEPROM->pCalData, sizeof(pEEPROM->pCalData));
            memcpy(&pDSM_EEPROM->pTagData, pEEPROM->pTagData, sizeof(pEEPROM->pTagData));
        }
        else
        {
            EEPROM_Unified_Format_Type const* pEEPROM = bsp->GetEEPROM();

            // Manually copy over all struct members due to potential alignment issues
            pDSM_EEPROM->type = pEEPROM->cypress.type;
            pDSM_EEPROM->VID_LSB = pEEPROM->cypress.VID_LSB;
            pDSM_EEPROM->VID_MSB = pEEPROM->cypress.VID_MSB;
            pDSM_EEPROM->PID_LSB = pEEPROM->cypress.PID_LSB;
            pDSM_EEPROM->PID_MSB = pEEPROM->cypress.PID_MSB;
            pDSM_EEPROM->DEVID_LSB = pEEPROM->cypress.DEVID_LSB;
            pDSM_EEPROM->DEVID_MSB = pEEPROM->cypress.DEVID_MSB;
            pDSM_EEPROM->CONFIG = pEEPROM->cypress.CONFIG;
            pDSM_EEPROM->Serial_Number = pEEPROM->platform.Serial_Number;

            // Not implemented in the M210 EEPROM
            pDSM_EEPROM->CPLD_Firmware_Revision = 0;

            pDSM_EEPROM->ManDateCode = pEEPROM->platform.ManDateCode;
            pDSM_EEPROM->Product_ID = pEEPROM->platform.Product_ID;
            pDSM_EEPROM->Platform_Type = pEEPROM->platform.Platform_Type;

            // Not implemented in the M210 EEPROM ... or is it?
            pDSM_EEPROM->Bx_Row = 0;
            pDSM_EEPROM->Bx_Col = 0;
            pDSM_EEPROM->PD_Row = 0;
            pDSM_EEPROM->PD_Col = 0;
            pDSM_EEPROM->DPI = 0;

            pDSM_EEPROM->MfgStateFlag = pEEPROM->platform.MfgStateFlag;

            // Not implemented in the M210 EEPROM
            memset(pDSM_EEPROM->pCalData, 0, sizeof(pDSM_EEPROM->pCalData));

            pDSM_EEPROM->pTagData[0] = pEEPROM->platform.nTagDataSize;
            memcpy(&pDSM_EEPROM->pTagData[1], pEEPROM->platform.pTagData, pEEPROM->platform.nTagDataSize);
        }
        return pDSM_EEPROM;
    }
    auto bsp = ISensorInstance::GetInstance()->GetBSP()->GetBSP<IBSP>();
    // Grab the EEPROM
    IEEPROM* pEEPROM = bsp->GetEEPROM();
    // Grab the EEPROM Data from BSP
    DSM_EEPROM_Calibration* pBSPEEPROM = pEEPROM->GetDSMSettings();
    // Grab the Data Managers EEPROM structure
    _VX00_DSM_EEPROM_DATA* pEEPROMSt = data_mgr->GetDSM_EEPROM();

    *pEEPROMSt = *((_VX00_DSM_EEPROM_DATA*)(pBSPEEPROM));

    return pEEPROMSt;
}




void CmdExecutiveCommon::Tokenize(std::string const& str, const char delim, std::vector<std::string>& out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}


//////////////////////////////////////////////////////////////////////////////////////
/// Private Helper Functions                                                        //
//////////////////////////////////////////////////////////////////////////////////////
CExecStatus CmdExecutiveCommon::ExecuteShellCommand(std::string script_cmd_line)
{
#ifdef _YOCTO_LINUX_
    using namespace std;
    // Parse out any arguments
    vector<string> cmd_line_args;
    Tokenize(script_cmd_line, ' ', cmd_line_args);

    // Make sure script/executable exists.
    struct stat buffer {};
    if (0 == (stat(cmd_line_args[0].c_str(), &buffer) == 0))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    RemoteExec remote_exec;

    remote_exec.set_exec_callback(function<RemoteExecStatusCallbackPrototype>() = [](const string& str)->void {
        fprintf(stdout, str.c_str());
    });
    fprintf(stdout, "before run\n");
    remote_exec.execute_shell_command(cmd_line_args, 60);

    while (remote_exec.get_status() == 0)
    {
        fprintf(stdout, "running...\n");
        this_thread::sleep_for(chrono::seconds(1));
    }
    fprintf(stdout, "run finished\n");

    auto ret_code = remote_exec.get_return_code();
    if (ret_code != 0)
    {
        return CExecStatus::CMD_ERR_REMOTE_EXEC_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
#endif
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



CExecStatus CmdExecutiveCommon::GetSensorConfig(uint8_t** p_config_data, uint32_t& size_sensor_cfg)
{
#ifdef _YOCTO_LINUX_
    //info("Read in SensorConfig.json file");
    const char* pFileName = "/etc/HID/SensorConfig.json";

    FILE* fp = fopen(pFileName, "r");
    if (NULL == fp)
    {
        //info("ERROR: Reading config file: /etc/HID/SensorConfig.json");
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    fseek(fp, 0, SEEK_END);
    size_sensor_cfg = ftell(fp);

    *p_config_data = new uint8_t[size_sensor_cfg];

    fseek(fp, 0, SEEK_SET);
    fread(*p_config_data, 1, size_sensor_cfg, fp);
    fclose(fp);

    return CExecStatus::CMD_EXEC_OK;
#endif
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

CExecStatus CmdExecutiveCommon::SetSensorConfig(u8* p_sensor_cfg, uint size_sensor_cfg)
{
#ifdef _YOCTO_LINUX_
    //info("Save configuration to SensorConfig.json file");
    const char* pFileName = "/etc/HID/SensorConfig.json";

    return WriteFile(pFileName, p_sensor_cfg, size_sensor_cfg);
#endif
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}


CExecStatus CmdExecutiveCommon::WriteFile(const char* p_file_path, u8* p_file_data, uint size_file_data)
{
#ifdef __linux__
    int fp = open(p_file_path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0600);
    if (-1 == fp)
    {
        //err("ERROR: Opening sensor config file %s", pFileName);
        //err("  error %d (%s)", errno, strerror(errno));
        //return -1;
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    size_t bytes_written = write(fp, p_file_data, size_file_data);

    if (bytes_written != size_file_data)
    {
        //err("ERROR: Bytes written (%d) to sensor config file do not match expected (%d)",
        //    bytes_written, config_data_size);
        //err("  error %d (%s)", errno, strerror(errno));
        //close(fp);
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    if (-1 == fsync(fp))
    {
        //err("ERROR: fsync() failed after writing sensor config file");
        //err("  error %d (%s)", errno, strerror(errno));
        //close(fp);
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    if (-1 == close(fp))
    {
        //err("ERROR: close() failed after writing sensor config file");
        //err("  error %d (%s)", errno, strerror(errno));
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    // required (with fsync() above) to ensure that not only the data gets
    // to physical memory, but the directory entry as well
    DIR* dp = opendir("/etc/HID");
    if (dp == NULL)
    {
        //err("ERROR: opendir() failed on /etc/HID");
        //err("  error %d (%s)", errno, strerror(errno));
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    if (-1 == fsync(dirfd(dp)))
    {
        //err("ERROR: fsync() failed on /etc/HID");
        //err("  error %d (%s)", errno, strerror(errno));
        //closedir(dp);
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    if (-1 == closedir(dp))
    {
        //err("ERROR: closedir() failed on /etc/HID");
        //err("  error %d (%s)", errno, strerror(errno));
        //return -1;
        return CExecStatus::CMD_ERR_BAD_DATA;
    }

    //info("Sensor configuration written");
    return CExecStatus::CMD_EXEC_OK;
#endif
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}

//CD - <TODO>: remove
//CExecStatus CmdExecutiveCommon::Verify(u8* pInData, _V100_ENC_VERIFY_RESULT_HDR* pVerifyResultHdr, uint* pVerifyScores)
//{
//    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
//    //memcpy(pVerifyResultHdr->nANSOL, &pVerifyResultHdr->nANSOL, ANSOL_SIZE); // TODO: IMPLEMENT ENCRYPTION.
//}
