#ifndef IMU_CALIBRATION_H
#define IMU_CALIBRATION_H

#include "vector3.h"
#include "imu_info.h"

#define STANDARD_GRAV               9.81

class IMUCalibration {
    public:
        IMUCalibration();
        //                          These should be calibrated before gravity
        bool                        calibrateX(IMUInfo inf);
        bool                        calibrateY(IMUInfo inf);
        bool                        calibrateZ(IMUInfo inf);
        
        //                          This calibrates which direction is up based on how the user is holding it
        bool                        calibrateGrav(IMUInfo inf);

        //                          Once the bias is calculated it can be obtained with the following functions
        Vector3                     getBias();
        float                       getBiasX();
        float                       getBiasY();
        float                       getBiasZ();

        Vector3                     getGrav();
        float                       getGravX();
        float                       getGravY();
        float                       getGravZ();
    private:
        Vector3                     grav_vec, bias_vec;
        bool                        calibration_x_set, calibration_y_set, calibration_z_set;
        bool                        calibration_grav_set;
};

#endif