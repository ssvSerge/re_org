/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2017 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
**                                                                                                                      **
** For a list of applicable patents and patents pending, visit www.hidglobal.com/patents                                **
**                                                                                                                      **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                           **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS                                     **
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR                                       **
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER                                       **
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                                              **
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                           **
**                                                                                                                      **
*************************************************************************************************************************/
#ifndef WIN32
    #include "windows_conv.h"
#else
    #include <windows.h>
#endif

#include <iostream>
#include <cassert>
#include <iterator>

#include "VCOMBase.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "IMemMgr.h"

#include <HFTypesPrivate.h>

#ifndef __GNUC__
#include "V100DeviceHandler.h"
EXTERN_C const GUID GUID_BULKLDI;
#endif

static HFResult   g_hfResult;

typedef struct tag_v100_hf_result {

}   v100_hf_result_t;

#define PERIOD 1
#define ROUTE_JENGINE        (0)
#define ROUTE_USBTRANSCEIVER (1)

V100_ERROR_CODE V100_Cancel_Operation(const V100_DEVICE_TRANSPORT_INFO* pDev) {

    Atomic_Cancel_Operation* pCommand = reinterpret_cast<Atomic_Cancel_Operation*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_CANCEL_OPERATION));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));
    
    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Cancel_Operation*>(pResponse);
    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }
    return GEN_OK;
}

V100_ERROR_CODE V100_Close(V100_DEVICE_TRANSPORT_INFO *pDev) {
    V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
    pVCH->Close(pDev);
    delete pVCH;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Acq_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ACQ_STATUS_TYPE* pACQ_Status) {

    Atomic_Get_Acq_Status* pCommand = reinterpret_cast<Atomic_Get_Acq_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_ACQ_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), 0, pCommand));
    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Get_Acq_Status*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    *pACQ_Status = pCommand->GetAcqStatus();
    delete pCommand;

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Config(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_CONFIGURATION_TYPE* ICT) {

    Atomic_Get_Config* pCommand = reinterpret_cast<Atomic_Get_Config*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_CONFIG));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Get_Config*>(pResponse);
    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    *ICT = pCommand->GetConfiguration();
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Sensor_Type(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SENSOR_TYPE& sensorType) {

    sensorType = UNKNOWN_LUMI_DEVICE;
    _V100_INTERFACE_CONFIGURATION_TYPE config;
    memset(&config, 0, sizeof(_V100_INTERFACE_CONFIGURATION_TYPE));

    V100_ERROR_CODE rc = GEN_OK;
    rc = V100_Get_Config(pDev, &config);
    if (rc != GEN_OK) {
        return rc;
    }

    // Figure out the Sensor Type
    // First check the VID/PID combo to figure out what the device is
    if (config.Vendor_Id == LUMI_VID_V_SERIES_EMBEDDED) {
        if (config.Product_Id == LUMI_PID_V_SERIES_EMBEDDED) {
            sensorType = VENUS_V30X;

            // Check to see if it is a V40x or a V42x
            if (config.Hardware_Rev == 400) {
                sensorType = VENUS_V40X;
                return rc;
            } else 
            if (config.Hardware_Rev == 420) {
                sensorType = VENUS_V42X;
            }
        } else {
            return rc;
        }
    } else 
    if (config.Vendor_Id == LUMI_VID_NON_V_SERIES_EMBEDDED) {
        if (config.Product_Id == LUMI_PID_M_SERIES_EMBEDED) {
            sensorType = MERCURY_M30X;
            if (config.Hardware_Rev == 320) {
                sensorType = MERCURY_M32X;
            } else 
            if (config.Hardware_Rev == 420) {
                sensorType = MERCURY_M42X;
            }
        } else 
        if (config.Product_Id == LUMI_PID_M_SERIES_STREAMING) {
            sensorType = MERCURY_M31X;
        } else 
        if (config.Product_Id == LUMI_PID_V_SERIES_STREAMING) {
            sensorType = VENUS_V31X;
            if (config.Hardware_Rev == 371) {
                sensorType = VENUS_V371;
            }
        } else 
        if (config.Product_Id == LUMI_PID_M210_SERIES_STREAMING) {
            sensorType = MERCURY_M21X;
        } else 
        if (config.Product_Id == LUMI_PID_V310_10_SERIES_STREAMING) {
            sensorType = VENUS_V310_10;
        } else {
            return rc;
        }
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Num_USB_Devices(int* nNumDevices) {
    #ifdef __GNUC__
        *nNumDevices = 1;
    #elif _WIN32_WCE
        *nNumDevices = 1;
    #else
        V100_ERROR_CODE rc = V100DeviceHandler::GetV100DeviceHandler()->GetNumDevices(nNumDevices);
        V100DeviceHandler::ReleaseV100DeviceHandler();
        return rc;
    #endif
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_OP_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_OP_STATUS& opStatus) {

    Atomic_Get_OP_Status* pCommand = reinterpret_cast<Atomic_Get_OP_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_OP_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Get_OP_Status*>(pResponse);
    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }
    pCommand->GetOPStatus(&opStatus);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Serial(const V100_DEVICE_TRANSPORT_INFO* pDev, u32* pSerialNumber) {

    Atomic_Get_Serial* pCommand = reinterpret_cast<Atomic_Get_Serial*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_SERIAL_NUMBER));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr;
        if (arg == 0) {
            return GEN_ERROR_INVALID_CMD;
        } else {
            return (V100_ERROR_CODE)(arg);
        }
    }

    pCommand = dynamic_cast<Atomic_Get_Serial*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    *pSerialNumber = pCommand->GetSerialNumber();
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Status(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_INTERFACE_STATUS_TYPE* pIST) {

    Atomic_Get_Status* pCommand = reinterpret_cast<Atomic_Get_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);

    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Get_Status*>(pResponse);
    *pIST = pCommand->GetInterfaceStatusType();
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_GetSystemDiagnostics(V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SYSTEM_DIAGNOSTICS* pSysDiag) {

    Atomic_Get_System_State* pCommand = reinterpret_cast<Atomic_Get_System_State*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand((_V100_COMMAND_SET)CMD_GET_SYSTEM_STATE));
    ICmd* pResponse = reinterpret_cast<Atomic_Get_System_State*>(V100CommandHandler::GetCommandHandler(pDev)->IssueCommand(pDev, ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)arg;
    }

    pCommand = reinterpret_cast<Atomic_Get_System_State*>(pResponse);
    _V100_SYSTEM_DIAGNOSTICS* pRespBuff = pCommand->GetSystemMetricsStruct();
    memcpy(pSysDiag, pRespBuff, sizeof(_V100_SYSTEM_DIAGNOSTICS));
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTag, u16& nTagLength) {

    Atomic_Get_Tag* pCommand = reinterpret_cast<Atomic_Get_Tag*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_TAG));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);

    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Get_Tag*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    char*  pTagToGet = NULL;
    pCommand->GetTag(&pTagToGet, nTagLength);
    memcpy(pTag, pTagToGet, nTagLength);
    delete pCommand;

    return GEN_OK;
}

V100_ERROR_CODE V100_Open(V100_DEVICE_TRANSPORT_INFO *pDev) {

    V100_ERROR_CODE rc = GEN_OK;

    #ifdef _WIN32
    #ifndef _WIN32_WCE

        Device* pDevice = new Device();

        rc = V100DeviceHandler::GetV100DeviceHandler()->GetDevice(pDev->DeviceNumber, pDevice);
        if (GEN_OK != rc && pDev->nBaudRate == 0)
        {
            delete pDevice;
            pDevice = NULL;
            pDev->hInstance = NULL;
            return rc;
        }

        // The Device object is now the hInstance
        pDev->hInstance = pDevice;

    #endif
    #endif

    rc = GEN_OK;
    V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
    if (0 != pVCH->Initialize(pDev)) {
        pVCH->Close(pDev);
        delete pVCH;
        rc = GEN_ERROR_START;
    }

    #ifdef _WIN32
    #ifndef _WIN32_WCE

        delete pDevice;
        pDevice = NULL;

        V100DeviceHandler::ReleaseV100DeviceHandler();

    #endif
    #endif

    return rc;
}

V100_ERROR_CODE V100_Reset(const V100_DEVICE_TRANSPORT_INFO* pDev) {

    Atomic_Reset* pCommand = reinterpret_cast<Atomic_Reset*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_RESET));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Reset*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; 
            return GEN_ERROR_INTERNAL; 
    }

    delete pCommand;

    return GEN_OK;
}

V100_ERROR_CODE V100_Release_Memory(const V100_DEVICE_TRANSPORT_INFO* pDev, void* pMem) {

    if (NULL != pMem) {
        FREE(pMem);
        pMem = NULL;
    }

    return GEN_OK;

}

V100_ERROR_CODE V100_Set_Tag (const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTagData, u16 nTagDataSize) {

    Atomic_Set_Tag* pCommand = reinterpret_cast<Atomic_Set_Tag*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_SET_TAG));
    pCommand->SetTag((char*)pTagData, nTagDataSize);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Set_Tag*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    delete pCommand;

    return GEN_OK;
}

// ========================================================================================================================
// ** CRYPTO CALLS **
// ========================================================================================================================

V100_ERROR_CODE V100_Enc_Clear(const V100_DEVICE_TRANSPORT_INFO *pDev) {
    Atomic_Enc_Clear* pCommand = reinterpret_cast<Atomic_Enc_Clear*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_CLEAR));
    if (pCommand == NULL) {
        return GEN_ERROR_MEMORY;
    }
    return SendEncCommandNoResponse(pDev, ROUTE_JENGINE, pCommand);
}

V100_ERROR_CODE V100_Enc_Generate_RSA_Keys(const V100_DEVICE_TRANSPORT_INFO *pDev) {
    Macro_Enc_Generate_RSA_Keys* pCommand = reinterpret_cast<Macro_Enc_Generate_RSA_Keys*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GENERATE_RSA_KEYS));
    if (pCommand == NULL) {
        return GEN_ERROR_MEMORY;
    }
    return SendEncCommandNoResponse(pDev, ROUTE_JENGINE, pCommand);
}

V100_ERROR_CODE V100_Enc_Get_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u16& nKeyVersion, u8* pKCV, u16& nKeySize, u16& nKeyMode) {

    Atomic_Enc_Get_Key* pCommand = reinterpret_cast<Atomic_Enc_Get_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_KEY));
    pCommand->SetArguement(nKeyType);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_Get_Key*>(pResponse);
    if (pCommand == NULL) { 
        delete pResponse; 
        return GEN_ERROR_INTERNAL; 
    }

    nKeyVersion = pCommand->GetKeyVersion();
    nKeyMode    = pCommand->GetKeyMode();
    nKeySize    = pCommand->GetKeyLength();
    memcpy(pKCV, pCommand->GetKCV(), KCV_LENGTH);
    memcpy(pKey, pCommand->GetKey(), nKeySize);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Get_KeyVersion(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeySlot, u16& nKeyVersion, u8* pKCV, u16& nKeyMode) {

    Atomic_Enc_Get_KeyVersion* pCommand = reinterpret_cast<Atomic_Enc_Get_KeyVersion*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_KEYVERSION));
    pCommand->SetKeySlot(nKeySlot);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));

    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) {
        short arg; 
        pErr->GetArguement(arg); 
        delete pErr; 
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_Get_KeyVersion*>(pResponse);

    if (pCommand == NULL) { 
        delete pResponse; return GEN_ERROR_INTERNAL; 
    }

    nKeyVersion = pCommand->GetKeyVersion();
    nKeyMode    = pCommand->GetKeyMode();
    memcpy(pKCV, pCommand->GetKCV(), KCV_LENGTH);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Get_Serial_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u64& SerialNumber) {
    Atomic_Enc_Get_Serial_Number* pCommand = reinterpret_cast<Atomic_Enc_Get_Serial_Number*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_SERIAL_NUMBER));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_Serial_Number*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    SerialNumber = pCommand->GetSerialNum();
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Get_Rnd_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u256 rndNumber, int nBytesRequested) {
    if (nBytesRequested > 32) {
        return GEN_ERROR_INVALID_SIZE;
    }
    Atomic_Enc_Get_Rnd_Number* pCommand = reinterpret_cast<Atomic_Enc_Get_Rnd_Number*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_RND_NUMBER));
    pCommand->SetArguement(2);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));
    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  {// Handle Error
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_Get_Rnd_Number*>(pResponse);
    if (pCommand == NULL) {
        delete pResponse; return GEN_ERROR_INTERNAL;
    }

    memset(rndNumber, 0, sizeof(u256));
    memcpy(rndNumber, pCommand->GetRandomNumber(), nBytesRequested);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Set_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pCGKey, uint nCGKeySize, _V100_ENC_KEY_TYPE nKeyType, u8** pCGOutputData, u32& nCGOutputDataSize) {

    Atomic_Enc_Set_Key* pCommand = reinterpret_cast<Atomic_Enc_Set_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_SET_KEY));

    pCommand->SetBuffer(pCGKey, nCGKeySize);
    pCommand->SetKeyType(nKeyType);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), ROUTE_JENGINE, pCommand));
    if (pResponse == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) { // Handle Error
        short arg;
        pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_Set_Key*>(pResponse);
    if ( pCommand == NULL ) {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }

    uint nSizeReturnedCG = pCommand->GetBufferSize();
    *pCGOutputData = NULL;
    *pCGOutputData = (u8*)MALLOC(nSizeReturnedCG);

    if (*pCGOutputData == NULL) {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }

    memcpy ( *pCGOutputData, pCommand->GetBuffer(), nSizeReturnedCG );
    nCGOutputDataSize = nSizeReturnedCG;

    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Unlock_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, const u8* pCGInputData, u32 nCGInputDataSize) {
    Atomic_Enc_Unlock_Key* pCommand = reinterpret_cast<Atomic_Enc_Unlock_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_UNLOCK_KEY));
    if (pCommand == NULL) return GEN_ERROR_MEMORY;
    pCommand->SetCryptogram(pCGInputData, nCGInputDataSize);
    return SendEncCommandNoResponse(pDev, ROUTE_JENGINE, pCommand);
}

// ========================================================================================================================
// ** JAGUAR CALLS **
// ========================================================================================================================

template<typename T>
static V100_ERROR_CODE V100_Hid_Prepare_Request(const V100_DEVICE_TRANSPORT_INFO* const pDev, _V100_COMMAND_SET iCmdId, T** pCmdInst) {

    ICmd*   pCmd    = nullptr;
    T*      pCmdOut = nullptr;

    // Function intended for internal use only. 
    assert(nullptr != pCmdInst);
    *pCmdInst = nullptr;

    pCmd = V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(iCmdId);

    if (nullptr == pCmd) {
        // Cannot allocate memory for ICMD instance.
        assert(nullptr != pCmd);
        return GEN_ERROR_MEMORY;
    }

    pCmdOut = dynamic_cast<T*>(pCmd);
    assert(nullptr != pCmdOut);

    if (nullptr == pCmd) {
        // Strange. Wrong TYPE of instance.
        delete (pCmd);
        pCmd = nullptr;
        return GEN_ERROR_INTERNAL;
    }

    (*pCmdInst) = pCmdOut;
    return GEN_OK;
}

template<typename T>
static V100_ERROR_CODE V100_Hid_Execute_Request(const V100_DEVICE_TRANSPORT_INFO* const pDev, int route_type, T* pInInst, T** pOutInst) {

    Atomic_Error*   pErr    = nullptr;
    ICmd*           pResp   = nullptr;
    T*              pRespIn = nullptr;
    short           errCode = 0;

    // Function intended for internal use only. 
    assert(nullptr != pInInst);
    assert(nullptr != pOutInst);

    (*pOutInst) = nullptr;

    pResp = V100CommandHandler::GetCommandHandler(pDev)->IssueCommand2(pDev, route_type, pInInst);

    if (pResp == NULL) {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    pErr = dynamic_cast<Atomic_Error*>(pResp);
    if (nullptr != pErr) {
        pErr->GetArguement(errCode);
        delete pResp;
        return (V100_ERROR_CODE)(errCode);
    }

    pRespIn = dynamic_cast<T*>(pResp);

    if (nullptr == pRespIn) {
        // Strange. Not an Error and not an Instance.
        assert(false);
        delete pResp;
        return GEN_ERROR_INTERNAL;
    }

    (*pOutInst) = pRespIn;
    return GEN_OK;

}

template<typename T>
static V100_ERROR_CODE V100_Hid_Send_Request(const V100_DEVICE_TRANSPORT_INFO* const pDev, int route_type, _V100_COMMAND_SET iCmdId, T** pCmdInst) {

    V100_ERROR_CODE retVal;
    T*  pCmdOut = nullptr;
    T*  pRespIn = nullptr;

    retVal = V100_Hid_Prepare_Request<T>(pDev, iCmdId, &pCmdOut);

    if (retVal != GEN_OK) {
        delete pCmdOut;
        return retVal;
    }

    retVal = V100_Hid_Execute_Request<T>(pDev, route_type, pCmdOut, &pRespIn);

    delete pCmdOut;

    if (retVal != GEN_OK) {
        delete pRespIn;
        pRespIn = nullptr;
    }

    (*pCmdInst) = pRespIn;
    return retVal;
}

V100_ERROR_CODE V100_Hid_Init(const V100_DEVICE_TRANSPORT_INFO* const pDev) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Init* pCmdInstance = nullptr;

    assert (nullptr != pDev);

    ioRes = V100_Hid_Send_Request<Atomic_Hid_Init> (pDev, ROUTE_JENGINE, CMD_HID_INIT, &pCmdInstance);

    if (ioRes != GEN_OK) {
        return ioRes;
    }

    if (nullptr == pCmdInstance) {
        assert(false);
        return GEN_ERROR_INTERNAL;
    }

    // Process command here.

    delete pCmdInstance;

    return GEN_OK;
}

V100_ERROR_CODE V100_Hid_Terminate(const V100_DEVICE_TRANSPORT_INFO* const pDev) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Terminate* pCmdInstance = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Send_Request<Atomic_Hid_Terminate>(pDev, ROUTE_JENGINE, CMD_HID_TERMINATE, &pCmdInstance);

    if (ioRes != GEN_OK) {
        return ioRes;
    }

    if (nullptr == pCmdInstance) {
        assert(false);
        return GEN_ERROR_INTERNAL;
    }

    // Process command here.

    delete pCmdInstance;

    return GEN_OK;
}

V100_ERROR_CODE V100_Hid_Enum_Cams(const V100_DEVICE_TRANSPORT_INFO* const pDev, string_list_t& str_list) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Enum_Cams* pCmdInstance = nullptr;

    assert(nullptr != pDev);

    str_list.clear();

    ioRes = V100_Hid_Send_Request<Atomic_Hid_Enum_Cams>(pDev, ROUTE_JENGINE, CMD_HID_ENUM_CAMS, &pCmdInstance);

    if (ioRes != GEN_OK) {
        return ioRes;
    }

    if (nullptr == pCmdInstance) {
        assert(false);
        return GEN_ERROR_INTERNAL;
    }

    pCmdInstance->GetCamsList(str_list);

    delete pCmdInstance;
    return GEN_OK;
}

V100_ERROR_CODE V100_Hid_Set_Param_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, int32_t val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Set_Param_Int* pCmdInst  = nullptr;
    Atomic_Hid_Set_Param_Int* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Set_Param_Int>(pDev, CMD_HID_SET_PARAM_INT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, id, val);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Set_Param_Int>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Param_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, int32_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Param_Int* pCmdInst = nullptr;
    Atomic_Hid_Get_Param_Int* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Param_Int>(pDev, CMD_HID_GET_PARAM_INT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(ctx, id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Param_Int>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Set_Param_Str(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, const char* const val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Set_Param_Str* pCmdInst = nullptr;
    Atomic_Hid_Set_Param_Str* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Set_Param_Str>(pDev, CMD_HID_SET_PARAM_STR, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, id, val);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Set_Param_Str>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Param_Str(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, std::string& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Param_Str* pCmdInst = nullptr;
    Atomic_Hid_Get_Param_Str* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Param_Str>(pDev, CMD_HID_GET_PARAM_STR, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(ctx, id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Param_Str>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Set_Param_Bin(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, const uint8_t* const val_ptr, uint32_t val_len) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Set_Param_Bin* pCmdInst = nullptr;
    Atomic_Hid_Set_Param_Bin* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Set_Param_Bin>(pDev, CMD_HID_SET_PARAM_BIN, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, id, val_ptr, val_len);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Set_Param_Bin>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Param_Bin(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, bin_data_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Param_Bin* pCmdInst = nullptr;
    Atomic_Hid_Get_Param_Bin* pRespInst = nullptr;

    assert(nullptr != pDev);

    val.clear();

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Param_Bin>(pDev, CMD_HID_GET_PARAM_BIN, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(ctx, id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Param_Bin>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Set_Param_Lng(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, double val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Set_Param_Long* pCmdInst = nullptr;
    Atomic_Hid_Set_Param_Long* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Set_Param_Long>(pDev, CMD_HID_SET_PARAM_LONG, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, id, val);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Set_Param_Long>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Param_Lng(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, uint32_t id, double& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Param_Long* pCmdInst = nullptr;
    Atomic_Hid_Get_Param_Long* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Param_Long>(pDev, CMD_HID_GET_PARAM_LONG, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(ctx, id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Param_Long>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Open_Context(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t cam_id, uint32_t algo_type, int32_t& ctx) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Open_Context* pCmdInst  = nullptr;
    Atomic_Hid_Open_Context* pRespInst = nullptr;

    assert(nullptr != pDev);

    ctx = 0;

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Open_Context>(pDev, CMD_HID_OPEN_CONTEXT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    pCmdInst->SetParam(cam_id, (HFAlgorithmType)algo_type);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Open_Context>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(ctx);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Close_Context(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Close_Context* pCmdInst = nullptr;
    Atomic_Hid_Close_Context* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Close_Context>(pDev, CMD_HID_CLOSE_CONTEXT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    pCmdInst->SetId(ctx);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Close_Context>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Stop_Operation(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Stop_Operation* pCmdInst = nullptr;
    Atomic_Hid_Async_Stop_Operation* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Stop_Operation>(pDev, CMD_HID_STOP_CMD_ASYNC, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId( (HFOperation)ctx);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Stop_Operation>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Close_Opeation(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Close_Operation* pCmdInst = nullptr;
    Atomic_Hid_Close_Operation* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Close_Operation>(pDev, CMD_HID_CLOSE_OPERATION, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId((HFOperation)ctx);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Close_Operation>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Extract_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int32_t imageEncoding, const void* const img_ptr, uint32_t img_len, uint64_t flags, int32_t& val ) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Extract_Template* pCmdInst = nullptr;
    Atomic_Hid_Async_Extract_Template* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Extract_Template>(pDev, CMD_HID_ASYNC_EXTRACT_TEMPLATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam( (HFContext)ctx, (HFImageEncoding)imageEncoding, img_ptr, img_len, flags);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Extract_Template>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Match_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const void* const templA_ptr, uint32_t len_A, const void* const templB_ptr, uint32_t len_B, int32_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Match_With_Template* pCmdInst = nullptr;
    Atomic_Hid_Async_Match_With_Template* pRespInst = nullptr;

    assert(nullptr != pDev);

    val = -1;

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Match_With_Template>(pDev, CMD_HID_ASYNC_MATCH_WITH_TEMPLATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, templA_ptr, len_A, templB_ptr, len_B);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Match_With_Template>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Verify_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const galery, const char* const id, double minScore) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Verify_With_Captured* pCmdInst = nullptr;
    Atomic_Hid_Async_Verify_With_Captured* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Verify_With_Captured>(pDev, CMD_HID_ASYNC_VERIFY_WITH_CAPTURED, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, galery, id, minScore);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Verify_With_Captured>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Identify_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const gallery, double minScore) {
    
    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Identify_With_Captured* pCmdInst = nullptr;
    Atomic_Hid_Async_Identify_With_Captured* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Identify_With_Captured>(pDev, CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, gallery, minScore);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Identify_With_Captured>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Match_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const uint8_t* const bin, uint32_t len) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Match_With_Captured* pCmdInst = nullptr;
    Atomic_Hid_Async_Match_With_Captured* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Match_With_Captured>(pDev, CMD_HID_ASYNC_MATCH_WITH_CAPTURED, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, bin, len);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Match_With_Captured>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Identify_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, const char* const gal_ptr, double minScore, const uint8_t* const templ_ptr, uint32_t teml_len, int32_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Identify_With_Template* pCmdInst = nullptr;
    Atomic_Hid_Async_Identify_With_Template* pRespInst = nullptr;

    assert(nullptr != pDev);

    val = -1;

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Identify_With_Template>(pDev, CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParams(ctx, gal_ptr, minScore, templ_ptr, teml_len);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Identify_With_Template>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Verify_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, double minScore, const char* const gal_ptr, const char* const id_ptr, const uint8_t* const templ_ptr, uint32_t templ_len, int32_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Async_Verify_With_Template* pCmdInst = nullptr;
    Atomic_Hid_Async_Verify_With_Template* pRespInst = nullptr;

    assert(nullptr != pDev);

    val = -1;

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Async_Verify_With_Template>(pDev, CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParams(ctx, minScore, gal_ptr, id_ptr, templ_ptr, templ_len);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Async_Verify_With_Template>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Capture_Img(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int32_t timeout, double minimalQuality, double minimalLivenessScore, uint64_t intermediateResultFlags, uint64_t finalResultFlags, int32_t& val) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Capture_Img* pCmdInst  = nullptr;
    Atomic_Hid_Capture_Img* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Capture_Img>(pDev, CMD_HID_CAPTURE_IMAGE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, timeout, minimalQuality, minimalLivenessScore, intermediateResultFlags, finalResultFlags);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Capture_Img>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Res_Int(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Res_Int* pCmdInst = nullptr;
    Atomic_Hid_Parse_Res_Int* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Res_Int>(pDev, CMD_HID_PARSE_RES_INT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Res_Int>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Res_Double(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Res_Double* pCmdInst = nullptr;
    Atomic_Hid_Parse_Res_Double* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Res_Double>(pDev, CMD_HID_PARSE_RES_DOUBLE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Res_Double>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Res_Data(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Res_Data* pCmdInst = nullptr;
    Atomic_Hid_Parse_Res_Data* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Res_Data>(pDev, CMD_HID_PARSE_RES_DATA, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Res_Data>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Res_Point(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Res_Point* pCmdInst = nullptr;
    Atomic_Hid_Parse_Res_Point* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Res_Point>(pDev, CMD_HID_PARSE_RES_POINT, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Res_Point>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Res_Image(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Res_Image* pCmdInst = nullptr;
    Atomic_Hid_Parse_Res_Image* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Res_Image>(pDev, CMD_HID_PARSE_RES_IMAGE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Res_Image>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Parse_Match_Gallery(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t id) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Parse_Match_Gallery* pCmdInst = nullptr;
    Atomic_Hid_Parse_Match_Gallery* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Parse_Match_Gallery>(pDev, CMD_HID_PARSE_MATCH_GALLERY, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Parse_Match_Gallery>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Video_Frame(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t ctx, int64_t sec, int64_t& seq, int32_t& encoding, bin_data_t& data_bin) {

    V100_ERROR_CODE ioRes;
    HFImageEncoding enc;
    Atomic_Hid_Get_Video_Frame* pCmdInst = nullptr;
    Atomic_Hid_Get_Video_Frame* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Video_Frame>(pDev, CMD_HID_GET_VIDEO_FRAME, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(ctx, sec);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Video_Frame>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(seq, enc, data_bin);
        encoding = enc;
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Intermediate_Res(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t operation, uint64_t resultFlags, int32_t lastSequenceNumber, v100_hfres_t& hfRes) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Intermediate_Res* pCmdInst = nullptr;
    Atomic_Hid_Get_Intermediate_Res* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Intermediate_Res>(pDev, CMD_HID_GET_INTERMEDIATE_RES, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(operation, resultFlags, lastSequenceNumber);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Intermediate_Res>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(&hfRes);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Get_Final_Res(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t operation, uint64_t resultFlags, v100_hfres_t& hfRes) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Get_Final_Res* pCmdInst = nullptr;
    Atomic_Hid_Get_Final_Res* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Get_Final_Res>(pDev, CMD_HID_GET_FINAL_RES, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(operation, resultFlags);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Get_Final_Res>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        pRespInst->GetValue(&hfRes);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Db_Add_Record_With_Captured(const V100_DEVICE_TRANSPORT_INFO* const pDev, int32_t op, bool replace, const char* const id, const char* const gallery, const char* const custom) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Db_Add_Record_With_Captured* pCmdInst = nullptr;
    Atomic_Hid_Db_Add_Record_With_Captured* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Db_Add_Record_With_Captured>(pDev, CMD_HID_DB_ADD_RECORD_WITH_CAPTURED, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(op, replace, id, gallery, custom);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Db_Add_Record_With_Captured>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Db_Add_Record_With_Template(const V100_DEVICE_TRANSPORT_INFO* const pDev, bool replace, const char* const id, const char* const gallery, const char* const custom, const uint8_t* const data_ptr, uint32_t data_len) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Db_Add_Record_With_Template* pCmdInst = nullptr;
    Atomic_Hid_Db_Add_Record_With_Template* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Db_Add_Record_With_Template>(pDev, CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(replace, id, gallery, custom, data_ptr, data_len);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Db_Add_Record_With_Template>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Db_Get_Record(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const id, const char* const gallery) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Db_Get_Record* pCmdInst = nullptr;
    Atomic_Hid_Db_Get_Record* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Db_Get_Record>(pDev, CMD_HID_DB_GET_RECORD, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetParam(id, gallery);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Db_Get_Record>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    if (ioRes == GEN_OK) {
        // pRespInst->GetValue(val);
    }

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Db_List_Records(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const gallery) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Db_List_Records* pCmdInst = nullptr;
    Atomic_Hid_Db_List_Records* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Db_List_Records>(pDev, CMD_HID_DB_LIST_RECORDS, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(gallery);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Db_List_Records>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_Db_Del_Record(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const id, const char* const gallery) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_Db_Del_Record* pCmdInst = nullptr;
    Atomic_Hid_Db_Del_Record* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_Db_Del_Record>(pDev, CMD_HID_DB_DEL_RECORD, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetId(id, gallery);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_Db_Del_Record>(pDev, ROUTE_JENGINE, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

V100_ERROR_CODE V100_Hid_FwUpdate(const V100_DEVICE_TRANSPORT_INFO* const pDev, const char* const file_name ) {

    V100_ERROR_CODE ioRes;
    Atomic_Hid_FwUpdate* pCmdInst = nullptr;
    Atomic_Hid_FwUpdate* pRespInst = nullptr;

    assert(nullptr != pDev);

    ioRes = V100_Hid_Prepare_Request<Atomic_Hid_FwUpdate>(pDev, CMD_HID_FW_UPDATE, &pCmdInst);

    if (ioRes != GEN_OK) {
        // Strange. Something went wrong.
        assert(false);
        delete pCmdInst;
        return ioRes;
    }

    assert(nullptr != pCmdInst);

    // Configure request.
    pCmdInst->SetFileName(file_name);

    ioRes = V100_Hid_Execute_Request<Atomic_Hid_FwUpdate>(pDev, ROUTE_USBTRANSCEIVER, pCmdInst, &pRespInst);

    // Process command here.
    delete pCmdInst;
    delete pRespInst;

    return ioRes;
}

// ========================================================================================================================
// ** Capture Helper Routines **
// ========================================================================================================================

V100_ERROR_CODE PollCaptureCompletion(const V100_DEVICE_TRANSPORT_INFO* pDev, StatusCallBack Callback)
{
    bool bTimeout = false;
    _V100_ACQ_STATUS_TYPE Acq_Status;

    //bool bFirstTime = true;
    //DWORD start = 0;

    // Poll for completion
    while (bTimeout == false)
    {
        if (GEN_OK != V100_Get_Acq_Status(pDev, &Acq_Status))
        {
            return GEN_ERROR_INTERNAL;
        }
        //fprintf(stdout,"\n %d", Acq_Status);
        switch (Acq_Status)
        {
        case ACQ_NOOP:         { Sleep(PERIOD); continue; } break;
        case ACQ_BUSY:         { Sleep(PERIOD); continue; } break;
        case ACQ_PROCESSING: { Sleep(PERIOD); continue; } break;
        case ACQ_NO_FINGER_PRESENT:
        case ACQ_MOVE_FINGER_UP:
        case ACQ_MOVE_FINGER_DOWN:
        case ACQ_MOVE_FINGER_LEFT:
        case ACQ_MOVE_FINGER_RIGHT:
        case ACQ_FINGER_POSITION_OK:
        {
            Sleep(PERIOD);
            int ret = Callback(Acq_Status);
            if (ret == -1)
            {
               if (GEN_OK != V100_Cancel_Operation(pDev))
               {
                   return GEN_ERROR_INTERNAL;
               }
               break;
            }
            continue;
        } break;

        case ACQ_FINGER_PRESENT:
        {
            //if(bFirstTime)
            //{
            //    start = GetTickCount();
            //    bFirstTime = false;
            //}

            Sleep(PERIOD);
            int ret = Callback(Acq_Status);
            if (ret == -1)
            {
               if (GEN_OK != V100_Cancel_Operation(pDev))
               {
                   return GEN_ERROR_INTERNAL;
               }
               break;
            }
            continue;
        } break;

        case ACQ_TIMEOUT:            { bTimeout = true; continue; } break;
        case ACQ_DONE:
        {
            //DWORD end = GetTickCount();
            //fprintf(stdout,"\n\n Total Capture time: %d\n\n", end - start);
            bTimeout = true;
            break;
        }
        case ACQ_CANCELLED_BY_USER:
        {
            return GEN_ERROR_CAPTURE_CANCELLED;
        }
        case ACQ_LATENT_DETECTED:
        {
            return GEN_ERROR_TIMEOUT_LATENT;
        }
        case ACQ_SPOOF_DETECTED:
        {
            return GEN_ERROR_SPOOF_DETECTED;
        }
        case ACQ_ERROR_MATCH:
        {
            return GEN_ERROR_MATCH;
        }
        case ACQ_ERROR_IMAGE:
        {
            return GEN_ERROR_MATCH;
        }
        case ACQ_ERROR_SYSTEM:
        {
            return GEN_ERROR_INTERNAL;
        }
        case ACQ_ERROR_PARAMETER:
        {
            return GEN_INVALID_ARGUEMENT;
        }

        }// End switch
    }// End while

    if (Acq_Status != ACQ_DONE)
    {
        return GEN_ERROR_TIMEOUT;
    }
    return GEN_OK;
}

V100_ERROR_CODE SendEncCommandNoResponse(const V100_DEVICE_TRANSPORT_INFO *pDev, int route_type, IEncCmd* pCmd)
{
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)pDev, route_type, pCmd));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    // Is it an Error packet, or...
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)    // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)arg;
    }
    // Genuine
    V100_ERROR_CODE rc = pResponse->GetReturnCode();
    delete pResponse;
    return rc;
}

V100_ERROR_CODE SendEncCommandResponse(const V100_DEVICE_TRANSPORT_INFO *pDev, int route_type, IEncCmd* pCommand, uchar** pOutCG, u32& nOutCGSize) {
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)pDev, route_type, pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    // Is it an Error packet, or...
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)    // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)arg;
    }
    // Genuine - Don't care what it is, will allocate buffer, and memcpy
    IEncCmd* pEncResponse = reinterpret_cast<IEncCmd*>(pResponse);
    nOutCGSize = pEncResponse->GetCryptogramSize();
    *pOutCG = (uchar*)MALLOC(nOutCGSize);
    if (*pOutCG == NULL) return GEN_ERROR_MEMORY;
    memcpy(*pOutCG, pEncResponse->GetCryptogram(), nOutCGSize);
    V100_ERROR_CODE rc = pResponse->GetReturnCode();
    delete pEncResponse;
    return rc;
}


// Translate OpError to string
VCOM_CORE_EXPORT const char* GetVCOMErrorStr(uint nEC) {
    switch (nEC) {
        case GEN_OK:                             {return "GEN_OK"; } break;
        case GEN_ENCRYPTION_FAIL:                {return "GEN_ENCRYPTION_FAIL"; } break;
        case GEN_DECRYPTION_FAIL:                {return "GEN_DECRYPTION_FAIL"; } break;
        case GET_PD_INIT_FAIL:                   {return "GET_PD_INIT_FAIL"; } break;
        case PD_HISTOGRAM_FAIL:                  {return "PD_HISTOGRAM_FAIL"; } break;
        case INVALID_ACQ_MODE:                   {return "INVALID_ACQ_MODE"; } break;
        case BURNIN_THREAD_FAIL:                 {return "BURNIN_THREAD_FAIL"; } break;
        case UPDATER_THREAD_FAIL:                {return "UPDATER_THREAD_FAIL"; } break;
        case GEN_ERROR_START:                    {return "GEN_ERROR_START"; } break;
        case GEN_ERROR_PROCESSING:               {return "GEN_ERROR_PROCESSING"; } break;
        case GEN_ERROR_VERIFY:                   {return "GEN_ERROR_VERIFY"; } break;
        case GEN_ERROR_MATCH:                    {return "GEN_ERROR_MATCH"; } break;
        case GEN_ERROR_INTERNAL:                 {return "GEN_ERROR_INTERNAL"; } break;
        case GEN_ERROR_INVALID_CMD:              {return "GEN_ERROR_INVALID_CMD"; } break;
        case GEN_ERROR_PARAMETER:                {return "GEN_ERROR_PARAMETER"; } break;
        case GEN_NOT_SUPPORTED:                  {return "GEN_NOT_SUPPORTED"; } break;
        case GEN_INVALID_ARGUEMENT:              {return "GEN_INVALID_ARGUEMENT"; } break;
        case GEN_ERROR_TIMEOUT:                  {return "GEN_ERROR_TIMEOUT"; } break;
        case GEN_ERROR_LICENSE:                  {return "GEN_ERROR_LICENSE"; } break;
        case GEN_ERROR_COMM_TIMEOUT:             {return "GEN_ERROR_COMM_TIMEOUT"; } break;
        case GEN_FS_ERR_CD:                      {return "GEN_FS_ERR_CD"; } break;
        case GEN_FS_ERR_DELETE:                  {return "GEN_FS_ERR_DELETE"; } break;
        case GEN_FS_ERR_FIND:                    {return "GEN_FS_ERR_FIND"; } break;
        case GEN_FS_ERR_WRITE:                   {return "GEN_FS_ERR_WRITE"; } break;
        case GEN_FS_ERR_READ:                    {return "GEN_FS_ERR_READ"; } break;
        case GEN_FS_ERR_FORMAT:                  {return "GEN_FS_ERR_FORMAT"; } break;
        case GEN_ERROR_MEMORY:                   {return "GEN_ERROR_MEMORY"; } break;
        case GEN_ERROR_RECORD_NOT_FOUND:         {return "GEN_ERROR_RECORD_NOT_FOUND"; } break;
        case GEN_VER_INVALID_RECORD_FORMAT:      {return "GEN_VER_INVALID_RECORD_FORMAT"; } break;
        case GEN_ERROR_DB_FULL:                  {return "GEN_ERROR_DB_FULL"; } break;
        case GEN_ERROR_INVALID_SIZE:             {return "GEN_ERROR_INVALID_SIZE"; } break;
        case GEN_ERROR_TAG_NOT_FOUND:            {return "GEN_ERROR_TAG_NOT_FOUND"; } break;
        case GEN_ERROR_APP_BUSY:                 {return "GEN_ERROR_APP_BUSY"; } break;
        case GEN_ERROR_DEVICE_UNCONFIGURED:      {return "GEN_ERROR_DEVICE_UNCONFIGURED"; } break;
        case GEN_ERROR_TIMEOUT_LATENT:           {return "GEN_ERROR_TIMEOUT_LATENT"; } break;
        case GEN_ERROR_DB_NOT_LOADED:            {return "GEN_ERROR_DB_NOT_LOADED"; } break;
        case GEN_ERROR_DB_DOESNOT_EXIST:         {return "GEN_ERROR_DB_DOESNOT_EXIST"; } break;
        case GEN_ERROR_ENROLLMENT_INCOMPLETE:    {return "GEN_ERROR_ENROLLMENT_INCOMPLETE"; } break;
        case GEN_ERROR_USER_NOT_FOUND:           {return "GEN_ERROR_USER_NOT_FOUND"; } break;
        case GEN_ERROR_DB_USER_FINGERS_FULL:     {return "GEN_ERROR_DB_USER_FINGERS_FULL"; } break;
        case GEN_ERROR_DB_USERS_FULL:            {return "GEN_ERROR_DB_USERS_FULL"; } break;
        case GEN_ERROR_USER_EXISTS:              {return "GEN_ERROR_USER_EXISTS"; } break;
        case GEN_ERROR_DEVICE_NOT_FOUND:         {return "GEN_ERROR_DEVICE_NOT_FOUND"; } break;
        case GEN_ERROR_DEVICE_NOT_READY:         {return "GEN_ERROR_DEVICE_NOT_READY"; } break;
        case GEN_ERROR_PIPE_READ:                {return "GEN_ERROR_PIPE_READ"; } break;
        case GEN_ERROR_PIPE_WRITE:               {return "GEN_ERROR_PIPE_WRITE"; } break;
        case GEN_ERROR_SENGINE_SHUTTING_DOWN:    {return "GEN_ERROR_SENGINE_SHUTTING_DOWN"; } break;
        case GEN_ERROR_SPOOF_DETECTED:           {return "GEN_ERROR_SPOOF_DETECTED"; } break;
        case GEN_ERROR_DATA_UNAVAILABLE:         {return "GEN_ERROR_DATA_UNAVAILABLE"; } break;
        case GEN_ERROR_CRYPTO_FAIL:              {return "GEN_ERROR_CRYPTO_FAIL"; } break;
        case GEN_ERROR_CAPTURE_CANCELLED:        {return "GEN_ERROR_CAPTURE_CANCELLED"; } break;
        case GEN_LAST:                           {return "GEN_LAST"; } break;
        default:                                 {return "** Unknown Error Code **"; } break;
    }

}

// Translate OpError to string
VCOM_CORE_EXPORT const char* GetOpErrorStr(uint nParameter) {
    switch (nParameter) {
        case STATUS_OK:                             {return "STATUS_OK"; } break;
        case ERROR_UID_EXISTS:                      {return "ERROR_UID_EXISTS"; } break;
        case ERROR_ENROLLMENT_QUALIFICATION:        {return "ERROR_ENROLLMENT_QUALIFICATION"; } break;
        case ERROR_UID_DOES_NOT_EXIST:              {return "ERROR_UID_DOES_NOT_EXIST"; } break;
        case ERROR_DB_FULL:                         {return "ERROR_DB_FULL"; } break;
        case ERROR_QUALIFICATION:                   {return "ERROR_QUALIFICATION"; } break;
        case ERROR_DEV_TIMEOUT:                     {return "ERROR_DEV_TIMEOUT"; } break;
        case ERROR_USER_CANCELLED:                  {return "ERROR_USER_CANCELLED"; } break;
        case ERROR_SPOOF_DETECTED:                  {return "ERROR_SPOOF_DETECTED"; } break;
        case ERROR_DB_EXISTS:                       {return "ERROR_DB_EXISTS"; } break;
        case ERROR_DB_DOES_NOT_EXIST:               {return "ERROR_DB_DOES_NOT_EXIST"; } break;
        case ERROR_ID_DB_TOO_LARGE:                 {return "ERROR_ID_DB_TOO_LARGE"; } break;
        case ERROR_ID_DB_EXISTS:                    {return "ERROR_ID_DB_EXISTS"; } break;
        case ERROR_ID_USER_EXISTS:                  {return "ERROR_ID_USER_EXISTS"; } break;
        case ERROR_ID_USER_NOT_FOUND:               {return "ERROR_ID_USER_NOT_FOUND"; } break;
        case STATUS_ID_MATCH:                       {return "STATUS_ID_MATCH"; } break;
        case STATUS_ID_NO_MATCH:                    {return "STATUS_ID_NO_MATCH"; } break;
        case ERROR_ID_PARAMETER:                    {return "ERROR_ID_PARAMETER"; } break;
        case ERROR_ID_GENERAL:                      {return "ERROR_ID_GENERAL"; } break;
        case ERROR_ID_FILE:                         {return "ERROR_ID_FILE"; } break;
        case ERROR_ID_NOT_INITIALIZED:              {return "ERROR_ID_NOT_INITIALIZED"; } break;
        case ERROR_ID_DB_FULL:                      {return "ERROR_ID_DB_FULL"; } break;
        case ERROR_ID_DB_DOESNT_EXIST:              {return "ERROR_ID_DB_DOESNT_EXIST"; } break;
        case ERROR_ID_DB_NOT_LOADED:                {return "ERROR_ID_DB_NOT_LOADED"; } break;
        case ERROR_ID_RECORD_NOT_FOUND:             {return "ERROR_ID_RECORD_NOT_FOUND"; } break;
        case ERROR_ID_FS:                           {return "ERROR_ID_FS"; } break;
        case ERROR_ID_CREATE_FAIL:                  {return "ERROR_ID_CREATE_FAIL"; } break;
        case ERROR_ID_INTERNAL:                     {return "ERROR_ID_INTERNAL"; } break;
        case ERROR_ID_MEM:                          {return "ERROR_ID_MEM"; } break;
        case STATUS_ID_USER_FOUND:                  {return "STATUS_ID_USER_FOUND"; } break;
        case STATUS_ID_USER_NOT_FOUND:              {return "STATUS_ID_USER_NOT_FOUND"; } break;
        case ERROR_ID_USER_FINGERS_FULL:            {return "ERROR_ID_USER_FINGERS_FULL"; } break;
        case ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND: {return "ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND"; } break;
        case ERROR_ID_USERS_FULL:                   {return "ERROR_ID_USERS_FULL"; } break;
        case ERROR_ID_OPERATION_NOT_SUPPORTED:      {return "ERROR_ID_OPERATION_NOT_SUPPORTED"; } break;
        case ERROR_ID_NOT_ENOUGH_SPACE:             {return "ERROR_ID_NOT_ENOUGH_SPACE"; } break;
        case ERROR_ID_DUPLICATE:                    {return "ERROR_ID_DUPLICATE"; } break;
        case ERROR_CAPTURE_TIMEOUT:                 {return "ERROR_CAPTURE_TIMEOUT"; } break;
        case ERROR_CAPTURE_LATENT:                  {return "ERROR_CAPTURE_LATENT"; } break;
        case ERROR_CAPTURE_CANCELLED:               {return "ERROR_CAPTURE_CANCELLED"; } break;
        case ERROR_CAPTURE_INTERNAL:                {return "ERROR_CAPTURE_INTERNAL"; } break;
        case ERROR_UPDATE_MEMORY_ERROR:             {return "ERROR_UPDATE_MEMORY_ERROR"; } break;
        case ERROR_UPDATE_DECRYPTION_FAIL:          {return "ERROR_UPDATE_DECRYPTION_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_VERSION_ERROR:   {return "ERROR_UPDATE_FIRMWARE_VERSION_ERROR"; } break;
        case ERROR_UPDATE_FLASH_WRITE_ERROR:        {return "ERROR_UPDATE_FLASH_WRITE_ERROR"; } break;
        case ERROR_UPDATE_INVALID_TYPE:             {return "ERROR_UPDATE_INVALID_TYPE"; } break;
        case ERROR_UPDATE_FORMAT_ERROR:             {return "ERROR_UPDATE_FORMAT_ERROR"; } break;
        case ERROR_UPDATE_FIRMWARE_SIZE_ERROR:      {return "ERROR_UPDATE_FIRMWARE_SIZE_ERROR"; } break;
        case ERROR_UPDATE_RESTORE_FAIL:             {return "ERROR_UPDATE_RESTORE_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_INVALID:         {return "ERROR_UPDATE_FIRMWARE_INVALID"; } break;
        case ERROR_CRYPTO_ERROR:                    {return "ERROR_CRYPTO_ERROR"; } break;
        case STATUS_NO_OP:                          {return "STATUS_NO_OP"; } break;
        default:                                    {return "UNKNOWN ERROR"; } break;
    }
//<TODO> update codes
    return "Unknown _V100_OP_STATUS";
}

// Translate sensor type to string
VCOM_CORE_EXPORT const char* GetSensorTypeStr(uint nType) {
    switch (nType) {
        case VENUS_V30X:          {return "VENUS V30X"; } break;
        case MERCURY_M30X:        {return "MERCURY M30X"; } break;
        case MERCURY_M31X:        {return "MERCURY M31X"; } break;
        case VENUS_V31X:          {return "VENUS V31X"; } break;
        case VENUS_V371:          {return "VENUS V371"; } break;
        case VENUS_V40X:          {return "VENUS V40X"; } break;
        case VENUS_V42X:          {return "VENUS V42X"; } break;
        case MERCURY_M32X:        {return "MERCURY M32X"; } break;
        case MERCURY_M42X:        {return "MERCURY M42X"; } break;
        case MERCURY_M21X:        {return "MERCURY M21X"; } break;
        case VENUS_V310_10:       {return "VENUS V310_10"; } break;
        case UNKNOWN_LUMI_DEVICE:
        default:                  {return "UNKOWN_LUMI_DEVICE"; } break;
    }
}