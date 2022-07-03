#ifndef __usb_transport_device_int_h__
#define __usb_transport_device_int_h__

#include <stdint.h>

#include <StreamPrefix.h>

#include <logging.h>
// #define err      printf
// #define info     printf
// #define warn     printf

struct usb_ctrlrequest;

namespace hid {
namespace transport {


//---------------------------------------------------------------------------//

#define IOCB_FLAG_RESFD             (  1 <<    0 )
#define USB_READ_TIMEOUT_MS         (  10 * 1000 )
#define USB_IO_TIMEOUT_MS           (  60 * 1000 )

//---------------------------------------------------------------------------//


enum class usb_state_t {
    STATE_UNKNOWN                   =  0, ///< Stuff object. memset with 0 not allowed.
    STATE_INITIALIZE                = 10, ///< Initialize endpoint at start/restart time.
    STATE_PHANTOM_READ              = 20, ///< Attempt to read from endpoint until short timeout. 
    STATE_RX_HEADER_PLACE           = 30, ///< Put the READ request for USB Header.    
    STATE_RX_HEADER_WAIT            = 31, ///< Wait for the header. Timeout is a valid state.    
    STATE_RX_PAYLOAD                = 40, ///< Read payload.
    STATE_HANDLE_REQUEST            = 50, ///< HEADER and PYALOAD are successfully read. Send it to J-Engine.
    STATE_TX_HEADER                 = 60, ///< Place the TX request for HEADER.
    STATE_TX_PAYLOAD                = 61, ///< 
    STATE_SPINUP                    = 70, ///< Event "ep0 down" received. Reset context and return to the state "INIT".
    STATE_RESTART                   = 80, ///< SYNC request received. Close the handles, Async objects and reopen all of them. 
    STATE_FAILED                    = 90, ///< Critical error. Application should go down.
    STATE_LAST_ID                         ///< Stuff object.
};

class transaction_ctx_t {
    public:
        hid::types::storage_t       inp_hdr;
        hid::types::storage_t       inp_pay;
        hid::types::storage_t       out_hdr;
        hid::types::storage_t       out_pay;
        int32_t                     out_code;
};

class state_monitor_t {

    public:
        state_monitor_t() {
            m_state = usb_state_t::STATE_UNKNOWN;
        }

        void set_state ( const usb_state_t next_state, const char* function, int line ) {
            m_state = next_state;
            info ("State: %s; (%s):(%d)", state_to_name(next_state), function, line );
        }

        usb_state_t get () { return m_state; }

        const char* state_to_name( const usb_state_t state ) {

            const char* ret_val = "UNDEFINED";

            switch ( state ) {

                case usb_state_t::STATE_INITIALIZE:          { ret_val = "Initialize";        break; }
                case usb_state_t::STATE_READ_ON_INITIALIZE:  { ret_val = "Read_on_init";      break; }
                case usb_state_t::STATE_RX_HEADER_PLACE:     { ret_val = "Rx_HDR_place";      break; }
                case usb_state_t::STATE_RX_HEADER_WAIT:      { ret_val = "Rx_HDR_wait";       break; }
                case usb_state_t::STATE_RX_HEADER_VALIDATE:  { ret_val = "Rx_HDR_validate";   break; }
                case usb_state_t::STATE_RX_PAYLOAD_OK_PLACE: { ret_val = "Rx_PAY_good_place"; break; }
                case usb_state_t::STATE_RX_PAYLOAD_OK_WAIT:  { ret_val = "Rx_PAY_good_wait";  break; }
                case usb_state_t::STATE_RX_PAYLOAD_BAD_PLACE:{ ret_val = "Rx_PAY_bad_place";  break; }
                case usb_state_t::STATE_RX_PAYLOAD_BAD_WAIT: { ret_val = "Rx_PAY_bad_wait";   break; }
                case usb_state_t::STATE_HANDLE_REQUEST:      { ret_val = "Handle request";    break; }
                case usb_state_t::STATE_TX_HEADER_PLACE:     { ret_val = "Tx_HDR_place";      break; }
                case usb_state_t::STATE_TX_HEADER_WAIT:      { ret_val = "Tx_HDR_wait";       break; }
                case usb_state_t::STATE_TX_PAYLOAD_PLACE:    { ret_val = "Tx_PAY_place";      break; }
                case usb_state_t::STATE_TX_PAYLOAD_WAIT:     { ret_val = "Tx_PAY_wait";       break; }
                case usb_state_t::STATE_RESTART:             { ret_val = "Restart";           break; }

                case usb_state_t::STATE_UNKNOWN:
                case usb_state_t::STATE_LAST_ID:
                default:
                     break;

            }

            return ret_val;
        }
        
    private:
        usb_state_t     m_state;
};



}
}


#endif
