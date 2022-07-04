#ifndef __usb_transport_device_int_h__
#define __usb_transport_device_int_h__

#include <stdint.h>

#include <hid/StreamPrefix.h>

#include <logging.h>
// #define err      printf
// #define info     printf
// #define warn     printf

struct usb_ctrlrequest;

namespace hid {
namespace transport {


//---------------------------------------------------------------------------//

constexpr uint32_t  IOCB_FLAG_RESFD      =  (1 << 0);
constexpr uint32_t  USB_READ_TIMEOUT_MS  =  (15 * 1000);
constexpr uint32_t  USB_IO_TIMEOUT_MS    =  (60 * 1000);
constexpr uint32_t  MAX_CLEANUP_LEN      =  (16 * 1024);
constexpr bool      CANNOT_CONTINUE      =  false;
constexpr bool      READY                =  true;

//---------------------------------------------------------------------------//

extern void cleanup(usb_frame_t& frame);

//---------------------------------------------------------------------------//

class state_monitor_t {

    public:

        void set_state ( const usb_state_t next_state, const char* function, int line ) {
            if (m_state != next_state) {
                info("State: %s; (%s):(%d)", state_to_name(next_state), function, line);
            }
            m_state = next_state;
        }

        usb_state_t get () { return m_state; }

        const char* state_to_name( const usb_state_t state ) {

            const char* ret_val = "UNDEFINED";

            switch ( state ) {
                case usb_state_t::STATE_SPINUP:            { ret_val = "Spin up";          break; }
                case usb_state_t::STATE_INITIALIZE:        { ret_val = "Initialize";       break; }
                case usb_state_t::STATE_PHANTOM_READ:      { ret_val = "Phantom Read";     break; }
                case usb_state_t::STATE_RX_HEADER_WAIT:    { ret_val = "Rx HDR wait";      break; }
                case usb_state_t::STATE_RX_PAYLOAD:        { ret_val = "Rx Payload";       break; }
                case usb_state_t::STATE_HANDLE_REQUEST:    { ret_val = "Process command";  break; }
                case usb_state_t::STATE_TX_RESPONSE:       { ret_val = "Tx Response";      break; }
                case usb_state_t::STATE_FAILED:            { ret_val = "Failed";           break; }
                default:
                     break;

            }

            return ret_val;
        }
        
    private:
        usb_state_t m_state = {};
};

//---------------------------------------------------------------------------//

extern state_monitor_t g_transaction_state;

//---------------------------------------------------------------------------//

#define LOG_USB_STATE(x)    { g_transaction_state.set_state(x, __FUNCTION__, __LINE__); }

//---------------------------------------------------------------------------//

}
}


#endif
