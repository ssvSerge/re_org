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

#pragma once

#include "V100_shared_types.h"
// #include "VCOMBase.h"
#include <cstdint>

class ICmd;
class ITransport;

typedef unsigned char uchar;
typedef unsigned int  uint;

class V100CommandHandler
{
public:
    V100CommandHandler(void);
    ~V100CommandHandler(void);
    static V100CommandHandler* GetCommandHandler(const V100_DEVICE_TRANSPORT_INFO* pID); // Get static V100CommandHandler
    static bool Lock_Handle(const V100_DEVICE_TRANSPORT_INFO* pDev);
    static uint32_t GetNumOpenHandles();
    ICmd* IssueCommand(V100_DEVICE_TRANSPORT_INFO* pDev, ICmd* pCmd);
    ICmd* CreateCommand(_V100_COMMAND_SET cmdSet);
    uint  Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport);
    bool  GetDeviceId(V100_DEVICE_TRANSPORT_INFO* pTransport, char* pStr, uint& nLength);
    void  Close(V100_DEVICE_TRANSPORT_INFO* pTransport);
protected:
    static HANDLE hSyncMutex;
private:
    ICmd* TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pDev, int route_flag, ICmd* pCommand);
    ICmd* GetResponse(V100_DEVICE_TRANSPORT_INFO* pDev, uchar* pResponseBytes, uint nSize);
};
