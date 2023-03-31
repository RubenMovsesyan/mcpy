#include "imu_calibration.h"

IMUCalibration::IMUCalibration() {
    calibration_x_set = false;
    calibration_y_set = false;
    calibration_z_set = false;
    calibration_grav_set = false;
}

bool IMUCalibration::calibrateX(IMUInfo inf) {
    bias_vec.x = STANDARD_GRAV / inf.getAX();
    calibration_x_set = true;

    return calibration_x_set;
}

bool IMUCalibration::calibrateY(IMUInfo inf) {
    bias_vec.y = STANDARD_GRAV / inf.getAY();
    calibration_y_set = true;

    return calibration_y_set;
}

bool IMUCalibration::calibrateZ(IMUInfo inf) {
    bias_vec.z = STANDARD_GRAV / inf.getAZ();
    calibration_z_set = true;

    return calibration_z_set;
}

bool IMUCalibration::calibrateGrav(IMUInfo inf) {
    if (!calibration_x_set || !calibration_y_set || !calibration_z_set) { return false; }

    grav_vec.x = bias_vec.x * inf.getAX();
    grav_vec.y = bias_vec.y * inf.getAY();
    grav_vec.z = bias_vec.z * inf.getAZ();
    calibration_grav_set = true;
    return calibration_grav_set;
}

bool IMUCalibration::isCalibrated() {
    return calibration_x_set && calibration_y_set && calibration_z_set && calibration_grav_set;
}

// Getters for the bias
Vector3 IMUCalibration::getBias() { return bias_vec; }
float IMUCalibration::getBiasX() { return bias_vec.x; }
float IMUCalibration::getBiasY() { return bias_vec.y; }
float IMUCalibration::getBiasZ() { return bias_vec.z; }

// Getters for the grav
Vector3 IMUCalibration::getGrav() { return grav_vec; }
float IMUCalibration::getGravX() { return grav_vec.x; }
float IMUCalibration::getGravY() { return grav_vec.y; }
float IMUCalibration::getGravZ() { return grav_vec.z; }