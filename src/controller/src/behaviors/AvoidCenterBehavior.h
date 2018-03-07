#ifndef AVOID_CENTER_BEHAVIOR_H
#define AVOID_CENTER_BEHAVIOR_H


#include "../behaviors/BehaviorInterface.h"

class AvoidCenterBehavior : public Behavior{
    enum Stage{
        WAIT = 0,
        TURN,
        DRIVE
    };
    Stage stage;

    float directionToDrive = 0;

    bool turnLock = false;
    bool isLeftTurn = false;


    public:
        AvoidCenterBehavior() : Behavior(AVOID_CENTER_BEHAVIOR_TYPE){
            waitTime = 1;
            stage = WAIT;
        }
        bool tick();


};


#endif
