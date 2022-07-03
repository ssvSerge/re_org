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
//  ->  STATE_FAILED    Critical errors (cannot continue).                   //
//                              failed enqueue_read                          //
//                              failed io_wait (not a timeout)               //
//                              failed io_cancel                             //
//  ->  STATE_SPINUP    EP0 unexpectedly switch down.                        //
//  ->  STATE_RX_HEADER_REQUEST Timeout while reading.                       // 
//  ->  next_state              Frame successfully read.                     //
//                                                                           //
//---------------------------------------------------------------------------//
bool usb_transport_device_t::read_frame (
    const checkpoint_t      time_out, 
    uint8_t* const          dst, 
    const size_t            len,
    usb_state_t             next_state
) {
    usb_transport_err_t  io_res;
    usb_transport_err_t  cancel_res;
    uint32_t             sleep_time_ms;

    const int ep2 = m_fs_eps[2];

    assert ( len > 0 );
    assert ( dst != nullptr );

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    get_timeout_ms ( time_out, sleep_time_ms );

    info( "Enqueue RX request; size: %d; time: %d ms; (%s):(%d)", (int)len, sleep_time_ms, __FUNCTION__, __LINE__ );
    io_res = enqueue_read ( &m_iocb_in, m_evfd_rd, m_io_ctx, ep2, dst, len );
    if ( io_res != usb_transport_err_t::USB_STATUS_READY ) {
        // Critical error. Failed to submit I/O request.
        err ( "Failed enqueue_read; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_FAILED);
        return CANNOT_CONTINUE;
    } 

    io_res = io_wait ( m_io_range, m_evfd_rd, m_fdset_read, m_io_ctx, sleep_time_ms );
    if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
        // Frame successfully read.
        debug ( "Read done: (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE( next_state );
        return READY;
    }

    warn  ( "Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
    debug ( "Cancel pending requests; (%s):(%d)", __FUNCTION__, __LINE__ );

    // There's one pending I/O request. Cancel it.
    cancel_res = io_cancel();
    if ( cancel_res != usb_transport_err_t::USB_STATUS_READY ) {
        // Critical error. Failed to cancel pending i/o requests.
        err ( "Failed io_cancel; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return CANNOT_CONTINUE;
    }

    debug ( "Canceled; (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( ! m_ep0_active ) {
        // Assume the inactivity of EP0 is the source of failure.
        err ( "EP0 is inactive; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        return CANNOT_CONTINUE;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_TIMEOUT ) {
        // Timeout happened. Seems host didn't send data.
        // Restart transaction and try to read new USB frame.
        err ( "Timeout on io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_REQUEST );
        return READY;
    }

    // Critical state. EP0 is active. And failure detected.
    err ( "Critical error on io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_FAILED );
    return CANNOT_CONTINUE;
}


//---------------------------------------------------------------------------//
//  ->  STATE_FAILED            Critical errors (cannot continue).           //
//                              failed enqueue_read                          //
//                              failed io_wait (not a timeout)               //
//                              failed io_cancel                             //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_RX_HEADER_REQUEST Timeout while sending. Host inactive.        // 
//  ->  next_state              Frame successfully sent.                     //
//---------------------------------------------------------------------------//
bool usb_transport_device_t::write_frame (
    const uint8_t* const    src, 
    const size_t            len, 
    usb_state_t             next_state 
) {
    const int ep1 = m_fs_eps[1];

    usb_transport_err_t io_res;
    usb_transport_err_t cancel_res;
    const uint32_t      write_time_ms = USB_READ_TIMEOUT_MS;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );
    info  ( "Enqueue TX request; size: %d; time: %d; (%s):(%d)", (int)len, write_time_ms, __FUNCTION__, __LINE__ );

    io_res = enqueue_write( &m_iocb_out, m_evfd_wr, m_io_ctx, ep1, src, len );
    if ( io_res != usb_transport_err_t::USB_STATUS_READY ) {
        err ( "Failed enqueue_write; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return CANNOT_CONTINUE;
    }
    
    io_res = io_wait ( m_io_range, m_evfd_wr, m_fdset_write, m_io_ctx, write_time_ms );
    if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
        debug ( "Write done: (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( next_state );
        return READY;
    }

    warn  ( "Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
    debug ( "Cancel pending requests; (%s):(%d)", __FUNCTION__, __LINE__);

    // There's one pending I/O request. Cancel it.
    cancel_res = io_cancel();
    if (cancel_res != usb_transport_err_t::USB_STATUS_READY) {
        // Critical error. Failed to cancel pending i/o requests.
        err ( "Failed io_cancel; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return CANNOT_CONTINUE;
    }

    debug ( "Canceled; (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( ! m_ep0_active ) {
        // Assume the inactivity of EP0 is the source of failure.
        warn ( "EP0 is inactive; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
        return CANNOT_CONTINUE;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_TIMEOUT ) {
        // Timeout happened. Seems host want send data.
        // Restart transaction and try to read new USB frame.
        err ( "Timeout on io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_REQUEST );
        return READY;
    }

    // Critical state. EP0 is active. And failure detected.
    err ( "Critical error on io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_FAILED );
    return CANNOT_CONTINUE;
}

    
//---------------------------------------------------------------------------//

}
}
