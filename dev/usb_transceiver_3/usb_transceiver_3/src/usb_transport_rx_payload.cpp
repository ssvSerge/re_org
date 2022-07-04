#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>


using namespace std::chrono_literals;


namespace hid {
namespace transport {


using   time_source_t  =  std::chrono::steady_clock;
using   duration_ms_t  =  std::chrono::milliseconds;
using   checkpoint_t   =  std::chrono::time_point<time_source_t>;

constexpr int USB_IO_MTU           = 65536;
constexpr int USB_MIN_WAIT_TIME_MS = 3;

void usb_transport_device_t::handle_read_bad ( uint32_t timeout_ms ) {

    usb_transport_err_t io_res          = usb_transport_err_t::USB_STATUS_FAILED;
    bool                error           = true;
    size_t              read_cnt        = 0; // 
    size_t              data_part       = 0; // 
    uint32_t            wait_time_ms    = 0; //
    usb_frame_t         local_buffer;        //

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );
    info  ( "Wait for %d bytes; (%s):(%d)", m_data.inp_prm.len, __FUNCTION__, __LINE__ );

    checkpoint_t start_time = time_source_t::now();

    error = false;
    try {
        // Assume there's at least 64KB of memory for us.
        local_buffer.resize ( USB_IO_MTU );
    } catch ( ... ) {
        error = true;
    }

    if ( error ) {
        err( "Error: No memory; (%s):(%d)", __FUNCTION__, __LINE__ );
        return;
    }

    error = true;
    for ( ; ; ) {

        auto duration = time_source_t::now() - start_time;
        auto duration_ms = std::chrono::duration_cast<duration_ms_t>(duration).count();

        debug("Read: %d from %d bytes; (%s):(%d)", (int)read_cnt, m_data.inp_prm.len, __FUNCTION__, __LINE__);

        if ( read_cnt == m_data.inp_prm.len ) {
            error = false;
            break;
        }
        if ( duration_ms <= USB_MIN_WAIT_TIME_MS ) {
            break;
        }
        if ( m_stop_request ) {
            break;
        }
        if ( m_ep0_inactive ) {
            break;
        }

        data_part = m_data.inp_prm.len - read_cnt;
        if ( data_part > local_buffer.size() ) {
            data_part = local_buffer.size();
        }

        io_res = rx_frame ( local_buffer.data(), data_part, (uint32_t)duration_ms );

        if ( io_res != usb_transport_err_t::USB_STATUS_READY ) {
            debug ( "rx_frame failed; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        read_cnt += data_part;

    }

    if ( m_stop_request ) {
        // External request to shutdown.
        err ( "Stop request; (%s):(%d); ", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SHUTDOWN );
        return;
    }

    if ( m_ep0_inactive ) {
        // Ignore all possible errors because of inactivity of EP0.
        err("EP0 inactive; Switch to SPIUP (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        return;
    }

    if ( ! error ) {
        // Payload successfully read. 
        // Do not handle command but send error code instead of handling.
        err ( "Payload ignored; Return error code; (%s):(%d)", __FUNCTION__, __LINE__ );
        m_data.out_pay.clear();
        m_data.out_prm.cmd = hid::stream::cmd_t::STREAM_CMD_ERROR;
        m_data.out_prm.param = USB_TRANSPORT_ERR_NO_MEMORY;
        LOG_USB_STATE ( usb_state_t::STATE_TX_RESPONSE );
        return;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_TIMEOUT ) {
        // Critical state!
        // Host want send data; and low memory condition at device side. 
        // But still possible to handle commands.
        err("Payload read; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_WAIT );
        return;
    }

    // Failure during read process. Can't continue.
    debug ( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE(usb_state_t::STATE_FAILED );
    
}

void usb_transport_device_t::handle_read_ok ( uint32_t timeout_ms ) {

    usb_transport_err_t io_res = usb_transport_err_t::USB_STATUS_FAILED;

    debug( "Enter: (%s):(%d)", __FUNCTION__, __LINE__);
    info ( "Wait for %d bytes; (%s):(%d)", m_data.inp_prm.len, __FUNCTION__, __LINE__ );

    io_res = rx_frame ( m_data.inp_pay.data(), m_data.inp_pay.size(), timeout_ms );

    if ( m_stop_request ) {
        // External request to shutdown.
        err("Stop request; (%s):(%d); ", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_SHUTDOWN);
        return;
    }

    if ( m_ep0_inactive ) {
        // Ignore all possible errors because of inactivity of EP0.
        err("EP0 inactive; Switch to SPIUP (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_SPINUP);
        return;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_TIMEOUT ) {
        // Host want send data; Ignore command and wait for the next header.
        err ( "Timeout while reading payload; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_WAIT );
        return;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_FAILED ) {
        err ( "Failed rx_frame; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    // Payload received. There's a chance to process command.
    debug ( "Payload received; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_HANDLE_REQUEST );
}

void usb_transport_device_t::handle_payload_read() {

    debug( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( ! hid::stream::Prefix::Valid(m_data.inp_hdr) ) {
        err("Wrong header received; Wait for another one; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_WAIT );
        return;
    }

    hid::stream::Prefix::GetParams ( m_data.inp_hdr, m_data.inp_prm );

    if ( m_data.inp_prm.len == 0 ) {
        // There could be SYNC request. Payload not expected.
        info("Payload not expected; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_HANDLE_REQUEST);
        return;
    }

    bool alloc_valid = true;

    try {
        m_data.inp_pay.resize ( m_data.inp_prm.len );
        cleanup( m_data.inp_pay );
    } catch (...) {
        alloc_valid = false;
        err("Error: Failed to allocate buffer %d bytes; (%s):(%d)", m_data.inp_prm.len, __FUNCTION__, __LINE__);
    }

    if ( alloc_valid ) {
        handle_read_ok  ( USB_IO_TIMEOUT_MS );
    } else {
        handle_read_bad ( USB_IO_TIMEOUT_MS );
    }

    debug("Leave: (%s):(%d)", __FUNCTION__, __LINE__);
}

//---------------------------------------------------------------------------//

}
}
