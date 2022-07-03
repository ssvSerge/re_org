#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

#include <fstream>
#include <mutex>
#include <functional>
#include <queue>
#include <atomic>

#include <CommSocket.h>

#include <application/types.h>
#include <application/stuff.h>


using MsgHandleFunctionPrototype = void(int file_descriptor);

class SocketServer {

    public:
        SocketServer();
        SocketServer(SocketServer&&) noexcept;
        SocketServer(const SocketServer&) = delete;
        ~SocketServer();

    public:
        SocketServer& operator=(SocketServer&&) noexcept;
        SocketServer& operator= (const SocketServer&) = delete;

    public:
        bool InitSocket(const std::string& socket_name, bool allow_multi_conn, bool use_unix_socket = true, bool use_async_mode = false);
        void SetMaxConnections(unsigned int max_conn);
        void BindConnectionHandleFunction(std::function<MsgHandleFunctionPrototype> function_instance);
        TransferStates ConnectState();

    public:
        bool StartServer();
        bool StopServer();
        
    public:
        // Section to send message to SocketClient
        // See SocketClient::RecvMessageAsync and SocketClient::RecvMessage

        bool SendMessageAsync (                                     // SocketClient will receive <LEN> <MSG>
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen           // Message length.
        );                                                          // 
                                                                    
        bool SendMessage (                                          // SocketClient will receive <LEN> <MSG>
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen,          // Message length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 
                                                                    
        bool SendMessage (                                          // SocketClient will receive <LEN> <MSG>
            IN      MANDATORY  const int           fd,              // SOCKET to send.
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen,          // Message length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          //
                                                 
        bool SendFrame(                                             // Helper function. Send the RAW frame.
            IN      MANDATORY  const int           fd,              // SOCKET to send.
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen,          // Length of the RAW frame to send.
            IN      OPTIONAL   unsigned int        timeout_ms       // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 
                                                                    
    public:

        // Section to receive messages from SocketClient
        // See SocketClient::SendMessageAsync and SocketClient::SendMessage

        bool RecvMessageAsync (                                     // Client will send <LEN> <MSG>
            IN      MANDATORY  void* const         msgPtr,          // Pointer where to store <MSG>
            IN_OUT  MANDATORY  unsigned int&       msgLen           // Input: Allocated length; Output: Real length.
        );                                                          // 
                                                                    
        bool RecvMessage (                                          // Client will send <LEN> <MSG>
            IN      MANDATORY  void* const         msgPtr,          // Pointer where to store <MSG>
            IN_OUT  MANDATORY  unsigned int&       msgLen,          // Input: Allocated length; Output: Real length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 

        bool RecvMessage (                                          // Client will send <LEN> <MSG>
            IN      MANDATORY  const int           fd,              // SOCKET to receive.
            IN      MANDATORY  void* const         msgPtr,          // Pointer where to store <MSG>
            IN_OUT  MANDATORY  unsigned int&       msgLen,          // Input: Allocated length; Output: Real length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 

        bool RecvFrame(                                             // Helper function. Receive raw frame.
            IN      MANDATORY  const int           fd,              // SOCKET to receive.
            IN      MANDATORY  void* const         msgPtr,          // Pointer where to store <MSG>
            IN      MANDATORY  unsigned int        msgLen,          // Input: length of frame to receive & store.
            IN      OPTIONAL   unsigned int        timeout_ms       // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 

    private:
        void ListenerWorkerFunc();
        void CommRecvWorker(int connection_fd);
        void CommSendWorker(int connection_fd);
        void ConfigureTimeout(const int fd, unsigned int timeout_ms);

    private:
        struct sockaddr_un  server_sock_ {};
        struct sockaddr_un  client_sock_ {};

        std::thread         listener_thread_;
        threads_list_t      comm_recv_worker_threads_{};
        threads_list_t      comm_write_worker_threads_{};

        std::vector<int>    comm_worker_file_descriptors_ {};
        unsigned int        max_connection_count_ = 1;

        bool                accepting_multiple_conn_ = false;
        std::atomic_bool    stopping_flag_{false};
        int                 socket_file_descriptor_ = -1;

        msg_queue_t         read_message_queue_ {};
        msg_queue_t         write_message_queue_{};
        SpinLock            read_queue_lock_;
        SpinLock            write_queue_lock_ ;
        std::mutex          transfer_state_mutex_;
        bool                is_async_mode_ = false;
        TransferStates      transfer_state_;

        std::function<MsgHandleFunctionPrototype> on_msg_callback_func = nullptr;
};

#endif
