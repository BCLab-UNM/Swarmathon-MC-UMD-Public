#include "AvoidCenterBehavior.h"

bool AvoidCenterBehavior::tick(){
    switch(stage){
        case WAIT:
        {
            // stop rover
            DriveController::instance()->sendDriveCommand(0, 0);
            // check if we see center tags
            if(TargetHandler::instance()->getNumberOfCenterTagsSeen() > 0){
                vector <Tag> tags = TargetHandler::instance()->getCenterTags();
                for ( auto & tag : tags ) {
                    cout << "AVOIDCENTER: yaw: "<< tag.calcYaw() << " pitch: "<<tag.calcPitch() << " roll : " << tag.calcRoll() << endl;
                }
            } else {
                // If we do not see center tags - exit
                return true;
            }

            break;
        }
        case TURN:
        {
            break;
        }
        case DRIVE:
        {
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
    // checks if tag is on the right or left side of the image
    if (tag.getPositionX() + camera_offset_correction > 0) {
      count_right_collection_zone_tags++;

    } else {
      count_left_collection_zone_tags++;
    }
      }

  }


  // Did any tags indicate that the robot is inside the collection zone?
  return count_left_collection_zone_tags + count_right_collection_zone_tags > 0;

}
