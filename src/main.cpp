#include <stdio.h>

// my include libraries
// #include "http_tcp_server_linux.h"
// #include "udp_server.h"
#include "broadcast_server.h"

// using namespace http;

int main() {
    // printf("This is the server starting\n");
    // TcpServer server = TcpServer("10.0.0.147", 8080);
    // server.startListen();
    UDPServer server(8080);

    char* buffer;

    while (true) {
        buffer = server.recieve();
	//printf("Client sent: %s\n", buffer);
    }
    // BroadcastServer server(8080, "Hi, This is the raspberry pi!");
    
    // bool running = true;

    // server.broadcast(&running, 1000);

    return 0;
}
