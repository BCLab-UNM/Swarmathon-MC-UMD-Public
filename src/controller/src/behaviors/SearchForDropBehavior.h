#ifndef SEARCH_FOR_DROP_BEHAVIOR_H
#define SEARCH_FOR_DROP_BEHAVIOR_H

#include "BehaviorInterface.h"

#include "../controllers/DriveController.h"
#include "../controllers/ClawController.h"
#include "../handlers/Handlers.h"
#include "DriveBehavior.h"
#include "../SMACS.h"
#include <cstdlib>
#include <ctime>
#include <random_numbers/random_numbers.h>

class SearchForDropBehavior : public Behavior{
    enum Stages{
        ODOM_TARGET = 0,
        TURN_TO_THETA,
        SEARCH_FOR_CENTER,
        SEARCH_CIRCLE,
        GPS_TARGET,
        SEARCH,
        CHECK_FOR_CUBE,
        DRIVE,
        DROP
    };

   Stages stage = ODOM_TARGET;
   int searchTry = 0;
   float distance = 0.25;
   float theta;
   float x, y;

   int left = 10;
   int right = 5;

   long initTime;

   bool checkCube = true;
   bool starSearch = true;
   bool circleSearch = true;
   bool gpsSearch = true;

   bool cubeChecked = false;

   random_numbers::RandomNumberGenerator* rng;

    public:
        SearchForDropBehavior() : Behavior(SEARCH_FOR_DROP_BEHAVIOR_TYPE){
            srand(time(NULL));
            rng = new random_numbers::RandomNumberGenerator();
        }

        bool tick();
};














#endif
