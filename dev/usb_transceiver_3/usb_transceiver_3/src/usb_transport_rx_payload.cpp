#include <thread>
#include <chrono>
#include <cassert>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>

using namespace std::chrono_literals;


namespace hid {
namespace transport {

//---------------------------------------------------------------------------//
//  ->  STATE_FAILED            Critical errors. Cannot continue.            //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_RX_HEADER_REQUEST Timeout while reading.                       // 
//  ->  STATE_HANDLE_REQUEST    Payload ready. Process transaction.          //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_read_ok ( const checkpoint_t time_out ) {

    // It is expected inp_pay has been validated by handle_payload_read.
    assert (  m_ctx.inp_pay.size() > 0  );
    assert (  m_ctx.inp_pay.data() != nullptr );

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );
    info  ( "Wait for %d bytes; (%s):(%d)", m_ctx.inp_prm.len, __FUNCTION__, __LINE__ );

    // -> STATE_FAILED              in case of I/O errors.
    // -> STATE_SPINUP              if EP0 is down.
    // -> STATE_RX_HEADER_REQUEST   if timeout.
    // -> STATE_HANDLE_REQUEST      if succeeded.
    read_frame ( time_out, m_ctx.inp_pay.data(), m_ctx.inp_pay.size(), usb_state_t::STATE_HANDLE_REQUEST );

    debug ( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
}
    
//---------------------------------------------------------------------------//
//  ->  STATE_FAILED            Critical errors. Cannot continue.            //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_RX_HEADER_REQUEST Timeout while reading.                       // 
//  ->  STATE_TX_RESPONSE       Payload discarded. Return error code.        //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_read_bad ( const checkpoint_t time_out ) {

    uint8_t     local_buffer [4096];    // Temporary buffer where to store payload.
    uint32_t    read_cnt  = 0;          // 
    uint32_t    data_part = 0;          // 
    bool        can_continue = true;    // 

    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    // Low memory condition.
    for ( ; ; ) {

        info ( "Read %d from %d bytes; (%s):(%d)", read_cnt, m_ctx.inp_prm.len, __FUNCTION__, __LINE__ );

        if ( read_cnt == m_ctx.inp_prm.len ) {
            // Whole frame read.
            info ( "Frame read; (%s):(%d)", __FUNCTION__, __LINE__ );
            can_continue = true;
            break;
        }

        data_part = m_ctx.inp_prm.len - read_cnt;
        if ( data_part > sizeof(local_buffer) ) {
            data_part = sizeof(local_buffer);
        }

        can_continue = read_frame ( time_out, local_buffer, data_part, usb_state_t::STATE_RX_PAYLOAD );
        if ( ! can_continue ) {
            err ( "Failed read_frame; Skip transaction. (%s):(%d)", __FUNCTION__, __LINE__ );
            break;
        }

        read_cnt += data_part;
    }

    if ( can_continue ) {
        info ( "Payload read; Ignore payload; Skip transaction; (%s):(%d)", __FUNCTION__, __LINE__ );
        m_ctx.out_pay.clear();
        m_ctx.out_prm.cmd    =  hid::stream::cmd_t::STREAM_CMD_ERROR;
        m_ctx.out_prm.param  =  USB_TRANSPORT_ERR_NO_MEMORY;
        LOG_USB_STATE ( usb_state_t::STATE_TX_RESPONSE );
    }
}
    
//---------------------------------------------------------------------------//
//  ->  STATE_FAILED            Critical errors. Cannot continue.            //
//  ->  STATE_SPINUP            EP0 unexpectedly switch down.                //
//  ->  STATE_RX_HEADER_REQUEST Timeout while reading.                       // 
//  ->  STATE_HANDLE_REQUEST    Payload ready. Process transaction.          //
//  ->  STATE_TX_RESPONSE       Payload discarded. Return error code.        //
//---------------------------------------------------------------------------//
void usb_transport_device_t::handle_payload_read() {
    
    debug ( "Enter: (%s):(%d)", __FUNCTION__, __LINE__ );

    hid::stream::Prefix::GetParams( m_ctx.inp_hdr, m_ctx.inp_prm );

    if ( ! hid::stream::Prefix::Valid(m_ctx.inp_hdr) ) {
       err("Wrong header received");
       LOG_USB_STATE(usb_state_t::STATE_RX_HEADER_REQUEST);
       return;
    } 

    if ( m_ctx.inp_prm.len == 0 ) {
        // There could be SYNC request. Payload not expected.
        info("Valid frame with empty payload; (%s):(%d)", __FUNCTION__, __LINE__ );
        LOG_USB_STATE(usb_state_t::STATE_HANDLE_REQUEST );
        return;
    }
        
    bool alloc_valid = true;

    try {
        m_ctx.inp_pay.resize( m_ctx.inp_prm.len );
        cleanup ( m_ctx.inp_pay );
    } catch ( ... ) {
        alloc_valid = false;
        err("Error: Failed to allocate buffer %d bytes; (%s):(%d)", m_ctx.inp_prm.len, __FUNCTION__, __LINE__ );
    }

    checkpoint_t start_time;
    checkpoint_t time_out = start_time + duration_ms_t(USB_IO_TIMEOUT_MS);

    if ( alloc_valid ) {
        handle_read_ok (time_out);
    } else {
        handle_read_bad (time_out);
    }

    debug( "Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
}

//---------------------------------------------------------------------------//

}
}
