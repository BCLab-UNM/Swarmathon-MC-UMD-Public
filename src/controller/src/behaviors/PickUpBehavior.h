#ifndef PICKUP_BEHAVIOR_H
#define PICKUP_BEHAVIOR_H

#include "../handlers/Handlers.h"
#include "../SMACS.h"
#include "../handlers/Tag.h"

#include "../controllers/ClawController.h"

#include "BehaviorInterface.h"
#include "DriveBehavior.h"
#include "SearchForDropBehavior.h"

#include <vector>

#include <time.h>

class PickUpBehavior : public Behavior{
    enum Stages{
        LOCK_TARGET = 0,
        TURN_TO_FACE_TARGET,
        DRIVE_TO_PICK_UP,
        PRECISION_TURN,
        PRECISION_DRIVE,
        PICK_UP,
        RETRY,
        DONE,
        DROP
    };
    Stages currentStage;

    bool targetLocked;
    bool precisionDrive = false;

    float cameraOffsetCorrection = 0.023; //meters
    float blockDistanceFromCamera = 0;
    float blockDistance = 0;
    float blockYawError = 0;

    float initTheta = 0;
    float initX = 0;
    float initY = 0;
    float angleTolerance = 0.0175;

    float driveBackDist = 0.15;

    int driveSpeed = 30;

    int e_set = 150;

    static int leftPos;
    static int rightPos;
    static int leftNeg;
    static int rightNeg;

    int prev_e_left;
    int prev_e_right;

    static long lastCheck;

    bool waiting;
    time_t initTime;
    time_t currTime;


    public:
        PickUpBehavior() : Behavior(PICKUP_BEHAVIOR_TYPE){
            currentStage = LOCK_TARGET;
            targetLocked = false;
        }

        bool tick();

        void fix(int, int);

        bool wait(int sec);
};




#endif
