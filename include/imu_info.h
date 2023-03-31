#ifndef IMU_INFO_H
#define IMU_INFO_H

#include "imu_calibration.h"
#include "vector3.h"
#include <string>

class IMUInfo {
    public:
        // Default constructor
        IMUInfo();

        IMUInfo(float acc_x, float acc_y, float acc_z, float gyr_x, float gyr_y, float gyr_z);

        // This is the parser contructor
        IMUInfo(std::string string_to_parse);

        void                        setCalibration(IMUCalibration cal);
        void                        print();

        void                        printOneLineAcc();
        // These functions should only be used for calibration purposes 
        // ----
        void                        addInf(IMUInfo inf);
        // Divides the IMUInfo by the scalar val
        void                        divInf(float val);
        // ----


        // getter functions for IMU info class
        float                       getAX();
        float                       getAY();
        float                       getAZ();

        float                       getGX();
        float                       getGY();
        float                       getGZ();
    private:
        // accelerometer values
        // float                       m_acc_x, m_acc_y, m_acc_z;
        Vector3                     m_acc;
        // gyroscope values
        // float                       m_gyr_x, m_gyr_y, m_gyr_z;
        Vector3                     m_gyr;

        IMUCalibration              m_cal;
};

#endif