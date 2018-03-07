#ifndef AVOID_CENTER_BEHAVIOR_H
#define AVOID_CENTER_BEHAVIOR_H


#include "../behaviors/BehaviorInterface.h"
#include <vector>
#include "../controllers/DriveController.h"
#include "../handlers/Handlers.h"

using namespace std;

class AvoidCenterBehavior : public Behavior{
    enum Stage{
        WAIT = 0,
        TURN,
        DRIVE
    };
    Stage stage;


    float directionToDrive = 0;
    float camera_offset_correction = 0.020; //meters;

    bool turnLock = false;
    bool isLeftTurn = false;

    vector<Tag> centerTags;


    public:
        AvoidCenterBehavior() : Behavior(AVOID_CENTER_BEHAVIOR_TYPE){
            stage = WAIT;
        }
        bool tick();
        void setTagData(vector<Tag> tags);
        bool checkForCollectionZoneTags(vector<Tag> tags);

};


#endif
