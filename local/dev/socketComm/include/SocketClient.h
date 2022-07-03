#ifndef __SOCKETCLIENT_H__
#define __SOCKETCLIENT_H__

#include <thread>
#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <stdio.h>

#include <CommSocket.h>

#include <application/types.h>
#include <application/stuff.h>

class SocketClient {

    public:
        SocketClient();
        SocketClient(const SocketClient&) = delete;
        ~SocketClient();

    public:
        SocketClient(SocketClient&&) noexcept;
        SocketClient& operator=(SocketClient&&) noexcept;
        SocketClient& operator=(const SocketClient&) = delete;

    public:
        bool InitSocket(const std::string& socket_name, bool use_unix_socket = true, bool use_async_mode = false);
        bool StartClient();
        bool StopClient();

    public:
        // Section to send message to SocketServer
        // See SocketServer::RecvMessageAsync and SocketServer::RecvMessage

        bool SendMessageAsync (                                     // SocketServer will receive <LEN> <MSG>
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen           // Message length.
        );                                                          // 
                                                                 
        bool SendMessage (                                          // SocketServer will receive <LEN> <MSG>
            IN      MANDATORY  const void* const   msgPtr,          // Pointer to message to send.
            IN      MANDATORY  unsigned int        msgLen,          // Message length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds. <0> - Infinite.
        );                                                          // 
                                                                 
        bool SendFrame(                                             // 
            IN      MANDATORY  const void* const    msgPtr,         // Pointer to message to send.
            IN      MANDATORY  unsigned int         msgLen,         // Message length.
            IN      MANDATORY  unsigned int         timeout_ms      // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 

    public:                                                      

        // Section to receive messages from SocketServer
        // See SocketServer::SendMessageAsync and SocketServer::SendMessage

        bool RecvMessageAsync (                                     // 
            IN      MANDATORY  void* const         msgPtr,          // Pointer to message to send.
            IN_OUT  MANDATORY  unsigned int&       msgLen           // Message length.
        );                                                          // 
                                                                    
        bool RecvMessage (                                          // 
            IN      MANDATORY  void* const         msgPtr,          // Pointer to message to send.
            IN_OUT  MANDATORY  unsigned int&       msgLen,          // Message length.
            IN      OPTIONAL   unsigned int        timeout_ms = 0   // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 
                                                                 
        bool RecvFrame (                                            // 
            IN      MANDATORY  void* const         msgPtr,          // Pointer to message to send.
            IN_OUT  MANDATORY  unsigned int        msgLen,          // Message length.
            IN      MANDATORY  unsigned int        timeout_ms       // Timeout, in Milliseconds.  <0> - Infinite.
        );                                                          // 
                                                                    
    private:
        void CommRecvWorker();
        void CommSendWorker();
        void ConfigureTimeout ( unsigned int timeout_ms );

    private:
        std::atomic_bool    stopping_flag_{false};
        bool                is_full_duplex_mode_ = false;
        int                 client_file_descriptor_;
        std::thread         client_read_thread_;
        std::thread         client_write_thread_;
        sockaddr_un         server_sock_ {};
        sockaddr_un         client_sock_ {};
        SpinLock            read_queue_lock_;
        SpinLock            write_queue_lock_ ;

        msg_queue_t         read_message_queue_ {};
        msg_queue_t         write_message_queue_ {};
};

#endif
