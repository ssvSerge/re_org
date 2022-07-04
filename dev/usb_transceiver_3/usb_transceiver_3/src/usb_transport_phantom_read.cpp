#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

namespace hid {
namespace transport {

constexpr int PHANTOM_READ_TIMEOUT_MS = 1;

void usb_transport_device_t::handle_phantom_read() {

    usb_transport_err_t io_res = usb_transport_err_t::USB_STATUS_UNKNOWN;
    uint32_t    reads_cnt = 0;
    uint8_t     dummy = 0;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    for ( ; ; ) {
        io_res = rx_frame ( &dummy, sizeof(dummy), PHANTOM_READ_TIMEOUT_MS );
        if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
            reads_cnt++;
            continue;
        }
        break;
    }

    assert ( io_res != usb_transport_err_t::USB_STATUS_READY );

    if ( io_res == usb_transport_err_t::USB_STATUS_FAILED) {

        warn ( "Failed: rx_frame; (%s):(%d)", __FUNCTION__, __LINE__);

        if ( m_ep0_inactive ) {
            // Assume the inactivity of EP0 is the source of read error.    
            err ( "EP0 inactive; Switch to SPIUP (%s):(%d)", __FUNCTION__, __LINE__);
            LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        } else {
            err ("Critical error; Failed to read; (%s):(%d)", __FUNCTION__, __LINE__ );
            LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        }

    } else {

        debug ("Read: %d bytes; (%s):(%d)", reads_cnt, __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_WAIT );

    }

}

//---------------------------------------------------------------------------//

}
}
