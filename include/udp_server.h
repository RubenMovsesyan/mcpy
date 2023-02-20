#ifndef UDP_SERVER
#define UDP_SERVER

// #include <its/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>

// The buffer size is a lot smaller for the UDP side because we are streaming data
#define BUFFER_SIZE 1024

// The UDP server will send and recieve messages with the UDP protocol
class UDPServer {
    public:
        UDPServer(int port);
        ~UDPServer();

        // TODO: make UDP work with diffent types of buffers and not just char
        // recieve will place the message from the client in m_buffer and return it
        char* recieve();
        // send will send the specified message the client
        void send(char* message);
    protected:
        int m_port;
        struct sockaddr_in m_server_addr, m_client_addr;
        socklen_t m_sock_len;
        int m_sock_fd;
        char m_buffer[BUFFER_SIZE];
        char* m_ip_address;

        UDPServer(int port, char* ip_address);
};

#endif