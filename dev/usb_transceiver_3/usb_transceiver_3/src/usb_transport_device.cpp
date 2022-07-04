#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include <thread>
#include <chrono>

#include <hid/epconst.h>
#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

state_monitor_t  g_transaction_state;

//---------------------------------------------------------------------------//

void usb_transport_device_t::cleanup ( usb_frame_t& frame ) {
    if ( frame.size() > 0 ) {
        size_t cleanup_len = frame.size();
        if (cleanup_len > MAX_CLEANUP_LEN) {
            // Seems there's no need to clean megabytes of data. 
            // Should be enough to wipe few kilo bytes only.
            cleanup_len = MAX_CLEANUP_LEN;
        }
        memset(frame.data(), 0x00, cleanup_len);
    }
}

//---------------------------------------------------------------------------//

usb_transport_device_t::usb_transport_device_t() {

    memset ( &m_io_ctx, 0, sizeof(m_io_ctx) );
    m_ep0_ctrl = -1;
    m_ep1_tx   = -1;
    m_evfd_tx  = -1;
    m_ep2_rx   = -1;
    m_evfd_rx  = -1;
}

void usb_transport_device_t::stop() {

    info ( "Stop request received; (%s):(%d)", __FUNCTION__, __LINE__ );
    m_stop_request = true;
}

void usb_transport_device_t::join() {

    info ( "Waiting for threads; (%s):(%d)", __FUNCTION__, __LINE__ );

    // At least one thread (ep0_thread or ep1_ep2_thread) terminated stopped.
    m_thread_fininsed.wait();

    info ( "Sending STOP request; (%s):(%d)", __FUNCTION__, __LINE__ );

    // Request to stop to second thread.
    stop();

    if ( m_tid_com.joinable() ) {
        info ( "Waiting for EP0; (%s):(%d)", __FUNCTION__, __LINE__ );
        m_tid_com.join();
        info ( "EP0 down; (%s):(%d)", __FUNCTION__, __LINE__ );
    }

    if ( m_tid_ep0.joinable() ) {
        info ( "Waiting for EP1_EP2; (%s):(%d)", __FUNCTION__, __LINE__ );
        m_tid_ep0.join();
        info ( "EP1_EP2 down; (%s):(%d)", __FUNCTION__, __LINE__ );
    }

    info ( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
}

int32_t usb_transport_device_t::ep0_init(std::string epDirectory) {

    Descriptors descriptors = get_descriptors();
    Strings     strings     = get_strings();
    int         ret         = {};
    bool        is_failed   = false;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    m_ep_directory = epDirectory;

    m_ep0_ctrl = -1;
    m_ep1_tx   = -1;
    m_ep2_rx   = -1;

    std::string ep0_path = epDirectory + "/" + "ep0";

    info  ( "writing descriptors (in v2 format) to: %s", ep0_path.c_str() );

    debug ( "Open endpoint: %s", ep0_path.c_str() );
    m_ep0_ctrl = open ( ep0_path.c_str(), O_RDWR|O_NDELAY|O_NONBLOCK );
    if ( m_ep0_ctrl == -1 ) {
        err ( "Failed to open endpoint: %s; (%s):(%d)", ep0_path.c_str(), __FUNCTION__, __LINE__ );
        is_failed = true;
    } else {

        int io_res;

        io_res = write ( m_ep0_ctrl, &descriptors, sizeof(descriptors) );
        if ( io_res == -1 ) {
            err ( "Failed to write descriptors; %s; (%s):(%d)", ep0_path.c_str(), __FUNCTION__, __LINE__ );
            is_failed = true;
        }

        io_res = write ( m_ep0_ctrl, &strings, sizeof(strings) );
        if ( io_res == -1 ) {
            err ( "Failed to write strings; %s; (%s):(%d)", ep0_path.c_str(), __FUNCTION__, __LINE__ );
            is_failed = true;
        }

        if ( ! is_failed ) {
            m_ep1_tx = open_ep(1);
            m_ep2_rx = open_ep(2);
        }

    }

    if ( is_failed ) {
        err("Failed to initialize endpoint: (%s):(%d)", __FUNCTION__, __LINE__);
        close ( m_ep0_ctrl );
        close ( m_ep1_tx );
        close ( m_ep2_rx );
        return -1;
    }

    err("Done: (%s):(%d)", __FUNCTION__, __LINE__);
    return 0;
}

int32_t usb_transport_device_t::open_ep ( int32_t endpoint_no ) {

    char buffer[1024];

    sprintf(buffer, "%s/ep%d", m_ep_directory.c_str(), endpoint_no );

    debug("Open endpoint: %s", buffer );

    int fd = open(buffer, O_RDWR);

    if ( fd == -1 ) {

    }
    return fd;
}

#if 0
bool usb_transport_device_t::get_timeout_ms(const checkpoint_t point_timeout, uint32_t& ms_cnt) {

    bool ret_val = false;

    ms_cnt = 0;

    // Time source is a steady time. We're free from time shift.
    checkpoint_t my_time = time_source_t::now();

    if (point_timeout > my_time) {

        auto duration = point_timeout - my_time;
        auto duration_ms = std::chrono::duration_cast<duration_ms_t>(duration);

        // Allow waiting only if it's more than 2 milliseconds.
        if (duration_ms.count() > 2) {
            ms_cnt = static_cast<uint32_t>(duration_ms.count());
            ret_val = true;
        }
    }

    return ret_val;
}
#endif


//---------------------------------------------------------------------------//

}
}
