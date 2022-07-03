/*************************************************************************************************************************
**                                                                                                                      **
** �Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "IThread.h"
#include "TransportLibUSB.h"

extern "C"
{
    char const *logFilename = "/dev/stderr";
    //<NOTE>: Uncomment to log messages
    //char const *logFilename = "./transpBio.txt";

    void DumpLogStr   (const char *fileName, const char *str);
    void DumpLogFmt   (const char *fileName, const char *fmt...);
    void DumpLogBuffer(const char *fileName, const char *str, unsigned char *buffer, int nSz);
}

TransportUSB::TransportUSB()  :
    m_libusb_device_handle(NULL),
    m_libusb_context(NULL)
{}

TransportUSB::~TransportUSB()
{ Close(NULL); }

bool TransportUSB::GetDeviceId(V100_DEVICE_TRANSPORT_INFO * pTransport, char * szDeviceId, uint32_t & nLength)
{
    if (szDeviceId)
        *szDeviceId = '\0';
    nLength = 0;
    return true;
}

static const char * GetTransferStatusString(libusb_transfer_status status)
{
    switch (status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            return "LIBUSB_TRANSFER_COMPLETED";
        case LIBUSB_TRANSFER_ERROR:
            return "LIBUSB_TRANSFER_ERROR";
        case LIBUSB_TRANSFER_TIMED_OUT:
            return "LIBUSB_TRANSFER_TIMED_OUT";
        case LIBUSB_TRANSFER_CANCELLED:
            return "LIBUSB_TRANSFER_CANCELLED";
        case LIBUSB_TRANSFER_STALL:
            return "LIBUSB_TRANSFER_STALL";
        case LIBUSB_TRANSFER_NO_DEVICE:
            return "LIBUSB_TRANSFER_NO_DEVICE";
        case LIBUSB_TRANSFER_OVERFLOW:
            return "LIBUSB_TRANSFER_OVERFLOW";
        default: break;
    }
    return "UNKNOWN libusb_transfer_status";
}

static const char * GetErrorString(libusb_error error)
{
    switch (error)
    {
        case LIBUSB_SUCCESS:
            return "LIBUSB_SUCCESS";
        case LIBUSB_ERROR_IO:
            return "LIBUSB_ERROR_IO";
        case LIBUSB_ERROR_INVALID_PARAM:
            return "LIBUSB_ERROR_INVALID_PARAM";
        case LIBUSB_ERROR_ACCESS:
            return "LIBUSB_ERROR_ACCESS";
        case LIBUSB_ERROR_NO_DEVICE:
            return "LIBUSB_ERROR_NO_DEVICE";
        case LIBUSB_ERROR_NOT_FOUND:
            return "LIBUSB_ERROR_NOT_FOUND";
        case LIBUSB_ERROR_BUSY:
            return "LIBUSB_ERROR_BUSY";
        case LIBUSB_ERROR_TIMEOUT:
            return "LIBUSB_ERROR_TIMEOUT";
        case LIBUSB_ERROR_OVERFLOW:
            return "LIBUSB_ERROR_OVERFLOW";
        case LIBUSB_ERROR_PIPE:
            return "LIBUSB_ERROR_PIPE";
        case LIBUSB_ERROR_INTERRUPTED:
            return "LIBUSB_ERROR_INTERRUPTED";
        case LIBUSB_ERROR_NO_MEM:
            return "LIBUSB_ERROR_NO_MEM";
        case LIBUSB_ERROR_NOT_SUPPORTED:
            return "LIBUSB_ERROR_NOT_SUPPORTED";
        case LIBUSB_ERROR_OTHER:
            return "LIBUSB_ERROR_OTHER";
        default:
            break;
    }
    return "UNKNOWN libusb_error";
}

#ifdef __ANDROID__
libusb_device_handle * TransportUSB::AndroidInitialize(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    libusb_device ** apDevices;
    uint32_t countFound = 0;
    libusb_device * deviceFound = NULL;
    ssize_t count = libusb_get_device_list(m_libusb_context, &apDevices);

    DumpLogFmt(logFilename, "TransportUSB::AndroidInitialize: searching %d devices", count);

    for (ssize_t i = 0; i < count; i++)
    {
        libusb_device_descriptor desc;
        if (!libusb_get_device_descriptor(apDevices[i], &desc))
        {
            DumpLogFmt(logFilename, "TransportUSB::AndroidInitialize: found %04X:%04X.", desc.idVendor, desc.idProduct);

            // right now we only support V30x and M30x
                 if (desc.idVendor == TransportUSB::V30X_VENDOR && desc.idProduct == TransportUSB::V30X_PRODUCT)
            { m_EPDown = V30X_ENDPOINT_OUT; m_EPUp = V30X_ENDPOINT_IN; countFound++; deviceFound = apDevices[i]; }
            else if (desc.idVendor == TransportUSB::M30X_VENDOR && desc.idProduct == TransportUSB::M30X_PRODUCT)
            { m_EPDown = M30X_ENDPOINT_OUT; m_EPUp = M30X_ENDPOINT_IN; countFound++; deviceFound = apDevices[i]; }
        }
    }

    DumpLogFmt(logFilename, "TransportUSB::AndroidInitialize: found %d devices", countFound);

    libusb_device_handle * h = NULL;
    if (countFound == 1)
    {
        int rc_libusb = libusb_open_fd(deviceFound, pTransport->DeviceNumber, &h);
        if (rc_libusb != 0)
        {
            DumpLogFmt(logFilename, "TransportUSB::AndroidInitialize: libusb_open_fd fd: %d failed %d",
                pTransport->DeviceNumber, rc_libusb);
            h = NULL;
        }
        else
            DumpLogFmt(logFilename, "TransportUSB::AndroidInitialize: libusb_open_fd fd: %d succeeded handle: %p",
                pTransport->DeviceNumber, h);
    }
    libusb_free_device_list(apDevices, 1/*unref*/);
    return h;
}
#endif

uint32_t TransportUSB::Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    int rc_libusb;
    //DumpLogStr(logFilename, "TransportUSB::Initialize.\n");

    rc_libusb = libusb_init(&m_libusb_context);
    if (rc_libusb != 0)
    {
        DumpLogFmt(logFilename, "TransportUSB::Initialize: libusb_init failed rc = %d.\n", rc_libusb);
        return rc_libusb;
    }

    libusb_set_debug(m_libusb_context, 3);

    libusb_device_handle * h = NULL;

#ifdef __ANDROID__
         h = AndroidInitialize(pTransport);
#else
         if ((h = libusb_open_device_with_vid_pid(m_libusb_context, M30X_VENDOR, M30X_PRODUCT)))
         { m_EPOut = M30X_ENDPOINT_OUT; m_EPIn = M30X_ENDPOINT_IN; }
    else if ((h = libusb_open_device_with_vid_pid(m_libusb_context, V30X_VENDOR, V30X_PRODUCT)))
         { m_EPOut = V30X_ENDPOINT_OUT; m_EPIn = V30X_ENDPOINT_IN; }
#endif

    if (!h)
    {
        // cannot initialize a venus device, already tried a mercury, log and abandon ship
        DumpLogFmt(logFilename, "TransportUSB::Initialize: unable to find supported device.\n");
        libusb_exit(m_libusb_context);
        m_libusb_context = NULL;
        return LIBUSB_ERROR_NO_DEVICE;
    }

    m_libusb_device_handle = h;

    rc_libusb = libusb_kernel_driver_active(m_libusb_device_handle, 0);
    if (rc_libusb == 1)
    {
        rc_libusb = libusb_detach_kernel_driver(m_libusb_device_handle, 0);
        if (rc_libusb != 0)
        {
            DumpLogFmt(logFilename, "TransportUSB::Initialize: libusb_detach_kernel_driver failed rc = %d.\n", rc_libusb);
            libusb_close(m_libusb_device_handle);
            m_libusb_device_handle = NULL;
            libusb_exit(m_libusb_context);
            m_libusb_context = NULL;
            return rc_libusb;
        }
    }

    rc_libusb = libusb_claim_interface(m_libusb_device_handle, 0);
    if (rc_libusb != 0)
    {
        DumpLogFmt(logFilename, "TransportUSB::Initialize: libusb_claim_interface failed rc = %d.\n", rc_libusb);
        libusb_close(m_libusb_device_handle);
        m_libusb_device_handle = NULL;
        libusb_exit(m_libusb_context);
        m_libusb_context = NULL;
        return rc_libusb;
    }

    //DumpLogStr(logFilename, "TransportUSB::Initialize: OK.\n");
    return 0;
}

uint32_t TransportUSB::Close(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    bool bClosed = false;
    {
        // scope-based lock
        Lock l(m_mutexXact);

        if (m_libusb_device_handle)
        {
            libusb_release_interface(m_libusb_device_handle, 0);
            libusb_close(m_libusb_device_handle);
            m_libusb_device_handle = NULL;
        }

        if (m_libusb_context)
        {
            libusb_exit(m_libusb_context);
            m_libusb_context = NULL;
            bClosed = true;
        }
    }

    // avoid spurious logging for multiple close operations, as the destructor will call this even if already closed...
    if (bClosed)
        DumpLogStr(logFilename, "TransportUSB::Close: OK.\n");
    return 0;
}

// "C" callback functions for each phase of the I/O transaction.
void usbcb_read_callback(libusb_transfer * pTransfer)
{
    TransportUSB * pTransport = static_cast<TransportUSB *>(pTransfer->user_data);
    pTransport->USBCB_read_complete(pTransfer);
}

void TransportUSB::USBCB_read_complete(libusb_transfer * pTransfer)
{
    if (pTransfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_read_complete: transfer status: %d (%s).\n",
            pTransfer->status, GetTransferStatusString(pTransfer->status));
        m_bError = true;
    }
    if (pTransfer->actual_length != pTransfer->length)
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_read_complete: length mismatch: %d != %d.\n",
            pTransfer->length, pTransfer->actual_length);
        m_bError = true;
    }
    if (pTransfer->actual_length != sizeof(USBCB))
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_read_complete: actual_length invalid: %d.\n",
            pTransfer->actual_length);
        m_bError = true;
    }
    m_bReadUSBCBCallback = true;
}

void payload_read_callback(libusb_transfer * pTransfer)
{
    TransportUSB * pTransport = static_cast<TransportUSB *>(pTransfer->user_data);
    pTransport->Payload_read_complete(pTransfer);
}

void TransportUSB::Payload_read_complete(libusb_transfer * pTransfer)
{
    if (pTransfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        DumpLogFmt(logFilename, "TransportUSB::Payload_read_complete: transfer status: %d (%s).\n",
            pTransfer->status, GetTransferStatusString(pTransfer->status));
        m_bError = true;
    }
    m_bReadPayloadCallback = true;
}

void usbcb_write_callback(libusb_transfer * pTransfer)
{
    TransportUSB * pTransport = static_cast<TransportUSB *>(pTransfer->user_data);
    pTransport->USBCB_write_complete(pTransfer);
}

void TransportUSB::USBCB_write_complete(libusb_transfer * pTransfer)
{
    if (pTransfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_write_complete: transfer status: %d (%s).\n",
            pTransfer->status, GetTransferStatusString(pTransfer->status));
        m_bError = true;
    }
    if (pTransfer->actual_length != pTransfer->length)
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_write_complete: length mismatch: %d != %d.\n",
            pTransfer->length, pTransfer->actual_length);
        m_bError = true;
    }
    if (pTransfer->actual_length != sizeof(USBCB))
    {
        DumpLogFmt(logFilename, "TransportUSB::USBCB_write_complete: actual_length invalid: %d.\n",
            pTransfer->actual_length);
        m_bError = true;
    }
    m_bWriteCallback = true;
}

void payload_write_callback(libusb_transfer * pTransfer)
{
    TransportUSB * pTransport = static_cast<TransportUSB *>(pTransfer->user_data);
    pTransport->Payload_write_complete(pTransfer);
}

void TransportUSB::Payload_write_complete(libusb_transfer * pTransfer)
{
    if (pTransfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        DumpLogFmt(logFilename, "TransportUSB::Payload_write_complete: transfer status: %d (%s).\n",
            pTransfer->status, GetTransferStatusString(pTransfer->status));
        m_bError = true;
    }
    if (pTransfer->actual_length != pTransfer->length)
    {
        DumpLogFmt(logFilename, "TransportUSB::Payload_write_complete: length mismatch: %d != %d.\n",
            pTransfer->length, pTransfer->actual_length);
        m_bError = true;
    }
    m_bWriteCallback = true;
}

// returns 1 - success
//           0 - failure
uint32_t TransportUSB::TransmitCommand(V100_DEVICE_TRANSPORT_INFO * pTransport, int route_flag,
    const uint8_t * pPacket,   uint32_t nTxSize,
    uint8_t * pResponse, uint32_t &nRxSize)
{
    // scope-based lock...
    Lock l(m_mutexXact);
    // assume unsuccessful, prove otherwise...
    int rc = 0;
    int nMaxRx = nRxSize;
    nRxSize = 0;

    // annotate the fact that we currently don't need to perform any cleanup...
    m_bReadUSBCBCallback    =    true;
    m_bReadPayloadCallback    =    true;
    m_bWriteCallback        =    true;
    m_bError                =    false;

    m_USBCB_request.ulCommand = 0;
    m_USBCB_request.ulCount      = nTxSize;
    m_USBCB_request.ulData      = 0;

    // assert correctly set up...
    if (!m_libusb_context || !m_libusb_device_handle)
        return LIBUSB_ERROR_OTHER;

    // allocate transfer object for all states of the protocol...
    libusb_transfer * pTransferReadUSBCB    = libusb_alloc_transfer(0/*iso_packets*/);
    libusb_transfer * pTransferReadPayload  = libusb_alloc_transfer(0/*iso_packets*/);
    libusb_transfer * pTransferWrite        = libusb_alloc_transfer(0/*iso_packets*/);
    if (pTransferReadUSBCB && pTransferReadPayload && pTransferWrite)
    {
        // load all of the bulk_transfer objects that make sense right now...
        libusb_fill_bulk_transfer(pTransferReadUSBCB, m_libusb_device_handle, m_EPIn,
                                  (unsigned char *) &m_USBCB_response, sizeof(m_USBCB_response),
                                  usbcb_read_callback, this, 15 * 1000/*msto*/);
        libusb_fill_bulk_transfer(pTransferReadPayload, m_libusb_device_handle, m_EPIn,
                                  pResponse, nMaxRx,
                                  payload_read_callback, this, 15 * 1000/*msto*/);
        libusb_fill_bulk_transfer(pTransferWrite, m_libusb_device_handle, m_EPOut,
                                  (unsigned char *) &m_USBCB_request, sizeof(m_USBCB_request),
                                  usbcb_write_callback, this, 15 * 1000/*msto*/);

        // so, submit all three transfers...
        int rc_libusb = libusb_submit_transfer(pTransferReadUSBCB);
        if (!rc_libusb)
        {
            m_bReadUSBCBCallback = false;
            rc_libusb = libusb_submit_transfer(pTransferReadPayload);
            if (!rc_libusb)
            {
                m_bReadPayloadCallback = false;
                rc_libusb = libusb_submit_transfer(pTransferWrite);
                if (!rc_libusb)
                {
                    // wait for indication that write USBCB has occurred
                    m_bWriteCallback = false;
                    do
                    {
                        struct timeval tv = { 15, 0 };
                        int rc_poll = libusb_handle_events_timeout(m_libusb_context, &tv);
                        if (rc_poll != 0)
                        {
                            DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: write USBCB poll: %d (%s).\n",
                                rc_poll, GetErrorString((libusb_error) rc_poll));
                            m_bError = true;
                            break;
                        }
                    } while (!m_bWriteCallback);

                    // if USBCB write was successful, continue...
                    if (!m_bError)
                    {
                        //DumpLogStr("TransportUSB::TransmitCommand: USBCB was written.\n");
                        while (nTxSize > 0)
                        {
                            // we limit transfer size on writes...
                            int nBytesThisTransfer = nTxSize;
                            if (nBytesThisTransfer > MAX_DATA_BYTES_EZEXTENDER)
                                nBytesThisTransfer = MAX_DATA_BYTES_EZEXTENDER;

                            // XXX - jbates - non-const cast for API call
                            libusb_fill_bulk_transfer(pTransferWrite, m_libusb_device_handle, m_EPOut,
                                (uint8_t *) pPacket, nBytesThisTransfer,
                                payload_write_callback, this, 15 * 1000/*msto*/);

                            pPacket += nBytesThisTransfer;
                            nTxSize -= nBytesThisTransfer;

                            rc_libusb = libusb_submit_transfer(pTransferWrite);
                            if (!rc_libusb)
                            {
                                // wait for indication that write payload block has occurred...
                                m_bWriteCallback = false;
                                do
                                {
                                    struct timeval tv = { 15, 0 };
                                    int rc_poll = libusb_handle_events_timeout(m_libusb_context, &tv);
                                    if (rc_poll != 0)
                                    {
                                        DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: write payload poll: %d (%s).\n",
                                            rc_poll, GetErrorString((libusb_error) rc_poll));
                                        m_bError = true;
                                        break;
                                    }
                                } while (!m_bWriteCallback);

                                if (pTransferWrite->actual_length != nBytesThisTransfer)
                                {
                                    DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: write payload length mismatch: %d != %d.\n",
                                        pTransferWrite->actual_length, nBytesThisTransfer);
                                    m_bError = true;
                                }
                                if (m_bError)
                                    break;

                                // XXX - jbates - 5.30.xx data throttle for large writes
                                if (nTxSize > 0 && nBytesThisTransfer == MAX_DATA_BYTES_EZEXTENDER)
                                    usleep(10 * 1000);

                                //DumpLogStr("TransportUSB::TransmitCommand: payload block was written.\n");
                            }
                            else
                            {
                                DumpLogStr(logFilename, "TransportUSB::TransmitCommand: submit write payload failed.\n");
                                m_bError = true;
                                break;
                            }
                        }

                        if (m_bWriteCallback)
                        {
                            // if the callback was hit...
                            // free and clear write transfer
                            libusb_free_transfer(pTransferWrite);
                            pTransferWrite = NULL;
                        }

                        if (!m_bError)
                        {
                            //DumpLogStr(logFilename, "TransportUSB::TransmitCommand: full payload was written.\n");
                            // wait for indication that USBCB read has occurred...
                            // NOTE: this could have happened BEFORE we got payload write complete...
                            while (!m_bReadUSBCBCallback)
                            {
                                struct timeval tv = { 15, 0 };
                                int rc_poll = libusb_handle_events_timeout(m_libusb_context, &tv);
                                if (rc_poll != 0)
                                {
                                    DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: read USBCB poll: %d (%s).\n",
                                        rc_poll, GetErrorString((libusb_error) rc_poll));
                                    m_bError = true;
                                    break;
                                }
                            }

                            int nPayloadSize = ((USBCB *) pTransferReadUSBCB->buffer)->ulCount;

                            if (m_bReadUSBCBCallback)
                            {
                                // if the callback was hit...
                                // free and clear usbcb transfer
                                libusb_free_transfer(pTransferReadUSBCB);
                                pTransferReadUSBCB = NULL;
                            }

                            if (!m_bError)
                            {
                                //DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: USBCB was read payload %d bytes.\n",
                                //    nPayloadSize);

                                // wait for indication that payload read has occurred...
                                while (!m_bReadPayloadCallback)
                                {
                                    struct timeval tv = { 15, 0 };
                                    int rc_poll = libusb_handle_events_timeout(m_libusb_context, &tv);
                                    if (rc_poll != 0)
                                    {
                                        DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: read payload poll: %d (%s).\n",
                                            rc_poll, GetErrorString((libusb_error) rc_poll));
                                        m_bError = true;
                                        break;
                                    }
                                }

                                int nActualPayloadSize = pTransferReadPayload->actual_length;

                                if (m_bReadPayloadCallback)
                                {
                                    // if the callback was hit...
                                    // free and clear payload transfer
                                    libusb_free_transfer(pTransferReadPayload);
                                    pTransferReadPayload = NULL;
                                }

                                if (!m_bError)
                                {
                                    if (nPayloadSize == nActualPayloadSize)
                                    {
                                        //DumpLogStr(logFilename, "TransportUSB::TransmitCommand: payload was read.\n");
                                        // OK, then...
                                        // we were successful!
                                        rc = 1;
                                        nRxSize = nPayloadSize;
                                    }
                                    else
                                    {
                                        DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: payload length mismatch: %d!=%d.\n",
                                            nPayloadSize, nActualPayloadSize);
                                    }
                                }
                                else
                                {
                                    DumpLogStr(logFilename, "TransportUSB::TransmitCommand: read payload failed.\n");
                                }
                            }
                            else
                            {
                                DumpLogStr(logFilename, "TransportUSB::TransmitCommand: read USBCB failed.\n");
                            }
                        }
                        else
                        {
                            DumpLogStr(logFilename, "TransportUSB::TransmitCommand: write payload block failed.\n");
                        }
                    }
                    else
                    {
                        DumpLogStr(logFilename, "TransportUSB::TransmitCommand: write USBCB failed.\n");
                    }
                }
                else
                {
                    DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: submit write USBCB failed %d (%s).\n",
                        rc_libusb, GetErrorString((libusb_error) rc_libusb));
                }
            }
            else
            {
                DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: submit read payload failed %d (%s).\n",
                    rc_libusb, GetErrorString((libusb_error) rc_libusb));
            }
        }
        else
        {
            DumpLogFmt(logFilename, "TransportUSB::TransmitCommand: submit read usbcb failed %d (%s).\n",
                rc_libusb, GetErrorString((libusb_error) rc_libusb));
        }
    }
    else
    {
        DumpLogStr(logFilename, "TransportUSB::TransmitCommand: unable to allocate libusb_transfer objects.\n");
    }

    // CLEANUP

    // if necessary, cancel transfers...
    if (pTransferReadUSBCB && !m_bReadUSBCBCallback)
        libusb_cancel_transfer(pTransferReadUSBCB);
    if (pTransferReadPayload && !m_bReadPayloadCallback)
        libusb_cancel_transfer(pTransferReadPayload);
    if (pTransferWrite && !m_bWriteCallback)
        libusb_cancel_transfer(pTransferWrite);

    // wait for any cancelled transfer to become freeable...
    while (!m_bReadUSBCBCallback || !m_bReadPayloadCallback || !m_bWriteCallback)
    {
        DumpLogStr(logFilename, "TransportUSB::TransmitCommand: cleaning up after failure.");
        struct timeval tv = { 15, 0 };
        // NOTE: we do no error checking here...
        libusb_handle_events_timeout(m_libusb_context, &tv);
    }

    // and, finally, free any transfer that needs it...
    if (pTransferReadUSBCB)
        libusb_free_transfer(pTransferReadUSBCB);
    if (pTransferReadPayload)
        libusb_free_transfer(pTransferReadPayload);
    if (pTransferWrite)
        libusb_free_transfer(pTransferWrite);

    return rc;
}

bool TransportUSB::FindSupportedDevice(uint16_t & vid, uint16_t & pid, uint8_t & bus, uint8_t & adr, bool bReset)
{
    vid = 0x0000;
    pid = 0x0000;
    bus = 0;
    adr = 0;
    int rc_libusb;
    libusb_context * context;
    rc_libusb = libusb_init(&context);
    if (rc_libusb != 0)
    {
        DumpLogStr(logFilename, "TransportUSB::FindSupportedDevice: libusb_init failed.\n");
        return false;
    }

    libusb_device ** apDevices;
    libusb_device * pDeviceKnown = NULL;
    ssize_t num_devices = libusb_get_device_list(context, &apDevices);
    int known_count = 0;
    for (ssize_t i = 0; i < num_devices; i++)
    {
        libusb_device * pDevice = apDevices[i];
        libusb_device_descriptor desc;
        if (!libusb_get_device_descriptor(pDevice, &desc))
        {
            //const char * dst = "unkn";
            bus = libusb_get_bus_number(pDevice);
            adr = libusb_get_device_address(pDevice);
                 if (desc.idVendor == TransportUSB::M30X_VENDOR && desc.idProduct == TransportUSB::M30X_PRODUCT)
            {
                //dst = "M30x";
                vid = TransportUSB::M30X_VENDOR;
                pid = TransportUSB::M30X_PRODUCT;
                pDeviceKnown = pDevice;
                known_count++;
            }
            else if (desc.idVendor == TransportUSB::V30X_VENDOR && desc.idProduct == TransportUSB::V30X_PRODUCT)
            {
                //dst = "V30x";
                vid = TransportUSB::V30X_VENDOR;
                pid = TransportUSB::V30X_PRODUCT;
                pDeviceKnown = pDevice;
                known_count++;
            }
            else if (desc.idVendor == TransportUSB::V31X_VENDOR && desc.idProduct == TransportUSB::V31X_PRODUCT)
            {
                //dst = "V31x";
                vid = TransportUSB::V31X_VENDOR;
                pid = TransportUSB::V31X_PRODUCT;
                pDeviceKnown = pDevice;
                known_count++;
            }
            else if (desc.idVendor == TransportUSB::M31X_VENDOR && desc.idProduct == TransportUSB::M31X_PRODUCT)
            {
                //dst = "M31x";
                vid = TransportUSB::M31X_VENDOR;
                pid = TransportUSB::M31X_PRODUCT;
                pDeviceKnown = pDevice;
                known_count++;
            }
            else
            {
                // NADA!
            }

            //DumpLogFmt(logFilename, "bus: %03d adr: %03d device: %04X:%04X %s.\n",
            //           bus, adr, desc.idVendor, desc.idProduct, dst);
        }
    }

    // do we want to reset the device?
    if (bReset)
    {
        if (known_count == 1)
        {
            if (pDeviceKnown)
            {
                libusb_device_handle * pHandle;
                if (libusb_open(pDeviceKnown, &pHandle) == 0)
                {
                    if (libusb_reset_device(pHandle) == 0)
                    {
                        DumpLogStr(logFilename, "Device was reset.\n");
                    }
                    else
                    {
                        DumpLogStr(logFilename, "Error during device reset.\n");
                    }
                    libusb_close(pHandle);
                }
                else
                {
                    DumpLogStr(logFilename, "Unable to open device for reset.\n");
                }
            }
            else
            {
                DumpLogStr(logFilename, "Unable to reset device.\n");
            }
        }
        else
        {
            DumpLogStr(logFilename, "non-singular device found - not resetting.\n");
        }
    }

    libusb_free_device_list(apDevices, 1/*unref*/);
    libusb_exit(context);
    return (known_count == 1);
}

