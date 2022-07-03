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

#include "Device.h"

Device::Device(void)
{
    memset(m_commPipeName, 0, SZ_OF_PIPE_NAME);
    memset(m_deviceName, 0, SZ_OF_PIPE_NAME);
    m_deviceType   = V100_NORMAL_DEVICE;
    m_deviceNumber = 0;
    m_nMemberIndex = 0;
    m_nSensorType  = 0;
    memset(m_commServerName, 0, SZ_OF_PIPE_NAME);
    GetCommServerName(m_commServerName);
}

Device::Device(uint deviceNumber, V100_DEVICE_TYPE deviceType, uchar* strCommPipeName, uchar* strDeviceName, uint nMemberIndex, uint nSensorType)
{
    memset(m_commPipeName, 0, SZ_OF_PIPE_NAME);
    memset(m_deviceName, 0, SZ_OF_PIPE_NAME);
    m_deviceType = deviceType;
    if(strCommPipeName)
    {
        memcpy(m_commPipeName, strCommPipeName, strlen((const char*)strCommPipeName));
    }
    if(strDeviceName)
    {
        memcpy(m_deviceName, strDeviceName, strlen((const char*)strDeviceName));
    }
    m_deviceNumber = deviceNumber;

    m_nMemberIndex = nMemberIndex;
    m_nSensorType = nSensorType;
    //
    memset(m_commServerName, 0, SZ_OF_PIPE_NAME);
    GetCommServerName(m_commServerName);
}

Device::~Device()
{

}

void Device::Init(uint deviceNumber, V100_DEVICE_TYPE deviceType, uchar* strCommPipeName, uchar* strDeviceName, uint nMemberIndex, uint nSensorType)
{
    memset(m_commPipeName, 0, SZ_OF_PIPE_NAME);
    memset(m_deviceName, 0, SZ_OF_PIPE_NAME);

    if(strCommPipeName)
    {
        memcpy(m_commPipeName, strCommPipeName, strlen((const char*)strCommPipeName));
    }

    if(strDeviceName)
    {
        memcpy(m_deviceName, strDeviceName, strlen((const char*)strDeviceName));
    }

    m_deviceType   = deviceType;
    m_deviceNumber = deviceNumber;
    m_nMemberIndex = nMemberIndex;
    m_nSensorType  = nSensorType;
}

V100_DEVICE_TYPE Device::GetDeviceType()
{
    return m_deviceType;
}

void Device::GetCommPipeName(uchar* strCommPipeName)
{
    //<TODO>: Check pointer comming in
    //int x = strlen((const char*)m_commPipeName);
    memcpy(strCommPipeName, m_commPipeName, strlen((const char*)m_commPipeName));
}
void Device::GetCommServerName(uchar* strCommPipeName)
{
    #if defined(WIN32)

        char* strEnvVar = new char[REG_BUF_SIZE];
        memset(strEnvVar, 0, REG_BUF_SIZE);

        bool bRC = GetEnvVar(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", L"XDClientName", strEnvVar);

        if(bRC == false)
            bRC = GetEnvVar(HKEY_CURRENT_USER, L"Environment", L"XDClientName", strEnvVar);

        if(bRC == true)
        {
            memcpy(strCommPipeName, strEnvVar, strlen((const char*)strEnvVar));
        }
        else
        {
            strCommPipeName[0] = '.';
            strCommPipeName[1] = '\0';
        }

        if(strEnvVar) {delete[] strEnvVar; strEnvVar = 0;}

    #else

        strCommPipeName[0] = '.';
        strCommPipeName[1] = '\0';

    #endif

    return;
}


bool Device::GetEnvVar(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, char* strEnvVarOut)
{
    #if defined(WIN32)
        HKEY hKeyResult;
        LONG lRet;
        TCHAR buffer[REG_BUF_SIZE];
        DWORD dwBufLen = REG_BUF_SIZE;
        char* strTmpEnvVar = NULL;

        lRet = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKeyResult);
        if(lRet != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            RegCloseKey(hKeyResult);
            return false;
        }

        lRet = RegQueryValueEx( hKeyResult, lpValueName, NULL, NULL,(LPBYTE) buffer, &dwBufLen);
        if(lRet != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            RegCloseKey(hKeyResult);
            return false;
        }

        size_t szEnvVar = wcslen((const wchar_t*) buffer) + 1;

        strTmpEnvVar = new char[szEnvVar];
        wcstombs_s(0, strTmpEnvVar, szEnvVar, buffer, _TRUNCATE);

        memcpy(strEnvVarOut, strTmpEnvVar, szEnvVar);
        delete [] strTmpEnvVar;
        strTmpEnvVar = NULL;

        RegCloseKey(hKey);
        RegCloseKey(hKeyResult);

    #else

        strEnvVarOut[0] = '\0';

    #endif

    return true;
}

uint Device::GetDeviceNumber()
{
    return m_deviceNumber;
}
