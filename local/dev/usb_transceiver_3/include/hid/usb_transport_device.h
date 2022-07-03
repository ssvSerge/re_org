#ifndef __hid_transport_usb_transport_device_h__
#define __hid_transport_usb_transport_device_h__

#include <sys/select.h>
#include <signal.h>
#include <atomic>
#include <thread>

#include <multitask.h>
#include <libaio.h>

#include <hid/HidTypes.h>
#include <hid/StreamPrefix.h>

namespace hid {
namespace transport {


using   time_source_t  =  std::chrono::steady_clock;
using   duration_ns_t  =  std::chrono::nanoseconds;
using   duration_us_t  =  std::chrono::microseconds;
using   duration_ms_t  =  std::chrono::milliseconds;
using   checkpoint_t   =  std::chrono::time_point<time_source_t>;
using   usb_frame_t    =  hid::types::storage_t;


constexpr uint32_t  USB_TRANSPORT_ERR_NO_MEMORY = 100;


enum class usb_transport_err_t {
    USB_STATUS_UNKNOWN,                 ///<
    USB_STATUS_READY,                   ///<
    USB_STATUS_TIMEOUT,                 ///<
    USB_STATUS_FAILED                   ///<
};

enum class usb_state_t {
    STATE_INITIALIZE               = 0, ///< Initialize endpoint at start/restart time.
    STATE_SPINUP                   = 1, ///< Event "ep0 down" received. Reset context and return to the state "INIT".
    STATE_PHANTOM_READ             = 2, ///< Attempt to read from endpoint until short timeout. 
    STATE_RX_HEADER_REQUEST        = 3, ///< Put the READ request for USB Header.    
    STATE_RX_HEADER_WAIT           = 4, ///< Wait for the header. Timeout is a valid state.    
    STATE_RX_PAYLOAD               = 5, ///< Read payload.
    STATE_HANDLE_REQUEST           = 6, ///< HEADER and PYALOAD are successfully read. Send it to J-Engine.
    STATE_TX_RESPONSE              = 7, ///< Place the TX request for HEADER.
    STATE_FAILED                   = 8, ///< Critical error. Application should go down.
};

class transaction_ctx_t {
    public:

        hid::types::storage_t       inp_hdr;  ///< Header received from host. Required to pick the length of payload; Command and params.
        hid::stream::params_t       inp_prm;  ///< Parameters received from host (extracted from inp_hdr)
        hid::types::storage_t       inp_pay;  ///< Payload received from host (length declared in inp_hdr).

        hid::stream::params_t       out_prm;  ///< Output parameters required to configure out_hdr. Taken from handle_command call. 
        hid::types::storage_t       out_pay;  ///< Output payload *optional, might be empty*. Taken from handle_command call.
        hid::types::storage_t       out_hdr;  ///< Output header required to send from device to host. Configured with out_prm.
};

class usb_transport_device_t {

    public:
        /// Default constructor.
        usb_transport_device_t();

    public:
        /// @brief Initialize FFS.
        /// 
        /// Initialize FFS with the descriptors defined in epconst.c 
        /// Failure is a critical state. Application must be closed.
        /// Path specified by epDirectory.
        /// Expected value is: /sys/kernel/config/usb_gadget/hidcam
        /// And could be modified via command line.
        /// Four files will be created in the path:
        ///     /sys/kernel/config/usb_gadget/hidcam/ep0
        ///     /sys/kernel/config/usb_gadget/hidcam/ep1
        ///     /sys/kernel/config/usb_gadget/hidcam/ep2
        ///     /sys/kernel/config/usb_gadget/hidcam/ep3
        /// Descriptors and Strings (see epconst.c file) will be stored in ep0. 
        /// Path epDirectory will be stored in the m_ep_directory variable.
        /// 
        /// \param      epDirectory     path in FFS where to create endpoints. 
        /// \returns    (-1) on error; (0) on success;
        /// 
        int32_t ep0_init ( std::string epDirectory );

        /// @brief Start threads to handle EP0, EP1 and EP2.
        /// 
        /// Starts the background threads (ep0_thread and ep1_ep2_thread).
        /// Failure is a critical state. Application must be closed.
        /// 
        /// \returns    (-1) on error; (0) on success;
        /// 
        int32_t init_ep_threads ();

        /// @brief External request to shutdown.
        /// 
        /// Request is asynchronous (returns without of waiting of the real stop). 
        /// 
        void    stop();

        /// @brief Waits for the termination of application.
        /// 
        /// Waits for the termination of ep0_thread and ep1_ep2_thread both.
        /// Reasons for termination:
        ///   Internal errors
        ///   External signal to shutdown (received via command "stop").
        /// 
        void    join ();

    private:
        /// @brief Creates endpoint (file) in FFS.
        /// 
        /// Just creates new file with the name "ep0", "ep1", etc in the 
        /// path defined in m_epDirectory (see ep0_init).
        ///  
        int32_t open_ep ( int32_t endpoint_no );

        /// @brief Thread. Receive and handle events from EP0 endpoint.
        /// 
        /// Thread pulls the EP0 events.
        /// Thread checks the m_stop_request variable and close if it's set.
        /// 
        void    ep0_thread ();

        /// @brief Thread. Service for EP1 and EP2.
        /// 
        /// Thread checks the m_stop_request and close if it's set.
        /// Function calls the "handle_request" to handle requests.
        /// 
        /// \param ep1     file descriptor for EP1
        /// \param ep2     file descriptor for EP2
        /// 
        void    ep1_ep2_thread (int ep1, int ep2);

    private:

        /// @brief Pull asynchronous file system events.
        /// 
        /// Place the event "read" and waits with "io_getevents".
        /// 
        /// \param evfd    file descriptor for "read" command.
        /// \param rfds    fd_set prepared before to wait on it.
        /// \param ctx     asynchronous context required to wait for.
        /// 
        /// \returns       USB_STATUS_FAILED in case of error.
        ///                USB_STATUS_READY  in success case.
        /// 
        usb_transport_err_t read_event ( int evfd, fd_set& rfds, io_context_t ctx );

        /// @brief Puts the read request to the FFS.
        /// 
        /// Function calls the io_prep_pread and io_submit requests.
        /// 
        /// \param iocb_read   i/o control block to prepare read request.
        /// \param event_fd    eventfd object created by eventfd.
        /// \param ctx         asynchronous context for the read request.
        /// \param endpoint    file descriptor (EndPoint) to read.
        /// \param data        pointer to the destination buffer.
        /// \param num_bytes   number of bytes to read.
        /// 
        /// \returns       USB_STATUS_FAILED in case of error.
        ///                USB_STATUS_READY  in success case.
        /// 
        usb_transport_err_t enqueue_read ( iocb* iocb_read, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes );

        /// @brief Puts the write request to the FFS.
        /// 
        /// Function calls the io_prep_pwrite and io_submit requests.
        /// 
        /// \param iocb_read   i/o control block to prepare write request.
        /// \param event_fd    eventfd object.
        /// \param ctx         asynchronous context for the read request.
        /// \param endpoint    file descriptor (EndPoint) to read.
        /// \param data        pointer to the destination buffer.
        /// \param num_bytes   number of bytes to read.
        /// 
        /// \returns       USB_STATUS_FAILED in case of error.
        ///                USB_STATUS_READY  in success case.
        /// 
        usb_transport_err_t enqueue_write ( iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, const void* data, size_t num_bytes );

        /// @brief Waits for the active i/o request.
        /// 
        /// 
        /// 
        /// \param eprange     range of file descriptors required by the select call.
        /// \param evfd        eventfd object.
        /// \param rfds        fd_set object required by the select call.
        /// \param ctx         asynchronous context.
        /// \param timeout_ms  wait time, in milliseconds.
        /// 
        /// \returns       USB_STATUS_FAILED   in case of error.
        ///                USB_STATUS_TIMEOUT  in case of timeout.
        ///                USB_STATUS_READY    in success case.
        /// 
        usb_transport_err_t io_wait ( int eprange, int evfd, fd_set& rfds, io_context_t ctx, int timeout_ms );

        usb_transport_err_t io_cancel ( struct iocb* iocb_ptr );

    private:
        void log_event ( const struct usb_ctrlrequest* setup );

        void handle_initialize   ();
        void handle_spinup       ();
        void handle_phantom_read ();
        void handle_hdr_place    ();
        void handle_hdr_wait     ();
        void handle_payload_read ();
        void handle_read_ok      ( const checkpoint_t time_out );
        void handle_read_bad     ( const checkpoint_t time_out );
        void handle_command      ();
        void handle_tx_resp      ();
        bool read_frame          ( const checkpoint_t  time_out, uint8_t* const dst, const size_t len, usb_state_t next_state );
        bool write_frame         ( const uint8_t* const src, const size_t len, usb_state_t next_state );

    private:
        bool get_timeout_ms      ( const checkpoint_t point_timeout, uint32_t& ms_cnt );


    private:
        utils::thread_event         m_thread_fininsed;          ///< at least one thread stopped.
        std::atomic_bool            m_stop_request  = false;    ///< external request to shutdown.
        std::atomic_bool            m_ep0_active    = false;    ///< true when ep0 is able to operate.
        std::atomic_bool            m_sync_request  = false;    ///< Requested to synchronize received from host. 
        int32_t                     m_fs_eps[3]     = {};       ///< File descriptors to EndPoint 0, 1 and 2.
        bool                        m_io_ctx_valid  = false;    ///< TRUE if m_io_ctx is valid.
        io_context_t                m_io_ctx        = {};       ///< Async I/O context created by io_setup.
        int                         m_evfd_rd       = -1;       ///< Handle to event Read
        int                         m_evfd_wr       = -1;       ///< Handle to event Write
        int                         m_io_range      = -1;       ///< Just the range for "select" call.
        std::string                 m_ep_directory;             ///< FFS directory to create End Points.
        std::thread                 m_tid_ep0;                  ///< Thread to serve EP0.
        std::thread                 m_tid_com;                  ///< Thread to serve bulk transfer.
        fd_set                      m_fdset_read;               ///< FD_SET for read from EP2
        fd_set                      m_fdset_write;              ///< FD_SET for write to EP1
        struct iocb                 m_iocb_in;                  ///< IOCB for input
        struct iocb                 m_iocb_out;                 ///< IOCB for output
        transaction_ctx_t           m_ctx;                      ///< INPUT header+payload and OUTPUT header+payload
};


}
}
#endif

