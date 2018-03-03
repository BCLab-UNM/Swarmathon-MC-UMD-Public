#ifndef SEARCH_FOR_DROP_BEHAVIOR_H
#define SEARCH_FOR_DROP_BEHAVIOR_H

#include "BehaviorInterface.h"

#include "../controllers/DriveController.h"
#include "../controllers/ClawController.h"
#include "../handlers/Handlers.h"
#include "DriveBehavior.h"
#include "../SMACS.h"

class SearchForDropBehavior : public Behavior{
    enum Stages{
        ODOM_TARGET = 0,
        SEARCH_FOR_CENTER,
        GPS_TARGET,
        SEARCH,
        ASK,
        DROP
    };

   Stages stage = ODOM_TARGET;
   int searchTry = 0;
   int distance = 1;
   float theta;
   float x, y;

    public:
        SearchForDropBehavior() : Behavior(SEARCH_FOR_DROP_BEHAVIOR_TYPE){

        }

        bool tick();
};














#endif
