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
    geometry_msgs::Quaternion  msg;
    msg.x = x;
    msg.y = y;

    msg.w = w;
    msg.z = z;

    tf::Quaternion q(0, 0, z, w);
    tf::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);
    DriveController::instance()->setResetTheta(yaw);

    offsetPublisher.publish(msg);
}

