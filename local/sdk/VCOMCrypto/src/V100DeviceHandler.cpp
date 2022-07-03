/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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
    int nNumDev = 0;

    // Get the number of devices from the LDS
    LDSNamedPipe* namedPipe = new LDSNamedPipe();
    char ServerPath[2048];
    
    
    Device* pDevice = new Device();

    pDevice->GetCommServerName((uchar*)m_ServerName);

    if(pDevice) delete pDevice;

    sprintf(ServerPath, "\\\\%s\\PIPE\\LumiDeviceServiceMainPipe", m_ServerName);
    if( true == namedPipe->Initialize(Client, ServerPath))
    {
        LumiDevServiceCmdStruct ldscmd;
        memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
        ldscmd.Command = LUMI_DS_CMD_GET_NUM_SENSORS;
        memset(ldscmd.strCommander, 0, 256);
        sprintf(ldscmd.strCommander, "V100DeviceHandler::GetNumDevices");

        if(false == namedPipe->WritePipe((uchar*) &ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_WRITE;
            goto ExitGetNumDev;
        }

        if(false == namedPipe->ReadPipe((uchar*) &ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_READ;
            goto ExitGetNumDev;
        }

        nNumDev = ldscmd.devNumber;
        result = GEN_OK;
    }
    else
    {
        // If LumiDeviceService is not running
        nNumDev = USBDriverInterface::QueryNumNonStrDevices( (LPGUID)&GUID_BULKLDI );
        result = GEN_OK;
    }
    
ExitGetNumDev:
    delete namedPipe;
    namedPipe = NULL;    

    *nNumDevices = nNumDev;

    return result;
}

#else

// NOTE: POSIX libusb support only handles a single (or no) device in this release.
V100_ERROR_CODE V100DeviceHandler::GetNumDevices(int * nNumDevices) {
    *nNumDevices = 0;
    uint16_t vid, pid;
    uint8_t bus, adr;
    bool bRC = TransportUSB::FindSupportedDevice(vid, pid, bus, adr);
    if (bRC) {
        *nNumDevices = 1;
    }
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
    V100_ERROR_CODE result = GEN_OK;
    char ServerPath[2048];
    memset(ServerPath, 0, 2048);
    sprintf(ServerPath,"\\\\%s\\PIPE\\LumiDeviceServiceMainPipe", pDevice->GetCommServerName());
    // Get the device information from the LDS
    LDSNamedPipe* namedPipe = new LDSNamedPipe();
    //
    if( true == namedPipe->Initialize(Client, ServerPath))
    {
        // Setup the command
        LumiDevServiceCmdStruct ldscmd;
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
        if(false == namedPipe->WritePipe((uchar*) &ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
        {
            result = GEN_ERROR_PIPE_WRITE;
            goto ExitGetDev;
        }

        // Get the info back
        if(false == namedPipe->ReadPipe((uchar*) &devInfo, sizeof(LumiDevStruct), 100))
        {
            result = GEN_ERROR_PIPE_READ;
            goto ExitGetDev;
        }

        // Check the error code
        if(LDEC_OK != devInfo.nErrorCode)
        {
            switch(devInfo.nErrorCode)
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

        if(devInfo.nSensorType >= MERCURY_M31X)
        {
            // We have a streaming device to map
            // Get the pipe name
            memset(&ldscmd, 0, sizeof(LumiDevServiceCmdStruct));
            ldscmd.Command = LUMI_DS_CMD_GET_NAMED_PIPE;
            ldscmd.devNumber = devInfo.nDeviceIndex;
            memset(ldscmd.strCommander, 0, 256);
            sprintf(ldscmd.strCommander, "V100DeviceHandler::GetDevice");

            if(false == namedPipe->WritePipe((uchar*) &ldscmd, sizeof(LumiDevServiceCmdStruct), 100))
            {
                result = GEN_ERROR_PIPE_WRITE;
                goto ExitGetDev;
            }

            if(false == namedPipe->ReadPipe((uchar*) &ldsps, sizeof(LumiDevServicePipeStruct), 100))
            {
                result = GEN_ERROR_PIPE_READ;
                goto ExitGetDev;
            }

            // Check for NULL pipe names, don't create the device pointer (don't care now about the device name)
            if(NULL == ldsps.ulPipeName)
            {
                result = GEN_ERROR_DEVICE_NOT_READY;
                goto ExitGetDev;
            }

            pDevice->Init(devInfo.nDeviceIndex, V100_STREAMING_DEVICE, ldsps.ulPipeName, ldsps.ulDeviceName, devInfo.nMemberIndex, devInfo.nSensorType);

            m_nNumStreamingDevices++;
        }
        else
        {
            // We have a regular device, just return it
            pDevice->Init(devInfo.nDeviceIndex, V100_NORMAL_DEVICE, ldsps.ulPipeName, ldsps.ulDeviceName, devInfo.nMemberIndex, devInfo.nSensorType);

            m_nNumRegularDevices++;
        }
    }
    else
    {
        // If LumiDeviceService is not running
        m_nNumRegularDevices = USBDriverInterface::QueryNumNonStrDevices( (LPGUID)&GUID_BULKLDI );
        // Add regular device to map
        for(int i = 0; i < m_nNumRegularDevices; i++)
        {
            if(i == deviceNumber)
            {
                pDevice->Init(i, V100_NORMAL_DEVICE, NULL, NULL, i, 0); 
                goto ExitGetDev;
            }
        }

        result = GEN_ERROR_DEVICE_NOT_FOUND;
    }

ExitGetDev:
    delete namedPipe;
    namedPipe = NULL;    

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
