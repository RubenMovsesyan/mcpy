#include "udp_server.h"
#include <iostream>

namespace {
    void log(const std::string &message) {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &error_message) {
        log("ERROR: " + error_message);
        exit(1);
    }
}

UDPServer::UDPServer(int port) {
    printf("Starting UDP Server\n");
    m_port = port;

    char buffer[BUFFER_SIZE];

    // creating socket file descriptor
    if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exitWithError("socket creation failed");
    }
    printf("Socket Created\n");

    memset(&m_server_addr, 0, sizeof(m_server_addr));
    memset(&m_client_addr, 0, sizeof(m_client_addr));

    // Filling server information
    m_server_addr.sin_family              = AF_INET; // IPv4
    m_server_addr.sin_addr.s_addr         = INADDR_ANY;
    m_server_addr.sin_port                = htons(m_port);

    // Bind the socket with the server address
    if (bind(m_sock_fd, (const struct sockaddr*) &m_server_addr, sizeof(m_server_addr)) < 0) {
        exitWithError("bind failed");
    }
    printf("Socket bound\n");
}

UDPServer::~UDPServer() {}

char* UDPServer::recieve() {
    int n;
    m_sock_len = sizeof(m_client_addr);

    n = recvfrom(m_sock_fd, (char*)m_buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*) &m_client_addr, &m_sock_len);
    m_buffer[n] = '\0';
    printf("Client: %s\n", m_buffer);

    return m_buffer;
}

void UDPServer::send(char* message) {
    // send the message the client
    sendto(m_sock_fd, (const char*) message, strlen(message), MSG_CONFIRM, (const struct sockaddr*) &m_client_addr, m_sock_len);

    printf("Message sent to client\n");
}