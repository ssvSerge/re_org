#ifndef __hid_transport_usb_transport_device_h__
#define __hid_transport_usb_transport_device_h__

// #include <stdint.h>
// #include <string>
// #include <thread>
// #include <vector>
#include <atomic>
// #include <condition_variable>
// #include <multitask.h>
// #include <libaio.h>

namespace hid {

namespace transport {

enum class usb_transport_err_t {
    USB_STATUS_UNKNOWN,
    USB_STATUS_READY,
    USB_STATUS_TIMEOUT,
    USB_STATUS_FAILED
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
        int32_t init_ep ( int32_t nEpNo );

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
        usb_transport_err_t enqueue_write ( iocb* iocb_write, int event_fd, io_context_t ctx, int endpoint, void* data, size_t num_bytes );

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

        /// @brief 
        /// 
        /// Handle requests from client.
        /// 
        /// \param in_hdr      vector<uint8_t> input request header.
        /// \param in_frame    vector<uint8_t> input request payload.
        /// \param out_frame   vector<uint8_t> output payload.
        /// \param err_code    return code from the client.
        /// 
        /// \returns       USB_STATUS_FAILED in case of error.
        ///                USB_STATUS_READY  in success case.
        /// 
        usb_transport_err_t handle_request ( const usb_frame_t& in_hdr, const usb_frame_t& in_frame, usb_frame_t& out_frame, int32_t& err_code );

    private:
        void handle_setup ( const struct usb_ctrlrequest* setup );

        void handle_initialize   ();
        void handle_phantom_read ();
        void handle_hdr_place    ();
        void handle_hdr_wait     ();

        void handle_payload_read ();
        bool handle_read_ok      ();
        bool handle_read_bad     ();

        void handle_command      ();
        void handle_tx_resp      ();
        void handle_restart      ();



        #if 0

        /// @brief Configure USB interface.
        /// 
        /// Opens the RD/WR handles, asynchronous I/O context, asynchronous notify events.
        /// 
        /// Output USB states:
        ///     STATE_FAILED              -> Failed to open / initialize OS objects. Seems application should go down.
        ///     STATE_READ_ON_INITIALIZE  -> Cleanup (perform reading until timeout) from endpoint.
        void handle_initialize();

        /// @brief Reads data from endpoint until timeout.
        /// 
        /// There's a chance that data are buffered and needs to clean-up after of restarts.
        /// Use the dummy read until timeout. 
        /// 
        /// Output USB states:
        ///     STATE_FAILED              -> Failed to read/write. Critical errors. Seems application should go down.
        ///     STATE_RX_HEADER_PLACE     -> Request to read the USB prefix.
        void handle_read_after_init();

        /// @brief Waits for USB header.
        /// 
        /// Place the Read Request and waits until completion. 
        /// Timeouts are expected.
        /// 
        /// Output USB states:
        ///     STATE_FAILED              -> Failed to put the Read request or read result. Critical errors. Seems application should go down.
        ///     STATE_RX_HEADER_VALIDATE  -> USB frame successfully received and has to be validating.
        void handle_hdr_wait();


        /// @brief Checks & validate the content of USB header.
        /// 
        /// Validation of Checksum, length and other parameters in the prefix received at handle_hdr_wait() call.
        /// 
        /// Output USB states:
        ///     STATE_RX_PAYLOAD_OK_PLACE  -> Header is valid. Length of payload is in the acceptable range. Place the read request.
        ///     STATE_RX_PAYLOAD_BAD_PLACE -> Header is valid but length of payload is out of range. Perform the dummy reading.
        ///     STATE_RX_HEADER_PLACE      -> Header is not valid. Let's just wait for the valid header.
        void handle_hdr_validate();

        /// @brief Loads the payload.
        /// 
        /// Place the read request for Payload and waits for the completion of data transfer.
        /// 
        /// Output USB states:
        ///     STATE_HANDLE_REQUEST       -> Payload successfully received. Both HDR and PAYLOAD are received both.
        ///     STATE_RX_HEADER_PLACE      -> Timeout while read PAYLOAD. Ignore current transaction and start the new one.
        ///     STATE_FAILED               -> Internal errors (read/write errors). Critical error. Seems application must go down.
        void handle_payload_ok();

        /// @brief read extra long payload. 
        /// 
        /// Payload is too big and cannot be processed.
        /// Perform the reading until condition: a) length of payload b) timeout.
        /// Content has to be ignored.
        /// 
        /// Output USB states:
        /// 
        void handle_payload_bad();

        /// 
        /// 
        /// 
        void handle_command();

        /// 
        /// 
        /// 
        void handle_tx_hdr();

        /// 
        /// 
        /// 
        void handle_tx_payload();

        #endif

    private:
     // usb_state_monitor           m_state_monitor;
     // transaction_ctx_t           m_ctx;

        std::atomic_bool            m_stop_request = false;     // external request to shutdown.
        std::atomic_bool            m_ep0_active   = false;     // true when ep0 is able to operate.


        std::atomic_bool            m_sync_request  = false;
        utils::thread_event         m_stop_event;            // m_terminate_event
        const std::string           m_udc_dir       = "/sys/class/udc";
        const std::string           m_gadget_dir    = "/home/root/ffs";
        std::string                 m_ep_directory;
        int32_t                     m_fs_eps[3];
        std::thread                 m_tid_ep0;
        std::thread                 m_tid_com;
        std::vector<uint8_t>        m_ep0Buffer;
        utils::thread_event         m_error_event;
        io_context_t                m_ctx;
        int                         m_evfd_rd   = -1;
        int                         m_evfd_wr   = -1;
        int                         m_nEPRange;
        fd_set                      m_fdset_read;
        fd_set                      m_fdset_write;
        struct iocb                 m_iocb_in;
        struct iocb                 m_iocb_out;
};