#include "global.h"

TDBG_DEFINE_AREA(sockserver);

#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <fcntl.h>
#include <string.h>

#include <SocketServer.h>

// #include <logging.h>
// #define info     printf
// #define err      printf

using namespace std;

SocketServer::SocketServer() {
};

SocketServer::SocketServer(SocketServer&& src) noexcept {

    listener_thread_            = std::move(src.listener_thread_);
    comm_recv_worker_threads_   = std::move(src.comm_recv_worker_threads_);
    server_sock_                = src.server_sock_;
    client_sock_                = src.client_sock_;
    accepting_multiple_conn_    = src.accepting_multiple_conn_;
    max_connection_count_       = src.max_connection_count_;
    socket_file_descriptor_     = src.socket_file_descriptor_;
    stopping_flag_              = src.stopping_flag_.load();
    is_async_mode_              = src.is_async_mode_;

    read_message_queue_     = std::move(src.read_message_queue_);
    write_message_queue_    = std::move(src.write_message_queue_);
    on_msg_callback_func    = std::move(src.on_msg_callback_func);
}

SocketServer::~SocketServer() {

    // err("Quit server called!");

    if (!StopServer()) {
        // err("Quitting server gracefully failed! Forcing closing socket...");
    }

    // info ("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
    close(socket_file_descriptor_);
    unlink(server_sock_.sun_path);
}

SocketServer& SocketServer::operator=(SocketServer&& src) noexcept {

    listener_thread_            = std::move(src.listener_thread_);
    comm_recv_worker_threads_   = std::move(src.comm_recv_worker_threads_);
    server_sock_                = src.server_sock_;
    client_sock_                = src.client_sock_;
    accepting_multiple_conn_    = src.accepting_multiple_conn_;
    max_connection_count_       = src.max_connection_count_;
    socket_file_descriptor_     = src.socket_file_descriptor_;
    stopping_flag_              = src.stopping_flag_.load();
    is_async_mode_              = src.is_async_mode_;
    read_message_queue_         = std::move(src.read_message_queue_);
    write_message_queue_        = std::move(src.write_message_queue_);
    on_msg_callback_func        = std::move(src.on_msg_callback_func);

    return *this;
}

bool SocketServer::InitSocket(const std::string &socket_name, const bool allow_multi_conn, const bool use_unix_socket, const bool use_async_mode) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    const int fd = socket((use_unix_socket ? AF_UNIX : AF_INET), SOCK_STREAM, 0);

    transfer_state_ = TransferStates::NOT_STARTED;

    if (fd == -1) {
        // err("Error: socket() failed.");
        return false;
    }

    socket_file_descriptor_ = fd;
    stringstream back_strm{};
    const string full_path = "/tmp/" + socket_name + ".sock";

    if (access(full_path.c_str(), 0) != -1) {
        unlink(full_path.c_str());
    }

    server_sock_.sun_family = (use_unix_socket ? AF_UNIX : AF_INET);
    strcpy(server_sock_.sun_path, full_path.c_str());

    if (::bind(socket_file_descriptor_, reinterpret_cast<sockaddr*>(&server_sock_), sizeof(server_sock_)) < 0)  {
        // err("Cannot bind to socket file! error code = %d ", errno);
        // info("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
        close(fd);
        return false;
    }

    if (!allow_multi_conn) {
        max_connection_count_ = 1;
    } else {
        max_connection_count_ = SOCK_CONN_MAX_CONN_CNT;
    }

    if ( !use_async_mode ) {
        is_async_mode_ = false;
    }

    // info("Server init: name = %s, fd = %d", server_sock_.sun_path, socket_file_descriptor_);
    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);

    return true;
}

void SocketServer::SetMaxConnections(unsigned int max_conn) {
    max_connection_count_ = max_conn;
}

void SocketServer::BindConnectionHandleFunction(std::function<MsgHandleFunctionPrototype> function_instance) {
    on_msg_callback_func = std::move(function_instance);
}

TransferStates SocketServer::ConnectState() {
    std::lock_guard<std::mutex> locker(transfer_state_mutex_);
    return transfer_state_;
}

bool SocketServer::StartServer() {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    transfer_state_mutex_.lock();
        transfer_state_ = TransferStates::WAIT_ACCEPT;
    transfer_state_mutex_.unlock();

    listener_thread_ = std::thread(&SocketServer::ListenerWorkerFunc, this);

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketServer::StopServer() {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    stopping_flag_ = true;

    // info("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
    shutdown(socket_file_descriptor_, SHUT_RDWR);
    close(socket_file_descriptor_);

    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }

    if ( ! comm_worker_file_descriptors_.empty() ) {
        // info("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
        shutdown(comm_worker_file_descriptors_.at(0), SHUT_RDWR);
        close(comm_worker_file_descriptors_.at(0));
        comm_worker_file_descriptors_.clear();
    }

    for (auto& comm_fd : comm_worker_file_descriptors_) {
        // info("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
        shutdown(comm_fd, SHUT_RDWR);
        close(comm_fd);
    }

    for (auto& comm_worker_thread : comm_recv_worker_threads_) {
        stopping_flag_ = true;
        if (comm_worker_thread.joinable()) {
            comm_worker_thread.join();
        }
    }

    for (auto& comm_writer_thread : comm_write_worker_threads_) {
        if (comm_writer_thread.joinable()) {
            comm_writer_thread.join();
        }
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketServer::SendMessageAsync ( const void* const msgPtr, unsigned int msgLen ) {

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

    if (!is_async_mode_) {
        // err("Command rejected. Full duplex mode required.");
        assert(false);
        return false;
    }

    bin_data_t msg_vector(msgLen);
    memcpy(msg_vector.data(), msgPtr, msgLen);

    write_queue_lock_.lock();
        write_message_queue_.push(msg_vector);
    write_queue_lock_.unlock();

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketServer::SendMessage ( const void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms ) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert(false);

    bool ioRes;

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

    if (is_async_mode_) {
        // err("You should not call SendMessage as socket is in async mode!");
        assert(false);
        return false;
    }

    if (comm_worker_file_descriptors_.size() == 0) {
        // err("there's no clients connected.");
        assert(false);
        return false;
    }

    ioRes = SendMessage(comm_worker_file_descriptors_.at(0), msgPtr, msgLen, timeout_ms);

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return ioRes;
}

bool SocketServer::SendMessage ( const int fd, const void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

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

    if (is_async_mode_) {
        // err("Async mode enabled");
        assert(false);
        return false;
    }

    bool    ioRes;
    size_t  outLen = msgLen;

    ioRes = SendFrame(fd, &outLen, sizeof(outLen), timeout_ms);

    if ( !ioRes ) {
        // err ("Send failed: LEN failed to send.");
        return false;
    }

    ioRes = SendFrame(fd, &msgPtr, msgLen, timeout_ms);
    if (!ioRes) {
        // err("Send failed: MSG failed to send.");
        return false;
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);

    return true;
}

bool SocketServer::SendFrame ( const int fd, const void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

    // info("Enter: send %d bytes (%s):(%d)", msgLen, __FUNCTION__, __LINE__);

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

    const char* const   src     = static_cast<const char*> (msgPtr);
    unsigned int        txCnt   = 0;
    int                 ioRes   = 0;
    bool                retVal  = true;

    ConfigureTimeout(fd, timeout_ms);

    while (txCnt < msgLen) {

        if (stopping_flag_) {
            // info("Stop Flag.");
            retVal = true;
            break;
        }

        ioRes = send(fd,  src+txCnt,  msgLen-txCnt,  0);

        if (ioRes < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            // err("Error Read failed with error: %d; %s !", errno, strerror(errno));
            retVal = false;
            break;
        }

        txCnt += ioRes;
    }

    // info("Leave: (%s):(%d)", __FUNCTION__, __LINE__);
    return retVal;
}

bool SocketServer::RecvMessageAsync ( void* const msgPtr, unsigned int& msgLen ) {

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

    if ( ! is_async_mode_ ) {
        // err("ASYNC mode is not enabled");
        assert(false);
        return false;
    }

    bin_data_t msg;
    bool msgReceived = false;

    while ( ! stopping_flag_ ) {

        msgReceived = false;

        read_queue_lock_.lock();
            if ( ! read_message_queue_.empty()) {
                msgReceived = true;
                msg = read_message_queue_.front();
                read_message_queue_.pop();
            }
        read_queue_lock_.unlock();

        if (msgReceived) {
            break;
        }

        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_SRV_CLI_COMM_DELAY));
    }

    if (stopping_flag_) {
        // info("STOP received");
        return false;
    }

    if ( ! msgReceived ) {
        // err("No message received");
        return false;
    }

    if (msg.size() > msgLen ) {
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

bool SocketServer::RecvMessage ( void* const msgPtr, unsigned int& msgLen, unsigned int timeout_ms) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    // Legacy Lumidigm code. Not used here.
    assert(false);

    bool ioRes;

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

    if (is_async_mode_) {
        // err("ASYNC mode enabled");
        assert(false);
        return false;
    }
    
    // Sleep 100 mSec => 10 times per second
    // Convert Seconds to iterations cnt.
    unsigned int count_ms = 0;
    size_t  workers_cnt;

    for ( ; ; ) { 

        if (stopping_flag_) {
            // err("STOP requested");
            return false;
        }

        // At least one client must be connected.
        workers_cnt = comm_worker_file_descriptors_.empty();
        if (workers_cnt != 0) {
            break;
        }

        if (timeout_ms > 0 ) {
            if (count_ms > timeout_ms) {
                // err("There's no clients connected.");
                return false;
            }
        }

        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        count_ms += 100;
    }

    if (workers_cnt == 0) {
        // err("Message not received");
        return false;
    }

    ioRes = RecvMessage(comm_worker_file_descriptors_.at(0), msgPtr, msgLen, timeout_ms);

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return ioRes;
}

bool SocketServer::RecvMessage ( const int fd, void* const msgPtr, unsigned int& msgLen, unsigned int timeout_ms) {

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

    bool ioRes;

    size_t frameLen;
    ioRes = RecvFrame(fd, &frameLen, sizeof(frameLen), timeout_ms);
    if (!ioRes) {
        // err("Can't read the length of frame");
        return false;
    }

    if (frameLen > msgLen) {
        // err("Fragment too long");
        assert(false);
        return false;
    }

    msgLen = frameLen;

    ioRes = RecvFrame(fd, msgPtr, frameLen, timeout_ms);
    if (!ioRes) {
        // err("Can't read frame");
        return false;
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return true;
}

bool SocketServer::RecvFrame ( const int fd, void* const msgPtr, unsigned int msgLen, unsigned int timeout_ms) {

    // info("Enter: read %d bytes (%s):(%d)", msgLen, __FUNCTION__, __LINE__);

    char* const     dst     = static_cast<char*> (msgPtr);
    unsigned int    rxCnt   = 0;
    int             ioCnt   = 0;
    bool            retVal  = true;

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

    ConfigureTimeout(fd, timeout_ms);

    while ( true ) {

        if (stopping_flag_) {
            // info("Stop Flag triggered at %s (%d)", __FUNCTION__, __LINE__);
            retVal = false;
            break;
        }

        ioCnt = recv(fd, dst+rxCnt, msgLen-rxCnt, MSG_WAITALL);

        if (ioCnt < 0) {

            if (errno == EAGAIN) {
                continue;
            }

            transfer_state_mutex_.lock();
                transfer_state_ = TransferStates::CLOSED;
            transfer_state_mutex_.unlock();

            // err("Read failed with error: %d; %s !", errno, strerror(errno) );
            retVal = false;
            break;
        }

        rxCnt += ioCnt;

        if (rxCnt == msgLen) {
            break;
        }
    }

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
    return retVal;
}

void SocketServer::ListenerWorkerFunc() {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    socklen_t client_socket_len;
    bool reject_conn;

    if (listen(socket_file_descriptor_, max_connection_count_) < 0) {
        // err("Error listening to socket file! error = %d", errno);
        stopping_flag_ = true;
        return;
    }

    // err("Listening on file: %s", server_sock_.sun_path);

    for ( ; ; ) {
        
        if ( stopping_flag_ ) {
            // info("Stop Flag triggered at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        client_socket_len = sizeof(sockaddr_un);
        
        // info("waiting for connection...");

        transfer_state_mutex_.lock();
            transfer_state_ = TransferStates::WAIT_ACCEPT;
        transfer_state_mutex_.unlock();

        // ??? accept_ex with timeout ???
        const int new_conn = accept(socket_file_descriptor_, reinterpret_cast<sockaddr*>(&client_sock_), &client_socket_len);

        if (new_conn < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            // err("Socket server not listening! error = %d client_socketl_len = %d", errno, client_socket_len);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        reject_conn = false;

        // Close unused comm_recv_worker_threads_ + comm_write_worker_threads_
        if (comm_recv_worker_threads_.size() + 1 > max_connection_count_) {
            reject_conn = true;
        }
        
        if (comm_write_worker_threads_.size() + 1 > max_connection_count_) {
            reject_conn = true;
        }

        if (reject_conn ) {
            char content[] = "SERR_NO_MORE_CONNECTION";
            auto buffer = DataSerialization<char*>::Serialize(content);
            write(new_conn, buffer.data(), buffer.size());

            // info("Socket closed at (%s):(%d)", __FUNCTION__, __LINE__);
            shutdown(new_conn, SHUT_RDWR);
            close(new_conn);

            // err("no more connection!");
            continue;
        }

        transfer_state_mutex_.lock();
            if (on_msg_callback_func != nullptr) {
                on_msg_callback_func(new_conn);
            } else {
                comm_worker_file_descriptors_.push_back(new_conn);
                if (is_async_mode_) {
                    auto recv_worker = std::thread(&SocketServer::CommRecvWorker, this, new_conn);
                    comm_recv_worker_threads_.push_back(std::move(recv_worker));
                    auto send_worker = std::thread(&SocketServer::CommSendWorker, this, new_conn);
                    comm_write_worker_threads_.push_back(std::move(send_worker));
                }
            }
            transfer_state_ = TransferStates::OK;
        transfer_state_mutex_.unlock();

        // err("new connection established: fd=%d", new_conn);

    }

    //info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
}

void SocketServer::CommRecvWorker(int connection_fd) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    bool        ioRes;
    bin_data_t  msg;

    for (; ; ) {
    
        if ( stopping_flag_ ) {
            // info("Stop Flag triggered at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        size_t msgLen = 0;

        ioRes = RecvFrame (connection_fd, &msgLen, sizeof(msgLen), 0);
        if ( !ioRes ) {
            break;
        }

        msg.resize(msgLen);

        ioRes = RecvFrame (connection_fd, msg.data(), msgLen, 0);
        if ( !ioRes ) {
            // err("Error: RecvFrame failed at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }

        read_queue_lock_.lock();
            read_message_queue_.push(msg);
        read_queue_lock_.unlock();
    }

    // err("no more connection!");
    shutdown(connection_fd, SHUT_RDWR);
    close(connection_fd);

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
}

void SocketServer::CommSendWorker(int connection_fd) {

    // info("Enter: (%s):(%d)", __FUNCTION__, __LINE__);

    bool        msgReceived = false;
    bool        ioRes;
    bin_data_t  msg;

    while ( ! stopping_flag_ ) {

        msgReceived = false;

        write_queue_lock_.lock();
            if (!write_message_queue_.empty()) {
                msgReceived = true;
                msg = write_message_queue_.front();
                write_message_queue_.pop();
            }
        write_queue_lock_.unlock();

        if ( !msgReceived ) {
            std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
            continue;
        }

        ioRes = SendFrame(connection_fd, msg.data(), msg.size(), 0);
        if (!ioRes) {
            // err("Error: SendFrame failed at %s (%d)", __FUNCTION__, __LINE__);
            break;
        }
    }

    // err("no more connection!");

    shutdown(connection_fd, SHUT_RDWR);
    close(connection_fd);

    // info("Leave (%s):(%d)", __FUNCTION__, __LINE__);
}

void SocketServer::ConfigureTimeout(const int fd, unsigned int timeout_ms) {

    timeval timeout_cfg{};

    if (timeout_ms == 0) {
        return;
    }

    timeout_cfg.tv_sec = (timeout_ms / 1000);
    timeout_cfg.tv_usec = (timeout_ms % 1000) * 1000;

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout_cfg, sizeof(timeout_cfg));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_cfg, sizeof(timeout_cfg));
}
