#include "AvoidCenterBehavior.h"

bool AvoidCenterBehavior::tick(){
    switch(stage){
        case WAIT:
        {
            // stop rover
            DriveController::instance()->sendDriveCommand(0, 0);
            // check if we see center tags
            if(TargetHandler::instance()->getNumberOfCenterTagsSeen() > 0){
//                vector <Tag> tags = TargetHandler::instance()->getCenterTags();
//                for ( auto & tag : tags ) {
//                    cout << "AVOIDCENTER: yaw: "<< tag.calcYaw() << " pitch: "<<tag.calcPitch() << " roll : " << tag.calcRoll() << endl;
//                }
                //save tags for later use
                centerTags = TargetHandler::instance()->getCenterTags();
                stage = DRIVEBACK;
                x = OdometryHandler::instance()->getX();
                y = OdometryHandler::instance()->getY();
                theta = OdometryHandler::instance()->getTheta();
            } else {
                // If we do not see center tags - exit
                return true;
            }
            break;
        }
        case DRIVEBACK:
        {
            float currX = OdometryHandler::instance()->getX();
            float currY = OdometryHandler::instance()->getY();
            float distance = hypot(x - currX, y - currY);
            if(distance < 0.2){
                //drive back 0.4 meters. safe distance
                DriveController::instance()->sendDriveCommand(-driveSpeed, -driveSpeed);
            } else {
                stage = TURN;
                x = OdometryHandler::instance()->getX();
                y = OdometryHandler::instance()->getY();
                theta = OdometryHandler::instance()->getTheta();
            }

            break;
        }
        case TURN:
        {
            //decide if it is better to turn right or left
            // if yaw in center tags is < 0 then turn right
            // if yaw in center tags is > 0 turn left
            // turn 45 degrees
            if(!turnLock){
                if(checkForCollectionZoneTags(centerTags) < 0){
                    theta += M_PI_4;
                } else {
                    theta -= M_PI_4;
                }
                turnLock = true;
            }

            if(DriveController::instance()->turnToTheta(theta)){
                stage = DRIVE;
                turnLock = false;
                x = OdometryHandler::instance()->getX();
                y = OdometryHandler::instance()->getY();
                theta = OdometryHandler::instance()->getTheta();
            }

            break;
        }
        case DRIVE:
        {
            if(TargetHandler::instance()->getNumberOfCenterTagsSeen() > 0){
                stage = WAIT;
            } else {
                float currX = OdometryHandler::instance()->getX();
                float currY = OdometryHandler::instance()->getY();
                float distance = hypot(x - currX, y - currY);
                if(distance < 0.4){
                     DriveController::instance()->sendDriveCommand(driveSpeed, driveSpeed);
                } else {
                    return true;
                }
            }
            break;
        }

        return false;
    }
}



bool AvoidCenterBehavior::checkForCollectionZoneTags( vector<Tag> tags ) {
    int count_right_collection_zone_tags = 0;
    int count_left_collection_zone_tags = 0;

  for ( auto & tag : tags ) {

    // Check the orientation of the tag. If we are outside the collection zone the yaw will be positive so treat the collection zone as an obstacle.
    //If the yaw is negative the robot is inside the collection zone and the boundary should not be treated as an obstacle.
    //This allows the robot to leave the collection zone after dropping off a target.
    if ( tag.calcYaw() > 0 )
    {
        count_right_collection_zone_tags++;
    } else {
        count_left_collection_zone_tags--;
    }

  }


  // Did any tags indicate that the robot is inside the collection zone?
  return count_left_collection_zone_tags + count_right_collection_zone_tags > 0;

}
