#include "udp_server.h"
#include "logging.h"


UDPServer::UDPServer(int port) {
    log("Starting UDP Server");
    m_port = port;

    char buffer[BUFFER_SIZE];

    // creating socket file descriptor
    if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exitWithError("socket creation failed");
    }
    log("Socket Created");

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
    log("Socket bound");
}

UDPServer::UDPServer(int port, char* ip_address) {
    m_ip_address = ip_address;

    log("Starting UDP Server");
    m_port = port;

    char buffer[BUFFER_SIZE];

    // creating socket file descriptor
    if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exitWithError("socket creation failed");
    }
    log("Socket Created");
	
    int broadcast = 1;
    setsockopt(m_sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    log("Broadcast setup");

    memset(&m_server_addr, 0, sizeof(m_server_addr));
    memset(&m_client_addr, 0, sizeof(m_client_addr));

    // Filling server information
    m_server_addr.sin_family              = AF_INET; // IPv4
    m_server_addr.sin_addr.s_addr         = inet_addr(m_ip_address);
    //m_server_addr.sin_addr.s_addr 	  = htonl(INADDR_ANY);
    m_server_addr.sin_port                = htons(m_port);

    // Bind the socket with the server address
    if (bind(m_sock_fd, (const struct sockaddr*) &m_server_addr, sizeof(m_server_addr)) < 0) {
        exitWithError("bind failed");
    }
    log("Socket bound");
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
    printf("Sent: %s to %s\n", message, m_ip_address);
    // log("Message sent to client");
}
