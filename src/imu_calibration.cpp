#include "imu_calibration.h"

IMUCalibration::IMUCalibration() {
    calibration_x_set = false;
    calibration_y_set = false;
    calibration_z_set = false;
    calibration_grav_set = false;
}

void IMUCalibration::calibrateX(IMUInfo inf) {
    scale_vec.x = STANDARD_GRAV / inf.getAX();
    calibration_x_set = true;
}

void IMUCalibration::calibrateY(IMUInfo inf) {
    scale_vec.y = STANDARD_GRAV / inf.getAY();
    calibration_y_set = true;
}

void IMUCalibration::calibrateZ(IMUInfo inf) {
    scale_vec.z = STANDARD_GRAV / inf.getAZ();
    calibration_z_set = true;
}

// Getters for the bias
Vector3 IMUCalibration::getBias() { return scale_vec; }
float IMUCalibration::getBiasX() { return scale_vec.x; }
float IMUCalibration::getBiasY() { return scale_vec.y; }
float IMUCalibration::getBiasZ() { return scale_vec.z; }

void IMUCalibration::calibrateGrav(IMUInfo inf) {
    grav_vec.x = scale_vec.x * inf.getAX();
    grav_vec.y = scale_vec.y * inf.getAY();
    grav_vec.z = scale_vec.z * inf.getAZ();
    calibration_grav_set = true;
}