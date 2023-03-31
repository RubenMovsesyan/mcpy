#ifndef IMU_CALIBRATION_H
#define IMU_CALIBRATION_H

#include "vector3.h"
#include "imu_info.h"

#define STANDARD_GRAV               9.81

class IMUCalibration {
    public:
        IMUCalibration();
        //                          These should be calibrated before gravity
        void                        calibrateX(IMUInfo inf);
        void                        calibrateY(IMUInfo inf);
        void                        calibrateZ(IMUInfo inf);
        
        //                          This calibrates which direction is up based on how the user is holding it
        void                        calibrateGrav(IMUInfo inf);

        //                          Once the bias is calculated it can be obtained with the following functions
        Vector3                     getBias();
        float                       getBiasX();
        float                       getBiasY();
        float                       getBiasZ();
    private:
        Vector3                     grav_vec, scale_vec;
        bool                        calibration_x_set, calibration_y_set, calibration_z_set;
        bool                        calibration_grav_set;
};

#endif