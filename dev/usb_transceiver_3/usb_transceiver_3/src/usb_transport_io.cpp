#include <unistd.h>
#include <sys/time.h>

#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

using namespace std::chrono_literals;


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

constexpr uint32_t IO_CANCEL_TIMEOUT_MS  = 10 * 1000; 

//---------------------------------------------------------------------------//
    
usb_transport_err_t usb_transport_device_t::read_event ( int evfd, fd_set& rfds, io_context_t ctx ) {

    usb_transport_err_t ret_val = usb_transport_err_t::USB_STATUS_READY;

    if ( FD_ISSET(evfd, &rfds) ) {

        struct io_event     e[2];
        uint64_t            ev_cnt;

        // Asynchronous Read from USB endpoint.

        // attempts to read up to count bytes from file descriptor
        // into the buffer starting at buffer.
        int ret = read ( evfd, &ev_cnt, sizeof(ev_cnt) );
        if ( ret < 0 ) {

            err ("Error: Failed read; (%s):(%d)", __FUNCTION__, __LINE__);
            ret_val = usb_transport_err_t::USB_STATUS_FAILED;

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
                    ret_val = usb_transport_err_t::USB_STATUS_FAILED;
                }

                break;
            }
        }
    }

    return ret_val;
}

usb_transport_err_t usb_transport_device_t::enqueue_read ( iocb* iocb_read, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes ) {

    usb_transport_err_t ret_val = usb_transport_err_t::USB_STATUS_READY;

    io_prep_pread ( iocb_read, endpoint, data, num_bytes, 0 );

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
        ret_val = usb_transport_err_t::USB_STATUS_FAILED;
    }

    return ret_val;
}

usb_transport_err_t usb_transport_device_t::enqueue_write ( iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, const void* data, size_t num_bytes ) {

    usb_transport_err_t ret_val = usb_transport_err_t::USB_STATUS_READY;

    io_prep_pwrite ( iocb_write, endpoint, const_cast<void*> (data), num_bytes, 0 );

    iocb_write->u.c.flags |= IOCB_FLAG_RESFD;
    iocb_write->u.c.resfd  = event_fd;

    // On success, io_submit() returns the number of iocbs submitted
    // which may be less than nr, or 0 if nr is zero. 
    // 
    // on error it returns a negated error number.
    //     EAGAIN, EBADF, EFAULT, EINVAL, ENOSYS, EPERM
    // 
    // We're on the asynchronous I/O. EAGAIN is not expected here.
    int ret = io_submit(ctx, 1, &iocb_write);

    if (ret < 0) {
        err("Error: Failed io_submit. (%s):(%d)", __FUNCTION__, __LINE__);
        ret_val = usb_transport_err_t::USB_STATUS_FAILED;
    }

    return ret_val;
}

usb_transport_err_t usb_transport_device_t::io_wait ( int eprange, int evfd, fd_set& rfds, io_context_t ctx, int timeout_ms ) {

    usb_transport_err_t ret_val = usb_transport_err_t::USB_STATUS_FAILED;
    int io_res;

    for ( ; ; ) {

        struct timeval timeout;

        debug ("io_wait sleep for %d ms; (%s):(%d)", timeout_ms, __FUNCTION__, __LINE__ );

        timeout.tv_sec  = (timeout_ms / 1000);
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        FD_ZERO ( &rfds );
        FD_SET  ( evfd, &rfds );

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
        io_res = select(eprange, &rfds, NULL, NULL, &timeout);

        if ( io_res < 0 ) {
            if (errno == EINTR) {
                continue;
            }
        }

        break;

    }

    if ( io_res < 0) {
        err ( "Failed on select; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = usb_transport_err_t::USB_STATUS_FAILED;
    } else
    if ( io_res == 0 ) {
        debug ( "Timeout on select; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = usb_transport_err_t::USB_STATUS_TIMEOUT;
    } else {
        debug ( "Data ready; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = read_event ( evfd, rfds, ctx );
    }

    return ret_val;
}

usb_transport_err_t usb_transport_device_t::io_cancel () {

    usb_transport_err_t ret_val = usb_transport_err_t::USB_STATUS_UNKNOWN;

    checkpoint_t    start_time      =  time_source_t::now();
    checkpoint_t    time_out        =  start_time + duration_ms_t(IO_CANCEL_TIMEOUT_MS);
    uint32_t        wait_time_ms    =  0;
    int             cancel_res      =  0;

    debug ( "io_cancel() start; (%s):(%d)", __FUNCTION__, __LINE__ );

    for ( ; ; ) {

        get_timeout_ms(time_out, wait_time_ms);

        if ( wait_time_ms <= 10 ) {
            cancel_res = 0;
            break;
        }

        // The io_cancel() attempts to cancel an asynchronous I/O operation 
        // previously submitted with io_submit. The iocb argument describes 
        // the operation to be canceled and the ctx_id argument is the AIO 
        // context to which the operation was submitted. If the operation is 
        // successfully canceled, the event will be copied into the memory 
        // pointed to by result without being placed into the completion queue.
        //
        // On success, io_cancel() returns 0.
        //  EAGAIN  ->  retry required
        //  EFAULT  ->  One of the data structures points to invalid data.
        //  EINVAL  ->  The AIO context specified by ctx_id is invalid.
        // 
        cancel_res = ::io_cancel ( m_io_ctx, &m_iocb_in, NULL );

        if (cancel_res < 0 ) {
            if ( errno == EAGAIN ) {
                std::this_thread::sleep_for ( 10ms );
                continue;
            }
        }

        break;
    }

    if ( cancel_res < 0 ) {
        debug ( "io_cancel() failed; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = usb_transport_err_t::USB_STATUS_FAILED;
    } else
    if ( cancel_res == 0 ) {
        debug ( "io_cancel() timeout; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = usb_transport_err_t::USB_STATUS_TIMEOUT;
    } else {
        debug ( "io_cancel() success; (%s):(%d)", __FUNCTION__, __LINE__ );
        ret_val = usb_transport_err_t::USB_STATUS_READY;
    }

    return ret_val;
}

//---------------------------------------------------------------------------//

}
}
