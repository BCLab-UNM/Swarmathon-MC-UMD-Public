#include "SearchForDropBehavior.h"


bool SearchForDropBehavior::tick(){
    if(TargetHandler::instance()->getHasCube()){
        //turn off camera for center avoid and cube pick up
        TargetHandler::instance()->setEnabled(false);
        switch(stage){       
            case ODOM_TARGET:
            {
                SonarHandler::instance()->setEnable(true);
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
                    stage = CHECK_FOR_CUBE;
                    initTime = millis();
                    SonarHandler::instance()->setEnable(false);

                    distance = 0.25;
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

                    if(searchTry > 15){
                        if(DriveController::instance()->goToLocation(0, 0)){
                            left = 88;
                            right = 1;
                            initTime = millis();
                            SonarHandler::instance()->setEnable(false);
                            stage = CHECK_FOR_CUBE;
                            searchTry = 0;
                        }
                    }
                }


                break;
            }
            case SEARCH_CIRCLE:
            {
                DriveController::instance()->sendDriveCommandNoFix(left, right);
                if((millis() - initTime) >= 20000){
                    initTime = millis();
                    left += 10;
                    right += 15;
                    searchTry++;
                    if(searchTry >= 5){
                        if(DriveController::instance()->goToLocation(0, 0)){
                            searchTry = 0;
                            SonarHandler::instance()->setEnable(false);
                            initTime = millis();
                            stage = CHECK_FOR_CUBE;
                        }
                    }
                }
                break;
            }
            case SEARCH:
            {
                if(DriveController::instance()->goToDistance(distance, theta)){
                    if(searchTry >= 10){
                        searchTry = 0;
                        stage = CHECK_FOR_CUBE;
                        initTime = millis();
                        SonarHandler::instance()->setEnable(false);
                    }

                    //get random angle (up to ~60 deg)
                    theta = rng->gaussian(OdometryHandler::instance()->getTheta(), 1);

                    //get rand dist
                    distance =  ((rand() %10) + 1) / 10;

                }

                break;
            }
            case CHECK_FOR_CUBE:
            {
                DriveController::instance()->stop();

                ClawController::instance()->wristUp();

                if(millis() - initTime > 3000){
                    if(cubeChecked){
                        ClawController::instance()->wristDownWithCube();
                        if(checkCube){
                            stage = SEARCH_FOR_CENTER;
                            checkCube = false;
                        } else if(starSearch){
                            stage = SEARCH_CIRCLE;
                            starSearch = false;
                        } else if (gpsSearch){
                            theta = OdometryHandler::instance()->getTheta();
                            distance = 0.5;
                            stage = SEARCH;
                            gpsSearch = false;
                        } else {
                            theta = OdometryHandler::instance()->getTheta();
                            distance = 0.5;
                            stage = SEARCH;
                        }
                    } else {
                        float center = SonarHandler::instance()->getSonarCenter();
                        if(center < 0.15){
                            cubeChecked = true;
                            break;
                        } else {
                            ClawController::instance()->fingerOpen();
                            TargetHandler::instance()->setHasCube(false);
                            return true;
                        }
                    }

                    initTime = millis();
                    cubeChecked = false;
                    SonarHandler::instance()->setEnable(true);
                } else {
                    float center = SonarHandler::instance()->getSonarCenter();
                    if(center < 0.15){
                        cubeChecked = true;
                    }
                }

                break;
            }
        }
    } else {
        return true;
    }

    return false;
}
