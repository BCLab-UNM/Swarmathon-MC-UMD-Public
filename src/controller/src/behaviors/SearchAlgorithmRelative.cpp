#include "SearchAlgorithmRelative.h"

//=============================================================================================================//
//====================================SearchAlgorithmRelativeBehavior==========================================//
bool SearchAlgorithmRelativeBehavior::tick(){
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


void SearchAlgorithmRelativeBehavior::determineRovers(){
    if (first_checking){ // first, moving forward to find the algorithm needed to run, works with 3 rovers
        if (initialDrive){
            theta = OdometryHandler::instance()->getTheta();
            distance = 0.5;
            x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
            y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
            initialDrive = false;
        }
        else if (initialCheck){
            msleep(2000); //this function is temprorary defined in searchalgorithm.h file. it makes it stop for 2 seconds
            left = SonarHandler::instance()->getSonarLeft();
            right = SonarHandler::instance()->getSonarRight();
            center = SonarHandler::instance()->getSonarCenter();
    
            if(center > 2.1){
                AlgorithmA = true;
                //Yellow, Aeness on the simulator by default
    
                cout<<"Algorithm: A"<<endl;
            }
    
            else if(left < center && left < right && center < 2){
                AlgorithmB = true;
                //White, Ajax on the simulator by default
    
                cout<<"Algorithm: B"<<endl;
            }
    
            else if(right < center && right < left && center < 2){
                //Black/green, Achilies on the simulator by defeault
                AlgorithmC = true;
    
                cout<<"Algorithm: C"<<endl;
            }
            initialCheck = false;
            first_checking=false;
        }
    } 
    else{
        if (AlgorithmA){
            if (initial){
                theta = OdometryHandler::instance()->getTheta() + 2*M_PI/3;
                distance = 1.0;
                x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
                y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
                cout<<"AlgorithmCPath: "<<theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
                initial = false;
            } else if (first){
                distance1 = sqrt(3.0);
                theta = OdometryHandler::instance()->getTheta() + M_PI - M_PI/6;
                x = OdometryHandler::instance()->getX() + ((distance1) * cos(theta));
                y = OdometryHandler::instance()->getY() + ((distance1) * sin(theta));
                cout<<"AlgorithmCPath: "<<theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
                first = false;
            } else if (second){
                distance2 = sqrt(pow((iterator*sqrt(3.0) + sqrt(3.0)/2), 2.0) + pow(0.5, 2.0));
                theta = OdometryHandler::instance()->getTheta() - M_PI + atan(0.5/(iterator*sqrt(3.0) + sqrt(3.0)/2));
                x = OdometryHandler::instance()->getX() + ((distance2) * cos(theta));
                y = OdometryHandler::instance()->getY() + ((distance2) * sin(theta));
                cout<<"AlgorithmCPath: "<<theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
                second = false;
                third = true;
            } else if (third){
                iterator = iterator + 1.0;
                distance1 = iterator*sqrt(3.0);
                theta = OdometryHandler::instance()->getTheta() + M_PI - atan(0.5/(iterator*sqrt(3.0) - sqrt(3.0)/2));
                x = OdometryHandler::instance()->getX() + ((distance1) * cos(theta));
                y = OdometryHandler::instance()->getY() + ((distance1) * sin(theta));
                cout<<"AlgorithmCPath: "<<theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
                second = true;
                third = false;
            }

        }
    }
}