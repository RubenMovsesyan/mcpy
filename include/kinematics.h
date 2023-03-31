#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "imu_info.h"
#include "vector3.h"

class Kinematics {
    public:
        // Constructor
        Kinematics();
        // Getters and setters

        // Getters
        Vector3                     getPos();
        Vector3                     getVel();
        Vector3                     getAcc();

        // Setters
        void                        setPos(Vector3 pos);
        void                        setVel(Vector3 vel);
        void                        setAcc(Vector3 acc);

        // Function to add acceleration with delta_t
        // Don't need to add pos or vel yet as they are not necessary for our purposes
        void                        addAcc(IMUInfo acc);

        // Debugging
        void                        printKin();
    private:
        Vector3                     m_pos;
        Vector3                     m_vel;
        Vector3                     m_acc;
};

#endif