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

#include "V100_shared_types.h"
#include "VCOMBase.h"
#include "Device.h"

typedef unsigned char uchar;
typedef unsigned int  uint;

class V100DeviceHandler
{
public:
    V100DeviceHandler(void);
    ~V100DeviceHandler(void);
    static V100DeviceHandler* GetV100DeviceHandler(); // Get static V100DeviceHandler
    static void ReleaseV100DeviceHandler();// Release the static V100DeviceHanlder
    V100_ERROR_CODE GetNumDevices(int* nNumDevices);
    V100_ERROR_CODE GetDevice(uint deviceNumber, Device* pDevice);
    void SetServerName(char* pServerName);
    void GetStreamingDevCount(int* nNumDevices);
    void GetRegDevCount(int* nNumDevices);
protected:
    static HANDLE hSyncMutex;
private:

    char m_ServerName[2048];
    int m_nNumStreamingDevices;
    int m_nNumRegularDevices;
};
