#ifndef OFFSET_CONTROLLER_H
#define OFFSET_CONTROLLER_H
#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include "../handlers/Handlers.h"

class OffsetController{
    ros::Publisher offsetPublisher;
    static OffsetController* s_instance;
    OffsetController(){}

public:
    static OffsetController* instance();
    void registerPublishers(ros::Publisher& offsetPublisher);

    void sendOffsets(float x, float y, float w, float z);
};

#endif
