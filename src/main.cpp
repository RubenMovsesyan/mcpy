#include <stdio.h>
#include <thread>
#include <vector>
#include <fstream>

// my include libraries
// #include "http_tcp_server_linux.h"
#include "udp_server.h"
// #include "broadcast_server.h"
// #include "imu_info.h"
#include "kinematics.h"

int main() {    
    // UDPServer server(8080);

    // char* buffer;

    // while (true) {
    //     buffer = server.recieve();
	//     printf("%s", buffer);
    // }


    // Temporary for testing 3D motion tracking
    std::vector<IMUInfo> imu_info;
    Kinematics k = Kinematics();

    std::fstream new_file;
    
    new_file.open("res/short_standing_still.imuinfo");

    if (new_file.is_open()) {
        std::string sa;

        while (getline(new_file, sa)) {
            imu_info.push_back(IMUInfo(sa));
        }
    }

    new_file.close();

    for (int i = 0; i < imu_info.size(); i++) {
        k.addAcc(imu_info[i]);
        k.printKin();
    }

    printf("Size: %d\n", imu_info.size());
    // Temporary for testing 3D motion tracking

    return 0;
}
