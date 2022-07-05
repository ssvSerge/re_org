#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <hid/StreamPrefix.h>

#include <logging.h>


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

constexpr int INFINITE_WAIT_MS = 0;

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_hdr_wait() {

    err_t io_res;

    // debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    // There's only 24 bytes in the prefix. Ignore the NO_MEMORY state.
    m_data.inp_hdr.resize ( hid::stream::Prefix::PrefixSize() );
    // Clear the previous content.
    cleanup ( m_data.inp_hdr );

    io_res = rx_frame ( m_data.inp_hdr.data(), m_data.inp_hdr.size(), INFINITE_WAIT_MS );

    if ( m_stop_request ) {
        err ( "Stop request; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SHUTDOWN );
        return;
    }

    if ( m_ep0_inactive ) {
        warn ( "EP0 inactive; Switch to SPINUP; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        return;
    }

    // Timeout expected only in combination with m_stop_request and m_ep0_active
    assert ( io_res != err_t::USB_STATUS_TIMEOUT );

    if ( io_res == err_t::USB_STATUS_FAILED ) {
        err ( "Critical error; Cannot read HEADER; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    debug ( "Header received; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_RX_PAYLOAD );
}

//---------------------------------------------------------------------------//

}
}
