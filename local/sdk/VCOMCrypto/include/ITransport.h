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
#include "VCOMBase.h"

typedef unsigned int uint;
typedef unsigned char uchar;

#define    TIMEOUT_VCOM_PROCESSING_MS        (5000)

class ITransport
{
public:
    ITransport() : m_mode(0) {};
    virtual ~ITransport() {};
    virtual uint    Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport) = 0;
    virtual uint    Close(V100_DEVICE_TRANSPORT_INFO* pTransport) = 0;
    virtual uint    TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pTransport, int route_flag,
                                    const uchar* myPacket, uint nTxSize, uchar* pResponse, uint& nRxSize) = 0;
    virtual bool    GetDeviceId(V100_DEVICE_TRANSPORT_INFO* pTransport, char* szDeviceId, uint& nLength) = 0;
    virtual void    SetMode(uint mode) { m_mode = mode;  }
protected:
    uint m_mode;
};
