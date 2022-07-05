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


using   usb_frame_t    =  hid::types::storage_t;


constexpr uint32_t  USB_TRANSPORT_ERR_NO_MEMORY = 100;


enum class err_t {
    USB_STATUS_UNKNOWN,                 ///<
    USB_STATUS_READY,                   ///<
    USB_STATUS_TIMEOUT,                 ///<
    USB_STATUS_FAILED                   ///<
};

enum class usb_state_t {
    STATE_INITIALIZE               = 0, ///< Initialize endpoint at start/restart time.
    STATE_SPINUP                   = 1, ///< Event "ep0 down" received. Reset context and return to the state "INIT".
    STATE_PHANTOM_READ             = 2, ///< Attempt to read from endpoint until short timeout. 
    STATE_RX_HEADER                = 4, ///< Wait for the header. Timeout is a valid state.    
    STATE_RX_PAYLOAD               = 5, ///< Read payload.
    STATE_HANDLE_REQUEST           = 6, ///< HEADER and PYALOAD are successfully read. Send it to J-Engine.
    STATE_TX_RESPONSE              = 7, ///< Place the TX request for HEADER.
    STATE_FAILED                   = 8, ///< Critical error. Application should go down.
    STATE_SHUTDOWN                 = 9  ///< Request to shutdown;
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
        void    ep1_ep2_thread ();

    private:

        void cleanup ( usb_frame_t& frame );

        err_t io_process  ( struct iocb& io_request, int fd, uint32_t timeout_ms );
        err_t rx_frame    ( uint8_t* const dst_ptr, size_t dst_len, uint32_t timeout_ms );
        err_t tx_frame    ( const uint8_t* const src_ptr, size_t src_len, uint32_t timeout_ms );


        void log_event ( const struct usb_ctrlrequest* setup );

        void handle_initialize   ();
        void handle_spinup       ();
        void handle_phantom_read ();
        void handle_command      ();
        void handle_tx_resp      ();
        void handle_hdr_wait     ();
        void handle_payload_read ();

    private:
        utils::thread_event         m_thread_fininsed;          ///< at least one thread stopped.
        std::atomic_bool            m_stop_request  = false;    ///< external request to shutdown.
        std::atomic_bool            m_ep0_inactive  = true;     ///< true when ep0 is NOT able to operate.
        std::atomic_bool            m_sync_request  = false;    ///< Requested to synchronize received from host. 

        std::thread                 m_tid_ep0;                  ///< Thread to serve EP0.
        std::thread                 m_tid_com;                  ///< Thread to serve bulk transfer.
        std::string                 m_ep_directory;             ///< FFS directory to create End Points.

        transaction_ctx_t           m_data;                     ///< INPUT header+payload and OUTPUT header+payload
        bool                        m_io_ctx_valid  = false;    ///< TRUE if m_io_ctx is valid.
        io_context_t                m_io_ctx        = {};       ///< Async I/O context created by io_setup.
        int                         m_ep0_ctrl      = -1;       ///<
        int                         m_ep1_tx        = -1;       ///<
        int                         m_evfd_tx       = -1;       ///< Handle to event Write
        int                         m_ep2_rx        = -1;       ///<
        int                         m_evfd_rx       = -1;       ///< Handle to event Read
};


}
}
#endif

