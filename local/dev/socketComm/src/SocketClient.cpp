#include "global.h"

TDBG_DEFINE_AREA(sockclient);

#include <chrono>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <SocketClient.h>

#include <application/const.h>

// #include <logging.h>
// #define info     printf
// #define err      printf

using namespace std;

SocketClient::SocketClient() {
    client_file_descriptor_ = 0;
}

SocketClient::SocketClient(SocketClient&& src) noexcept {

    client_read_thread_     = std::move(src.client_read_thread_);
    client_write_thread_    = std::move(src.client_write_thread_);
    read_message_queue_     = std::move(src.read_message_queue_);
    write_message_queue_    = std::move(src.write_message_queue_);
    client_file_descriptor_ = src.client_file_descriptor_;
    stopping_flag_          = src.stopping_flag_.load();
    server_sock_            = src.server_sock_;
    client_sock_            = src.client_sock_;
}

SocketClient::~SocketClient() {

    // info ("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    stopping_flag_ = true;
    StopClient();
}

SocketClient& SocketClient::operator=(SocketClient&& src) noexcept {

    client_read_thread_     = std::move(src.client_read_thread_);
    client_write_thread_    = std::move(src.client_write_thread_);
    read_message_queue_     = std::move(src.read_message_queue_);
    write_message_queue_    = std::move(src.write_message_queue_);
    client_file_descriptor_ = src.client_file_descriptor_;
    stopping_flag_          = src.stopping_flag_.load();
    server_sock_            = src.server_sock_;
    client_sock_            = src.client_sock_;

    return *this;
}

bool SocketClient::InitSocket(const std::string& socket_name, const bool use_unix_socket, const bool use_async_mode) {

    // info ("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    const int fd = socket( (use_unix_socket ? AF_UNIX : AF_INET), SOCK_STREAM, 0 );

    if (fd == -1) {
        // err ("Error: socket() failed.");
        return false;
    }

    client_file_descriptor_ = fd;

    const string client_socket_path = "/tmp/" + socket_name + "_client.sock";
    const string server_socket_path = "/tmp/" + socket_name + ".sock";

    // info("creating server socket %s", server_socket_path.c_str());
    // info("creating client socket %s", client_socket_path.c_str());

    // assuming the socket server is not exist, otherwise delete it.
    if (access(client_socket_path.c_str(), 0) != -1) {
        unlink(client_socket_path.c_str());
    }

    client_sock_.sun_family = AF_UNIX;
    strcpy(client_sock_.sun_path, client_socket_path.c_str());

    if (bind(client_file_descriptor_, reinterpret_cast<sockaddr*>(&client_sock_), sizeof(client_sock_)) < 0) {
        // err("SIZE 1 =%lu, SIZE 2 = %lu", sizeof(reinterpret_cast<sockaddr*>(&client_sock_)), sizeof(client_sock_));
        // err("Cannot bind to socket file! error code = %d \n", errno);
        return false;
    }

    server_sock_.sun_family = (use_unix_socket ? AF_UNIX : AF_INET);
    strcpy(server_sock_.sun_path, server_socket_path.c_str());

    is_full_duplex_mode_ = use_async_mode;

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::StartClient() {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    int   err_cnt = 0;

    for (int err_cnt = 0; err_cnt < SOCK_CONN_RETRY_CNT; err_cnt++) {

        const auto conn_state = connect(client_file_descriptor_, (sockaddr*)&server_sock_, sizeof(server_sock_));

        if (conn_state >= 0) {
            // info("connection established. target: %s, conn_state = %d, client = %s\n", server_sock_.sun_path, conn_state, client_sock_.sun_path);
            break;
        }

        // info("Retry connecting to server. error = %d, file = %s", errno, server_sock_.sun_path);
        std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_CONN_RETRY_DELAY_MSEC));
    }

    if (err_cnt == SOCK_CONN_RETRY_CNT) {
        // err("Error connecting client to server... error = %d, file = %s", errno, server_sock_.sun_path);
        return false;
    }

    if (is_full_duplex_mode_) {
        // info("Full duplex mode selected.");
        client_read_thread_  = std::thread(&SocketClient::CommRecvWorker, this);
        client_write_thread_ = std::thread(&SocketClient::CommSendWorker, this);
    }

    // info("Leave: (%s):(%d)", __FUNCTION__, __LINE__ );
    return true;
}

bool SocketClient::StopClient() {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    if (stopping_flag_) {
        // err ("Stopping in progress");
        return true;
    }

    stopping_flag_ = true;

    if (client_read_thread_.joinable()) {
        client_read_thread_.join();
    }

    if (client_write_thread_.joinable()) {
        client_write_thread_.join();
    }

    shutdown(client_file_descriptor_, SHUT_RDWR);
    close(client_file_descriptor_);

    client_file_descriptor_ = -1;

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::SendMessageAsync ( const void* const msgPtr, unsigned int msgLen ) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert (false);

    if (nullptr == msgPtr) {
        // err("Invalid parameter msgPtr");
        assert(false);
        return false;
    }

    if (msgLen == 0) {
        // err("Invalid parameter msgLen");
        assert(false);
        return false;
    }

    if (!is_full_duplex_mode_) {
        // err ("Command rejected. Full duplex mode required.");
        assert(false);
        return false;
    }

    bin_data_t msg(msgLen);
    memcpy(msg.data(), msgPtr, msgLen);

    write_queue_lock_.lock();
        write_message_queue_.push(msg);
    write_queue_lock_.unlock();

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::SendMessage ( const void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert(false);

    if (nullptr == msgPtr) {
        // err("Invalid parameter msgPtr");
        assert(false);
        return false;
    }

    if (msgLen == 0) {
        // err("Invalid parameter msgLen");
        assert(false);
        return false;
    }

    if (is_full_duplex_mode_) {
        // err("Command rejected. Full duplex mode enabled.");
        assert(false);
        return false;
    }

    bool ioRes;
    size_t outLen = msgLen;
    
    ioRes = SendFrame(&outLen, sizeof(outLen), 0);
    if (!ioRes) {
        // err ("SendFrame failed. Cannot send Length of frame.");
        return false;
    }

    ioRes = SendFrame(msgPtr, msgLen, timeout_ms);
    if (!ioRes) {
        // err("SendFrame failed. Cannot send Frame.");
        return false;
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::RecvMessageAsync ( void* const msgPtr, unsigned int& msgLen ) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert(false);

    if (nullptr == msgPtr) {
        // err("Invalid parameter");
        assert(false);
        return false;
    }

    if (msgLen == 0) {
        // err("Invalid parameter");
        assert(false);
        return false;
    }

    if ( ! is_full_duplex_mode_ ) {
        // err("Command rejected. Full duplex mode required.");
        assert(false);
        return false;
    }


    bin_data_t msg;
    bool msgReceived = false;

    while ( ! stopping_flag_ ) {

        msgReceived = false;

        read_queue_lock_.lock();
            if ( ! read_message_queue_.empty() ) {
                msgReceived = true;
                msg = read_message_queue_.front();
                read_message_queue_.pop();
            }
        read_queue_lock_.unlock();

        if (msgReceived) {
            break;
        }

        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_CONN_RETRY_DELAY_MSEC));
    }

    if ( stopping_flag_ ) {
        // info("STOP received");
        return false;
    }

    if ( ! msgReceived ) {
        // err("No message received");
        return false;
    }

    if (msg.size() > msgLen) {
        // err("Message too big");
        return false;
    }

    if (msg.size() == 0) {
        // err("Empty message received");
        return false;
    }

    msgLen = static_cast<unsigned int> (msg.size());
    if (msgLen > 0) {
        memcpy(msgPtr, msg.data(), msgLen);
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::RecvMessage ( void* const msgPtr, unsigned& msgLen, unsigned timeout_ms) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert(false);

    if (nullptr == msgPtr) {
        // err("Invalid parameter msgPtr");
        assert(false);
        return false;
    }

    if (msgLen == 0) {
        // err("Invalid parameter msgLen");
        assert(false);
        return false;
    }

    if (is_full_duplex_mode_) {
        // err("ASYNC mode enabled");
        assert(false);
        return false;
    }

    bool ioRes = RecvFrame(msgPtr, msgLen, timeout_ms);

    if (!ioRes) {
        // err("RecvFrame failed.");
        return false;
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::RecvFrame ( void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

    // info("Sock client recv %d bytes (%s):(%d)", msgLen, __FUNCTION__, __LINE__);

    assert(nullptr != msgPtr);
    assert(msgLen > 0 );

    char* const     dst     = static_cast<char*> (msgPtr);
    unsigned int    rxCnt   = 0;
    int             ioRes   = 0;

    ConfigureTimeout(timeout_ms);
    while (rxCnt < msgLen) {

        if (stopping_flag_) {
            // info ("Stop Flag.");
            return false;
        }

        ioRes = recv(client_file_descriptor_,  dst+rxCnt,  msgLen-rxCnt, MSG_WAITALL);

        if (ioRes < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            // err("Read buffer got an error: %d; %s !", errno, strerror(errno) );
            return false;
        }

        rxCnt += ioRes;
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketClient::SendFrame ( const void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

    // info("Sock client send %d bytes (%s):(%d)", msgLen, __FUNCTION__, __LINE__);

    const char* const   src     = static_cast<const char*> (msgPtr);
    unsigned int        txCnt   = 0;
    int                 ioRes   = 0;
    bool                retVal  = true;

    if (nullptr == msgPtr) {
        // err("Invalid parameter msgPtr");
        assert(false);
        return false;
    }

    if (msgLen == 0) {
        // info("Zero msgLen");
        assert(false);
        return true;
    }

    ConfigureTimeout(timeout_ms);

    while (txCnt < msgLen) {

        if (stopping_flag_) {
            // info("Stop Flag.");
            retVal = true;
            break;
        }

        ioRes = send(client_file_descriptor_,  src+txCnt,  msgLen-txCnt,  MSG_WAITALL);

        if (ioRes < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            // err("Error Read failed with error: %d %s !", errno, strerror(errno));
            retVal = false;
            break;
        }

        txCnt += ioRes;
    }

    // info("Leave: (%s):(%d)", __FUNCTION__, __LINE__);
    return retVal;
}

void SocketClient::CommRecvWorker() {

    bool        ioRes;
    size_t      frameLen;
    bin_data_t  msg;

    for ( ; ; ) { 

        if (stopping_flag_) {
            // info("Stop Flag triggered at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        ioRes = RecvFrame ( &frameLen, sizeof(frameLen), 0);

        if ( ! ioRes ) {
            // err("Error: RecvFrame failed at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        msg.resize(frameLen);

        ioRes = RecvFrame( msg.data(), frameLen, 0);

        if ( ! ioRes ) {
            // err("Error: RecvFrame failed at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        read_queue_lock_.lock();
            read_message_queue_.push(msg);
        read_queue_lock_.unlock();
    }

    // err("Leave (%s):(%d)", __FUNCTION__, __LINE__);
}

void SocketClient::CommSendWorker() {

    bool msgReceived;
    bin_data_t msg;
    size_t     msgLen;

    while ( ! stopping_flag_ ) {

        msgReceived = false;

        write_queue_lock_.lock();
            if ( ! write_message_queue_.empty() ) {
                msgReceived = true;
                msg = write_message_queue_.front();
                write_message_queue_.pop();
            }
        write_queue_lock_.unlock();

        if ( ! msgReceived ) {
            std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_CONN_RETRY_DELAY_MSEC));
            continue;
        }

        msgLen = msg.size();

        SendFrame(&msgLen, sizeof(msgLen), -1);
        SendFrame(msg.data(), msg.size(), -1);
    }
}

void SocketClient::ConfigureTimeout(unsigned int timeout_ms) {

    #if 0
    timeval timeout_cfg{};

    if (timeout_ms == 0) {
        return;
    }
    
    timeout_cfg.tv_sec  = (timeout_ms / 1000);
    timeout_cfg.tv_usec = (timeout_ms % 1000) * 1000;

    setsockopt(client_file_descriptor_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout_cfg, sizeof(timeout_cfg));
    setsockopt(client_file_descriptor_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_cfg, sizeof(timeout_cfg));
    #endif
}
