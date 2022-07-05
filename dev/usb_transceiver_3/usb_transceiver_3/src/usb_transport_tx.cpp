#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

using namespace std::chrono_literals;

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_tx_resp() {

    err_t io_res;

    // out_prm.cmd & out_prm.param are configured before.
    m_data.out_prm.len = static_cast<uint32_t> ( m_data.out_pay.size());
    hid::stream::Prefix::SetParams ( m_data.out_prm, m_data.out_hdr );

    io_res = tx_frame ( m_data.out_hdr.data(), m_data.out_hdr.size(), USB_IO_TIMEOUT_MS );
    if ( io_res == err_t::USB_STATUS_READY ) {
        io_res = tx_frame ( m_data.out_pay.data(), m_data.out_pay.size(), USB_IO_TIMEOUT_MS );
        if ( io_res != err_t::USB_STATUS_READY ) {
            err ( "Failed to send payload; (%s):(%d)", __FUNCTION__, __LINE__ );
        }
    } else {
        err ( "Failed to send header; (%s):(%d)", __FUNCTION__, __LINE__ );
    }

    if ( m_stop_request ) {
        // External request to shutdown.
        err ( "Stop request; (%s):(%d); ", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SHUTDOWN );
        return;
    }

    if ( m_ep0_inactive ) {
        // Ignore all possible errors because of inactivity of EP0.
        err ( "EP0 inactive; Switch to SPIUP (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        return;
    }

    if ( io_res == err_t::USB_STATUS_FAILED )  {
        err ( "Failed tx_frame; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    if ( io_res == err_t::USB_STATUS_TIMEOUT )  {
        err ( "Timeout while reading payload; (%s):(%d)", __FUNCTION__, __LINE__);
    }

    debug ("Transaction done; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE( usb_state_t::STATE_RX_HEADER );
}

//---------------------------------------------------------------------------//

}
}
