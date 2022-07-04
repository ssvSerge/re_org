#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/usb/functionfs.h>

#include <thread>
#include <chrono>

#include <hid/epconst.h>
#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>
#include <logging.h>

using namespace std::chrono_literals;

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

void usb_transport_device_t::log_event(const struct usb_ctrlrequest* setup) {
    info("bRequestType = %d\n", setup->bRequestType);
    info("bRequest     = %d\n", setup->bRequest);
    info("wValue       = %d\n", le16_to_cpu(setup->wValue));
    info("wIndex       = %d\n", le16_to_cpu(setup->wIndex));
    info("wLength      = %d\n", le16_to_cpu(setup->wLength));
}

int32_t usb_transport_device_t::init_ep_threads() {

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( m_ep_directory.size() == 0 ) {
        err ( "Error: m_ep_directory is not defined. (%s):(%d)", __FUNCTION__, __LINE__ );
        return -1;
    }

    m_ep0_inactive = true;

    m_tid_ep0 = std::thread( &usb_transport_device_t::ep0_thread, this);

    debug ( "Waiting on EP0..." );

    int wait_time_ms = 0;
    while (wait_time_ms < (240 * 1000) ) {

        if ( m_stop_request ) {
            err("Stop request; (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }

        if ( m_ep0_inactive ) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        break;
    }

    if ( m_stop_request ) {
        err ( "Stop request; (%s):(%d); ", __FUNCTION__, __LINE__ );        
        return -1;
    }

    if ( m_ep0_inactive ) {
        err ( "Unable to initialize endpoint. Exiting. (%s):(%d)", __FUNCTION__, __LINE__ );
        return -1;
    }

    debug ( "EP0: ready." );
    m_tid_com = std::thread( &usb_transport_device_t::ep1_ep2_thread, this );

    debug ( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
    return 0;
}

void usb_transport_device_t::ep0_thread() {

    int         select_res;
    fd_set      rfds;
    timeval     t;
    int         bytes_read;

    info ( "Enter: ep0_thread; (%s):(%d)", __FUNCTION__, __LINE__ );

    size_t buffSize = 4 * sizeof(struct usb_functionfs_event);
    std::vector<uint8_t> ep0_buffer; 

    ep0_buffer.resize(buffSize);

    for ( ; ; ) {

        select_res = 0;

        for ( ; ; ) {

            if ( m_stop_request ) {
                err( "Stop request; (%s):(%d); ", __FUNCTION__, __LINE__ );
                break;
            }

            // t.tv_sec  = 0;      // Seconds
            // t.tv_usec = 100000; // Microseconds

            t.tv_sec  = 5; // Seconds
            t.tv_usec = 0; // Microseconds

            FD_ZERO ( &rfds );
            FD_SET  ( m_ep0_ctrl, &rfds );

            info("ep0_thread + select; (%s):(%d)", __FUNCTION__, __LINE__);
            select_res = select ( m_ep0_ctrl + 1, &rfds, NULL, NULL, &t );
            info("ep0_thread + select; result: %d; (%s):(%d)", select_res, __FUNCTION__, __LINE__);

            if ( select_res<0 ) {
                if (errno == EINTR) {
                    continue;
                }
            }
        }

        if ( m_stop_request ) {
            err ( "Stop request; (%s):(%d); ", __FUNCTION__, __LINE__ );        
            break;
        }

        if ( select_res < 0 ) {
            err ( "Failed on select; Exit; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        if ( select_res == 0 ) {
            // Timeout on select. Read the next one.
            continue;
        }

        bytes_read = read ( m_ep0_ctrl, ep0_buffer.data(), buffSize );
        if (  (bytes_read == 0)  ||  (bytes_read==-1)  ) {
            warn("EP0:  Spinning....");
            continue;
        }

        const struct usb_functionfs_event* event = (usb_functionfs_event*)ep0_buffer.data();
    
        size_t n;
        for (n = bytes_read / sizeof(*event); n; --n, ++event) {
            switch (event->type) {
                case FUNCTIONFS_BIND:
                    info("EP0:  FUNCTIONFS_BIND");
                    break;

                case FUNCTIONFS_UNBIND:
                    info("EP0:  FUNCTIONFS_UNBIND");
                    break;

                case FUNCTIONFS_ENABLE:
                    info("EP0:  FUNCTIONFS_ENABLE");
                    m_ep0_inactive = false;
                    break;

                case FUNCTIONFS_DISABLE:
                    info("EP0:  FUNCTIONFS_DISABLE");
                    m_ep0_inactive = true;
                    break;

                case FUNCTIONFS_SUSPEND:
                    info("EP0:  FUNCTIONFS_SUSPEND");
                    m_ep0_inactive = true;
                    break;

                case FUNCTIONFS_RESUME:
                    info("EP0:  FUNCTIONFS_RESUME");
                    m_ep0_inactive = false;
                    break;

                case FUNCTIONFS_SETUP:
                    info("EP0:  FUNCTIONFS_SETUP");
                    log_event(&event->u.setup);
                    break;

                default:
                    info("Event %03u (unknown)", event->type);
            }
        }

    }

    info ( "Leave: ep0_thread; (%s):(%d)", __FUNCTION__, __LINE__ );

    m_thread_fininsed.notify_one();
}

void usb_transport_device_t::ep1_ep2_thread () {

    info ( "Enter: ep1_ep2_thread; (%s):(%d)", __FUNCTION__, __LINE__ );

    LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);

    for ( ; ; ) {

        if ( m_stop_request ) {
            // External request to shutdown.
            err ( "Exit; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        if ( m_sync_request ) {
            m_sync_request = false;
            // External request to synchronize read process. 
            // EP0 should receive a special request to Synchronize.
            // Currently not implemented because limitation one host side.
            err ( "SYNC request received; Reset state to \"INITIALIZE\"; (%s):(%d)", __FUNCTION__, __LINE__ );
            LOG_USB_STATE( usb_state_t::STATE_INITIALIZE );
        }

        switch ( g_transaction_state.get() ) {

            case usb_state_t::STATE_SPINUP:
                handle_spinup();
                break;

            case usb_state_t::STATE_INITIALIZE:
                handle_initialize();
                break;

            case usb_state_t::STATE_PHANTOM_READ:
                handle_phantom_read();
                break;

            case usb_state_t::STATE_RX_HEADER_WAIT:
                handle_hdr_wait();
                break;

            case usb_state_t::STATE_RX_PAYLOAD:
                handle_payload_read();
                break;

            case usb_state_t::STATE_HANDLE_REQUEST:
                handle_command();
                break;

            case usb_state_t::STATE_TX_RESPONSE:
                handle_tx_resp();
                break;

            case usb_state_t::STATE_FAILED:
                err("Critical error; (%s):(%d)", __FUNCTION__, __LINE__);
                m_stop_request = true;
                break;

            case usb_state_t::STATE_SHUTDOWN:
                err ( "Request to shutdown. (%s):(%d)", __FUNCTION__, __LINE__ );
                m_stop_request = true;
                break;

            default:
                break;
        }
    }

    err("Close i/o context; (%s):(%d)", __FUNCTION__, __LINE__);
    io_destroy ( m_io_ctx );

    err("Close m_evfd_rx; (%s):(%d)", __FUNCTION__, __LINE__);
    close ( m_evfd_rx );

    err("Close m_evfd_tx; (%s):(%d)", __FUNCTION__, __LINE__);
    close ( m_evfd_tx );

    err("Close EP0; (%s):(%d)", __FUNCTION__, __LINE__);
    close ( m_ep0_ctrl );

    err("Close EP1; (%s):(%d)", __FUNCTION__, __LINE__);
    close ( m_ep1_tx );

    err("Close EP2; (%s):(%d)", __FUNCTION__, __LINE__);
    close ( m_ep2_rx );

    info("Leave: ep1_ep2_thread; (%s):(%d)", __FUNCTION__, __LINE__);
    m_thread_fininsed.notify_one();
}

//---------------------------------------------------------------------------//

}
}
