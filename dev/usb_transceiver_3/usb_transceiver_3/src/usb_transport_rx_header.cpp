#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
//  ->  STATE_RX_HEADER_WAIT    Wait for the USB header.                     //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_FAILED            Critical error. Cannot continue.             //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_hdr_place() {

    const int ep2 = m_fs_eps[2];
    usb_transport_err_t io_res;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    // There's only 24 bytes size. Do not check the NO_MEM error.
    m_ctx.inp_hdr.resize ( hid::stream::Prefix::PrefixSize() );
    cleanup ( m_ctx.inp_hdr );

    io_res = enqueue_read ( &m_iocb_in, m_evfd_rd, m_io_ctx, ep2, m_ctx.inp_hdr.data(), m_ctx.inp_hdr.size() );

    if ( io_res != usb_transport_err_t::USB_STATUS_READY ) {
        err ( "Failed enqueue_read; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_FAILED);
    } else {
        LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_WAIT);
    }

    debug( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
}


//---------------------------------------------------------------------------//
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_RX_HEADER_WAIT    Continue waiting                             //
//  ->  STATE_RX_PAYLOAD        Read payload                                 //
//  ->  STATE_FAILED            Critical errors. Cannot continue.            //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_hdr_wait() {

    usb_transport_err_t io_res;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( ! m_ep0_active ) {
        info ("EP0 inactive; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_SPINUP);
        return;
    } 

    io_res = io_wait ( m_io_range, m_evfd_rd, m_fdset_read, m_io_ctx, USB_READ_TIMEOUT_MS );

    if ( io_res == usb_transport_err_t::USB_STATUS_READY ) {
        // Timeout to be ignored here.
        LOG_USB_STATE(usb_state_t::STATE_RX_PAYLOAD);
    } else
    if (io_res == usb_transport_err_t::USB_STATUS_FAILED ) {
        // Critical error. Cannot continue.
        err ( "Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_FAILED);
    } else {
        // Timeout. Usual case. Just ignore it.
        debug ( "Timeout io_wait; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_WAIT);
    }

    debug ( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
}

//---------------------------------------------------------------------------//

}
}
