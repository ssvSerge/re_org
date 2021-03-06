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

    m_ep0_active = false;

    m_tid_ep0 = std::thread( &usb_transport_device_t::ep0_thread, this);

    debug ( "Waiting on EP0..." );

    int wait_time_ms = 0;
    while (wait_time_ms < (30 * 1000) ) {

        std::this_thread::sleep_for(100ms);
        wait_time_ms += 100;

        if ( m_ep0_active ) {
            break;
        }
        if ( m_stop_request ) {
            break;
        }
    }

    if ( m_stop_request ) {
        err ( "Stop request triggered. Exit from: (%s):(%d)", __FUNCTION__, __LINE__ );
        return -1;
    }

    if ( ! m_ep0_active ) {
        err ( "Unable to initialize endpoint. Exiting. (%s):(%d)", __FUNCTION__, __LINE__ );
        return -1;
    }

    debug ( "EP0: ready." );
    m_tid_com = std::thread( &usb_transport_device_t::ep1_ep2_thread, this, m_fs_eps[1], m_fs_eps[2] );

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

            if ( m_stop_request == true ) {
                break;
            }

            FD_ZERO(&rfds);
            FD_SET(m_fs_eps[0], &rfds);

            t.tv_sec  =      0; // Seconds
            t.tv_usec = 500000; // Microseconds

            select_res = select(m_fs_eps[0] + 1, &rfds, NULL, NULL, &t);

            if (select_res < 0 ) {
                if (errno == EINTR) {
                    continue;
                }
            }

            break;
        }

        if ( m_stop_request == true ) {
            err ( "Shutdown request received; Exit; (%s):(%d)", __FUNCTION__, __LINE__ );
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

        bytes_read = read(m_fs_eps[0], ep0_buffer.data(), buffSize);
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
                    m_ep0_active = true;
                    break;

                case FUNCTIONFS_DISABLE:
                    info("EP0:  FUNCTIONFS_DISABLE");
                    m_ep0_active = false;
                    break;

                case FUNCTIONFS_SUSPEND:
                    info("EP0:  FUNCTIONFS_SUSPEND");
                    m_ep0_active = false;
                    break;

                case FUNCTIONFS_RESUME:
                    info("EP0:  FUNCTIONFS_RESUME");
                    m_ep0_active = false;
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

void usb_transport_device_t::ep1_ep2_thread ( int ep1, int ep2 ) {

    info ( "Enter: ep1_ep2_thread; (%s):(%d)", __FUNCTION__, __LINE__ );

    UNUSED ( ep1 );
    UNUSED ( ep2 );

    LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);

    for ( ; ; ) {

        if ( m_stop_request ) {
            // External request to shutdown.
            err ( "Shutdown request received; Exit; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        if ( m_sync_request ) {
            m_sync_request = false;

            // External request to synchronize read process. 
            // EP0 should receive a special request to Synchronize.
            // Currently not implemented because limitation one host side.
            err ("SYNC request received; Reset state to \"INITIALIZE\"; (%s):(%d)", __FUNCTION__, __LINE__);
            LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);
        }

        switch ( g_transaction_state.get() ) {

            case usb_state_t::STATE_SPINUP: {
                // Infinite wait for EP0. (see ep0_thread & m_ep0_active).
                //  -> STATE_INITIALIZE         When EP0 switch to UP.
                //  -> STATE_SPINUP             To check the Stop condition.
                handle_spinup();
            }   break;

            case usb_state_t::STATE_INITIALIZE:
                // Opens handles, i/o context, etc.
                //  ->  STATE_PHANTOM_READ      Try to read data buffered before.
                //  ->  STATE_SPINUP            EP0 unexpectedly switch down.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_initialize();
                break;

            case usb_state_t::STATE_PHANTOM_READ:
                // Cleanup buffers. Read until timeout.
                //  ->  STATE_RX_HEADER_REQUEST Transaction handling.
                //  ->  STATE_SPINUP            EP0 unexpectedly switch down.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_phantom_read();
                break;

            case usb_state_t::STATE_RX_HEADER_REQUEST:
                // Place the read request "HEADER".
                //  ->  STATE_RX_HEADER_WAIT    Wait for the USB header.
                //  ->  STATE_SPINUP            EP0 unexpectedly switch down.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_hdr_place();
                break;

            case usb_state_t::STATE_RX_HEADER_WAIT:
                // Wait for the completion of the read request "HEADER".
                //  ->  STATE_RX_PAYLOAD        Read payload
                //  ->  STATE_RX_HEADER_WAIT    Continue waiting 
                //  ->  STATE_SPINUP            EP0 unexpectedly switch down.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_hdr_wait();
                break;

            case usb_state_t::STATE_RX_PAYLOAD:
                // Allocate buffer and read payload.
                // Three options:
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                //  ->  STATE_SPINUP            EP0 unexpectedly switch down.
                //  ->  STATE_RX_HEADER_REQUEST Timeout while reading.
                //  ->  STATE_HANDLE_REQUEST    Payload ready. Process transaction.
                //  ->  STATE_TX_RESPONSE       Payload discarded. Return error code.
                handle_payload_read();
                break;

            case usb_state_t::STATE_HANDLE_REQUEST:
                // Handle request.
                //  ->  STATE_TX_HEADER_PLACE   Request processed.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_command();
                break;

            case usb_state_t::STATE_TX_RESPONSE:
                // Send Header and Payload both.
                //  ->  STATE_RX_HEADER_REQUEST Successfully sent.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_tx_resp();
                break;

            case usb_state_t::STATE_FAILED:
                err("Critical error. Application going down; (%s):(%d)", __FUNCTION__, __LINE__);
                m_stop_request = true;
                break;

            default:
                break;
        }
    }

    err("Destroy I/O context; (%s):(%d)", __FUNCTION__, __LINE__);
    io_destroy(m_io_ctx);

    err("Close m_evfd_rd; (%s):(%d)", __FUNCTION__, __LINE__);
    close(m_evfd_rd);

    err("Close m_evfd_wr; (%s):(%d)", __FUNCTION__, __LINE__);
    close(m_evfd_wr);

    info("Leave: ep1_ep2_thread; (%s):(%d)", __FUNCTION__, __LINE__);
    m_thread_fininsed.notify_one();
}

//---------------------------------------------------------------------------//

}
}
