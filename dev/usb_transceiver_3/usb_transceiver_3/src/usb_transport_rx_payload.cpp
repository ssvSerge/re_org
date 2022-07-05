#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>


using namespace std::chrono_literals;


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_payload_read() {

    err_t io_res = err_t::USB_STATUS_FAILED;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( ! hid::stream::Prefix::Valid(m_data.inp_hdr) ) {
        err ( "Wrong header received; Wait for another one; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER );
        return;
    }

    hid::stream::Prefix::GetParams ( m_data.inp_hdr, m_data.inp_prm );

    if ( m_data.inp_prm.len == 0 ) {
        // There could be SYNC request. Payload not expected.
        info ( "Payload not expected; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_HANDLE_REQUEST );
        return;
    }

    bool alloc_valid = true;
    try {
        m_data.inp_pay.resize(m_data.inp_prm.len);
        cleanup(m_data.inp_pay);
    }
    catch (...) {
        alloc_valid = false;
    }

    if ( ! alloc_valid ) {
        err ( "No memory; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE( usb_state_t::STATE_FAILED );
        return;
    }

    // Clear previews context.
    cleanup ( m_data.inp_pay );

    io_res = rx_frame ( m_data.inp_pay.data(), m_data.inp_pay.size(), USB_IO_TIMEOUT_MS );

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

    if ( io_res == err_t::USB_STATUS_TIMEOUT ) {
        // Host want send data; Ignore command and wait for the next header.
        err ( "Timeout while reading payload; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE( usb_state_t::STATE_RX_HEADER );
        return;
    }

    if ( io_res == err_t::USB_STATUS_FAILED ) {
        // Ignore all possible errors because of inactivity of EP0.
        err ( "Critical error; Cannot read HEADER; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    debug ( "Payload received; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_HANDLE_REQUEST );
}

//---------------------------------------------------------------------------//

}
}
