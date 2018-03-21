#include "DriveController.h"


DriveController* DriveController::s_instance = 0;

DriveController* DriveController::instance(){
    if (!s_instance)
      s_instance = new DriveController;
    return s_instance;
}

void DriveController::registerDrivePublisher(ros::Publisher& drivePublisher){
    this->drivePublisher = drivePublisher;
}


bool DriveController::goToLocation(float x, float y){
    //If the requested coords are the same as before
    //Means we are still driving to the same location as requested before
    if(currentDrive.x == x && currentDrive.y == y){
        currentLocation.x = OdometryHandler::instance()->getX();
        currentLocation.y = OdometryHandler::instance()->getY();
        currentLocation.theta = OdometryHandler::instance()->getTheta();
        linear = OdometryHandler::instance()->getLinear();
        switch(stateMachineState){
            case STATE_MACHINE_ROTATE:
            {
                // Calculate angle between currentLocation.theta and theta we desire
                // Rotate left or right depending on sign of angle
                // Stay in this state until angle is minimized

                // Calculate the theta from our position to the desired position
                // Where we should face to drive to where we want
                currentDrive.theta = atan2(currentDrive.y - currentLocation.y, currentDrive.x - currentLocation.x);

                // Calculate the difference between current and desired heading in radians.
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);

                // Check if prev error was initialized
                // This is used because the PID with cause the robot to "Dance" left and right
                // So we save the previous angle to make sure that it is increasing and not decreasing
                if(!isDistanceTurnedInit){
                    //init prev error
                    isDistanceTurnedInit = true;
                    initDirection = currentLocation.theta;
                    prevDistanceTurned = fabs(angles::shortest_angular_distance(currentLocation.theta, initDirection));
                }

                // Calculate the current distance we turned. It should increase
                float currDistanceTurned = fabs(angles::shortest_angular_distance(currentLocation.theta, initDirection));

                // If previous distance turned is greater than the current distance turned means we are dancing
                // Also check if the dance is big enough to activate our failsafe. Meaning if the error between  
                // angles is more than certain amount of degrees then activate the failsafe
                if((prevDistanceTurned > currDistanceTurned) && (prevDistanceTurned - currDistanceTurned) > 0.0349){
                    stop();
                    //move to final rotate step where robots rotate as slow as possible to minimize the angle
                    stateMachineState = FINAL_ROTATE;
                    isDistanceTurnedInit = false;
                } else {
                    prevDistanceTurned = currDistanceTurned;
                    //Calculate absolute value of angle
                    float abs_error = fabs(angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta));

                    // If angle > rotateOnlyAngleTolerance radians rotate but don't drive forward.
                    if (abs_error > rotateOnlyAngleTolerance){
                        slowPID(0.0, errorYaw, 0.0, currentDrive.theta);
                        break;
                    } else {
                        stop();
                        //move to final rotate step where robots rotate as slow as possible to minimize the angle
                        stateMachineState = FINAL_ROTATE;
                        //fall through on purpose.
                    }
                }
            }
            case FINAL_ROTATE:
            {
                // Calculate angle between currentLocation.theta and waypoints.front().theta
                // Rotate left or right depending on sign of angle
                // Stay in this state until angle is minimized


                // Calculate the difference between current and desired heading in radians.
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);

                //Calculate absolute value of angle
                float abs_error = fabs(angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta));

                // If we have not completed the turn 
                // TODO: This needs fixing because there might be a case where we overshoot
                if(abs_error >= finalRotationTolerance){
                    //find out if left or right
                    //if need to turn right
                    if (errorYaw < 0){
                        //sendDriveCommand(leftMin, -rightMin);
                        left = leftMin;
                        right = -rightMin;
                    } else {
                        //sendDriveCommand(-leftMin, rightMin);
                        left = -leftMin;
                        right = rightMin;
                    }

                    break;
                } else {
                     stop();
                     stateMachineState = STATE_MACHINE_SKID_STEER;
                     break;
		}

            }

            case STATE_MACHINE_SKID_STEER:
            {


                // calculate the angle between the current x and y and the desired to find out if we need to turn a little
                currentDrive.theta = atan2(currentDrive.y - currentLocation.y, currentDrive.x - currentLocation.x);

                // calculate the angle error. This can let us turn and drive if the angle is big
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);

                // Distance driven
                float distance = hypot(currentDrive.x - currentLocation.x, currentDrive.y - currentLocation.y);

                if(fabs(errorYaw) < rotateOnlyAngleTolerance){
                    // goal not yet reached drive while maintaining proper heading.
                    if (distance > waypointTolerance){
                        slowPID((searchVelocity-linear) ,0, searchVelocity, 0);
                    } else {
                        stop();
                        stateMachineState = STATE_MACHINE_ROTATE;
                        isDistanceTurnedInit = false;

                        // return true because drive is completed
                        return true;
                    }
                } else {
                    stateMachineState = FINAL_ROTATE;
                }


                break;
            }
            default:
            {
                break;
            }
        }
        sendDriveCommand(left, right);
    } else {
        //reset the drive controller and drive to new location
        resetDriveController(x, y);

        // Reset the previous yaw error
        isDistanceTurnedInit = false;
    }

    return false;

}


bool DriveController::goToDistance(float distance, float direction){
    if(this->distance == distance && this->direction == direction){
        currentLocation.x = OdometryHandler::instance()->getX();
        currentLocation.y = OdometryHandler::instance()->getY();
        currentLocation.theta = OdometryHandler::instance()->getTheta();
        linear = OdometryHandler::instance()->getLinear();
        switch(stateMachineState){
            case STATE_MACHINE_ROTATE:
            {
                // Calculate angle between currentLocation.theta and theta we desire
                // Rotate left or right depending on sign of angle
                // Stay in this state until angle is minimized

                // Calculate the theta from our position to the desired position
                // Where we should face to drive to where we want
                currentDrive.theta = direction;

                // Calculate the difference between current and desired heading in radians.
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);

                // Check if prev error was initialized
                // This is used because the PID with cause the robot to "Dance" left and right
                // So we save the previous angle to make sure that it is increasing and not decreasing
                if(!isDistanceTurnedInit){
                    //init prev error
                    isDistanceTurnedInit = true;
                    initDirection = currentLocation.theta;
                    prevDistanceTurned = fabs(angles::shortest_angular_distance(currentLocation.theta, initDirection));
                }

                // Calculate the current distance we turned. It should increase
                float currDistanceTurned = fabs(angles::shortest_angular_distance(currentLocation.theta, initDirection));

                // If previous distance turned is greater than the current distance turned means we are dancing
                // Also check if the dance is big enough to activate our failsafe. Meaning if the error between
                // angles is more than certain amount of degrees then activate the failsafe
                if((prevDistanceTurned > currDistanceTurned) && (prevDistanceTurned - currDistanceTurned) > 0.0349){
                    stop();
                    //move to final rotate step where robots rotate as slow as possible to minimize the angle
                    stateMachineState = FINAL_ROTATE;
                    isDistanceTurnedInit = false;
                } else {

                    prevDistanceTurned = currDistanceTurned;
                    //Calculate absolute value of angle
                    float abs_error = fabs(angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta));

                    // If angle > rotateOnlyAngleTolerance radians rotate but don't drive forward.
                    if (abs_error > rotateOnlyAngleTolerance){
                        slowPID(0.0, errorYaw, 0.0, currentDrive.theta);
                        break;
                    } else {
                        stop();
                        //move to final rotate step where robots rotate as slow as possible to minimize the angle
                        stateMachineState = FINAL_ROTATE;
                        //fall through on purpose.
                    }
                }
            }
            case FINAL_ROTATE:
            {
                // Calculate angle between currentLocation.theta and waypoints.front().theta
                // Rotate left or right depending on sign of angle
                // Stay in this state until angle is minimized


                // Calculate the difference between current and desired heading in radians.
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);

                //Calculate absolute value of angle
                float abs_error = fabs(angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta));

                // If we have not completed the turn
                // TODO: This needs fixing because there might be a case where we overshoot
                if(abs_error >= finalRotationTolerance){

                    //find out if left or right
                    //if need to turn right
                    if (errorYaw < 0){

                        //sendDriveCommand(leftMin, -rightMin);
                        left = leftMin;
                        right = -rightMin;
                    } else {

                        //sendDriveCommand(-leftMin, rightMin);
                        left = -leftMin;
                        right = rightMin;
                    }

                    break;
                } else {
                     stop();
                     stateMachineState = STATE_MACHINE_SKID_STEER;

                     break;
                }

            }
            case STATE_MACHINE_SKID_STEER:
            {

                // calculate the distance between current and desired heading in radians
                float errorYaw = angles::shortest_angular_distance(currentLocation.theta, currentDrive.theta);
                float distanceDriven = fabs(hypot(currentDrive.x - currentLocation.x, currentDrive.y - currentLocation.y));

                if(fabs(errorYaw) < rotateOnlyAngleTolerance){

                    // goal not yet reached drive while maintaining proper heading.
                    if (distanceDriven < this->distance){
                        fastPID((searchVelocity-linear) ,0, searchVelocity, 0);
                    } else {
                        // stopno change
                        stop();
                        // move back to transform step
                        stateMachineState = STATE_MACHINE_ROTATE;
                        this->distance = -1;
                        this->direction = -1;
                        return true;
                    }
                } else {
                    stateMachineState = FINAL_ROTATE;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        sendDriveCommand(left, right);
    } else {

        //values are different from last time, so change to new values
        this->distance = distance;
        this->direction = direction;

        currentDrive.x = OdometryHandler::instance()->getX();     //Find x
        currentDrive.y = OdometryHandler::instance()->getY();     //Find y
        currentDrive.theta = direction;
        isDistanceTurnedInit = false;
    }
    return false;

}

bool DriveController::turnToTheta(float theta){
    // Calculate angle between currentLocation.theta and waypoints.front().theta
    // Rotate left or right depending on sign of angle
    // Stay in this state until angle is minimized
    float currTheta = OdometryHandler::instance()->getTheta();

    // Calculate the difference between current and desired heading in radians.
    float errorYaw = angles::shortest_angular_distance(currTheta, theta);

    //Calculate absolute value of angle
    float abs_error = fabs(angles::shortest_angular_distance(currTheta, theta));

    if(abs_error >= finalRotationTolerance){

        //find out if left or right
        //if need to turn right
        if (errorYaw < 0){

            sendDriveCommand(rightMin, -rightMin);
        } else {

            sendDriveCommand(-leftMin, leftMin);
        }

        return false;
    } else {
         stop();
         return true;
    }
}


bool DriveController::stop(){
    left = 0;
    right = 0;
    sendDriveCommand(left, right);
}



void DriveController::setLeftRightMin(double leftMin, double rightMin){
    this->leftMin = leftMin;
    this->rightMin = rightMin;
}

void DriveController::resetDriveController(float x, float y){
    left = 0;
    right = 0;
    stateMachineState = STATE_MACHINE_ROTATE;
    currentDrive.x = x;
    currentDrive.y = y;
    currentDrive.theta = atan2(currentDrive.y - OdometryHandler::instance()->getY(), currentDrive.x - OdometryHandler::instance()->getX());
    sendDriveCommand(left, right);
}

void DriveController::sendDriveCommand(double left, double right){
    if(left != 0 && fabs(left) < left){
        left = left*(leftMin/fabs(left));
    }

    if(right !=0 && fabs(right) < rightMin){
        right = right*(rightMin/fabs(right));
    }

    velocity.linear.x = left;
    velocity.angular.z = right;

    // publish the drive commands
    drivePublisher.publish(velocity);
}


void DriveController::sendDriveCommandNoFix(double left, double right){
    velocity.linear.x = left;
    velocity.angular.z = right;

    drivePublisher.publish(velocity);
}







void DriveController::fastPID(float errorVel, float errorYaw , float setPointVel, float setPointYaw)
{

  // cout << "PID FAST" << endl;

  float velOut = fastVelPID.PIDOut(errorVel, setPointVel);
  float yawOut = fastYawPID.PIDOut(errorYaw, setPointYaw);

  int left = velOut - yawOut;
  int right = velOut + yawOut;

  int sat = 180;
  if (left  >  sat) {left  =  sat;}
  if (left  < -sat) {left  = -sat;}
  if (right >  sat) {right =  sat;}
  if (right < -sat) {right = -sat;}

  this->left = left;
  this->right = right;
}

void DriveController::slowPID(float errorVel,float errorYaw, float setPointVel, float setPointYaw)
{
  //cout << "PID SLOW" << endl;

  float velOut = slowVelPID.PIDOut(errorVel, setPointVel);
  float yawOut = slowYawPID.PIDOut(errorYaw, setPointYaw);

  int left = velOut - yawOut;
  int right = velOut + yawOut;

  int sat = 180;
  if (left  >  sat) {left  =  sat;}
  if (left  < -sat) {left  = -sat;}
  if (right >  sat) {right =  sat;}
  if (right < -sat) {right = -sat;}

  this->left = left;
  this->right = right;
}

void DriveController::constPID(float erroVel,float constAngularError, float setPointVel, float setPointYaw)
{

  //cout << "PID CONST" << endl;

  float velOut = constVelPID.PIDOut(erroVel, setPointVel);
  float yawOut = constYawPID.PIDOut(constAngularError, setPointYaw);

  int left = velOut - yawOut;
  int right = velOut + yawOut;

  int sat = 180;
  if (left  >  sat) {left  =  sat;}
  if (left  < -sat) {left  = -sat;}
  if (right >  sat) {right =  sat;}
  if (right < -sat) {right = -sat;}

  this->left = left;
  this->right = right;
}




PIDConfig DriveController::fastVelConfig()
{
  PIDConfig config;

  config.Kp = 60;
  config.Ki = 10;
  config.Kd = 2;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper;
  config.errorHistLength = 4;
  config.alwaysIntegral = true;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 610; //gives 127 pwm at 0.4 commandedspeed  ORIG:320
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper/2;
  config.derivativeAlpha = 0.7;

  return config;

}

PIDConfig DriveController::fastYawConfig() {
  PIDConfig config;

  config.Kp = 60;
  config.Ki = 50;
  config.Kd = 5;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper/6;
  config.errorHistLength = 4;
  config.alwaysIntegral = false;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 0;
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper/3;
  config.derivativeAlpha = 0.7;

  return config;

}

PIDConfig DriveController::slowVelConfig() {
  PIDConfig config;

  config.Kp = 100;
  config.Ki = 8;
  config.Kd = 1.1;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper/2;
  config.errorHistLength = 4;
  config.alwaysIntegral = true;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 320; //gives 127 pwm at 0.4 commandedspeed
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper/2;
  config.derivativeAlpha = 0.7;

  return config;

}

PIDConfig DriveController::slowYawConfig() {
  PIDConfig config;

  config.Kp = 70;
  config.Ki = 16;
  config.Kd = 10;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper/4;
  config.errorHistLength = 4;
  config.alwaysIntegral = false;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 0;
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper/6;
  config.derivativeAlpha = 0.7;

  return config;

}

PIDConfig DriveController::constVelConfig() {
  PIDConfig config;

  config.Kp = 60;
  config.Ki = 10;
  config.Kd = 2;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper;
  config.errorHistLength = 4;
  config.alwaysIntegral = true;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 610; //gives 127 pwm at 0.4 commandedspeed  ORIG:320
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper/2;
  config.derivativeAlpha = 0.7;

  return config;

}

PIDConfig DriveController::constYawConfig() {
  PIDConfig config;

  config.Kp = 5;
  config.Ki = 5;
  config.Kd = 0;
  config.satUpper = 255;
  config.satLower = -255;
  config.antiWindup = config.satUpper/4;
  config.errorHistLength = 4;
  config.alwaysIntegral = true;
  config.resetOnSetpoint = true;
  config.feedForwardMultiplier = 0;
  config.integralDeadZone = 0.01;
  config.integralErrorHistoryLength = 10000;
  config.integralMax = config.satUpper;
  config.derivativeAlpha = 0.6;

  return config;

}
