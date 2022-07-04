#include <unistd.h>
#include <sys/eventfd.h>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
//  ->  STATE_FAILED            Critical error. Cannot continue.             //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_PHANTOM_READ      Try to read data buffered before.            //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_initialize() {

    // Attempt to start (or re-initialize) the read/write context.
    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( m_evfd_tx != -1 ) {
        info ( "close(em_evfd_wr); (%s):(%d)", __FUNCTION__, __LINE__ );
        close ( m_evfd_tx );
        m_evfd_tx = -1;
    }

    if ( m_evfd_rx != -1 ) {
        info ( "close(m_evfd_rd); (%s):(%d)", __FUNCTION__, __LINE__ );
        close ( m_evfd_rx );
        m_evfd_rx = -1;
    }

    if ( m_io_ctx_valid ) {
        m_io_ctx_valid = false;
        info ( "io_destroy(m_io_ctx); (%s):(%d)", __FUNCTION__, __LINE__ );
        io_destroy ( m_io_ctx );
        memset ( &m_io_ctx, 0x00, sizeof(m_io_ctx) );
    }

    // io_setup   -> create an asynchronous I/O context
    // io_destroy -> destroy an asynchronous I/O context
    // 
    // creates an asynchronous I/O context suitable for 
    // concurrently processing nr_events operations.
    // The ctx_idp argument must not point to an AIO context 
    // that already exists, and must be initialized to 0 prior 
    // to the call. On successful creation of the AIO context
    // ctx_idp is filled in with the resulting handle.
    if ( io_setup(2, &m_io_ctx) < 0 ) {

        // Critical error. Cannot continue.
        err ( "Failed io_setup; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_FAILED);

    }  else {

        m_io_ctx_valid = true;

        // Creates an "eventfd object" that can be used as an event wait / notify 
        // mechanism by user - space applications, and by the kernel to 
        // notify user - space applications of events. The object contains an 
        // unsigned 64 - bit integer(uint64_t) counter that is maintained by the kernel.
        // This counter is initialized with the value specified in the argument initval.
        //
        // int eventfd(unsigned int initval, int flags);
        // flags    -> EFD_CLOEXEC
        //          -> EFD_NONBLOCK
        //          -> EFD_SEMAPHORE
        //
        // error    -> EINVAL 
        //          -> EMFILE 
        //          -> ENODEV
        //          -> ENOMEM
        //
        // Seems all errors are critical. Application cannot continue and should be closed.
        m_evfd_tx = eventfd(0, 0);
        debug ( "m_evfd_wr = %d; (%s):(%d)", m_evfd_tx, __FUNCTION__, __LINE__ );

        m_evfd_rx = eventfd(0, 0);
        debug ( "m_evfd_rd = %d; (%s):(%d)", m_evfd_rx, __FUNCTION__, __LINE__ );

        if ( (m_evfd_tx<0) || (m_evfd_rx<0) ) {

            err("EP2:  Failed eventfd. Error: %d; (%s):(%d)", errno, __FUNCTION__, __LINE__);
            LOG_USB_STATE(usb_state_t::STATE_FAILED);

        } else {
            LOG_USB_STATE(usb_state_t::STATE_PHANTOM_READ);
        }

    }

    debug("Leave: (%s):(%d)", __FUNCTION__, __LINE__);
}

//---------------------------------------------------------------------------//

}
}
