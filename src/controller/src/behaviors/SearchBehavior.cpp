#include "SearchBehavior.h"

//=============================================================================================================//
//============================================SearchBehavior===================================================//
bool SearchBehavior::tick(){
    if(first){
        nextPoint();
    } else {
        if(DriveController::instance()->goToDistance(distance, theta)){
            cout<<"SEARCH: NEW Point"<<endl;
            nextPoint();
            SMACS::instance()->setTime();
        }
    }


    return false;
}


void SearchBehavior::nextPoint(){
    if(first){
        theta = OdometryHandler::instance()->getTheta() + M_PI;
        distance = 0.5;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
        cout<<"SEARCH: Driving to first: "<<x<<" "<<y<<endl;
        first = false;
    } else if(second){
        theta = OdometryHandler::instance()->getTheta() + M_PI_2;
        distance = 2;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
        cout<<"SEARCH: Driving to second: "<<x<<" "<<y<<endl;
        second = false;
    } else if(third){
        theta = OdometryHandler::instance()->getTheta() + M_PI_2;
        distance = 3.5;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
        third = false;
        iterCount ++;
    } else {
        distance += 0.25;

        theta = OdometryHandler::instance()->getTheta() + M_PI_2;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
    }


}


bool RandomSearchBehavior::tick(){
    if(DriveController::instance()->goToDistance(distance, theta)){
        cout<<"SEARCH: NEW Point"<<endl;
        nextPoint();
        cout<<"SEARCH: next: "<< theta <<" "<< distance <<endl;
        SMACS::instance()->setTime();
    }

    return false;
}

void RandomSearchBehavior::nextPoint(){

    //get random angle (up to ~60 deg)
    theta = rng->gaussian(OdometryHandler::instance()->getTheta(), 1.0);

    //get rand dist
    distance = ((float)((rand() %10) + 1) / 10.0) + 0.4;
}
























