#include "SearchForDropBehavior.h"

bool SearchForDropBehavior::tick(){
    if(TargetHandler::instance()->getHasCube()){
        //turn on sonars for avoid
        SonarHandler::instance()->setEnable(false);
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
                if(DriveController::instance()->goToDistance(distance, theta)){
                    distance+=0.25;

                    theta = OdometryHandler::instance()->getTheta() + 3*M_PI/4;
                    x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                    y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                    searchTry++;

                    if(searchTry > 0){
                        left = 10;
                        right = 40;
                        initTime = millis();
                        stage = GPS_TARGET;
                        searchTry = 0;
                    }
                }


                break;
            }
            case GPS_TARGET:
            {
                DriveController::instance()->sendDriveCommandNoFix(left, right);
                if((millis() - initTime) >= 5000){
                    initTime = millis();
                    left += 5;
                    right += 5;
                    searchTry++;
                    if(searchTry >= 5){
                        searchTry = 0;
                        stage = SEARCH_FOR_CENTER;
                    }
                }
                break;
            }
            case SEARCH:
            {
                searchTry = 0;
                if(DriveController::instance()->goToLocation(0, 0)){
                    stage = SEARCH_FOR_CENTER;
                }
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
