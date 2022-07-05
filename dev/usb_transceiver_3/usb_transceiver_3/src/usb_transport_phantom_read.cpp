#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
    
constexpr int PHANTOM_READ_TIMEOUT_MS = 1;

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_phantom_read() {

    err_t       io_res      = err_t::USB_STATUS_UNKNOWN;
    uint8_t     dummy       = 0;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    do {
        io_res = rx_frame ( &dummy, sizeof(dummy), PHANTOM_READ_TIMEOUT_MS );
    } while ( io_res == err_t::USB_STATUS_READY );

    if ( m_stop_request ) {
        err ( "Stop request; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SHUTDOWN );
        return;
    }

    if ( m_ep0_inactive ) {
        err ( "EP0 inactive; Switch to SPIUP (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_SPINUP);
        return;
    }

    if ( io_res == err_t::USB_STATUS_FAILED ) {
        err ( "Failed rx_frame(); (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    if ( io_res == err_t::USB_STATUS_TIMEOUT ) {
        debug ( "Timeout; Go to RX_HEADER; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER );
        return;
    }

}

//---------------------------------------------------------------------------//

}
}
