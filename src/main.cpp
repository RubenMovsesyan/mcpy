#include <stdio.h>

// my include libraries
#include "http_tcp_server_linux.h"

using namespace http;

int main() {
    printf("This is the server starting\n");
    TcpServer server = TcpServer("10.0.0.147", 8080);
    server.startListen();
    return 0;
}