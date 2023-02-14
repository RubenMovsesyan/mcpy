#include "http_tcp_server_linux.h"

// extra includes for cpp file
#include <iostream>
#include <sstream>
#include <unistd.h>


namespace {
    const int BUFFER_SIZE = 30720;

    void log(const std::string &message) {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &error_message) {
        log("ERROR: " + error_message);
        exit(1);
    }
}

namespace http {
    TcpServer::TcpServer(std::string ip_address, int port) : 
        m_ip_address(ip_address),
        m_port(port),
        m_socket(),
        m_new_socket(),
        m_incoming_message(),
        m_socket_address(),
        m_socket_address_len(sizeof(m_socket_address)),
        m_server_message(buildResponse()) 
    {
        // Here we are naming the socket
        m_socket_address.sin_family =               AF_INET;
        m_socket_address.sin_port =                 htons(m_port);
        m_socket_address.sin_addr.s_addr =          inet_addr(m_ip_address.c_str());
        
        if (startServer()) {
            std::ostringstream ss;
            ss << "Failed to start server with PORT: " << ntohs(m_socket_address.sin_port);
            log(ss.str());
        }
    }

    TcpServer::~TcpServer() {
        closeServer();
    }

    int TcpServer::startServer() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            exitWithError("cannot create socket");
            return 1;
        }

        if (bind(m_socket, (sockaddr*)&m_socket_address, m_socket_address_len) < 0) {
            exitWithError("Cannot connect socket to address");
            return 1;
        }

        return 0;
    }

    void TcpServer::closeServer() {
        close(m_socket);
        close(m_new_socket);
        exit(0);
    }

    void TcpServer::startListen() {
        if (listen(m_socket, 20) < 0) {
            exitWithError("Socket listen failed");
        }

        std::ostringstream ss;
        ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_socket_address.sin_addr) << " PORT: " << ntohs(m_socket_address.sin_port) << " ***\n\n";
        log(ss.str());

        int bytes_received;

        while(true) {
            log("====== Waiting for a new connection ======\n\n\n");
            acceptConnection(m_new_socket);

            char buffer[BUFFER_SIZE] = {0};
            bytes_received = read(m_new_socket, buffer, BUFFER_SIZE);
            if (bytes_received < 0) {
                exitWithError("Failed to read bytes from client socket connection");
            }

            std::ostringstream ss;
            ss << "------ Received Request from client ------\n\n";
            log(ss.str());

            sendResponse();

            close(m_new_socket);
        }
    }

    void TcpServer::acceptConnection(int &new_socket) {
        new_socket = accept(m_socket, (sockaddr*)&m_socket_address, &m_socket_address_len);
        if (new_socket < 0) {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socket_address.sin_addr) << "; PORT: " << ntohs(m_socket_address.sin_port);
            exitWithError(ss.str());
        }
    }

    // TODO: edit this function to create the webpage for the server
    std::string TcpServer::buildResponse() {
        std::string html_file = "<DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html_file.size() << "\n\n" << html_file;

        return ss.str();
    }

    void TcpServer::sendResponse() {
        long bytes_sent;

        bytes_sent = write(m_new_socket, m_server_message.c_str(), m_server_message.size());

        if (bytes_sent == m_server_message.size()) {
            log("------ Server Response sent to client ------\n\n");
        } else {
            log("Error sending response to client");
        }
    }
}