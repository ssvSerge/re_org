// #include <chrono>
// #include <cstring>
// #include <algorithm>

// #include <fcntl.h>
// #include <unistd.h>
// #include <dirent.h>
#include <epconst.h>
#include <sys/eventfd.h>
#include <linux/usb/functionfs.h>

#include <StreamPrefix.h>
#include <consts.h>
#include <epinit.h>

using std::chrono_literals;


#ifndef UNUSED
#define UNUSED(x)                   (void)(x)
#endif

//---------------------------------------------------------------------------//

#define LOG_USB_STATE(x)   { m_state_monitor.set_state(x, __FUNCTION__, __LINE__); }

constexpr uint32_t MAX_CLEANUP_LEN     = 16 * 1024;

//---------------------------------------------------------------------------//

static void print_errno() {
    switch (errno) {
        case EBADF:
            err("\nAn invalid file descriptor was given in one of the sets.");
            break;
        case EINTR:
            err("\nA signal was caught; see signal(7).");
            break;
        case EINVAL:
            err("\nnfds is negative or exceeds the RLIMIT_NOFILE resource or invalid timeout");
            break;
        case ENOMEM:
            err("\nUnable to allocate memory for internal tables.");
            break;
        default:
            err("\nUnknown error, errno = %d", errno);
            break;
    };
}

static void cleanup( usb_frame_t& frame ) {

    if ( frame.size() > 0 ) {
        size_t cleanup_len = frame.size();
        if ( cleanup_len > MAX_CLEANUP_LEN) {
            // Seems there's no need to clean megabytes of data. 
            // Should be enough to wipe few kilo bytes only.
            cleanup_len = MAX_CLEANUP_LEN;
        }
        memset (frame.data(), 0x00, cleanup_len );
    }
}

//---------------------------------------------------------------------------//

epinit::epinit() { 
    m_evfd_rd    = -1; 
    m_evfd_wr    = -1;
    m_sync_request  = false;
    m_stop_request  = false;

}

//---------------------------------------------------------------------------//

int32_t epinit::ep0_init(std::string epDirectory) {

    Descriptors descriptors = get_descriptors();
    Strings strings = get_strings();
    int ret = {};

    std::string ep0Path = epDirectory + "/" + "ep0";
    info("%s: writing descriptors (in v2 format)\n", ep0Path.c_str());
    int fd = open(ep0Path.c_str(), O_RDWR | O_NDELAY | O_NONBLOCK);
    if (fd == -1) {
        err("Error opening endpoint (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    ret = write(fd, &descriptors, sizeof(descriptors));
    if (ret == -1) {
        err("Endpoint rejected descriptors (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    ret = write(fd, &strings, sizeof(strings));
    if (ret == -1) {
        err("Error: Endpoint rejected strings (%s):(%d)", __FUNCTION__, __LINE__);
        print_errno();
        close(fd);
        m_error_event.notify_one();
        return -1;
    }

    m_fs_eps[0] = fd;
    m_ep_directory = epDirectory;

    for (size_t ii = 1; ii < 3; ii++) {
        m_fs_eps[ii] = init_ep(ii);
        if (m_fs_eps[ii] == -1) {
            return -1;
        };
    }

    return 0;
}

//---------------------------------------------------------------------------//

int32_t epinit::init_ep ( int32_t nEpNo ) {
    char buffer[1024];
    sprintf(buffer, "%s/ep%d", m_ep_directory.c_str(), nEpNo);
    int fd = open(buffer, O_RDWR);
    return fd;
}

int32_t epinit::init_ep_threads() {

    if (m_ep_directory.size() == 0) {
        err("Error Not initialized (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    m_tid_ep0 = std::thread(&epinit::ep0_thread, this);

    info("Waiting on EP0...");

    int wait_time_ms = 0;
    while (wait_time_ms < (30 * 1000) ) {

        std::this_thread::sleep_for(100ms);
        wait_time_ms += 100;

        if ( m_ep0_active ) {
            break;
        }

        if (m_stop_request) {
            break;
        }
    }

    if ( m_stop_request ) {
        err("Stop request triggered. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
        break;
    }

    if ( ! m_ep0_active ) {
        err("Unable to initialize endpoint. Exiting. (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    info("EP0: ready.");
    m_tid_com = std::thread(&epinit::ep1_ep2_thread, this, m_fs_eps[1], m_fs_eps[2]);

    return 0;
}

void epinit::handle_setup ( const struct usb_ctrlrequest* setup ) {
    info("bRequestType = %d\n", setup->bRequestType);
    info("bRequest     = %d\n", setup->bRequest);
    info("wValue       = %d\n", le16_to_cpu(setup->wValue));
    info("wIndex       = %d\n", le16_to_cpu(setup->wIndex));
    info("wLength      = %d\n", le16_to_cpu(setup->wLength));
}

void epinit::stop() {

    m_stop_request = true;
}

void epinit::join() {

    m_stop_event.wait();

    if ( m_tid_com.joinable()) {
        m_tid_com.join();
    }

    if ( m_tid_ep0.joinable() ) {
        m_stop_request = true;
        m_tid_ep0.join();
    }
}

//---------------------------------------------------------------------------//

USB_STATUS epinit::handle_request ( const usb_frame_t& in_hdr, const usb_frame_t& in_frame, usb_frame_t& out_frame, int32_t& err_code ) {

    USB_STATUS ret_val = USB_STATUS::USB_STATUS_READY;

    UNUSED(in_hdr);
    UNUSED(in_frame);

    out_frame.resize (66);
    err_code = 77;

    return ret_val;
}

USB_STATUS epinit::read_event ( int evfd, fd_set& rfds, io_context_t ctx ) {

    USB_STATUS ret_val = USB_STATUS::USB_STATUS_READY;

    if ( FD_ISSET(evfd, &rfds) ) {

        struct io_event     e[2];
        uint64_t            ev_cnt;

        // Asynchronous Read from USB endpoint.

        // attempts to read up to count bytes from file descriptor
        // into the buffer starting at buffer.
        int ret = read ( evfd, &ev_cnt, sizeof(ev_cnt) );
        if ( ret < 0 ) {
            err ("Error: Failed read; (%s):(%d)", __FUNCTION__, __LINE__);
            ret_val = USB_STATUS::USB_STATUS_FAILED;
        } else {

            for ( ; ; ) {

                // io_getevents - read asynchronous I/O events from the completion queue
                // 
                // On success, io_getevents() returns the number of events read.
                // This may be 0, or a value less than min_nr, if the timeout
                // expired. It may also be a nonzero value less than min_nr, 
                // if the call was interrupted by a signal handler.
                // 
                // on error it returns a negated error number with error codes:
                // EFAULT, EINTR, EINVAL, ENOSYS
                // 
                ret = io_getevents(ctx, 1, 2, e, NULL);

                if ( ret == -1 ) {
                    if ( errno == EINTR ) {
                        continue;
                    }
                    err("Error: Failed io_getevents; (%s):(%d)", __FUNCTION__, __LINE__);
                    ret_val = USB_STATUS::USB_STATUS_FAILED;
                }

                break;
            }
        }
    }

    return ret_val;
}

USB_STATUS epinit::io_wait ( int eprange, int evfd, fd_set& rfds, io_context_t ctx, int timeout_ms ) {

    USB_STATUS ret_val = USB_STATUS::USB_STATUS_FAILED;

    for ( ; ; ) {

        struct timeval timeout;

        timeout.tv_sec  = (timeout_ms / 1000);
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        FD_ZERO(&rfds);
        FD_SET(evfd, &rfds);

        // On success, return the number of file descriptors contained in 
        // the three returned descriptor sets(that is, the total number of 
        // bits that are set in readfds, writefds, exceptfds).
        // Return value may be zero 
        // if the timeout expired before any file descriptors became ready.
        //    EBADF, EINTR, EINVAL, ENOMEM
        // 
        // ret == -1  --> Error
        // ret ==  0  --> Timeout
        // ret >   0  --> Event triggered.
        int ret = select(eprange, &rfds, NULL, NULL, &timeout);

        if ( ret < 0 ) {
            if (errno == EINTR) {
                continue;
            }
        }

        if (ret < 0) {
            err ("Error: Failed select. (%s):(%d)", __FUNCTION__, __LINE__);
            ret_val = USB_STATUS::USB_STATUS_FAILED;
        } else
        if ( ret == 0 ) {
            ret_val = USB_STATUS::USB_STATUS_TIMEOUT;
        } else {
            ret_val = USB_STATUS::USB_STATUS_READY;
        }

        break;

    }

    if ( ret_val == USB_STATUS::USB_STATUS_READY ) {
        ret_val = read_event(evfd, rfds, ctx);
    } else
    if ( ret_val != USB_STATUS::USB_STATUS_TIMEOUT ) {
        err("EP2:  io_wait failed with error: %d; (%s):(%d)", errno, __FUNCTION__, __LINE__);
    }

    return ret_val;
}

USB_STATUS epinit::enqueue_read ( iocb* iocb_read, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes ) {

    USB_STATUS ret_val = USB_STATUS::USB_STATUS_READY;

    io_prep_pread(iocb_read, endpoint, data, num_bytes, 0);

    iocb_read->u.c.flags |= IOCB_FLAG_RESFD;
    iocb_read->u.c.resfd  = event_fd;

    // On success, io_submit() returns the number of iocbs submitted
    // which may be less than nr, or 0 if nr is zero. 
    // 
    // on error it returns a negated error number.
    //     EAGAIN, EBADF, EFAULT, EINVAL, ENOSYS, EPERM
    // 
    // We're on the asynchronous I/O. EAGAIN is not expected here.
    int ret = io_submit(ctx, 1, &iocb_read);

    if (ret < 0) {
        err("Error: Failed io_submit. (%s):(%d)", __FUNCTION__, __LINE__);
        ret_val = USB_STATUS::USB_STATUS_FAILED;
    }

    return ret_val;
}

USB_STATUS epinit::enqueue_write (iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes) {

    USB_STATUS ret_val = USB_STATUS::USB_STATUS_READY;

    io_prep_pwrite(iocb_write, endpoint, data, num_bytes, 0);

    iocb_write->u.c.flags |= IOCB_FLAG_RESFD;
    iocb_write->u.c.resfd  = event_fd;

    int ret = io_submit(ctx, 1, &iocb_write);

    if (ret < 0) {
        err("Error: Failed io_submit. (%s):(%d)", __FUNCTION__, __LINE__);
        ret_val = USB_STATUS::USB_STATUS_FAILED;
    }

    return ret_val;
}

void epinit::ep0_thread () {

    info("EP0:  Thread Started.");

    size_t buffSize = 4 * sizeof(struct usb_functionfs_event);

    m_ep0Buffer.resize(buffSize);

    do {

        fd_set  rfds;
        timeval t;
        int     ret;

        for ( ; ; ) {

            // 10 checks per second
            t.tv_sec  =      0;     // Seconds
            t.tv_usec = 100000;     // Microseconds

            FD_ZERO(&rfds);
            FD_SET(m_fs_eps[0], &rfds);
            ret = select(m_fs_eps[0] + 1, &rfds, NULL, NULL, &t);

            if (ret < 0) {
                if (errno == EINTR) {
                    ret = 0;
                }
            }

            if (ret != 0) {
                break;
            }

            if (m_stop_request == true) {
                break;
            }
        }

        if (m_stop_request == true) {
            warn("EP0:  Shutdown request received.");
            break;
        }

        int bytes_read = read(m_fs_eps[0], m_ep0Buffer.data(), buffSize);
        if (bytes_read == 0 || bytes_read == -1) {
            warn("EP0:  Spinning....");
            continue;
        }

        const struct usb_functionfs_event* event = (usb_functionfs_event*)m_ep0Buffer.data();
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
                    handle_setup(&event->u.setup);
                    break;

                default:
                    info("Event %03u (unknown)", event->type);
            }
        }

    } while (1);

    m_stop_event.notify_one();
}

void epinit::handle_initialize() {

    USB_STATUS io_res;

    // Attempt to start (or re-initialize) the read/write context.
    info("EP2:  USB Queue initializing; (%s):(%d)", __FUNCTION__, __LINE__);

    if ( m_evfd_wr != -1 ) {
        close(m_evfd_wr);
        m_evfd_wr = -1;
    }

    if ( m_evfd_rd != -1 ) {
        close(m_evfd_rd);
        m_evfd_rd = -1;
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
    //
    memset(&m_ctx, 0, sizeof(m_ctx));
    if ( io_setup(2, &m_ctx) < 0 ) {

        err("EP2:  Failed io_setup. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_FAILED);

    } else {

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
        m_evfd_wr = eventfd(0, 0);
        m_evfd_rd = eventfd(0, 0);

        if ( (m_evfd_wr<0) || (m_evfd_rd<0) ) {

            err("EP2:  Failed eventfd. Error: %d; (%s):(%d)", errno, __FUNCTION__, __LINE__);
            LOG_USB_STATE(usb_state_t::STATE_FAILED);

        } else {

            if ( m_evfd_rd > m_evfd_wr ) {
                m_nEPRange = m_evfd_rd + 1;
            } else {
                m_nEPRange = m_evfd_wr + 1;
            }

            LOG_USB_STATE(usb_state_t::STATE_READ_ON_INITIALIZE);
        }

    }
}

void epinit::handle_phantom_read () {

    USB_STATUS io_res;

    // Attempt to start (or re-initialize) the read/write context.
    info("EP2:  Phantom reading; (%s):(%d)", __FUNCTION__, __LINE__);

    for ( ; ; ) {

        uint8_t dummy = 0;

        // io_prep_pread + io_submit
        io_res = enqueue_read ( &m_iocb_in, m_evfd_rd, m_ctx, ep2, &dummy, sizeof(dummy) );
        if ( io_res != USB_STATUS::USB_STATUS_READY) {
            err("Error: Failed enqueue_read; (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        // select 
        io_res = io_wait(m_nEPRange, m_evfd_rd, m_fdset_read, m_ctx, USB_READ_TIMEOUT_MS);
        if ( io_res == USB_STATUS::USB_STATUS_FAILED ) {
            err("Error: Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }
        if ( io_res == USB_STATUS::USB_STATUS_READY ) {
            // Data received. Try to read the next one.
            continue;
        }

        // Timeout -> no more data.
        io_res = USB_STATUS::USB_STATUS_READY;
        break;
    }

    if ( io_res != USB_STATUS::USB_STATUS_READY) {

        err ("Error: Failed handle_phantom_read; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_FAILED);

    } else {

        // There's one request pending.
        // 
        // io_cancel - cancel an outstanding asynchronous I/O operation
        // 
        // The io_cancel() system call attempts to cancel an asynchronous I/O 
        // operation previously submitted with io_submit(2). The iocb argument 
        // describes the operation to be canceled and the ctx_id argument is 
        // the AIO context to which the operation was submitted. If the operation 
        // is successfully canceled, the event will be copied into the memory 
        // pointed to by result without being placed into the completion queue.
        // 
        int cancel_res = io_cancel (m_iocb_in);

        if ( cancel_res != 0 ) {
            err("Error: Failed io_cancel with error: %d; (%s):(%d)", (int)errno, __FUNCTION__, __LINE__);
            LOG_USB_STATE(usb_state_t::STATE_FAILED);
        } else {
            LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_PLACE);
        }
    }
}

void epinit::handle_hdr_place() {

    info("EP2:  Place RX request for Header; (%s):(%d)", __FUNCTION__, __LINE__);

    // PrefixSize is 24 bytes only. Assume there's no chance to have "no memory" exception.
    inp_hdr.resize(hid::stream::Prefix::PrefixSize());

    // There's a chance old data still stored in the buffer.
    cleanup(inp_hdr);

    // io_prep_pread -> io_submit
    io_res = enqueue_read(&m_iocb_in, m_evfd_rd, m_ctx, ep2, inp_hdr.data(), inp_hdr.size());
    if ( io_res == USB_STATUS::USB_STATUS_READY ) {
        LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_WAIT);
    } else {
        err("EP2:  Failed enqueue_read; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_FAILED);
    }
}

void epinit::handle_hdr_wait() {

    info("EP2:  Wait for usb frame header; (%s):(%d)", __FUNCTION__, __LINE__);

    // io_prep_pread -> io_submit -> select
    io_res = io_wait(m_nEPRange, m_evfd_rd, m_fdset_read, m_ctx, USB_READ_TIMEOUT_MS);

    if ( io_res == USB_STATUS::USB_STATUS_TIMEOUT ) {
        LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_WAIT);
    } else
    if ( io_res != USB_STATUS::USB_STATUS_READY ) {
        LOG_USB_STATE(usb_state_t::STATE_RX_PAYLOAD);
    } else {
        err("EP2:  Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__);
        LOG_USB_STATE(usb_state_t::STATE_FAILED);
    }

}

USB_STATUS epinit::handle_read_ok() {

}

USB_STATUS epinit::handle_read_bad() {

}

void epinit::handle_payload_read() {

    info("EP2:  Handle USB header; (%s):(%d)", __FUNCTION__, __LINE__);

    bool is_header_valid;
    bool is_allocated;

    hid::stream::params_t inp_params;
    hid::stream::params_t out_params;

    is_header_valid = hid::stream::Prefix::GetParams(m_ctx.inp_hdr, inp_params);
    if ( ! is_header_valid ) {
        // Wrong content of usb header. 
        // Transaction has to be ignored; 
        // Do not send response try to receive the next frame and process it.
        err ("Error: Invalid header received; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_PLACE);
    } else {

        // There's a chance that there will be no memory.
        is_allocated = true;
        try {
            inp_pay.resize(m_ctx.params.len);
            cleanup(m_ctx.inp_pay);
        } catch ( ... ) {
            is_allocated = false;
            err("Error: Failed to allocate buffer with %d bytes; (%s):(%d)", params.len, __FUNCTION__, __LINE__);
        }

        // It is expected functions (handle_read_ok and handle_read_bad) will change usb_state with:
        //  -> STATE_HANDLE_REQUEST Payload successfully read and needs to be processed.
        //  -> STATE_SEND_RESPONSE  Ignore handling and send the response.
        //  -> STATE_FAILED         Critical error. Cannot recover.
        if ( is_allocated ) {
            handle_read_ok ();
        } else {
            handle_read_bad();
        }
    }

}

void epinit::handle_command() {

}

void epinit::handle_send_resp() {

}

void epinit::handle_restart() {

}

void epinit::ep1_ep2_thread ( int ep1, int ep2 ) {

    USB_STATUS          io_res;


    uint32_t            io_wait_time;

    usb_state_monitor   usb_state;

    info("ep1_ep2_thread started.");

    LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);

    for ( ; ; ) {

        if ( m_stop_request ) {
            // External request to shutdown.
            info("EP2:  STOP request received; (%s):(%d)", __FUNCTION__, __LINE__);
            break;
        }

        if ( m_sync_request ) {
            m_sync_request = false;
            // External request to synchronize read process. 
            info ("EP2:  SYNC request received; (%s):(%d)", __FUNCTION__, __LINE__);
            LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);            
        }

        if ( ! m_ep0_active ) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        switch ( usb_state.get() ) {

            case usb_state_t::STATE_INITIALIZE:
                // Opens handles, i/o context, etc.
                //  ->  STATE_PHANTOM_READ      Try to read data buffered before.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_initialize();
                break;

            case usb_state_t::STATE_PHANTOM_READ:
                // Cleanup buffers. Read until timeout.
                //  ->  STATE_RX_HEADER_PLACE   Transaction handling.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_phantom_read();
                break;

            case usb_state_t::STATE_RX_HEADER_PLACE:
                // Place the read request "HEADER".
                //  ->  STATE_RX_HEADER_WAIT    Wait for the USB header.
                //  ->  STATE_FAILED            Critical error. Cannot continue.
                handle_hdr_place ();
                break;

            case usb_state_t::STATE_RX_HEADER_WAIT:
                // Wait for the completion of the read request "HEADER".
                //  ->  STATE_RX_PAYLOAD        Read payload
                //  ->  STATE_RX_HEADER_WAIT    Continue waiting 
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_hdr_wait();
                break;

            case usb_state_t::STATE_RX_PAYLOAD:
                // Allocate buffer and read payload.
                // Three options:
                //  ->  STATE_HANDLE_REQUEST    Header is valid & Payload successfully read.
                //  ->  STATE_TX_HEADER_PLACE   Header is valid but payload is too big for us. Return error code.
                //  ->  STATE_RESTART           Unable to read payload. Seems remote side stuck.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_payload_read ();
                break;

            case usb_state_t::STATE_HANDLE_REQUEST:
                // Handle request.
                //  ->  STATE_TX_HEADER_PLACE   Request processed.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_command();
                break;


            case usb_state_t::STATE_TX_RESPONSE:
                // Send Header and Payload both.
                //  ->  STATE_RX_HEADER_PLACE   Successfully sent.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_tx_resp();
                break;

            case usb_state_t::STATE_RESTART:
                // Re-open handles; Async contexts, etc after of failure.
                //  ->  STATE_INITIALIZE        Try to re-open handles.
                //  ->  STATE_FAILED            Critical errors. Cannot continue.
                handle_restart();
                break;

            case usb_state_t::STATE_FAILED:
                break;

            default:
                break;
        }


RX_HEADER_START:
        {   // An attempt to place the READ REQUEST.
            // Failure not expected (means the resource leak or general error).

            info("EP2:  USB Enqueue HDR read request; (%s):(%d)", __FUNCTION__, __LINE__);

            inp_hdr.resize( hid::stream::Prefix::PrefixSize() );
        
            io_res = enqueue_read ( &m_iocb_in, m_evfd_rd, m_ctx, ep2, inp_hdr.data(), inp_hdr.size() );
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                err( "EP2:  Failed to enqueue read request. Exit from: (%s):(%d)", __FUNCTION__, __LINE__ );
                break;
            }
        }

RX_HEADER_WAIT:
        {   // Wait for the header.

            info("EP2:  Wait for USB Header; (%s):(%d)", __FUNCTION__, __LINE__);

            io_res = io_wait (m_nEPRange, m_evfd_rd, m_fdset_read, m_ctx, USB_READ_TIMEOUT_MS );
            if ( m_stop_request ) {
                // External request to shutdown.
                info("EP2:  Stop request received. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
            if ( m_sync_request ) {
                // External request to synchronize read process. 
                info ("EP2:  SYNC request received; Sync from RX_HEADER_WAIT; (%s):(%d)", __FUNCTION__, __LINE__);
                goto PROCESS_SYNC_REQUEST;
            }
            if ( io_res == USB_STATUS::USB_STATUS_TIMEOUT ) {
                // Timeout to be ignored here.
                goto RX_HEADER_WAIT;
            }
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                // Failed! Bus error? Let's try to re-initialize.
                err ("EP2:  Failed io_wait; Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

        {   // Load parameters and validate them.
            info("EP2:  USB Header arrived; (%s):(%d)", __FUNCTION__, __LINE__);

            hid::stream::params_t params;
            hid::stream::Prefix::GetParams(inp_hdr, params);
            inp_pay.resize( params.len );
        }

        {   // Input request with no payload. Just pass request to the handler.
            if ( inp_pay.size() == 0) {
                goto PROCESS_REQUEST;
            }
        }

        {   // Place request to read payload.
            info("EP2:  USB Enqueue PAYLOAD read request; Size: %d; (%s):(%d)", (int)(inp_pay.size()), __FUNCTION__, __LINE__);
            io_wait_time = 0;
            io_res = enqueue_read(&m_iocb_in, m_evfd_rd, m_ctx, ep2, inp_pay.data(), inp_pay.size());
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                err("EP2:  Failed enqueue_read; Exit from (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

RX_PAYLOAD_WAIT:
        {   // Wait for payload.
            info("EP2:  Wait for Payload; (%s):(%d)", __FUNCTION__, __LINE__);

            io_res = io_wait(m_nEPRange, m_evfd_rd, m_fdset_read, m_ctx, USB_READ_TIMEOUT_MS );
            if ( m_stop_request ) {
                // Shutdown.
                info("EP2:  Stop request received. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
            if ( m_sync_request ) {
                // External request to synchronize read process. 
                info("EP2:  SYNC request received.");
                goto PROCESS_SYNC_REQUEST;
            }
            if ( io_res == USB_STATUS::USB_STATUS_TIMEOUT ) {
                // Controlled timeout.
                io_wait_time += USB_READ_TIMEOUT_MS;
                info("EP2:  Payload timeout. Timeout: %d ms; (%s):(%d)", (int)(USB_IO_TIMEOUT_MS-io_wait_time), __FUNCTION__, __LINE__);
                if ( io_wait_time > USB_IO_TIMEOUT_MS ) {
                    // Extra long timeout. There's no reason to wait more.
                    err ("EP2:  Timeout while read payload. Restart from: (%s):(%d)", __FUNCTION__, __LINE__);
                    goto PROCESS_SYNC_REQUEST;
                }
                // Seems just a large data transfer is in progress. Wait for the finish.
                goto RX_PAYLOAD_WAIT;
            }
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                // Failed! Bus error? Let's try to re-initialize.
                err("EP2:  Failed io_wait; (%s):(%d)", __FUNCTION__, __LINE__);
                goto PROCESS_SYNC_REQUEST;
            }
        }

PROCESS_REQUEST:
        {   // Handle incoming frame.
            info("EP2:  Process request; (%s):(%d)", __FUNCTION__, __LINE__);
            handle_request(inp_hdr, inp_pay, out_pay, status_code);
            info("EP2:  Response len: %d; Status code: %d; (%s):(%d)", (int)out_pay.size(), status_code, __FUNCTION__, __LINE__);
        }

        {   // Process data frame.
            info("EP2:  Formating Response Header; (%s):(%d)", __FUNCTION__, __LINE__);
            hid::stream::params_t params;
            params.command = hid::stream::cmd_t::STREAM_CMD_RESPONSE;
            params.code    = status_code;
            params.len     = static_cast<uint32_t> (out_pay.size());
            hid::stream::Prefix::SetParams(params, out_hdr);
        }

        {   // Place the request to send header
            info("EP1:  Enqueue TX request Header; Len:%d; (%s):(%d)", (int)(out_hdr.size()), __FUNCTION__, __LINE__);
            io_wait_time = 0;
            io_res = enqueue_write( &m_iocb_out, m_evfd_wr, m_ctx, ep1, out_hdr.data(), out_hdr.size() );
            if (io_res != USB_STATUS::USB_STATUS_READY) {
                err("EP1:  Failed to enqueue write request. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

TX_HEADER_WAIT:
        {   // Wait for the host to read header
            info("EP1:  Wait for Header TX; (%s):(%d)", __FUNCTION__, __LINE__);
            io_res = io_wait(m_nEPRange, m_evfd_wr, m_fdset_write, m_ctx, USB_READ_TIMEOUT_MS);
            if ( io_res == USB_STATUS::USB_STATUS_TIMEOUT ) {
                // Controlled timeout.
                io_wait_time += USB_READ_TIMEOUT_MS;
                if (io_wait_time > USB_IO_TIMEOUT_MS) {
                    // Extra long timeout. There's no reason to wait more.
                    err("EP1:  Timeout while sending header. Restart from: (%s):(%d)", __FUNCTION__, __LINE__);
                    goto PROCESS_SYNC_REQUEST;
                }
                // Seems just a large data transfer is in progress. Wait for the finish.
                goto TX_HEADER_WAIT;
            }
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                // Failed! Bus error? Let's try to re-initialize.
                err("EP1:  Failed io_wait; Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

        {   // Place the request to send payload
            info("EP1:  Enqueue TX request Payload; Len: %d; (%s):(%d)", (int)(out_pay.size()), __FUNCTION__, __LINE__);
            io_wait_time = 0;
            io_res = enqueue_write(&m_iocb_out, m_evfd_wr, m_ctx, ep1, out_pay.data(), out_pay.size());
            if (io_res != USB_STATUS::USB_STATUS_READY) {
                err("EP2:  Failed to enqueue payload write request. Exit from: (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

TX_PAYLOAD_WAIT:
        {   // 
            info("EP1:  Wait for Payload TX; (%s):(%d)", __FUNCTION__, __LINE__);
            io_res = io_wait(m_nEPRange, m_evfd_wr, m_fdset_write, m_ctx, USB_READ_TIMEOUT_MS);
            if ( io_res == USB_STATUS::USB_STATUS_TIMEOUT ) {
                // Controlled timeout.
                io_wait_time += USB_READ_TIMEOUT_MS;
                info("EP1:  Payload timeout. Timeout: %d ms; (%s):(%d)", (int)(USB_IO_TIMEOUT_MS-io_wait_time), __FUNCTION__, __LINE__);
                if (io_wait_time > USB_IO_TIMEOUT_MS) {
                    // Extra long timeout. There's no reason to wait more.
                    err("EP1:  Timeout while payload sending. Restart from: (%s):(%d)", __FUNCTION__, __LINE__);
                    goto PROCESS_SYNC_REQUEST;
                }
                // Seems just a large data transfer is in progress. Wait for the finish.
                goto TX_PAYLOAD_WAIT;
            }
            if ( io_res != USB_STATUS::USB_STATUS_READY ) {
                info("EP1:  Failed io_wait. Exit from (%s):(%d)", __FUNCTION__, __LINE__);
                break;
            }
        }

        {   // Request processed.
            info("EP1:  Request processed; (%s):(%d)", __FUNCTION__, __LINE__);
            goto RX_HEADER_START;
        }

PROCESS_SYNC_REQUEST:
        {   // Cancel IO requests.
            info("EP1:  Request to synchronize; (%s):(%d)", __FUNCTION__, __LINE__);
            m_sync_request = false;
            info("EP1:  Destroy I/O context; Cancel all pending requests; (%s):(%d)", __FUNCTION__, __LINE__);
            io_destroy(m_ctx);
            goto QUEUE_INITIALIZE;
        }
    }

    err("EP2:  Destroy I/O context; (%s):(%d)", __FUNCTION__, __LINE__);
    io_destroy(m_ctx);

    err("EP2:  Close m_evfd_rd; (%s):(%d)", __FUNCTION__, __LINE__);
    close(m_evfd_rd);

    err("EP1:  Close m_evfd_wr; (%s):(%d)", __FUNCTION__, __LINE__);
    close(m_evfd_wr);

    err("EP1:  Notify stop; (%s):(%d)", __FUNCTION__, __LINE__);
    m_stop_event.notify_one();

}
