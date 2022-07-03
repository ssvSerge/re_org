#include <unistd.h>
#include <sys/reboot.h>
#include <ICmd.h>
#include <HandledCmdSet.h>
#include <VCOMTransceiver.h>
#include <logging.h>

#include <application/vcom_tools.h>


VCOMTransceiver::VCOMTransceiver() {
    m_pTransactionBroker    = new TransactionBroker();
    cmd_handle_flags_       = CmdFlagEnum::TRANSPORT_TO_SE | CmdFlagEnum::ATOMIC_FLAG;
    m_recovery_flag_        = false;
    m_recovery_reason_      = 0;
}

uint32_t VCOMTransceiver::connect_engine() {

    char vcom_socket_name_buffer[128]{};

    snprintf(vcom_socket_name_buffer, 128, "LumiVCOMSocket_%03d_%03d", 0, 0);

    vcom_socket_name = std::string(vcom_socket_name_buffer);

    auto success = vcom_socket_client_.InitSocket(vcom_socket_name);
    if ( ! success ) {
        err( "ERROR: Socket client init failed!!!");
    }

    success = false;
    int fail_count = 0;

    while ( !success ) {

        success = vcom_socket_client_.StartClient();
        if (!success) {
            err( "ERROR: Cannot start client!");
            fail_count++;
        }
        if (fail_count > 2) {
            m_connected = false;
            break;
        }
    }

    auto ret = m_pTransactionBroker->Initialize();
    if (ret != 0) {
        if (ret == 1) {
            return 0;
        } else {
            return -2;
        }
    }

    m_connected = true;
    return 0;
}

VCOMTransceiver::~VCOMTransceiver() {
    info ("Enter: (%s):(%d)", __FUNCTION__, __LINE__ );
    delete m_pTransactionBroker;
    m_pTransactionBroker = nullptr;
    m_connected = false;
    vcom_socket_client_.StopClient();
}

void VCOMTransceiver::SetResetRxBufCallback(CallbackPrototype functor) {
    callback_func_ = functor;
}

bool VCOMTransceiver::Should_Reboot() {
    if(m_pTransactionBroker == nullptr) {
        return false;
    }
    return m_pTransactionBroker->Should_Reboot();
}

void VCOMTransceiver::StopConnection() {
    m_connected = false;
    vcom_socket_client_.StopClient();
}

void VCOMTransceiver::ResetUnit() {

    if (m_pTransactionBroker == NULL) {
        sync();
        reboot(RB_AUTOBOOT);
        return;
    }

    m_pTransactionBroker->Reset();
    exit(0);
}

void VCOMTransceiver::SetRecoveryMode(int32_t reason_num) {

    m_recovery_flag_ = true;
    m_recovery_reason_ = reason_num;
    m_pTransactionBroker->SetRecoveryMode(reason_num);
}

uint32_t VCOMTransceiver::message_arrived(const bin_data_t& request, bin_data_t& response) {

    USBCB       header_send{};
    USBCB       header_recv{};
    uint8_t*    io_ptr = nullptr;
    uint32_t    io_len = 0;

    // info ("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    response.clear();

    if ( ! m_connected ) {
        info ("Not connected (%s):(%d)", __FUNCTION__, __LINE__);
        connect_engine();
    }

    if ( ! m_connected ) {
        err("No connection to server. (%s):(%d)", __FUNCTION__, __LINE__);
        return -1;
    }

    vcom_hdr_config ( &header_send, 0, request.size(), 0);

    io_ptr = reinterpret_cast<uint8_t*> (&header_send);
    io_len = sizeof(header_send);

    if ( ! vcom_socket_client_.SendFrame(io_ptr, io_len, 0) ) {
        err("Error: Failed to send Header (%s):(%d)", __FUNCTION__, __LINE__);
        StopConnection();
        return -2;
    }

    io_ptr = (uint8_t*) (request.data());
    io_len = request.size();

    if ( ! vcom_socket_client_.SendFrame(io_ptr, io_len, 0)) {
        err("Error: Failed to send payload (%s):(%d)", __FUNCTION__, __LINE__);
        StopConnection();
        return -3;
    }

    io_ptr = reinterpret_cast<uint8_t*> (&header_recv);
    io_len = sizeof(header_recv);

    if ( ! vcom_socket_client_.RecvFrame(io_ptr, io_len, 0) ) {
        err("Error: Failed to receive Header (%s):(%d)", __FUNCTION__, __LINE__);
        StopConnection();
        return -4;
    }

    if ( ! vcom_hdr_validate(&header_recv) ) {
        err("Error: Failed to validate Header (%s):(%d)", __FUNCTION__, __LINE__);
        StopConnection();
        return -5;
    }

    response.resize(header_recv.ulCount);

    io_ptr = response.data();
    io_len = response.size();

    if ( ! vcom_socket_client_.RecvFrame(io_ptr, io_len, 0) ) {
        err("Error: Failed to receive Payload (%s):(%d)", __FUNCTION__, __LINE__);
        StopConnection();
        return -6;
    }

    // info("Leave: (%s):(%d)", __FUNCTION__, __LINE__);
    return 0;
}
