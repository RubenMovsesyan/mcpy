#include <stdio.h>
#include <thread>
#include <vector>
#include <fstream>


// #include <netdb.h>


// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/ioctl.h>
// #include <netinet/in.h>
// #include <net/if.h>
// #include <arpa/inet.h>



// my include libraries
// #include "http_tcp_server_linux.h"
#include "udp_server.h"
// #include "broadcast_server.h"
#include "imu_info.h"

int main() {    
    UDPServer server(8080);

    char* buffer;

    while (true) {
        buffer = server.recieve();
	    printf("%s", buffer);
    }

    // std::vector<IMUInfo> imu_info;

    // std::fstream new_file;
    
    // new_file.open("res/short_standing_still.imuinfo");

    // if (new_file.is_open()) {
    //     std::string sa;

    //     while (getline(new_file, sa)) {
    //         imu_info.push_back(IMUInfo(sa));
    //     }
    // }

    // new_file.close();

    // for (int i = 0; i < imu_info.size(); i++) {
    //     imu_info[i].print();
    // }

    return 0;
}
