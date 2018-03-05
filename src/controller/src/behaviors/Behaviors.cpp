#include "Behaviors.h"


//=============================================================================================================//
//==============================================SimpleBehavior=================================================//

bool TestTurn::tick(){
    if(DriveController::instance()->goToLocation(x, y)){
        theta = OdometryHandler::instance()->getTheta() + M_PI_2;
        float distance = 0.5;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
    }


    return false;
}

////=============================================================================================================//
////============================================SquarePathBehavior===============================================//

//bool SquarePathBehavior::tick()
//{
//    return DriveController::instance()->goToLocation(5, 5);
//}

