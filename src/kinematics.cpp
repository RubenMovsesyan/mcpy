#include "kinematics.h"
#include "delta_t.h"
#include <stdio.h>

Kinematics::Kinematics() {
    m_pos = {0, 0, 0};
    m_vel = {0, 0, 0};
    m_acc = {0, 0, 0};
}

Vector3 Kinematics::getPos() {
    return m_pos;
}

Vector3 Kinematics::getVel() {
    return m_vel;
}

Vector3 Kinematics::getAcc() {
    return m_acc;
}

void Kinematics::setPos(Vector3 pos) {
    m_pos = pos;
}

void Kinematics::setVel(Vector3 vel) {
    m_vel = vel;
}

void Kinematics::setAcc(Vector3 acc) {
    m_acc = acc;
}

// Helper function for addAcc
Vector3 getAccVec(IMUInfo acc) {
    Vector3 ret;

    ret.x = acc.getAX();
    ret.y = acc.getAY();
    ret.z = acc.getAZ();

    return ret;
}

// This is the most important function
void Kinematics::addAcc(IMUInfo acc) {
    m_pos.x += m_vel.x * DELTA_T;
    m_pos.y += m_vel.y * DELTA_T;
    m_pos.z += m_vel.z * DELTA_T;

    m_vel.x += m_acc.x * DELTA_T;
    m_vel.y += m_acc.y * DELTA_T;
    m_vel.z += m_acc.z * DELTA_T;

    m_acc = getAccVec(acc);
}

void Kinematics::printKin() {
    printf("\nKinematic Info:\n");
    printf("Position:        Velocity:        Acceleration:\n");
    printf("X: %05.2f         %05.2f            %05.2f\n", m_pos.x, m_vel.x, m_acc.x);
    printf("Y: %05.2f         %05.2f            %05.2f\n", m_pos.y, m_vel.y, m_acc.y);
    printf("Z: %05.2f         %05.2f            %05.2f\n", m_pos.z, m_vel.z, m_acc.z);
}