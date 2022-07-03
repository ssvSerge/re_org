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
#pragma once
#ifndef WIN32
    #include "windows_conv.h"
#else
    #include <windows.h>
#endif
#include "stdio.h"

typedef unsigned int uint;
typedef unsigned char uchar;

typedef enum
{
    V100_NORMAL_DEVICE        = 0x00,
    V100_STREAMING_DEVICE    = 0x01
} V100_DEVICE_TYPE;

#define SZ_OF_PIPE_NAME 260
#define REG_BUF_SIZE 512

class Device
{
public:
    Device(void);
    Device(uint deviceNumber, V100_DEVICE_TYPE deviceType, uchar* strCommPipeName, uchar* strDeviceName, uint nMemberIndex, uint nSensorType);
    ~Device();
    void Init(uint deviceNumber, V100_DEVICE_TYPE deviceType, uchar* strCommPipeName, uchar* strDeviceName, uint nMemberIndex, uint nSensorType);
    V100_DEVICE_TYPE GetDeviceType();
    void GetCommPipeName(uchar* strCommPipeName);
    void GetCommServerName(uchar* strCommPipeName);
    uchar* GetCommServerName() { return m_commServerName; }
    uint GetDeviceNumber();
    uint m_nMemberIndex;
private:
    V100_DEVICE_TYPE m_deviceType;
    uchar m_commPipeName[SZ_OF_PIPE_NAME];
    uchar m_commServerName[SZ_OF_PIPE_NAME];
    uchar m_deviceName[SZ_OF_PIPE_NAME];
    uint m_deviceNumber;
    uint m_nSensorType;
    bool GetEnvVar(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, char* strEnvVarOut);
};
