#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

using namespace std::chrono_literals;

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_tx_resp() {

    bool io_res;

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    m_ctx.out_prm.len = static_cast<uint32_t> ( m_ctx.out_hdr.size() );
    hid::stream::Prefix::SetParams( m_ctx.out_prm, m_ctx.out_hdr );

    info ( "Sending Response; Header: %d bytes; Payload: %d bytes; (%s):(%d)", (int)(m_ctx.out_hdr.size()), (int)m_ctx.out_pay.size(), __FUNCTION__, __LINE__);

    //---------------------------------------------------------------------------//
    //  ->  STATE_FAILED            Critical errors (cannot continue).           //
    //  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
    //  ->  STATE_RX_HEADER_REQUEST Timeout while sending. Host inactive.        // 
    //  ->  STATE_RX_HEADER_REQUEST Frame successfully sent.                     //
    //---------------------------------------------------------------------------//
    io_res = write_frame ( m_ctx.out_hdr.data(), m_ctx.out_hdr.size(), usb_state_t::STATE_RX_HEADER_REQUEST );

    if ( ! io_res ) {
        err ( "Failed to send header; (%s):(%d)", __FUNCTION__, __LINE__ );
    } else
    if ( m_ctx.out_pay.size() > 0 ) {

        info ( "Sending payload; Len:%d; (%s):(%d)", (int)(m_ctx.out_pay.size()), __FUNCTION__, __LINE__ );
        io_res = write_frame ( m_ctx.out_pay.data(), m_ctx.out_pay.size(), usb_state_t::STATE_RX_HEADER_REQUEST );
        if ( ! io_res ) {
            err ( "Failed to send payload; (%s):(%d)", __FUNCTION__, __LINE__ );
        }
    }

    debug ( "Done: (%s):(%d)", __FUNCTION__, __LINE__ );
}
    
//---------------------------------------------------------------------------//

}
}
