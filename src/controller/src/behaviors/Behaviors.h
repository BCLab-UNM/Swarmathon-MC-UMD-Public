#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include "BehaviorInterface.h"

#include "SearchBehavior.h"
#include "CalibrateBehavior.h"
#include "AvoidBehavior.h"
#include "PickUpBehavior.h"
#include "DriveBehavior.h"
#include "SearchForDropBehavior.h"
#include "DropBehavior.h"
#include "SearchAlgorithm.h"
#include "SearchAlgorithmRelative.h"
#include "SearchAlgorithmSixRovers.h"

#include "ros/ros.h"
#include "../controllers/DriveController.h"
#include "../controllers/ClawController.h"
#include "std_msgs/Float32.h"

class TestTurn : public Behavior{
    float theta, x, y;

public:

    TestTurn():Behavior(SEARCH_BEHAVIOR_TYPE){theta = OdometryHandler::instance()->getTheta();}
    bool tick();
};


#endif



































