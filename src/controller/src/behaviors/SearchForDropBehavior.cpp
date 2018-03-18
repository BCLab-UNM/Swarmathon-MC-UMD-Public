#include "SearchForDropBehavior.h"

bool SearchForDropBehavior::tick(){
    if(TargetHandler::instance()->getHasCube()){
        //turn on sonars for avoid
        SonarHandler::instance()->setEnable(true);
        //turn off camera for center avoid and cube pick up
        TargetHandler::instance()->setEnabled(false);
        switch(stage){       
            case ODOM_TARGET:
            {
                TargetHandler::instance()->setEnabled(false);
                float centerX = OffsetController::instance()->centerX;
                float centerY = OffsetController::instance()->centerY;
                if(DriveController::instance()->goToLocation(centerX, centerY)){
                    stage = TURN_TO_THETA;
                    theta = OdometryHandler::instance()->getTheta();
                    x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                    y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                }


                break;
            }
            case TURN_TO_THETA:
            {
                if(DriveController::instance()->turnToTheta(OffsetController::instance()->centerTheta)){
                    stage = SEARCH_FOR_CENTER;
                    theta = OdometryHandler::instance()->getTheta();
                    x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                    y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                }
                break;
            }
            case SEARCH_FOR_CENTER:
            {
                cout<<"SEARCHCENTER: "<<x << " "<<y<<endl;
                //If we do not see any tags yet. Try to drive around
                //If this is our first search try
                if(searchTry == 0){
                    // Drive one meter forward
                    if(DriveController::instance()->goToDistance(distance, theta)){
                        searchTry++;
                        distance = 1;
                        theta = OdometryHandler::instance()->getTheta() + M_PI_4;
                        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                    }
                } else if(searchTry == 1){
                    if(DriveController::instance()->goToDistance(distance, theta)){
                        distance = 1;
                        theta = OdometryHandler::instance()->getTheta() + M_PI_4;
                        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                        searchTry++;
                    }

                } else if(searchTry > 1){
                    if(DriveController::instance()->goToDistance(distance, theta)){
                        if(searchTry % 2 == 0)
                            distance+=0.5;

                        theta = OdometryHandler::instance()->getTheta() + M_PI_4;
                        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                        searchTry++;

                        if(searchTry > 15){
                            stage = GPS_TARGET;
                        }
                    }
                }

                break;
            }
            case GPS_TARGET:
            {
                searchTry = 0;
                if(DriveController::instance()->goToLocation(0, 0)){
                    stage = SEARCH_FOR_CENTER;
                }
                break;
            }
            case SEARCH:
            {
                stage = ASK;
                break;
            }
            case ASK:
            {
                SonarHandler::instance()->setEnable(false);
                stage = DROP;
                break;
            }
        }
    } else {
        return true;
    }

    return false;
}
