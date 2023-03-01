#ifndef IMU_INFO_H
#define IMU_INFO_H

#include <string>

class IMUInfo {
    public:
        IMUInfo(float acc_x, float acc_y, float acc_z, float gyr_x, float gyr_y, float gyr_z);

        // This is the parser contructor
        IMUInfo(std::string string_to_parse);

        void print();


        // getter functions for IMU info class
        float getAX();
        float getAY();
        float getAZ();

        float getGX();
        float getGY();
        float getGZ();
    private:
        // accelerometer values
        float m_acc_x, m_acc_y, m_acc_z;
        // gyroscope values
        float m_gyr_x, m_gyr_y, m_gyr_z;
};

#endif