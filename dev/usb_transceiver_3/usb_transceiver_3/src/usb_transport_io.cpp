#include <unistd.h>
#include <sys/time.h>
#include <sys/eventfd.h>

#include <thread>
#include <chrono>
#include <algorithm>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

using namespace std::chrono_literals;


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

constexpr uint32_t IO_CANCEL_TIMEOUT_MS  = 10 * 1000; 

//---------------------------------------------------------------------------//

err_t usb_transport_device_t::io_process ( struct iocb& io_request, int fd, uint32_t timeout_ms ) {

    struct iocb*    iocbs       = &io_request;
    struct timeval  timeout     = {};
    fd_set          rfds        = {};
    uint64_t        ev_cnt      = 0;
    struct io_event io_evt[2]   = {};
    int             io_res;
    int             io_range;
    uint32_t        sleep_time_ms;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    io_range = std::max (  (m_evfd_rx+1),  (m_evfd_tx+1)  );

    // io_submit - submit asynchronous I/O blocks for processing
    // 
    //   The io_submit() system call queues NR I/O request blocks for processing 
    //   in the AIO context ctx_id. The IOCBpp argument should be an array of NR 
    //   AIO control blocks, which will be submitted to context ctx_id.
    // 
    // Notes:
    //   LIBAIO does not follow the usual C library conventions for indicating errors.
    //   on error it returns a negated error number. 
    //   If the system call is invoked via SYSCALL, then the return value follows the 
    //   usual conventions for indicating an error: -1, with ERRNO set to a (positive) 
    //   value that indicates the error.
    // 
    //   EAGAIN  Insufficient resources are available to queue any IOCBs.
    //   EBADF   The file descriptor specified in the first IOCB is invalid.
    //   EFAULT  One of the data structures points to invalid data.
    //   EINVAL  The AIO context specified by ctx_id is invalid. 
    //   ENOSYS  io_submit() is not implemented on this architecture.
    //   EPERM   Submitting context does not have the CAP_SYS_ADMIN capability.
    // 
    //   EAGAIN seems the critical state of the system. There's no resources in the system.
    io_res = io_submit ( m_io_ctx, 1, &iocbs );
    if ( io_res < 0 ) {
        err ("Error: Failed io_submit with code: %d; text: %s; (%s):(%d)", io_res, strerror(io_res), __FUNCTION__, __LINE__ );
        // Logic error or insufficient resources in the OS.
        return err_t::USB_STATUS_FAILED;
    }

    sleep_time_ms = 0;
    for ( ; ; ) {

        FD_ZERO ( &rfds );
        FD_SET  ( fd, &rfds );

        // 5 times per second.
        timeout.tv_sec  = 0;          // seconds
        timeout.tv_usec = 100 * 1000; // microseconds

        // select - synchronous I/O multiplexing.
        // 
        // On success, select() return the number of file descriptors contained in 
        // the three returned descriptor sets. The return value may be zero if 
        // the timeout expired before any file descriptors became ready.
        // 
        // EBADF    An invalid file descriptor was given in one of the sets.
        // EINTR    A signal was caught.
        // EINVAL   NFDS is negative or exceeds the RLIMIT_NOFILE.
        // EINVAL   The value contained within timeout is invalid.
        // ENOMEM   Unable to allocate memory for internal tables.
        // 
        io_res = select ( io_range, &rfds, NULL, NULL, &timeout );

        if ( io_res == -1 ) {
            if ( errno == EINTR ) {
                continue;
            }
        }

        if ( io_res == 0 ) {
            sleep_time_ms += 100;
            if (  (timeout_ms>0)  &&  (sleep_time_ms>timeout_ms)  ) {
                break;
            }
        }

        if ( m_stop_request ) {
            // Simulate (or force) timeout because of external request to STOP.
            err ( "Stop request; (%s):(%d)", __FUNCTION__, __LINE__ );
            io_res = 0;
            break;
        }

        if ( m_ep0_inactive ) {
            // Simulate (or force) timeout because of external request to STOP.
            err ( "EP0 inactive; (%s):(%d)", __FUNCTION__, __LINE__ );
            io_res = 0;
            break;
        }

        break;
    }

    if ( io_res > 0 ) {

        // read - read from a file descriptor
        // 
        // On success, the number of bytes read is returned. 
        // Zero indicates end of file.
        // It is NOT an error if this number is smaller than the number of bytes requested; 
        // this may happen for example because fewer bytes are actually available right now
        // because read() was interrupted by a signal.
        // 
        // On error, -1 is returned, and errno is set to indicate the error.
        // 
        //   EAGAIN       The file descriptor has been marked nonblocking, and the read would block.
        //   EWOULDBLOCK  The same like EAGAIN.
        //   EBADF        FD is not a valid file descriptor or is not open for reading.
        //   EFAULT       BUF is outside your accessible address space.
        //   EINTR        The call was interrupted by a signal before any data was read;
        //   EINVAL       FD is attached to an object which is unsuitable for reading;
        //   EIO          I/O error.
        //   EISDIR       FD refers to a directory.
        // 
        // Reading only 64bit variable. There's no option for EINTR.
        // All other are real and critical errors.
        io_res = read ( fd, &ev_cnt, sizeof(ev_cnt) );
        if ( io_res < 0 ) {
            err ( "Error: Failed read with code: %d; str: %s; (%s):(%d)", errno, strerror(errno), __FUNCTION__, __LINE__);
            // Logic error or insufficient resources in the OS.
            return err_t::USB_STATUS_FAILED;
        }

        // io_getevents - read asynchronous I/O events from the completion queue
        //
        // The io_getevents() system call attempts to read at least min_nr 
        // events and up to NR events from the completion queue of the AIO 
        // context specified by ctx_id.
        // 
        // Notes:
        //   LIBAIO does not follow the usual C library conventions for indicating errors: 
        //   on error it returns a negated error number.
        // 
        //   EFAULT       Either events or timeout is an invalid pointer.
        //   EINTR        Interrupted by a signal handler.
        //   EINVAL       ctx_id is invalid.min_nr is out of range or NR is out of range.
        //   
        // There's a "select" operation in front of get_events.
        // That means data are prepared and pending on.
        // Seems there's no option for "EINTR".
        // All other errors are real and critical errors.
        io_res = io_getevents ( m_io_ctx, 1, 2, io_evt, NULL );
        if ( io_res < 0 ) {
            err ( "Error: Failed io_getevents with code: %d; text: %s; (%s):(%d)", io_res, strerror(io_res), __FUNCTION__, __LINE__ );
            // Logic error or insufficient resources in the OS.
            return err_t::USB_STATUS_FAILED;
        }

        debug ( "Frame processed; (%s):(%d)", __FUNCTION__, __LINE__ );

    } else {

        if ( io_res < 0 ) {
            err ( "Error: Failed select with code: %d; text: %s; (%s):(%d)", errno, strerror(errno), __FUNCTION__, __LINE__ );
        } else {
            debug ( "timeout with %d ms; (%s):(%d)", sleep_time_ms, __FUNCTION__, __LINE__ );
        }

        for ( ; ; ) {

            // io_cancel - cancel an outstanding asynchronous I/O operation.
            //
            //   The io_cancel() system call attempts to cancel an asynchronous I/O operation 
            //   previously submitted with io_submit. The IOCB describes the operation to be 
            //   canceled and the ctx_id argument is the AIO context to which the operation 
            //   was submitted. If the operation is successfully canceled, the event will be copied 
            //   into the memory pointed to by result without being placed into the completion queue.
            // 
            // Notes:
            //   LIBAIO does not follow the usual C library conventions for indicating errors.
            //   on error it returns a negated error number. 
            // 
            //   EAGAIN       The IOCB specified was not canceled.
            //   EFAULT       One of the data structures points to invalid data.
            //   EINVAL       The AIO context specified by ctx_id is invalid.
            //   ENOSYS       io_cancel() is not implemented on this architecture.
            //  
            io_res = io_cancel ( m_io_ctx, &io_request, io_evt );

            if ( io_res == EAGAIN ) {
                continue;
            }

            break;

        }

        if ( io_res != 0 ) {
            err ( "Error: Failed io_cancel with code: %d; text: %s; (%s):(%d)", io_res, strerror(errno), __FUNCTION__, __LINE__ );
            return err_t::USB_STATUS_FAILED;
        }

    }

    debug ( "Done; (%s):(%d)", __FUNCTION__, __LINE__ );

    return err_t::USB_STATUS_READY;
}

err_t usb_transport_device_t::rx_frame ( uint8_t* const dst_ptr, size_t dst_len, uint32_t timeout_ms ) {

    err_t ret_val;

    struct iocb iocb_in;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    io_prep_pread ( 
        &iocb_in, 
        m_ep2_rx, 
        const_cast<uint8_t*>  ( dst_ptr ),
        static_cast<uint32_t> ( dst_len ),
        0 
    );

    iocb_in.u.c.flags |= IOCB_FLAG_RESFD;
    iocb_in.u.c.resfd  = m_evfd_rx;

    ret_val = io_process ( iocb_in, m_evfd_rx, timeout_ms );

    debug ( "Done; (%s):(%d)", __FUNCTION__, __LINE__ );

    return ret_val;
}

err_t usb_transport_device_t::tx_frame ( const uint8_t* const src_ptr, size_t src_len, uint32_t timeout_ms ) {

    err_t ret_val;

    struct iocb iocb_out;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    io_prep_pwrite ( 
        &iocb_out, 
        m_ep1_tx, 
        const_cast<uint8_t*>  (src_ptr),
        static_cast<uint32_t> (src_len),
        0 
    );

    iocb_out.u.c.flags |= IOCB_FLAG_RESFD;
    iocb_out.u.c.resfd  = m_evfd_tx;

    ret_val = io_process ( iocb_out, m_ep1_tx, timeout_ms );

    debug ( "Done; (%s):(%d)", __FUNCTION__, __LINE__ );

    return ret_val;
}


}
}
