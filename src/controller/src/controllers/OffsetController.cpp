#include "OffsetController.h"

OffsetController* OffsetController::s_instance = 0;

OffsetController* OffsetController::instance(){
    if(!s_instance)
        s_instance = new OffsetController;
    return s_instance;
}

void OffsetController::registerPublishers(ros::Publisher& offsetPublisher){
        this->offsetPublisher = offsetPublisher;
}


void OffsetController::sendOffsets(float x, float y, float w, float z){
    geometry_msgs::Twist msg;
    msg.angular.x = x;
    msg.angular.y = y;

    msg.linear.x = w;
    msg.linear.y = z;
}

