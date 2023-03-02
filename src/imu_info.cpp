#include "imu_info.h"
#include <vector>
#include <stdio.h>

// These are the indexes for the acceleration and gyroscope values in the IMU Info format
#define ACC_X_IND 2
#define ACC_Y_IND 4
#define ACC_Z_IND 6

#define GYR_X_IND 10
#define GYR_Y_IND 12
#define GYR_Z_IND 14

IMUInfo::IMUInfo(float acc_x, float acc_y, float acc_z, float gyr_x, float gyr_y, float gyr_z) : 
                m_acc_x(acc_x),
                m_acc_y(acc_y),
                m_acc_z(acc_z),
                m_gyr_x(gyr_x),
                m_gyr_y(gyr_y),
                m_gyr_z(gyr_z) {}

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

    m_acc_x = std::stof(split_string[ACC_X_IND]);
    m_acc_y = std::stof(split_string[ACC_Y_IND]);
    m_acc_z = std::stof(split_string[ACC_Z_IND]);

    m_gyr_x = std::stof(split_string[GYR_X_IND]);
    m_gyr_y = std::stof(split_string[GYR_Y_IND]);
    m_gyr_z = std::stof(split_string[GYR_Z_IND]);
}

void IMUInfo::print() {
    printf("Acceleration:\n");
    // printf("\tX: %05.2f\n\tY: %05.2f\n\tZ: %05.2f\n", m_acc_x, m_acc_y, m_acc_z);
    printf("\tX: %f\n\tY: %f\n\tZ: %f\n", m_acc_x, m_acc_y, m_acc_z);
    printf("Rotation:\n");
    // printf("\tX: %05.2f\n\tY: %05.2f\n\tZ: %05.2f\n", m_gyr_x, m_gyr_y, m_gyr_z);
    printf("\tX: %f\n\tY: %f\n\tZ: %f\n", m_gyr_x, m_gyr_y, m_gyr_z);
}


float IMUInfo::getAX() { return m_acc_x; }
float IMUInfo::getAY() { return m_acc_y; }
float IMUInfo::getAZ() { return m_acc_z; }

float IMUInfo::getGX() { return m_gyr_x; }
float IMUInfo::getGY() { return m_gyr_y; }
float IMUInfo::getGZ() { return m_gyr_z; }