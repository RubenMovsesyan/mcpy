#ifndef BROADCAST_SERVER
#define BROADCAST_SERVER

#include "udp_server.h"

class BroadcastServer : public UDPServer {
    public:
        BroadcastServer(int port, char* message);
        ~BroadcastServer();

        void broadcast(bool* running, int delay);

        void setServerIP();
        void broadcastIP(bool* running, int delay);
    private:
        char*                       m_message;
        char                        m_ip[16];
};

#endif