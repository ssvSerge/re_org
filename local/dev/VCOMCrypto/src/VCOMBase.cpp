/***************************************************************************************/
// ©Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/

#ifndef WIN32
    #include "windows_conv.h"
#else
    #include <windows.h>
#endif

#include "VCOMBase.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "IMemMgr.h"
#include <iostream>
#include "V100DeviceHandler.h"

#ifndef __GNUC__
EXTERN_C const GUID GUID_BULKLDI;
#endif

#define PERIOD 1



#define __LOCK_API__
#ifdef __LOCK_API__
#include <mutex>
static std::recursive_mutex io_rec_mutex;

//
// LOCK_FUNC makes certain that a client does not call various VCOM functions
// from multiple threads, regarding the same device.
// Multi-threaded applications with multiple device handles is allowed, however.

#define LOCK_FUNC()                        \
    if(false == V100CommandHandler::Lock_Handle(pDev) ){    \
        return GEN_ERROR_APP_BUSY;                \
    }


// BLOCK_LOCK
#define BLOCK_LOCK() std::lock_guard<std::recursive_mutex> lg(io_rec_mutex);
#else
#define LOCK_FUNC()
#define BLOCK_LOCK()
#endif


V100_ERROR_CODE V100_Cancel_Operation(const V100_DEVICE_TRANSPORT_INFO* pDev)
{
    LOCK_FUNC();
    Atomic_Cancel_Operation* pCommand = reinterpret_cast<Atomic_Cancel_Operation*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_CANCEL_OPERATION));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Cancel_Operation*>(pResponse);
    if (pCommand == NULL)
    {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }
    return GEN_OK;
}

V100_ERROR_CODE V100_Close(V100_DEVICE_TRANSPORT_INFO *pDev)
{
    BLOCK_LOCK();
    V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
    pVCH->Close(pDev);
    delete pVCH;
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Acq_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ACQ_STATUS_TYPE* pACQ_Status)
{
    LOCK_FUNC();
    Atomic_Get_Acq_Status* pCommand = reinterpret_cast<Atomic_Get_Acq_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_ACQ_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if(pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Get_Acq_Status*>(pResponse);
    if (pCommand != NULL)
    {
        *pACQ_Status = pCommand->GetAcqStatus();
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Config(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_INTERFACE_CONFIGURATION_TYPE* pICT)
{
    LOCK_FUNC();
    Atomic_Get_Config* pCommand = reinterpret_cast<Atomic_Get_Config*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_CONFIG));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Get_Config*>(pResponse);
    if (pCommand != NULL)
    {

        *pICT = pCommand->GetConfiguration();
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Sensor_Type(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SENSOR_TYPE& nSensorType)
{
    LOCK_FUNC();

    nSensorType = UNKNOWN_LUMI_DEVICE;
    _V100_INTERFACE_CONFIGURATION_TYPE config;
    memset(&config, 0, sizeof(_V100_INTERFACE_CONFIGURATION_TYPE));

    V100_ERROR_CODE rc = GEN_OK;
    rc = V100_Get_Config(pDev, &config);
    if (rc != GEN_OK)
    {
        return rc;
    }

    // Figure out the Sensor Type
    // First check the VID/PID combo to figure out what the device is
    if (config.Vendor_Id == LUMI_VID_V_SERIES_EMBEDDED)
    {
        if (config.Product_Id == LUMI_PID_V_SERIES_EMBEDDED)
        {
            // Check to see if it is a V40x or a V42x
            if (config.Hardware_Rev == 400)
            {
                nSensorType = VENUS_V40X;
            }
            else if (config.Hardware_Rev == 420)
            {
                nSensorType = VENUS_V42X;
            }
            else
            {
                // Sensor is a V30x, only support V30x-40 at this time
                if (config.Firmware_Rev >= 29991)
                {
                    nSensorType = VENUS_V30X;
                }
                else
                {
                    nSensorType = UNSUPPORTED_LUMI_DEVICE;
                }
            }
            return rc;
        }
        else
        {
            return rc;
        }
    }
    else if (config.Vendor_Id == LUMI_VID_NON_V_SERIES_EMBEDDED)
    {
        if (config.Product_Id == LUMI_PID_M_SERIES_EMBEDED)
        {
            nSensorType = MERCURY_M30X;

            // Check to see if it is a M32x or a M42x
            if (config.Hardware_Rev == 320)
            {
                nSensorType = MERCURY_M32X;
            }
            else if (config.Hardware_Rev == 420)
            {
                nSensorType = MERCURY_M42X;
            }
        }
        else if (config.Product_Id == LUMI_PID_M_SERIES_STREAMING)
        {
            nSensorType = MERCURY_M31X;
        }
        else if (config.Product_Id == LUMI_PID_V_SERIES_STREAMING)
        {
            nSensorType = VENUS_V31X;
            if (config.Hardware_Rev == 371)
            {
                nSensorType = VENUS_V371;
            }
        }
        else if (config.Product_Id == LUMI_PID_M210_SERIES_STREAMING)
        {
            nSensorType = MERCURY_M21X;
        }
        else if (config.Product_Id == LUMI_PID_V310_10_SERIES_STREAMING)
        {
            nSensorType = VENUS_V31X_10;
        }
        else if (config.Product_Id == LUMI_PID_V32x_EMBEDDED)
        {
            nSensorType = VENUS_V32X;
        }
        else if (config.Product_Id == LUMI_PID_V52x_EMBEDDED)
        {
            nSensorType = VENUS_V52X;
        }
        else
        {
            return rc;
        }
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Num_USB_Devices(int* nNumDevices)
{
    BLOCK_LOCK();
    V100_ERROR_CODE rc = V100DeviceHandler::GetV100DeviceHandler()->GetNumDevices(nNumDevices);
    V100DeviceHandler::ReleaseV100DeviceHandler();
    return rc;
}

V100_ERROR_CODE V100_Get_OP_Status(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_OP_STATUS& opStatus)
{
    LOCK_FUNC();
    Atomic_Get_OP_Status* pCommand = reinterpret_cast<Atomic_Get_OP_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_OP_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Get_OP_Status*>(pResponse);
    if (pCommand != NULL)
    {
        if (pCommand == NULL)
        {
            delete pResponse;
            return GEN_ERROR_INTERNAL;
        }
        pCommand->GetOPStatus(&opStatus);
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Serial(const V100_DEVICE_TRANSPORT_INFO* pDev, u32* pSerialNumber)
{
    LOCK_FUNC();
    Atomic_Get_Serial* pCommand = reinterpret_cast<Atomic_Get_Serial*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand((_V100_COMMAND_SET)CMD_GET_SERIAL_NUMBER));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr;
        if (arg == 0) // If arg is not set, the sensor does not support CMD_GET_SERIAL_NUMBER
        {
            return GEN_ERROR_INVALID_CMD;
        }
        else
        {
            return (V100_ERROR_CODE)(arg);
        }
    }
    pCommand = dynamic_cast<Atomic_Get_Serial*>(pResponse);
    if (pCommand != NULL)
    {
        *pSerialNumber = pCommand->GetSerialNumber();
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Status(const V100_DEVICE_TRANSPORT_INFO *pDev, _V100_INTERFACE_STATUS_TYPE* pIST)
{
    LOCK_FUNC();
    Atomic_Get_Status* pCommand = reinterpret_cast<Atomic_Get_Status*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_STATUS));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Get_Status*>(pResponse);
    if (pCommand != NULL)
    {
        *pIST = pCommand->GetInterfaceStatusType();
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_GetSystemDiagnostics(V100_DEVICE_TRANSPORT_INFO *pDev, _V100_SYSTEM_DIAGNOSTICS* pSysDiag)
{
    LOCK_FUNC();
    Atomic_Get_System_State* pCommand = reinterpret_cast<Atomic_Get_System_State*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand((_V100_COMMAND_SET)CMD_GET_SYSTEM_STATE));
    ICmd* pResponse = reinterpret_cast<Atomic_Get_System_State*>(V100CommandHandler::GetCommandHandler(pDev)->IssueCommand(pDev, pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = reinterpret_cast<Atomic_Get_System_State*>(pResponse);
    if (pCommand != NULL)
    {
        _V100_SYSTEM_DIAGNOSTICS* pRespBuff = pCommand->GetSystemMetricsStruct();
        memcpy(pSysDiag, pRespBuff, sizeof(_V100_SYSTEM_DIAGNOSTICS));
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Get_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTag, u16& nTagLength)
{
    LOCK_FUNC();
    Atomic_Get_Tag* pCommand = reinterpret_cast<Atomic_Get_Tag*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_GET_TAG));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Get_Tag*>(pResponse);
    if (pCommand != NULL)
    {
        char* pTagToGet = NULL;
        pCommand->GetTag(&pTagToGet, nTagLength);
        memcpy(pTag, pTagToGet, nTagLength);
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Open(V100_DEVICE_TRANSPORT_INFO *pDev)
{
    BLOCK_LOCK();

    V100_ERROR_CODE rc = GEN_OK;
    // Loop for device connected

    // We currently only support one device at a time on Linux.
    #ifdef __linux__
        if (V100CommandHandler::GetNumOpenHandles())
        {
            return GEN_ERROR_APP_BUSY;
        }
    #endif

    int nConnectAttempt = 0;
    // Loop for device connected
    do
    {
        _V100_INTERFACE_CONFIGURATION_TYPE sensor_configuration{};

        Device* pDevice = new Device();

        rc = V100DeviceHandler::GetV100DeviceHandler()->GetDevice(pDev->DeviceNumber, pDevice);
        if (GEN_OK != rc && pDev->nBaudRate == 0)
        {
            fprintf(stdout, "\n Could not set up Device object.");
            delete pDevice;
            pDevice = NULL;
            pDev->hInstance = NULL;
        }
        else
        {
            // The Device object is now the hInstance
            pDev->hInstance = pDevice;

            rc = GEN_OK;
            V100CommandHandler* pVCH = V100CommandHandler::GetCommandHandler(pDev);
            rc = (V100_ERROR_CODE)pVCH->Initialize(pDev);

            delete pDevice;
            pDevice = NULL;

            V100DeviceHandler::ReleaseV100DeviceHandler();

            if (GEN_OK != rc)
            {
                pVCH->Close(pDev);
                delete pVCH;
                #ifdef __linux__
                if (rc == GEN_ERROR_APP_BUSY)
                #else
                if (rc == GEN_ERROR_APP_BUSY || rc == GEN_ERROR_CONNECTION_REFUSED) // Check for embedded sensor refused connection on Windows only
                #endif
                {
                    fprintf(stdout, "\n\n Error! Another VCOM instance is already established!\n");
                    break;
                }
                {
                    fprintf(stdout, "\n Could not initialize Command Handler.");
                }
            }
            else
            {
                // Make sure the sensor is ready to communicate
                rc = V100_Get_Config(pDev, &sensor_configuration);
                if (rc == GEN_OK)
                {
                    break;
                }
                else
                {
                    fprintf(stdout, "\n Error getting Device configuration.");
                }
            }
        }

        // Try again
        fprintf(stdout, "\n WARNING: Could not open communication handle with Device. The Device may be still coming up. Trying again!\n");
        #ifdef __linux__
            usleep(1000 * 1000);
        #else
            Sleep(1000);
        #endif
    } while (nConnectAttempt++ < 5);

    if (rc != GEN_OK)
    {
        fprintf(stdout, "\n ERROR: Could not open communication handle with Device!\n");
    }
    return rc;
}

V100_ERROR_CODE V100_Reset(const V100_DEVICE_TRANSPORT_INFO* pDev)
{
    LOCK_FUNC();
    Atomic_Reset* pCommand = reinterpret_cast<Atomic_Reset*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_RESET));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Reset*>(pResponse);
    if (pCommand != NULL)
    {
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Release_Memory(const V100_DEVICE_TRANSPORT_INFO* pDev, void* pMem)
{
    if (NULL != pMem)
    {
        FREE(pMem);
        pMem = NULL;
    }

    return GEN_OK;

}

V100_ERROR_CODE V100_Set_Tag(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pTagData, u16 nTagDataSize)
{

    LOCK_FUNC();
    Atomic_Set_Tag* pCommand = reinterpret_cast<Atomic_Set_Tag*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_SET_TAG));
    pCommand->SetTag((char*)pTagData, nTagDataSize);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Set_Tag*>(pResponse);
    if (pCommand != NULL)
    {
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE V100_Update_Firmware(const V100_DEVICE_TRANSPORT_INFO* pDev, const u8* pFirmwareStream, u32 nFWStreamSize)
{
    LOCK_FUNC();
    Macro_Update_Firmware* pCommand = reinterpret_cast<Macro_Update_Firmware*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_UPDATE_FIRMWARE)));
    // Set the command
    if (false == pCommand->SetData(pFirmwareStream, nFWStreamSize))
    {
        delete pCommand; return (V100_ERROR_CODE)(1);
    }
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Macro_Update_Firmware*>(pResponse);
    if (pCommand != NULL)
    {
        delete pCommand;
    }
    else
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    return GEN_OK;
}

// ========================================================================================================================
// ** CRYPTO CALLS **
// ========================================================================================================================

V100_ERROR_CODE V100_Enc_Clear(const V100_DEVICE_TRANSPORT_INFO *pDev)
{
    LOCK_FUNC();
    Atomic_Enc_Clear* pCommand = reinterpret_cast<Atomic_Enc_Clear*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_CLEAR));
    if (pCommand == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    return SendEncCommandNoResponse(pDev, pCommand);
}

V100_ERROR_CODE V100_Enc_Generate_RSA_Keys(const V100_DEVICE_TRANSPORT_INFO *pDev)
{
    Macro_Enc_Generate_RSA_Keys* pCommand = reinterpret_cast<Macro_Enc_Generate_RSA_Keys*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GENERATE_RSA_KEYS));
    if (pCommand == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    return SendEncCommandNoResponse(pDev, pCommand);
}

V100_ERROR_CODE V100_Enc_Get_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u16& nKeyVersion, u8* pKCV, u16& nKeySize, u16& nKeyMode)
{
    Atomic_Enc_Get_Key* pCommand = reinterpret_cast<Atomic_Enc_Get_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_KEY));
    pCommand->SetArguement(nKeyType);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_Key*>(pResponse);
    if (pCommand == NULL)
    {
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

V100_ERROR_CODE V100_Enc_Get_KeyVersion(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_KEY_TYPE nKeySlot, u16& nKeyVersion, u8* pKCV, u16& nKeyMode)
{
    Atomic_Enc_Get_KeyVersion* pCommand = reinterpret_cast<Atomic_Enc_Get_KeyVersion*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_KEYVERSION));
    pCommand->SetKeySlot(nKeySlot);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_KeyVersion*>(pResponse);
    if (pCommand == NULL)
    {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }
    nKeyVersion = pCommand->GetKeyVersion();
    nKeyMode    = pCommand->GetKeyMode();
    memcpy(pKCV, pCommand->GetKCV(), KCV_LENGTH);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Get_Serial_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u64& SerialNumber)
{
    Atomic_Enc_Get_Serial_Number* pCommand = reinterpret_cast<Atomic_Enc_Get_Serial_Number*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_SERIAL_NUMBER));
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_Serial_Number*>(pResponse);
    if (pCommand == NULL)
    {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }
    SerialNumber = pCommand->GetSerialNum();
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Get_Rnd_Number(const V100_DEVICE_TRANSPORT_INFO* pDev, u256 rndNumber, int nBytesRequested)
{
    if (nBytesRequested > 32) return GEN_ERROR_INVALID_SIZE;
    Atomic_Enc_Get_Rnd_Number* pCommand = reinterpret_cast<Atomic_Enc_Get_Rnd_Number*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_GET_RND_NUMBER));
    pCommand->SetArguement(2);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_Rnd_Number*>(pResponse);
    if (pCommand == NULL)
    {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }
    memset(rndNumber, 0, sizeof(u256));
    memcpy(rndNumber, pCommand->GetRandomNumber(), nBytesRequested);
    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Set_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, u8* pCGKey, uint nCGKeySize, _V100_ENC_KEY_TYPE nKeyType, u8** pCGOutputData, u32& nCGOutputDataSize)
{
    Atomic_Enc_Set_Key* pCommand = reinterpret_cast<Atomic_Enc_Set_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_SET_KEY));
    pCommand->SetBuffer(pCGKey, nCGKeySize);
    pCommand->SetKeyType(nKeyType);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr) // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_Set_Key*>(pResponse);
    if (pCommand == NULL)
    {
        delete pResponse;
        return GEN_ERROR_INTERNAL;
    }

    uint nSizeReturnedCG = pCommand->GetBufferSize();
    *pCGOutputData = NULL;
    *pCGOutputData = (u8*)MALLOC(nSizeReturnedCG);
    if (*pCGOutputData == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    memcpy(*pCGOutputData, pCommand->GetBuffer(), nSizeReturnedCG);
    nCGOutputDataSize = nSizeReturnedCG;

    delete pCommand;
    return GEN_OK;
}

V100_ERROR_CODE V100_Enc_Unlock_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, const u8* pCGInputData, u32 nCGInputDataSize)
{
    Atomic_Enc_Unlock_Key* pCommand = reinterpret_cast<Atomic_Enc_Unlock_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_ENC_UNLOCK_KEY));
    if (pCommand == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    pCommand->SetCryptogram(pCGInputData, nCGInputDataSize);
    return SendEncCommandNoResponse(pDev, pCommand);
}

// ========================================================================================================================
// ** JAGUAR CALLS **
// ========================================================================================================================

V100_ERROR_CODE V100_Hid_Init(const V100_DEVICE_TRANSPORT_INFO* pDev) {
    LOCK_FUNC();
    Atomic_Hid_Init* pCommand = reinterpret_cast<Atomic_Hid_Init*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(CMD_HID_INIT));
    if (pCommand == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    return SendEncCommandNoResponse(pDev, pCommand);
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
        switch (Acq_Status) {

            case ACQ_NOOP:        { Sleep(PERIOD); continue; } break;
            case ACQ_BUSY:        { Sleep(PERIOD); continue; } break;
            case ACQ_PROCESSING:     { Sleep(PERIOD); continue; } break;
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

            case ACQ_TIMEOUT: { bTimeout = true; continue; } break;
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

        } // End switch

    } // End while

    if (Acq_Status != ACQ_DONE)
    {
        return GEN_ERROR_TIMEOUT;
    }

    return GEN_OK;
}

V100_ERROR_CODE SendEncCommandNoResponse(const V100_DEVICE_TRANSPORT_INFO *pDev, IEncCmd* pCmd)
{
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)pDev, pCmd));
    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }
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

V100_ERROR_CODE SendEncCommandResponse(const V100_DEVICE_TRANSPORT_INFO *pDev, IEncCmd* pCommand, uchar** pOutCG, u32& nOutCGSize)
{
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)pDev, pCommand));
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
        case GEN_OK:                                 {return "GEN_OK"; } break;
        case GEN_ENCRYPTION_FAIL:                    {return "GEN_ENCRYPTION_FAIL"; } break;
        case GEN_DECRYPTION_FAIL:                    {return "GEN_DECRYPTION_FAIL"; } break;
        case GET_PD_INIT_FAIL:                       {return "GET_PD_INIT_FAIL"; } break;
        case PD_HISTOGRAM_FAIL:                      {return "PD_HISTOGRAM_FAIL"; } break;
        case INVALID_ACQ_MODE:                       {return "INVALID_ACQ_MODE"; } break;
        case BURNIN_THREAD_FAIL:                     {return "BURNIN_THREAD_FAIL"; } break;
        case UPDATER_THREAD_FAIL:                    {return "UPDATER_THREAD_FAIL"; } break;
        case GEN_ERROR_START:                        {return "GEN_ERROR_START"; } break;
        case GEN_ERROR_PROCESSING:                   {return "GEN_ERROR_PROCESSING"; } break;
        case GEN_ERROR_VERIFY:                       {return "GEN_ERROR_VERIFY"; } break;
        case GEN_ERROR_MATCH:                        {return "GEN_ERROR_MATCH"; } break;
        case GEN_ERROR_INTERNAL:                     {return "GEN_ERROR_INTERNAL"; } break;
        case GEN_ERROR_INVALID_CMD:                  {return "GEN_ERROR_INVALID_CMD"; } break;
        case GEN_ERROR_PARAMETER:                    {return "GEN_ERROR_PARAMETER"; } break;
        case GEN_NOT_SUPPORTED:                      {return "GEN_NOT_SUPPORTED"; } break;
        case GEN_INVALID_ARGUEMENT:                  {return "GEN_INVALID_ARGUEMENT"; } break;
        case GEN_ERROR_TIMEOUT:                      {return "GEN_ERROR_TIMEOUT"; } break;
        case GEN_ERROR_LICENSE:                      {return "GEN_ERROR_LICENSE"; } break;
        case GEN_ERROR_COMM_TIMEOUT:                 {return "GEN_ERROR_COMM_TIMEOUT"; } break;
        case GEN_FS_ERR_CD:                          {return "GEN_FS_ERR_CD"; } break;
        case GEN_FS_ERR_DELETE:                      {return "GEN_FS_ERR_DELETE"; } break;
        case GEN_FS_ERR_FIND:                        {return "GEN_FS_ERR_FIND"; } break;
        case GEN_FS_ERR_WRITE:                       {return "GEN_FS_ERR_WRITE"; } break;
        case GEN_FS_ERR_READ:                        {return "GEN_FS_ERR_READ"; } break;
        case GEN_FS_ERR_FORMAT:                      {return "GEN_FS_ERR_FORMAT"; } break;
        case GEN_ERROR_MEMORY:                       {return "GEN_ERROR_MEMORY"; } break;
        case GEN_ERROR_RECORD_NOT_FOUND:             {return "GEN_ERROR_RECORD_NOT_FOUND"; } break;
        case GEN_VER_INVALID_RECORD_FORMAT:          {return "GEN_VER_INVALID_RECORD_FORMAT"; } break;
        case GEN_ERROR_DB_FULL:                      {return "GEN_ERROR_DB_FULL"; } break;
        case GEN_ERROR_INVALID_SIZE:                 {return "GEN_ERROR_INVALID_SIZE"; } break;
        case GEN_ERROR_TAG_NOT_FOUND:                {return "GEN_ERROR_TAG_NOT_FOUND"; } break;
        case GEN_ERROR_APP_BUSY:                     {return "GEN_ERROR_APP_BUSY"; } break;
        case GEN_ERROR_DEVICE_UNCONFIGURED:          {return "GEN_ERROR_DEVICE_UNCONFIGURED"; } break;
        case GEN_ERROR_TIMEOUT_LATENT:               {return "GEN_ERROR_TIMEOUT_LATENT"; } break;
        case GEN_ERROR_DB_NOT_LOADED:                {return "GEN_ERROR_DB_NOT_LOADED"; } break;
        case GEN_ERROR_DB_DOESNOT_EXIST:             {return "GEN_ERROR_DB_DOESNOT_EXIST"; } break;
        case GEN_ERROR_ENROLLMENT_INCOMPLETE:        {return "GEN_ERROR_ENROLLMENT_INCOMPLETE"; } break;
        case GEN_ERROR_USER_NOT_FOUND:               {return "GEN_ERROR_USER_NOT_FOUND"; } break;
        case GEN_ERROR_DB_USER_FINGERS_FULL:         {return "GEN_ERROR_DB_USER_FINGERS_FULL"; } break;
        case GEN_ERROR_DB_USERS_FULL:                {return "GEN_ERROR_DB_USERS_FULL"; } break;
        case GEN_ERROR_USER_EXISTS:                  {return "GEN_ERROR_USER_EXISTS"; } break;
        case GEN_ERROR_DEVICE_NOT_FOUND:             {return "GEN_ERROR_DEVICE_NOT_FOUND"; } break;
        case GEN_ERROR_DEVICE_NOT_READY:             {return "GEN_ERROR_DEVICE_NOT_READY"; } break;
        case GEN_ERROR_PIPE_READ:                    {return "GEN_ERROR_PIPE_READ"; } break;
        case GEN_ERROR_PIPE_WRITE:                   {return "GEN_ERROR_PIPE_WRITE"; } break;
        case GEN_ERROR_SENGINE_SHUTTING_DOWN:        {return "GEN_ERROR_SENGINE_SHUTTING_DOWN"; } break;
        case GEN_ERROR_SPOOF_DETECTED:               {return "GEN_ERROR_SPOOF_DETECTED"; } break;
        case GEN_ERROR_DATA_UNAVAILABLE:             {return "GEN_ERROR_DATA_UNAVAILABLE"; } break;
        case GEN_ERROR_CRYPTO_FAIL:                  {return "GEN_ERROR_CRYPTO_FAIL"; } break;
        case GEN_ERROR_CAPTURE_CANCELLED:            {return "GEN_ERROR_CAPTURE_CANCELLED"; } break;
        case GEN_ERROR_CONNECTION_REFUSED:           {return "GEN_ERROR_CONNECTION_REFUSED"; } break;
        case GEN_LAST:                               {return "GEN_LAST"; } break;
        //<TODO> update codes
        default:                                     {return "** Unknown Error Code **"; } break;
    }

}

// Translate OpError to string
VCOM_CORE_EXPORT const char* GetOpErrorStr(uint nParameter) {
    switch (nParameter) {
        case STATUS_OK:                                 {return "STATUS_OK"; } break;
        case ERROR_UID_EXISTS:                          {return "ERROR_UID_EXISTS"; } break;
        case ERROR_ENROLLMENT_QUALIFICATION:            {return "ERROR_ENROLLMENT_QUALIFICATION"; } break;
        case ERROR_UID_DOES_NOT_EXIST:                  {return "ERROR_UID_DOES_NOT_EXIST"; } break;
        case ERROR_DB_FULL:                             {return "ERROR_DB_FULL"; } break;
        case ERROR_QUALIFICATION:                       {return "ERROR_QUALIFICATION"; } break;
        case ERROR_DEV_TIMEOUT:                         {return "ERROR_DEV_TIMEOUT"; } break;
        case ERROR_USER_CANCELLED:                      {return "ERROR_USER_CANCELLED"; } break;
        case ERROR_SPOOF_DETECTED:                      {return "ERROR_SPOOF_DETECTED"; } break;
        case ERROR_DB_EXISTS:                           {return "ERROR_DB_EXISTS"; } break;
        case ERROR_DB_DOES_NOT_EXIST:                   {return "ERROR_DB_DOES_NOT_EXIST"; } break;
        case ERROR_ID_DB_TOO_LARGE:                     {return "ERROR_ID_DB_TOO_LARGE"; } break;
        case ERROR_ID_DB_EXISTS:                        {return "ERROR_ID_DB_EXISTS"; } break;
        case ERROR_ID_USER_EXISTS:                      {return "ERROR_ID_USER_EXISTS"; } break;
        case ERROR_ID_USER_NOT_FOUND:                   {return "ERROR_ID_USER_NOT_FOUND"; } break;
        case STATUS_ID_MATCH:                           {return "STATUS_ID_MATCH"; } break;
        case STATUS_ID_NO_MATCH:                        {return "STATUS_ID_NO_MATCH"; } break;
        case ERROR_ID_PARAMETER:                        {return "ERROR_ID_PARAMETER"; } break;
        case ERROR_ID_GENERAL:                          {return "ERROR_ID_GENERAL"; } break;
        case ERROR_ID_FILE:                             {return "ERROR_ID_FILE"; } break;
        case ERROR_ID_NOT_INITIALIZED:                  {return "ERROR_ID_NOT_INITIALIZED"; } break;
        case ERROR_ID_DB_FULL:                          {return "ERROR_ID_DB_FULL"; } break;
        case ERROR_ID_DB_DOESNT_EXIST:                  {return "ERROR_ID_DB_DOESNT_EXIST"; } break;
        case ERROR_ID_DB_NOT_LOADED:                    {return "ERROR_ID_DB_NOT_LOADED"; } break;
        case ERROR_ID_RECORD_NOT_FOUND:                 {return "ERROR_ID_RECORD_NOT_FOUND"; } break;
        case ERROR_ID_FS:                               {return "ERROR_ID_FS"; } break;
        case ERROR_ID_CREATE_FAIL:                      {return "ERROR_ID_CREATE_FAIL"; } break;
        case ERROR_ID_INTERNAL:                         {return "ERROR_ID_INTERNAL"; } break;
        case ERROR_ID_MEM:                              {return "ERROR_ID_MEM"; } break;
        case STATUS_ID_USER_FOUND:                      {return "STATUS_ID_USER_FOUND"; } break;
        case STATUS_ID_USER_NOT_FOUND:                  {return "STATUS_ID_USER_NOT_FOUND"; } break;
        case ERROR_ID_USER_FINGERS_FULL:                {return "ERROR_ID_USER_FINGERS_FULL"; } break;
        case ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND:     {return "ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND"; } break;
        case ERROR_ID_USERS_FULL:                       {return "ERROR_ID_USERS_FULL"; } break;
        case ERROR_ID_OPERATION_NOT_SUPPORTED:          {return "ERROR_ID_OPERATION_NOT_SUPPORTED"; } break;
        case ERROR_ID_NOT_ENOUGH_SPACE:                 {return "ERROR_ID_NOT_ENOUGH_SPACE"; } break;
        case ERROR_ID_DUPLICATE:                        {return "ERROR_ID_DUPLICATE"; } break;
        case ERROR_CAPTURE_TIMEOUT:                     {return "ERROR_CAPTURE_TIMEOUT"; } break;
        case ERROR_CAPTURE_LATENT:                      {return "ERROR_CAPTURE_LATENT"; } break;
        case ERROR_CAPTURE_CANCELLED:                   {return "ERROR_CAPTURE_CANCELLED"; } break;
        case ERROR_CAPTURE_INTERNAL:                    {return "ERROR_CAPTURE_INTERNAL"; } break;
        case ERROR_UPDATE_MEMORY_ERROR:                 {return "ERROR_UPDATE_MEMORY_ERROR"; } break;
        case ERROR_UPDATE_DECRYPTION_FAIL:              {return "ERROR_UPDATE_DECRYPTION_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_VERSION_ERROR:       {return "ERROR_UPDATE_FIRMWARE_VERSION_ERROR"; } break;
        case ERROR_UPDATE_FLASH_WRITE_ERROR:            {return "ERROR_UPDATE_FLASH_WRITE_ERROR"; } break;
        case ERROR_UPDATE_INVALID_TYPE:                 {return "ERROR_UPDATE_INVALID_TYPE"; } break;
        case ERROR_UPDATE_FORMAT_ERROR:                 {return "ERROR_UPDATE_FORMAT_ERROR"; } break;
        case ERROR_UPDATE_FIRMWARE_SIZE_ERROR:          {return "ERROR_UPDATE_FIRMWARE_SIZE_ERROR"; } break;
        case ERROR_UPDATE_RESTORE_FAIL:                 {return "ERROR_UPDATE_RESTORE_FAIL"; } break;
        case ERROR_UPDATE_FIRMWARE_INVALID:             {return "ERROR_UPDATE_FIRMWARE_INVALID"; } break;
        case ERROR_CRYPTO_ERROR:                        {return "ERROR_CRYPTO_ERROR"; } break;
        case ERROR_UPDATE_UNPACK_FAILURE:               {return "ERROR_UPDATE_UNPACK_FAILURE"; } break;
        case ERROR_UPDATE_THREAD_START:                 {return "ERROR_UPDATE_THREAD_START"; } break;
        case ERROR_UPDATE_EXEC_SCRIPT_FAIL:             {return "ERROR_UPDATE_EXEC_SCRIPT_FAIL"; } break;
        case ERROR_UPDATE_EXEC_SCRIPT_TIMEOUT:          {return "ERROR_UPDATE_EXEC_SCRIPT_TIMEOUT"; } break;
        case ERROR_UPDATE_EXEC_NON_ZERO:                {return "ERROR_UPDATE_EXEC_NON_ZERO"; } break;
        case ERROR_UPDATE_CFG_INVALID:                  {return "ERROR_UPDATE_CFG_INVALID"; } break;
        case ERROR_UPDATE_REMOVE_KEYS_FAILED:           {return "ERROR_UPDATE_REMOVE_KEYS_FAILED"; } break;
        case STATUS_NO_OP:                              {return "STATUS_NO_OP"; } break;
        default:                                        {return "UNKNOWN ERROR"; } break;
    }
    return "Unknown _V100_OP_STATUS";
}

// Translate sensor type to string
VCOM_CORE_EXPORT const char* GetSensorTypeStr(uint nType) {
    switch (nType) {
        case VENUS_V30X:            {return "VENUS V30X"; } break;
        case MERCURY_M30X:          {return "MERCURY M30X"; } break;
        case MERCURY_M31X:          {return "MERCURY M31X"; } break;
        case VENUS_V31X:            {return "VENUS V31X"; } break;
        case VENUS_V371:            {return "VENUS V371"; } break;
        case VENUS_V40X:            {return "VENUS V40X"; } break;
        case VENUS_V42X:            {return "VENUS V42X"; } break;
        case MERCURY_M32X:          {return "MERCURY M32X"; } break;
        case MERCURY_M42X:          {return "MERCURY M42X"; } break;
        case MERCURY_M21X:          {return "MERCURY M21X"; } break;
        case VENUS_V31X_10:         {return "VENUS V31X_10"; } break;
        case VENUS_V32X:            {return "VENUS V32X"; } break;
        case VENUS_V52X:            {return "VENUS_V52X"; } break;
        case UNKNOWN_LUMI_DEVICE:
        default:                    {return "UNKOWN_LUMI_DEVICE"; } break;
    }
}
