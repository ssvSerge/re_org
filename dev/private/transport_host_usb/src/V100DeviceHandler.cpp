/***************************************************************************************/
// �Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
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


#include <string.h>

#include "V100DeviceHandler.h"

#include "IMemMgr.h"

#ifndef __GNUC__

    #include "windows.h"
    #include "ldiguid.h"
    #include "setupapi.h"
    #include "usbdriver.h"
    #include "LDSNamedPipe.h"
    #include "LumiServiceTypes.h"

    #define ACQUIRE_SYNC_MUTEX if (hSyncMutex) WaitForSingleObject(hSyncMutex, INFINITE);
    #define RELEASE_SYNC_MUTEX if (hSyncMutex) ReleaseMutex(hSyncMutex);

    HANDLE V100DeviceHandler::hSyncMutex = 0;

#else
    #include "windows_conv.h"
    #include "TransportLibUSB.h"

    #define ACQUIRE_SYNC_MUTEX
    #define RELEASE_SYNC_MUTEX

#endif

#include <iostream>
using namespace std;

static V100DeviceHandler* g_pV100DeviceHandler = NULL;

V100DeviceHandler* V100DeviceHandler::GetV100DeviceHandler()
{
    ACQUIRE_SYNC_MUTEX
    {
        if (!g_pV100DeviceHandler)
            g_pV100DeviceHandler = new V100DeviceHandler();
    }
    RELEASE_SYNC_MUTEX
    return g_pV100DeviceHandler;
}
void V100DeviceHandler::ReleaseV100DeviceHandler()
{
    ACQUIRE_SYNC_MUTEX
    {
        if (g_pV100DeviceHandler != NULL)
        {
            delete g_pV100DeviceHandler;
            g_pV100DeviceHandler = NULL;
        }
    }
    RELEASE_SYNC_MUTEX
}

V100DeviceHandler::V100DeviceHandler(void)
{
    m_nNumStreamingDevices = 0;
    m_nNumRegularDevices = 0;
    //
    memset(m_ServerName, 0, 2048);
    #ifndef __GNUC__
        if( NULL == V100DeviceHandler::hSyncMutex)
            V100DeviceHandler::hSyncMutex = CreateMutex(NULL, FALSE, L"LumiDeviceHdlMutex");
    #endif
}

V100DeviceHandler::~V100DeviceHandler(void)
{
    #ifndef __GNUC__
        if (NULL != V100DeviceHandler::hSyncMutex)
        {
            // Enter critical section to release and NULL static member
            // If you don't use a critical section, and the constructor for
            // V100CommandHandler is called by a different thread, there is a chance
            // that the mutex is closed, but not nulled, and bad things ensue.
            CRITICAL_SECTION critSec;
            InitializeCriticalSection(&critSec);
            EnterCriticalSection(&critSec);
            {
                ReleaseMutex(V100DeviceHandler::hSyncMutex);
                CloseHandle(V100DeviceHandler::hSyncMutex);
                V100DeviceHandler::hSyncMutex = NULL;
            }
            LeaveCriticalSection(&critSec);
            DeleteCriticalSection(&critSec);
        }
    #endif
    g_pV100DeviceHandler = NULL;
}


void V100DeviceHandler::SetServerName(char* pServerName)
{
    strcpy(m_ServerName, pServerName);
}

#ifndef __GNUC__
V100_ERROR_CODE V100DeviceHandler::GetNumDevices(int * nNumDevices)
{
    V100_ERROR_CODE result = GEN_OK;
    *nNumDevices = 0;
    m_nNumStreamingDevices = 0;
    m_nNumRegularDevices = 0;

    // Need to check for both embedded and streaming devices.

    // Part 1
    // - Check for embedded devices using the normal USB driver interface
    m_nNumRegularDevices = USBDriverInterface::QueryNumNonStrDevices((LPGUID)&GUID_BULKLDI);


    // Part 2
    // - Check for streaming devices by getting the number of devices from the LDS
    LDSNamedPipe* pNamedPipe = new LDSNamedPipe();
    Device*       pDevice = new Device();

    // Get the pipe name to the LDS
    pDevice->GetCommServerName((uchar*)m_ServerName);

    if (pDevice)
    {
        delete pDevice;
        pDevice = nullptr;
    }

    char strServerPath[2048] = {};
    sprintf(strServerPath, "\\\\%s\\PIPE\\LumiDeviceServiceMainPipe", m_ServerName);

    // Try to connect to the LDS
    if (true == pNamedPipe->Initialize(Client, strServerPath))
    {
        LumiDevServiceCmdStruct ldscmd;
        memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
        ldscmd.Command = LUMI_DS_CMD_GET_NUM_SENSORS;
        memset(ldscmd.strCommander, 0, 256);
        sprintf(ldscmd.strCommander, "V100DeviceHandler::GetNumDevices");

        if (false == pNamedPipe->WritePipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_WRITE;
            goto ExitGetNumDev;
        }

        if (false == pNamedPipe->ReadPipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_READ;
            goto ExitGetNumDev;
        }

        m_nNumStreamingDevices = ldscmd.devNumber;
        result = GEN_OK;
    }

ExitGetNumDev:
    delete pNamedPipe;
    pNamedPipe = NULL;

    *nNumDevices = m_nNumRegularDevices + m_nNumStreamingDevices;

    return result;
}

#else

// NOTE: POSIX libusb support only handles a single (or no) device in this release.
V100_ERROR_CODE V100DeviceHandler::GetNumDevices(int * nNumDevices)
{
    *nNumDevices = 0;
    uint16_t vid, pid;
    uint8_t bus, adr;
    bool bRC = TransportUSB::FindSupportedDevice(vid, pid, bus, adr);
    if (bRC)
        *nNumDevices = 1;
    return GEN_OK;
}
#endif

void V100DeviceHandler::GetStreamingDevCount(int* nNumDevices)
{
    *nNumDevices = m_nNumStreamingDevices;
}

void V100DeviceHandler::GetRegDevCount(int* nNumDevices)
{
    *nNumDevices = m_nNumRegularDevices;
}

#ifndef __GNUC__
// LumiDeviceService version...
V100_ERROR_CODE V100DeviceHandler::GetDevice(uint deviceNumber, Device* pDevice)
{
    //NOTE:
    // This function now has two parts. Part 1 is for looking for the specified device index (deviceNumber) in the set of known
    // streaming devices. Part 2 is for looking the specified device index (deviceNumber) in the set of known embedded devices.
    //
    // If no Lumi Device Service is running or if only embedded devices are connected then the device requested should be found in Part 2.
    // If only streaming devices are connected, the device requested should be found in Part 1.
    //
    // If mixing streaming and embedded devices the streaming devices will be indexed first followed by embedded devices.
    //      For example, V30x in port 1, V31x in port 2, M21x in port 3 and V42x in port 4 they will be indexed as follows:
    //         0 V31x; 1 M21x; 2 V30x; 3 V42x
    //
    // If using a mixture, it will use Part 1 to find the device requested. If it determines that the index is past the number of streaming
    // devices it will use Part 2 to find the requested device.

    V100_ERROR_CODE result = GEN_OK;

    // Need to check for both embedded and streaming devices.

    // Part 1
    // - Check if the deviceNumber requested falls into the range of streaming devices. If so get device info from LDS
    //   If we can't talk to the LDS just skip to part 2.
    char ServerPath[2048];
    memset(ServerPath, 0, 2048);
    sprintf(ServerPath, "\\\\%s\\PIPE\\LumiDeviceServiceMainPipe", pDevice->GetCommServerName());
    // Get the device information from the LDS
    LDSNamedPipe* pPipeToLDS = new LDSNamedPipe();

    if (true == pPipeToLDS->Initialize(Client, ServerPath))
    {
        // First check how many streaming devices we should expect
        LumiDevServiceCmdStruct ldscmd;
        memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
        ldscmd.Command = LUMI_DS_CMD_GET_NUM_SENSORS;
        memset(ldscmd.strCommander, 0, 256);
        sprintf(ldscmd.strCommander, "V100DeviceHandler::GetNumDevices");

        if (false == pPipeToLDS->WritePipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_WRITE;
            goto ExitGetDev;
        }

        if (false == pPipeToLDS->ReadPipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_READ;
            goto ExitGetDev;
        }

        m_nNumStreamingDevices = ldscmd.devNumber;

        // Make sure we are trying to connect to one of the streaming devices.
        // If the index we are looking for is greater than the number of streaming devices
        // then break out of this section and try to connect to an embedded device
        if (deviceNumber < (uint)m_nNumStreamingDevices)
        {
            // Setup the command to get each streaming device info
            memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
            ldscmd.Command = LUMI_DS_CMD_GET_SENSOR_INFO;
            ldscmd.devNumber = deviceNumber;
            memset(ldscmd.strCommander, 0, 256);
            sprintf(ldscmd.strCommander, "V100DeviceHandler::GetDevice");

            // Setup the return structures
            LumiDevStruct devInfo;
            memset(&devInfo, 0, sizeof(LumiDevStruct));
            LumiDevServicePipeStruct ldsps;
            memset(&ldsps, 0, sizeof(LumiDevServicePipeStruct));

            // Send the command
            if (false == pPipeToLDS->WritePipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
            {
                result = GEN_ERROR_PIPE_WRITE;
                goto ExitGetDev;
            }

            // Get the device info back
            if (false == pPipeToLDS->ReadPipe((uchar*)&devInfo, sizeof(LumiDevStruct), 100))
            {
                result = GEN_ERROR_PIPE_READ;
                goto ExitGetDev;
            }

            // Check the error code
            if (LDEC_OK != devInfo.nErrorCode)
            {
                switch (devInfo.nErrorCode)
                {
                case LDEC_SEngine_Not_Ready:
                case LDEC_SEngine_Not_Running:
                    result = GEN_ERROR_DEVICE_NOT_READY;
                    break;
                case LDEC_Device_Pipe_Read_Error:
                    result = GEN_ERROR_PIPE_READ;
                    break;
                case LDEC_Device_Pipe_Write_Error:
                    result = GEN_ERROR_PIPE_WRITE;
                    break;
                case LDEC_Device_Not_Found:
                default:
                    result = GEN_ERROR_DEVICE_NOT_FOUND;
                    break;
                }
                goto ExitGetDev;
            }

            // Get the streaming device's pipe name
            memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
            ldscmd.Command = LUMI_DS_CMD_GET_NAMED_PIPE;
            ldscmd.devNumber = devInfo.nDeviceIndex;
            memset(ldscmd.strCommander, 0, 256);
            sprintf(ldscmd.strCommander, "V100DeviceHandler::GetDevice");

            if (false == pPipeToLDS->WritePipe((uchar*)&ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
            {
                result = GEN_ERROR_PIPE_WRITE;
                goto ExitGetDev;
            }

            if (false == pPipeToLDS->ReadPipe((uchar*)&ldsps, sizeof(LumiDevServicePipeStruct), 100))
            {
                result = GEN_ERROR_PIPE_READ;
                goto ExitGetDev;
            }

            // Check for NULL pipe names, don't create the device pointer (don't care now about the device name)
            if (NULL == ldsps.ulPipeName)
            {
                result = GEN_ERROR_DEVICE_NOT_READY;
                goto ExitGetDev;
            }

            // Found it, initialize the device object and bail
            pDevice->Init(devInfo.nDeviceIndex, V100_STREAMING_DEVICE, ldsps.ulPipeName, ldsps.ulDeviceName, devInfo.nMemberIndex, devInfo.nSensorType);
            result = GEN_OK;
            goto ExitGetDev;
        }
    }

    // Part 2
    // - If the deviceNumber requested falls into the range of embedded devices or if the LumiDeviceService is not running,
    //   check for an embedded device
    m_nNumRegularDevices = USBDriverInterface::QueryNumNonStrDevices((LPGUID)&GUID_BULKLDI);
    if (m_nNumRegularDevices == 0 || deviceNumber > (uint)(m_nNumRegularDevices + m_nNumStreamingDevices))
    {
        result = GEN_ERROR_DEVICE_NOT_FOUND;
    }
    else
    {
        // Initialize the device object and bail
        pDevice->Init(deviceNumber, V100_NORMAL_DEVICE, NULL, NULL, deviceNumber - m_nNumStreamingDevices, 0);
        result = GEN_OK;
    }


ExitGetDev:
    delete pPipeToLDS;
    pPipeToLDS = NULL;

    return result;
}

#else    // this is the POSIX libusb version, which does a quick trip to libusbland...

#include <dlfcn.h>
#include "IXServiceProvider.h"
#include "strm_shared.h"

// libSEngineCore.so bindings for tethered / streaming sensor support
static void * DLSEngineHandle = NULL;
static IXServiceProvider * (*GSP)() = NULL;
static int (*SECs)(int argc, char ** argv) = NULL;
static void (*SECx)() = NULL;
IStreamDvc * SEngineGetStreamDvc(int type)
{
    assert(GSP);
    return (*GSP)()->GetStreamDvc(type);
}
int SEngineCore_start(int argc, char ** argv)
{
    assert(SECs);
    return (*SECs)(argc, argv);
}
void SEngineCore_shutdown()
{
    assert(SECx);
    return (*SECx)();
}

// libSEngineCore.so loader
static int DLOpenSEngine()
{
    // this simply clears the last dl error, just in case any were hanging around...
    dlerror();
    if (DLSEngineHandle)
        return 0;
    DLSEngineHandle = dlopen("libSEngine.so", RTLD_LAZY);
    if (DLSEngineHandle)
    {
        GSP  = (IXServiceProvider*(*)()) dlsym(DLSEngineHandle, "GetServiceProvider");
        SECs = (int (*)(int, char**))    dlsym(DLSEngineHandle, "SEngineCore_start");
        SECx = (void (*)())              dlsym(DLSEngineHandle, "SEngineCore_shutdown");
        if (GSP && SECs && SECx)
            return 0;
        dlclose(DLSEngineHandle);
        DLSEngineHandle = NULL;
        GSP  = NULL;
        SECs = NULL;
        SECx = NULL;
        fprintf(stderr, "dlsym: unable to locate required SEngine symbols - aborting load.\n");
        return -1;
    }
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
}

V100_ERROR_CODE V100DeviceHandler::GetDevice(uint deviceNumber, Device* pDevice)
{
    V100_ERROR_CODE rc = GEN_ERROR_DEVICE_NOT_FOUND;

    uint16_t vid, pid;
    uint8_t bus, adr;
    bool bRC = TransportUSB::FindSupportedDevice(vid, pid, bus, adr);
    if (bRC)
    {
        //fprintf(stdout, "Device: %04X:%04X\n", desc.idVendor, desc.idProduct);

             if (vid == TransportUSB::M30X_VENDOR && pid == TransportUSB::M30X_PRODUCT)
        {
            pDevice->Init(deviceNumber, V100_NORMAL_DEVICE, (uchar*) "COMM-PIPE-DUMMY", (uchar*) "M30X", 0, MERCURY_M30X);
            //fprintf(stderr, "V100DeviceHandler::GetDevice: found Mercury 30x\n");
            rc = GEN_OK;
        }
        else if (vid == TransportUSB::V30X_VENDOR && pid == TransportUSB::V30X_PRODUCT)
        {
            pDevice->Init(deviceNumber, V100_NORMAL_DEVICE, (uchar*) "COMM-PIPE-DUMMY", (uchar*) "V30X", 0, VENUS_V30X);
            //fprintf(stderr, "V100DeviceHandler::GetDevice: found Venus NS\n");
            rc = GEN_OK;
        }
        else if (vid == TransportUSB::V52X_VENDOR && pid == TransportUSB::V52X_PRODUCT)
        {
            pDevice->Init(deviceNumber, V100_NORMAL_DEVICE, (uchar*)"COMM-PIPE-DUMMY", (uchar*)"V52X", 0, VENUS_V32X);
            rc = GEN_OK;
        }
        else
        {
            const char * pcType = NULL;
                 if (vid == TransportUSB::V31X_VENDOR && pid == TransportUSB::V31X_PRODUCT)
                pcType = "Venus";
            else if (vid == TransportUSB::M31X_VENDOR && pid == TransportUSB::M31X_PRODUCT)
                pcType = "Mercury";
            else if (vid == TransportUSB::M21X_VENDOR && pid == TransportUSB::M21X_PRODUCT)
                pcType = "M210";
            if (pcType)
            {
                if (!DLOpenSEngine())
                {
                    pDevice->Init(deviceNumber, V100_STREAMING_DEVICE, (uchar*) "COMM-PIPE-DUMMY", (uchar*) "V31X", 0, VENUS_V31X);
                    char * argv[13] =
                                    {
                                        (char*) "binary", (char*) "-D", (char*) pcType, (char*) "-C", (char*) "-", (char*) "-S",
                                        (char*) "-", (char*) "-I", (char*) "0", (char*) "-U", (char*) "-", (char*) "-L", (char*) "1"
                                    };
                    if (SEngineCore_start(13, argv) == 0)
                    {
                        rc = GEN_OK;
                    }
                    else
                    {
                        rc = GEN_ERROR_DEVICE_NOT_READY;
                    }
                }
                else
                {
                    rc = GEN_ERROR_DEVICE_NOT_READY;
                }
            }
            else
            {
                rc = GEN_ERROR_DEVICE_NOT_FOUND;
            }
        }
    }
    else
    {
        rc = GEN_ERROR_DEVICE_NOT_FOUND;
    }

    return rc;
}

#endif
