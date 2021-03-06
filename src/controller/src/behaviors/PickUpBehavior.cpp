#include "PickUpBehavior.h"


int PickUpBehavior::leftPos = 1;
int PickUpBehavior::rightPos = 1;
int PickUpBehavior::leftNeg = -1;
int PickUpBehavior::rightNeg = -1;
long PickUpBehavior::lastCheck = 0;

bool PickUpBehavior::tick(){
    switch (currentStage){
        case LOCK_TARGET:
        {
            SonarHandler::instance()->setEnable(false);
            ClawController::instance()->fingerOpen();
            ClawController::instance()->wristDown();

            targetLocked = false;
            if(TargetHandler::instance()->getNumberOfCubeTags() > 0){
                //get all the tags
                std::vector<Tag> tags = TargetHandler::instance()->getCubeTags();

                // Find closest tag and lock it
                double closest = std::numeric_limits<double>::max();
                int target  = 0;

                //this loop selects the closest visible block to makes goals for it
                for (int i = 0; i < tags.size(); i++)
                {

                  if (tags[i].getID() == 0)
                  {

                    targetLocked = true;

                    //absolute distance to block from camera lens
                    double test = hypot(hypot(tags[i].getPositionX(), tags[i].getPositionY()), tags[i].getPositionZ());


                    if (closest > test)
                    {
                      target = i;
                      closest = test;
                    }
                  }
                }

                if(targetLocked){
                    blockDistanceFromCamera = hypot(hypot(tags[target].getPositionX(), tags[target].getPositionY()), tags[target].getPositionZ());

                    if ( (blockDistanceFromCamera*blockDistanceFromCamera - 0.195*0.195) > 0 )
                    {
                        blockDistance = sqrt(blockDistanceFromCamera*blockDistanceFromCamera - 0.195*0.195);
                    }
                    else
                    {
                        float epsilon = 0.00001; // A small non-zero positive number
                        blockDistance = epsilon;
                    }

                    cout << "TARGET: Distance to closest: " << blockDistance << endl;

                    //angle to block from bottom center of chassis on the horizontal.
                    blockYawError = atan((tags[target].getPositionX() + cameraOffsetCorrection)/blockDistance)*1.05;

                    cout << "TARGET: Angle to closest:  " << blockYawError << endl;

                    if(!precisionDrive)
                        currentStage = TURN_TO_FACE_TARGET;
                    else
                        currentStage = PRECISION_TURN;

                    //get the current theta to start counting the turn
                    initTheta = OdometryHandler::instance()->getTheta();
                }

            } else {
                if(precisionDrive){
                    currentStage = RETRY;
                    initX = OdometryHandler::instance()->getX();
                    initY = OdometryHandler::instance()->getY();
                }else{
                    DriveController::instance()->stop();
                    SonarHandler::instance()->setEnable(true);
                    ClawController::instance()->fingerOpen();
                    ClawController::instance()->wristUp();
                    return true;
                }
            }
            break;

        }
        case TURN_TO_FACE_TARGET:
        {
            //get current angle
            float currentTheta = OdometryHandler::instance()->getTheta();

            // Check if we have turned to face the cube
            float abs_error = fabs(angles::shortest_angular_distance(currentTheta, initTheta));

            // If angle turned is not within the tolerance
            float abs_blockYaw = fabs(blockYawError);

            //if within the angle tolerance
            if(abs_blockYaw - abs_error <= angleTolerance){
                currentStage = DRIVE_TO_PICK_UP;
                DriveController::instance()->stop();

                //get x and y
                initX = OdometryHandler::instance()->getX();
                initY = OdometryHandler::instance()->getY();
            } else {
                if (blockYawError < 0){
                    //turn left
                    if(abs_blockYaw - abs_error > 0){
                        DriveController::instance()->sendDriveCommand(leftNeg, rightPos);
                        fix(true, false);
                    }else{
                        DriveController::instance()->sendDriveCommand(leftPos, rightNeg);
                        fix(false, true);
                    }

                } else {
                    //trun right
                    if(abs_blockYaw - abs_error > 0){
                        DriveController::instance()->sendDriveCommand(leftPos, rightNeg);
                        fix(false, true);
                    } else {
                        DriveController::instance()->sendDriveCommand(leftNeg, rightPos);
                        fix(true, false);
                    }
                }


            }

            break;

        }
        case DRIVE_TO_PICK_UP:
        {
            float currX = OdometryHandler::instance()->getX();
            float currY = OdometryHandler::instance()->getY();

            ClawController::instance()->fingerOpen();
            ClawController::instance()->wristDown();

            //Drive and count how far we have driven
            float distance = hypot(initX - currX, initY - currY);

            if(blockDistance - distance <= 0.35){
                currentStage = LOCK_TARGET;
                precisionDrive = true;
                DriveController::instance()->stop();

            }else{
                DriveController::instance()->sendDriveCommand(driveSpeed, driveSpeed);
            }

            break;
        }
        case PRECISION_TURN:
        {
            //get current angle
            float currentTheta = OdometryHandler::instance()->getTheta();

            // Check if we have turned to face the cube
            float abs_error = fabs(angles::shortest_angular_distance(currentTheta, initTheta));

            // If angle turned is not within the tolerance
            float abs_blockYaw = fabs(blockYawError);

            if(abs_blockYaw - abs_error - 0.175 <= angleTolerance){
                //if within the angle tolerance
                currentStage = PRECISION_DRIVE;
                DriveController::instance()->stop();

                //get x and y
                initX = OdometryHandler::instance()->getX();
                initY = OdometryHandler::instance()->getY();


            } else {
                if (blockYawError < 0){
                    //turn left
                    if(abs_blockYaw - abs_error - 0.175 > 0){
                        DriveController::instance()->sendDriveCommand(leftNeg, rightPos);
                        fix(true, false);
                    }else{
                        DriveController::instance()->sendDriveCommand(leftPos, rightNeg);
                        fix(false, true);
                    }
                } else {
                    //trun right
                    if(abs_blockYaw - abs_error - 0.175 > 0){
                        DriveController::instance()->sendDriveCommand(leftPos, rightNeg);
                        fix(false, true);
                    }else{
                        DriveController::instance()->sendDriveCommand(leftNeg, rightPos);
                        fix(true, false);
                    }
                }
            }

            break;
        }
        case PRECISION_DRIVE:
        {
            float currX = OdometryHandler::instance()->getX();
            float currY = OdometryHandler::instance()->getY();

            ClawController::instance()->fingerOpen();
            ClawController::instance()->wristDown();

            //Drive and count how far we have driven
            float distance = hypot(initX - currX, initY - currY);

            if(blockDistance - distance <= 0.13){
                currentStage = PICK_UP;

                DriveController::instance()->stop();
            }else{
                DriveController::instance()->sendDriveCommand(driveSpeed, driveSpeed);
            }

            break;
        }
        case PICK_UP:
        {
            ClawController::instance()->fingerClose();
            sleep(1);
            ClawController::instance()->wristUp();


            if(!wait(2)){
                // check if picked up
                float sonarCenter = SonarHandler::instance()->getSonarCenter();

                if(sonarCenter < 0.14){
                    //TODO: maybe add a camera block seen chack by checking how far is the picked up block from camera
                    //target was picked up
                    ClawController::instance()->wristDownWithCube();
                    TargetHandler::instance()->setEnabled(false);
                    TargetHandler::instance()->setHasCube(true);
                    //get x and y
                    initX = OdometryHandler::instance()->getX();
                    initY = OdometryHandler::instance()->getY();
                    currentStage = DRIVE_BACK;
                } else {
                    targetLocked = false;
                    if(TargetHandler::instance()->getNumberOfCubeTags() > 0){
                        //get all the tags
                        std::vector<Tag> tags = TargetHandler::instance()->getCubeTags();

                        // Find closest tag and lock it
                        double closest = std::numeric_limits<double>::max();
                        int target  = 0;

                        //this loop selects the closest visible block to makes goals for it
                        for (int i = 0; i < tags.size(); i++)
                        {

                          if (tags[i].getID() == 0)
                          {

                            targetLocked = true;

                            //absolute distance to block from camera lens
                            double test = hypot(hypot(tags[i].getPositionX(), tags[i].getPositionY()), tags[i].getPositionZ());


                            if (closest > test)
                            {
                              target = i;
                              closest = test;
                            }
                          }
                        }

                        if(targetLocked){
                            blockDistanceFromCamera = hypot(hypot(tags[target].getPositionX(), tags[target].getPositionY()), tags[target].getPositionZ());
                        }
                    }
                    // make a camera check
                    if (blockDistanceFromCamera < 0.14){
                        ClawController::instance()->wristDownWithCube();
                        TargetHandler::instance()->setEnabled(false);
                        TargetHandler::instance()->setHasCube(true);
                        //get x and y
                        initX = OdometryHandler::instance()->getX();
                        initY = OdometryHandler::instance()->getY();
                        currentStage = DRIVE_BACK;
                    } else {
                        initX = OdometryHandler::instance()->getX();
                        initY = OdometryHandler::instance()->getY();
                        currentStage = RETRY;
                    }
                }
            }

            break;
        }
        case RETRY:
        {
            ClawController::instance()->wristDown();
            ClawController::instance()->fingerOpen();

            float currX = OdometryHandler::instance()->getX();
            float currY = OdometryHandler::instance()->getY();
            // target was not seen. Drive back and pick up
            //Drive and count how far we have driven
            float distance = hypot(initX - currX, initY - currY);

            if(distance >= driveBackDist){
                int numberOftags = TargetHandler::instance()->getNumberOfCenterTagsSeen();
                if(numberOftags > 0){
                    ClawController::instance()->wristUp();
                    ClawController::instance()->fingerOpen();
                    precisionDrive = false;
                    currentStage = LOCK_TARGET;
                    DriveController::instance()->stop();
                } else {
                    // no tragets are seen after drive back. try to turn.
                    float lastYaw = TargetHandler::instance()->getLastSeenBlockError();
                    if(lastYaw < 0){
                        DriveController::instance()->sendDriveCommand(-driveSpeed, driveSpeed);
                        sleep(0.5);
                        DriveController::instance()->stop();
                    } else {
                         DriveController::instance()->sendDriveCommand(driveSpeed, -driveSpeed);
                         sleep(0.5);
                         DriveController::instance()->stop();
                    }

                    ClawController::instance()->wristUp();
                    ClawController::instance()->fingerOpen();
                    precisionDrive = false;
                    currentStage = LOCK_TARGET;
                    DriveController::instance()->stop();
                }
            }else{
                DriveController::instance()->sendDriveCommand(-driveSpeed, -driveSpeed);
            }

            break;
        }
        case DRIVE_BACK:
        {

            float currX = OdometryHandler::instance()->getX();
            float currY = OdometryHandler::instance()->getY();

            //Drive and count how far we have driven
            float distance = hypot(initX - currX, initY - currY);

            if(distance >= driveBackDist){
                DriveController::instance()->stop();
                currentStage = TURN_TO_BASE;
            } else {
                DriveController::instance()->sendDriveCommand(-driveSpeed, -driveSpeed);
            }
            break;
        }

        case TURN_TO_BASE:
        {
            //turn to face the base
            float baseX = OffsetController::instance()->centerX;
            float baseY = OffsetController::instance()->centerY;
            float x = OdometryHandler::instance()->getX();
            float y = OdometryHandler::instance()->getY();
            //calculate base theta from current location
            baseTheta = atan2(baseY - y, baseX - x);

            currentStage = DROP;
            break;
        }
        case DROP:
        {
            if(DriveController::instance()->turnToTheta(baseTheta)){
                //Get current x and y
                float x = OdometryHandler::instance()->getX();
                float y = OdometryHandler::instance()->getY();
                //Put return behavior in the stack
                SMACS::instance()->pushNext(new DriveBehavior(x, y));
                //Put drop behavior to the stack
                SMACS::instance()->pushNext(new SearchForDropBehavior());

                SonarHandler::instance()->setEnable(true);

                //return true to pop pick up from stack and execute DropBehavior()
                return true;
            }
            break;
        }

    }


    return false;

}

void PickUpBehavior::fix(bool left, bool right){
    //get encoders
	if(left){
        cout << "PICKUPLEARN: "<<leftNeg<<" "<<rightPos<<endl;
	} else {
        cout << "PICKUPLEARN: "<<leftPos<<" "<<rightNeg<<endl;
	}
    int e_left = EncoderHandler::instance()->getEncoderLeft();
    int e_right = EncoderHandler::instance()->getEncoderRight();
    if(millis() - lastCheck > 1000){
        if(prev_e_left != e_left || prev_e_left == 0){
            if(fabs(e_left) < e_set){
                if(!left){
                    leftPos += 1;
                } else {
                    leftNeg -=1;
                }
            } else if (fabs(e_left) > e_set){
                if(!left){
                    leftPos -= 1;
                } else {
                    leftNeg +=1;
                }
            }

            prev_e_left = e_left;
        }

        if(prev_e_right != e_right || prev_e_right == 0){
            if(fabs(e_right) < e_set){
                if(!right){
                    rightPos += 1;
                } else {
                    rightNeg -=1;
                }
            } else if(fabs(e_right) > e_set){
                if(!right){
                    rightPos -= 1;
                } else {
                    rightNeg +=1;
                }
            }

            prev_e_right = e_right;
        }

        lastCheck = millis();
    }
}



bool PickUpBehavior::wait(int sec){
    if(!waiting){
        waiting = true;
        time(&initTime);
        return true;
    } else {
        time(&currTime);
        int secSince = difftime(currTime, initTime);
        if(difftime(currTime, initTime) >= sec){
            waiting = false;
            return false;
        }
    }

    return true;
}






























