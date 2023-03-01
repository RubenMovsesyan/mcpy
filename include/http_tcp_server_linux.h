#ifndef HTTP_TCP_SERVER_LINUX_H
#define HTTP_TCP_SERVER_LINUX_H

// includes for TcpServer
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>

// working in the http namespace
namespace http {
    class TcpServer {
        public:
            // initialize TcpServer with ip address and port num
            TcpServer(std::string ip_address, int port);
            ~TcpServer();
            // user functions
            void startListen();
        private:
            std::string             m_ip_address;
            int                     m_port;
            int                     m_socket;
            int                     m_new_socket;
            long                    m_incoming_message;
            struct sockaddr_in      m_socket_address;
            unsigned int            m_socket_address_len;
            std::string             m_server_message;

            int startServer();
            void closeServer();
            void acceptConnection(int &new_socket);
            std::string buildResponse();
            void sendResponse();
    };
}

#endif