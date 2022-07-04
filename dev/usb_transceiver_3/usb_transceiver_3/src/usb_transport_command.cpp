#include <thread>
#include <chrono>
#include <cstdlib>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
    
void usb_transport_device_t::handle_command() {

    static bool first_call = true;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    if ( first_call ) {
        first_call = false;
        std::srand( static_cast<int>(std::time(nullptr)) );
    }

    hid::stream::cmd_t  resp_cmd    = hid::stream::cmd_t::STREAM_CMD_ERROR;
    uint32_t            resp_code   = 0;

    if ( m_data.inp_prm.cmd == hid::stream::cmd_t::STREAM_CMD_PING_REQUEST ) {

        // Ping request received.
        debug ( "Handling transaction \"PING\"; param: %d; (%s):(%d)", m_data.inp_prm.param, __FUNCTION__, __LINE__ );

        m_data.out_pay.clear();

        resp_cmd  = hid::stream::cmd_t::STREAM_CMD_PING_RESPONSE;
        resp_code = m_data.inp_prm.param;

    } else
    if ( m_data.inp_prm.cmd == hid::stream::cmd_t::STREAM_CMD_REQUEST ) {
        
        // Ping request received.
        debug ( "Handling transaction \"COMMAND\"; param: %d; (%s):(%d)", m_data.inp_prm.param, __FUNCTION__, __LINE__ );

        size_t new_len = std::rand() % 64;
        new_len += 16;
        m_data.out_pay.resize ( new_len );

        for ( size_t i = 0; i < m_data.out_pay.size(); i++ ) {
            m_data.out_pay[i] = 'a' + (std::rand() % 24 );
        }

        resp_cmd  = hid::stream::cmd_t::STREAM_CMD_RESPONSE;
        resp_code = 1 + std::rand() % 64;
    }

    m_data.out_prm.cmd    =  resp_cmd;
    m_data.out_prm.param  =  resp_code;

    debug ( "Leave with cmd: %d; code: %d; (%s):(%d)", (int)resp_cmd, (int)resp_code, __FUNCTION__, __LINE__ );

    LOG_USB_STATE ( usb_state_t::STATE_TX_RESPONSE );
}

//---------------------------------------------------------------------------//

}
}
