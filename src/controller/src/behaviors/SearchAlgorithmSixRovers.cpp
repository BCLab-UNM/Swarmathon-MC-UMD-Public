#include "SearchAlgorithmSixRovers.h"

//=============================================================================================================//
//============================================SearchAlgorithmSixRoversBehavior===================================================//
bool SearchAlgorithmSixRoversBehavior::tick(){
        if(initialDrive){
            determineRovers();
        } else {
            cout<<"SEARCH: Drive"<<endl;
            if(DriveController::instance()->goToLocation(x, y)){
                cout<<"SEARCH: NEW Point"<<endl;
                determineRovers();
            }
        }
    return false;
}

void SearchAlgorithmSixRoversBehavior::determineRovers(){
    if(initialDrive){
        TargetHandler::instance()->setEnabled(false);
        theta = OdometryHandler::instance()->getTheta();
        // This ensures that the theta value used to rotate (rotational transformation) the algorithm are precice. This makes the transformation precise. 
        if (theta < M_PI/6 && theta > -M_PI/6){
            initial_theta = 0.0;
        } else if (theta > M_PI/3 && theta < 2*M_PI/3){
            initial_theta = M_PI/2;
        } else if ((theta > 2*M_PI/3 && theta < 4*M_PI/3)||(theta > -2*M_PI/3 && theta < -4*M_PI/3)){
            initial_theta = M_PI;
        } else if (theta > 4*M_PI/3 && theta < 5*M_PI/3){
            initial_theta = 3*M_PI/2;
        } else if ((theta > M_PI/6 && theta < M_PI/3)){
            initial_theta = M_PI/4;
            AlgorithmC = true;
        } else if ((theta > 2*M_PI/3 && theta < 5*M_PI/6)){
            initial_theta = 3*M_PI/4;
            AlgorithmC = true;
        } else if ((theta > 7*M_PI/6 && theta < 4*M_PI/3)){
            initial_theta = 5*M_PI/4;
            AlgorithmC = true;
        } else if ((theta > 5*M_PI/3 && theta < 11*M_PI/6)){
            initial_theta = 7*M_PI/4;
            AlgorithmC = true;
        }

        if(!AlgorithmC){
            DriveController::instance()->turnToTheta(theta + M_PI/2);
            
            left = SonarHandler::instance()->getSonarLeft();
            right = SonarHandler::instance()->getSonarRight();
            center = SonarHandler::instance()->getSonarCenter();
        }
    }
}