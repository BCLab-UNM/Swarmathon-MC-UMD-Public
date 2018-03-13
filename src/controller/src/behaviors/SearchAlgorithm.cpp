#include "SearchAlgorithm.h"

//=============================================================================================================//
//============================================SearchAlgorithmBehavior===================================================//
bool SearchAlgorithmBehavior::tick(){
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

void SearchAlgorithmBehavior::determineRovers(){
    if (initialDrive){
        TargetHandler::instance()->setEnabled(false);
        theta = OdometryHandler::instance()->getTheta();
        // This ensures that the theta value used to rotate (rotational transformation) the algorithm are precice. This makes the transformation precise. 
        if (theta < M_PI/4 && theta > -M_PI/4){
            initial_theta = 0.0;
        } else if (theta > M_PI/4 && theta < 3*M_PI/4){
            initial_theta = M_PI/2;
        } else if ((theta > 3*M_PI/4 && theta < 5*M_PI/4)||(theta > -3*M_PI/4 && theta < -5*M_PI/4)){
            initial_theta = M_PI;
        } else if (theta > 5*M_PI/4 && theta < 7*M_PI/4){
            initial_theta = 3*M_PI/2;
        }

        distance = 0.5;
        x = OdometryHandler::instance()->getX() + ((distance) * cos(theta));
        y = OdometryHandler::instance()->getY() + ((distance) * sin(theta));
        initialDrive = false;
    } else if (initialCheck){ 
        left = SonarHandler::instance()->getSonarLeft();
        right = SonarHandler::instance()->getSonarRight();
        center = SonarHandler::instance()->getSonarCenter();

        // 
        if(center > 2.0){
            AlgorithmA = true;
    
            cout<<"Algorithm: A"<<endl;
        }

        if(left < center && left < right && center < 2.5){
            AlgorithmB = true;
  
            cout<<"Algorithm: B"<<endl;
        }

        if(right < center && right < left && center < 2.5){
            AlgorithmC = true;
      
            cout<<"Algorithm: C"<<endl;
        }
        initialCheck = false;
    } else if (AlgorithmA){
        
        if(first){
            xi = xiterator;
            yi = (-3.0/sqrt(3.0))*xiterator + sqrt(3.0)/2.0;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);

            cout<<"AlgorithmAPath: "<<theta<<"\t1 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
            first = false;
            second = true;
        } else if(second){
            TargetHandler::instance()->setEnabled(true);
            xi = xiterator;
            yi = (3.0/sqrt(3.0))*xiterator - sqrt(3.0)/2.0;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);

            xiterator = xiterator - 0.5;
            cout<<"AlgorithmAPath: "<<theta<<"\t2 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
            first = true;
            second = false;
        }
    } else if (AlgorithmB){

        if(first){
            xi = 0.5 - (3.0/sqrt(3.0))*yiterator;
            yi = yiterator;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);
            
            cout<<"AlgorithmBPath: "<<theta<<"\t1 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
            first = false;
            second = true;
        } else if(second){
            TargetHandler::instance()->setEnabled(true);
            xi = 0.5;
            yi = -2.0*yiterator;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);

            yiterator = yiterator + 0.5;
            cout<<"AlgorithmBPath: "<<theta<<"\t2 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
            first = true;
            second = false;
        }
    } else if (AlgorithmC){
        if(first){
            xi = 0.5;
            yi = 2.0*yiterator;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);
            
            cout<<"AlgorithmCPath: "<<theta<<"\t1 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl; 
            first = false;
            second = true;
        } else if(second){
            TargetHandler::instance()->setEnabled(true);
            xi = (-3.0/sqrt(3.0))*yiterator + 0.5;
            yi = -yiterator;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);

            yiterator = yiterator + 0.5;
            cout<<"AlgorithmCPath: "<<theta<<"\t2 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
            first = true;
            second = false;
        }
    }
}

