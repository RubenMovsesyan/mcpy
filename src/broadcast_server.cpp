#include "broadcast_server.h"
#include "logging.h"

#include <chrono>
#include <thread>
#include <net/if.h>
#include <string>
#include <sys/ioctl.h>


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

void BroadcastServer::broadcastIP(bool* running, int delay) {
    while(*running) {
        send(m_ip);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void BroadcastServer::setServerIP() {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    // I want to get the IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    // I want the ip address attached to wlan0
    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ -1 );
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    sprintf(m_ip, "%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
