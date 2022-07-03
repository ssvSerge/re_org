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
#pragma once
#include <string>
#include "LumiDeviceServiceCommands.h"


typedef unsigned int  uint;
typedef unsigned char uchar;

typedef struct
{
  unsigned char ulDeviceName[MAX_PATH];            // generic data field
  unsigned char ulPipeName[MAX_PATH];            // generic data field
} LumiDevServicePipeStruct;

typedef struct
{
  unsigned char strData[MAX_PATH];            // generic data field
 } LumiDevServiceGenericStrStruct;

typedef struct
{
    unsigned int nDeviceIndex;
    unsigned int nMemberIndex;
    unsigned int nSensorType;
    unsigned int nSerialNumber;
    unsigned int nFWRev;
    unsigned int nIdentification;
    unsigned int nImageOut;
    unsigned int nSpoofOut;
    char         strSKU[MAX_PATH];
    unsigned int nTransportType;
    unsigned int nExtractor;
    char         strIDDBLoc[2048];
    unsigned int nDevType;
    unsigned int nErrorCode;
} LumiDevStruct;

typedef enum
{
    LDEC_OK = 0,
    LDEC_Invalid_Command,
    LDEC_SEngine_Not_Running,
    LDEC_SEngine_Not_Ready,
    LDEC_Device_Comm_Error,
    LDEC_Device_Not_Found,
    LDEC_Device_Pipe_Read_Error,
    LDEC_Device_Pipe_Write_Error,
    LDEC_Fatal_Error = 99,
} LumiDevErrCode;

//Class guid from bulkusb.inf {06B62F68-E35E-4f57-8BE2-A5BC6CCCC708} - This is the class guid for LumiSensors
//static const GUID MY_GUID = { 0x06B62F68, 0xE35E, 0x4f57, { 0x8B, 0xE2, 0xA5, 0xBC, 0x6C, 0xCC, 0xC7, 0x08 } };

//5F70A64F-A8BE-456e-9ADA-D341C668410B
// Streaming Device GUID
static const GUID STRM_DVC_GUID = { 0x5F70A64F, 0xA8BE, 0x456E, { 0x9A, 0xdA, 0xD3, 0x41, 0xC6, 0x68, 0x41, 0x0B } };

// Regular Device GUID
static const GUID REG_DVC_GUID = { 0x06B62F68, 0xE35E, 0x4f57, { 0x8B, 0xE2, 0xA5, 0xBC, 0x6C, 0xCC, 0xC7, 0x08 } };



// Define the message from the LumiDevice object to the Lumi Device Service
#define LD_THREAD_MESSAGE    WM_USER+1

// Define the Thread Message Parameters
#define RESTART_SENGINE        99