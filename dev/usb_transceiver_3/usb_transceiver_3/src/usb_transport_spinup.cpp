#include <thread>
#include <chrono>

#include <hid/usb_transport_device.h>
#include <hid/usb_transport_device_int.h>

#include <logging.h>


using namespace std::chrono_literals;

namespace hid {
namespace transport {

//---------------------------------------------------------------------------//

void usb_transport_device_t::handle_spinup() {

    static int spin_cnt = 0;
    
    if ( spin_cnt == 0 ) {
        // Once per 5 seconds.
        err ( "EP0 down; (%s):(%d)", __FUNCTION__, __LINE__ );
    }

    spin_cnt++;
    spin_cnt %= 50;

    if ( m_ep0_inactive ) {
        // Give a time to system to manage EP0.
        // Seems 10 times per second is fast enough.
        std::this_thread::sleep_for(100ms);
    } else {
        err ( "EP0 ready; (%s):(%d)", __FUNCTION__, __LINE__ );
        spin_cnt = 0;
        LOG_USB_STATE(usb_state_t::STATE_INITIALIZE);
    }
}

//---------------------------------------------------------------------------//

}
}
