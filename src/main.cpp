#include <stdio.h>

// my include libraries
// #include "http_tcp_server_linux.h"
#include "udp_server.h"

// using namespace http;

int main() {
    // printf("This is the server starting\n");
    // TcpServer server = TcpServer("10.0.0.147", 8080);
    // server.startListen();
    UDPServer server(8080);

    char* buffer;

    while (true) {
        buffer = server.recieve();
        server.send("The server is sending this message.");
    }

    return 0;
}