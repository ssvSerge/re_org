/*************************************************************************************************************************
**                                                                                                                      **
** �Copyright 2017 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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

// NOTE: This code only supports one Lumidigm device per OS instance.

#include <stdint.h>
#include <libusb.h>
#include "V100_shared_types.h"
#include "ITransport.h"
#include "BaseMutex.h"
#include "usbcmdset.h"

#include <application/types.h>

class TransportUSB : public ITransport
{
public:
                TransportUSB();
    virtual     ~TransportUSB();

    bool        GetDeviceId            (V100_DEVICE_TRANSPORT_INFO * pTransport, char * szDeviceId, uint32_t & nLength);
    uint32_t    Initialize            (V100_DEVICE_TRANSPORT_INFO * pTransport);
    uint32_t    Close                (V100_DEVICE_TRANSPORT_INFO * pTransport);
    uint32_t    TransmitCommand        (V100_DEVICE_TRANSPORT_INFO * pTransport, int route_flag, 
                                    const uint8_t * myPacket, uint32_t nTxSize, uint8_t * pResponse, uint32_t & nRxSize);

    static bool FindSupportedDevice(uint16_t & vid, uint16_t & pid, uint8_t & bus, uint8_t & adr, bool bReset = false);

    enum
    {
        V30X_VENDOR                =    0x0525,
        V30X_PRODUCT            =    0x3424,
        V30X_ENDPOINT_OUT         =    0x02,
        V30X_ENDPOINT_IN        =    0x81,

        M30X_VENDOR                =    0x1FAE,
        M30X_PRODUCT            =    0x212C,
        M30X_ENDPOINT_OUT        =    0x06,
        M30X_ENDPOINT_IN        =    0x85,

        V31X_VENDOR                =    0x1FAE,
        V31X_PRODUCT_UNRENUM    =    0x0020,
        V31X_PRODUCT            =    0x0021,

        M31X_VENDOR                =    0x1FAE,
        M31X_PRODUCT_UNRENUM    =    0x0040,
        M31X_PRODUCT            =    0x0041,

        M21X_VENDOR             =   0x1FAE,
        M21X_PRODUCT_UNRENUM    =   0x0011,
        M21X_PRODUCT            =   0x0012
    };

private:
    libusb_device_handle *    m_libusb_device_handle;
    libusb_context *        m_libusb_context;
    uint8_t                    m_EPOut;
    uint8_t                    m_EPIn;
    USBCB                    m_USBCB_request;
    USBCB                    m_USBCB_response;

    BaseMutex                m_mutexXact;            // serialize all I/O transactions
    bool                    m_bReadUSBCBCallback;
    bool                    m_bReadPayloadCallback;
    bool                    m_bWriteCallback;
    bool                    m_bError;

    // libusb callbacks
    friend void usbcb_read_callback(libusb_transfer * pTransfer);
    friend void payload_read_callback(libusb_transfer * pTransfer);
    friend void usbcb_write_callback(libusb_transfer * pTransfer);
    friend void payload_write_callback(libusb_transfer * pTransfer);

    // libusb callbacks proxy to these methods
    void USBCB_read_complete(libusb_transfer * pTransfer);
    void Payload_read_complete(libusb_transfer * pTransfer);
    void USBCB_write_complete(libusb_transfer * pTransfer);
    void Payload_write_complete(libusb_transfer * pTransfer);

    #ifdef __ANDROID__
    libusb_device_handle * AndroidInitialize(V100_DEVICE_TRANSPORT_INFO * pTransport);
    #endif
};

