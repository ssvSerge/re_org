#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
//  ->  STATE_RX_HEADER_REQUEST Transaction handling.                        //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_FAILED            Critical error. Cannot continue.             //
//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_phantom_read() {

    usb_transport_err_t io_res      = usb_transport_err_t::USB_STATUS_UNKNOWN;
    usb_transport_err_t cancel_res  = usb_transport_err_t::USB_STATUS_READY;

    const int   ep2                 = m_fs_eps[2];
    const int   READ_TIMEOUT_MS     = 1;
    uint32_t    skip_cnt            = 0;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    for ( ; ; ) {

        uint8_t dummy = 0;

        if ( ! m_ep0_active ) {
            warn ( "EP0 is inactive; (%s):(%d)", __FUNCTION__, __LINE__ );
            LOG_USB_STATE ( usb_state_t::STATE_SPINUP );
            break;
        }

        // io_prep_pread + io_submit
        io_res = enqueue_read ( &m_iocb_in, m_evfd_rd, m_io_ctx, ep2, &dummy, sizeof(dummy) );
        if ( io_res != usb_transport_err_t::USB_STATUS_READY) {
            err ( "Error: Failed enqueue_read; (%s):(%d)", __FUNCTION__, __LINE__ );
            LOG_USB_STATE ( usb_state_t::STATE_FAILED );
            break;
        }

        // io_prep_pread + io_submit -> select
        io_res = io_wait ( m_io_range, m_evfd_rd, m_fdset_read, m_io_ctx, READ_TIMEOUT_MS );
        if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
            // Data received. Try to read the next one.
            skip_cnt++;
            continue;
        }
        if ( io_res == usb_transport_err_t::USB_STATUS_TIMEOUT ) {
            // No more data.
            io_res = usb_transport_err_t::USB_STATUS_READY;
            break;
        }
        if ( io_res == usb_transport_err_t::USB_STATUS_FAILED ) {
            err ( "Error: Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }
    }

    cancel_res = io_cancel( &m_iocb_in );

    if ( cancel_res != usb_transport_err_t::USB_STATUS_READY ) {
        // Critical error. Failed to cancel pending i/o requests.
        err( "Failed io_cancel; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_FAILED );
        return;
    }

    if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
        // I/O subsystem is ready.
        debug ( "Done; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE ( usb_state_t::STATE_RX_HEADER_REQUEST );
        return;
    }

    // io_wait unexpectedly failed. Cannot recover.
    err ( "Critical error. io_wait failed; (%s):(%d)", __FUNCTION__, __LINE__ );
    LOG_USB_STATE ( usb_state_t::STATE_FAILED );
}

//---------------------------------------------------------------------------//

}
}
