#ifndef MCPY_SETUP_H
#define MCPY_SETUP_H

/*

    Need to implement logic to work with multiple devices and not just 1

*/

#include "udp_server.h"
#include "c_queue.h"
#include "imu_info.h"
#include "imu_calibration.h"
#include "kinematics.h"
#include <atomic>

// debug
#include <stdio.h>

#define CALIBRATION_ACCURACY        200

// global varaibles bad >:( find better way to do this
std::atomic_bool                    mcpy_running;
CQueue<IMUInfo>                     con_queue;
IMUCalibration                      imu_calibration;
Kinematics                          kin;

void mcpy_init() {
    mcpy_running.store(true);
}
// We are going to be reading and adding to our position concurrently as to be able to work with multiple devices at the same time

// Here we read from the remote device and queue it into our buffer
void deviceReader() {
    UDPServer server(8080);

    // probably should change this name as to not confuse with con_queue
    char* buffer;
    IMUInfo inf;

    // while mcpy is running we will take all data that is recieved from the server
    // and enqueue it into our buffer
    printf("Here 1\n");
    while (mcpy_running.load()) {
        buffer = server.recieve();
        // convert the raw string to IMUInfo before queuing
        inf = IMUInfo(buffer);
        con_queue.enq(inf);
        inf.printOneLineAcc();
    }
    printf("Here 2\n");
}

// This takes info from the queue and updates our current position based on that
void posUpdater() {
    IMUInfo inf;
    while (mcpy_running.load()) {
        inf = con_queue.deq();
        inf.setCalibration(imu_calibration.getGrav(), imu_calibration.getBias());
        kin.addAcc(inf);
    }
}

// This is a private function used to get the average values across a 
// set number of queued acc vals
static IMUInfo getAvgIMUInfo() {
    IMUInfo ret;
    for (int i = 0; i < CALIBRATION_ACCURACY; i++) { ret.addInf(con_queue.deq()); }
    ret.divInf(CALIBRATION_ACCURACY);
    return ret;
}

// These functions might not be thread safe
// TAKE CAUTION WHEN USING
// ----
void calibrateX() {
    con_queue.clear();
    IMUInfo avg_inf = getAvgIMUInfo();
    imu_calibration.calibrateX(avg_inf);
}

void calibrateY() {
    con_queue.clear();
    IMUInfo avg_inf = getAvgIMUInfo();
    imu_calibration.calibrateY(avg_inf);
}

void calibrateZ() {
    con_queue.clear();
    IMUInfo avg_inf = getAvgIMUInfo();
    imu_calibration.calibrateZ(avg_inf);
}

void calibrateGrav() {
    con_queue.clear();
    IMUInfo avg_inf = getAvgIMUInfo();
    imu_calibration.calibrateGrav(avg_inf);
}
// ----

#endif