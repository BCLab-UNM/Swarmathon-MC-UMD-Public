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
    if (first_checking){ // first, moving forward to find the algorithm needed to run, works with 3 rovers

        if (initialDrive)
        {
            TargetHandler::instance()->setEnabled(false);
            SonarHandler::instance()->setEnable(false);
            initial_theta = OdometryHandler::instance()->getTheta();

            /*if (theta < M_PI/4 && theta > -M_PI/4){
                initial_theta = 0.0; }
            else if (theta > M_PI/4 && theta < 3*M_PI/4){
                initial_theta = M_PI/2; }
            else if ((theta > 3*M_PI/4 && theta <= M_PI) || (theta >= -M_PI && theta < -3*M_PI/4){
                initial_theta = M_PI; }
            else if (theta > -3*M_PI/4 && theta < -M_PI/4){
                initial_theta = -M_PI/2; }
            else{
                cout<<"Error in finding initial theta"<<endl;
            }*/

            distance = 0.5;
            x = OdometryHandler::instance()->getX() + ((distance) * cos(initial_theta));
            y = OdometryHandler::instance()->getY() + ((distance) * sin(initial_theta));
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

            theta = OdometryHandler::instance()->getTheta() + M_PI;
            //run_algorithm=true;
        }
    }
/*    else if(turnAround)
    {
        if(DriveController::instance()->turnToTheta(theta)){
            turnAround = false;
            TargetHandler::instance()->setEnabled(true);
            SonarHandler::instance()->setEnable(true);
        }
    }
*/
// Now rover know what algorithm should be run, it starts running the algorithm
    else
    {

        if (AlgorithmA){
        
            if(first){
                xi = xiterator;
                yi = (-3.0/sqrt(3.0))*xiterator + sqrt(3.0)/2.0;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                cout<<"AlgorithmAPath: "<<theta<<"\t1 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                if (xiterator <= -3.0){
                    second = false;
                    first = false;
                } else {
                    first = false;
                    second = true;
                }
            } else if(second){
                xi = xiterator;
                yi = (3.0/sqrt(3.0))*xiterator - sqrt(3.0)/2.0;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                xiterator = xiterator - 0.5;
                cout<<"AlgorithmAPath: "<<theta<<"\t2 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                    first = true;
                    second = false;
            } else if(third){
                xi = xiterator;
                yi = (-9.0/sqrt(3.0)) - sqrt(3.0)/2.0;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                xiterator = xiterator - 0.5;
                cout<<"AlgorithmAPath: "<<theta<<"\t3 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;

                third = false;
                fourth = true;
            } else if(fourth){
                xi = xiterator;
                yi = (9.0/sqrt(3.0)) + sqrt(3.0)/2.0;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                cout<<"AlgorithmAPath: "<<theta<<"\t4 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                
                third = true;
                fourth = false;
            }
        }
        else if (AlgorithmB){

            if(first){
                xi = 0.5 - (3.0/sqrt(3.0))*yiterator;
                yi = yiterator;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                cout<<"AlgorithmBPath: "<<theta<<"\t1 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                if (yiterator >= 3.0){
                    first = false;
                    second = false;
                } else {
                    first = false;
                    second = true;
                }
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
            } else if(third){
                xi = 0.5 - xiterator;
                yi = -6.0;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);

                xiterator = xiterator + 0.5;
                cout<<"AlgorithmBPath: "<<theta<<"\t3 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                third = false;
                fourth = true;
            } else if(fourth){
                xi = 0.5 - (9.0/sqrt(3.0));
                yi = yiterator - xiterator;

                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);
                
                cout<<"AlgorithmBPath: "<<theta<<"\t4 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                third = true;
                fourth = false;
            }
        }

        else if (AlgorithmC){
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
            if (yiterator >= 3.0){
                first = false;
                second = false;
            } else {
                first = true;
                second = false;
            }
        } else if(third){
            xi = 0.5 - xiterator;
            yi = 6.0;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);

            xiterator = xiterator + 0.5;
            cout<<"AlgorithmBPath: "<<theta<<"\t3 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
            third = false;
            fourth = true;
        } else if(fourth){
            xi = 0.5 - (9.0/sqrt(3.0));
            yi = xiterator - yiterator;

            x = xi*cos(initial_theta) - yi*sin(initial_theta);
            y = xi*sin(initial_theta) + yi*cos(initial_theta);
            
            cout<<"AlgorithmBPath: "<<theta<<"\t4 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
            third = true;
            fourth = false;
        }
    }
        else { //default algorithm should be here, I chose AlgorithmB for now

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
                if (yiterator >= 3.0){
                    first = false;
                    second = false;
                } else {
                    first = true;
                    second = false;
                }
            } else if(third){
                xi = 0.5 - xiterator;
                yi = 6.0;
    
                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);
    
                xiterator = xiterator + 0.5;
                cout<<"AlgorithmBPath: "<<theta<<"\t3 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                third = false;
                fourth = true;
            } else if(fourth){
                xi = 0.5 - (9.0/sqrt(3.0));
                yi = xiterator - yiterator;
    
                x = xi*cos(initial_theta) - yi*sin(initial_theta);
                y = xi*sin(initial_theta) + yi*cos(initial_theta);
                
                cout<<"AlgorithmBPath: "<<theta<<"\t4 inittheta: "<<initial_theta<<"\tX: "<<x<<"\tY: "<<y<<endl;
                third = true;
                fourth = false;
            }
        }
  }
}

