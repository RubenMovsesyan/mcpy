#include "broadcast_server.h"
#include "logging.h"

#include <chrono>
#include <thread>


// Start a udp server on the broadcast ip address
BroadcastServer::BroadcastServer(int port, char* message) : UDPServer(port, "10.0.0.255") {
    m_message = message;

    if(setsockopt(m_sock_fd, SOL_SOCKET, SO_BROADCAST, m_message, sizeof(m_message)) < 0) {
        exitWithError("Error in setting Broadcast option\n");
    }
}

BroadcastServer::~BroadcastServer() {}

void BroadcastServer::broadcast(bool* running, int delay) {
    while(*running) {
        send(m_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}