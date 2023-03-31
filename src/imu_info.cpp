#include "imu_info.h"
#include <vector>
#include <stdio.h>

// These are the indexes for the acceleration and gyroscope values in the IMU Info format
// Might need to change these if the information that is sent is changed
#define ACC_X_IND 2
#define ACC_Y_IND 4
#define ACC_Z_IND 6

#define GYR_X_IND 10
#define GYR_Y_IND 12
#define GYR_Z_IND 14

IMUInfo::IMUInfo() :
                m_acc( { 0.0, 0.0, 0.0 } ),
                m_gyr( { 0.0, 0.0, 0.0 } ) {}

IMUInfo::IMUInfo(float acc_x, float acc_y, float acc_z, float gyr_x, float gyr_y, float gyr_z) : 
                m_acc( { acc_x, acc_y, acc_z } ),
                m_gyr( { gyr_x, gyr_y, gyr_z } ) {}

// helper function for IMUInfo
std::vector<std::string> splitString(std::string string_to_split, char delimeter) {
    std::vector<std::string> out;
    
    std::string word = "";
    for (auto x : string_to_split) {
        if (x == delimeter) {
            out.push_back(word);
            word = "";
        } else {
            word += x;
        }
    }
    return out;
}

IMUInfo::IMUInfo(std::string string_to_parse) {
    std::vector<std::string> split_string = splitString(string_to_parse, ' ');

    m_acc.x = std::stof(split_string[ACC_X_IND]);
    m_acc.y = std::stof(split_string[ACC_Y_IND]);
    m_acc.z = std::stof(split_string[ACC_Z_IND]);

    m_gyr.x = std::stof(split_string[GYR_X_IND]);
    m_gyr.y = std::stof(split_string[GYR_Y_IND]);
    m_gyr.z = std::stof(split_string[GYR_Z_IND]);
}

void IMUInfo::setCalibration(IMUCalibration cal) {
    m_cal = cal;

    m_acc.x *= m_cal.getBiasX();
    m_acc.y *= m_cal.getBiasY();
    m_acc.z *= m_cal.getBiasZ();

    m_acc.x -= m_cal.getGravX();
    m_acc.y -= m_cal.getGravY();
    m_acc.z -= m_cal.getGravZ();

    // Might need to implement the same thing for attitude once that is figured out
}

void IMUInfo::print() {
    printf("Acceleration:\n");
    printf("\tX: %05.2f\n\tY: %05.2f\n\tZ: %05.2f\n", m_acc.x, m_acc.y, m_acc.z);
    printf("Rotation:\n");
    printf("\tX: %05.2f\n\tY: %05.2f\n\tZ: %05.2f\n", m_gyr.x, m_gyr.y, m_gyr.z);
    // printf("\tX: %f\n\tY: %f\n\tZ: %f\n", m_gyr_x, m_gyr_y, m_gyr_z);
}

void IMUInfo::printOneLineAcc() {
    printf("\rX: %05.2f Y: %05.2f Z: %05.2f | ");
}

void IMUInfo::addInf(IMUInfo inf) {
    m_acc.x += inf.m_acc.x;
    m_acc.y += inf.m_acc.y;
    m_acc.z += inf.m_acc.z;

    m_gyr.x += inf.m_gyr.x;
    m_gyr.y += inf.m_gyr.y;
    m_gyr.z += inf.m_gyr.z;
}

void IMUInfo::divInf(float val) {
    m_acc.x /= val;
    m_acc.y /= val;
    m_acc.z /= val;

    m_gyr.x /= val;
    m_gyr.y /= val;
    m_gyr.z /= val;
}


float IMUInfo::getAX() { return m_acc.x; }
float IMUInfo::getAY() { return m_acc.y; }
float IMUInfo::getAZ() { return m_acc.z; }

float IMUInfo::getGX() { return m_gyr.x; }
float IMUInfo::getGY() { return m_gyr.y; }
float IMUInfo::getGZ() { return m_gyr.z; }