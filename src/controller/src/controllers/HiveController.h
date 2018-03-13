#ifndef HIVE_CONTROLLER_H
#define HIVE_CONTROLLER_H
#include "ros/ros.h"
#include <string>
#include "hive_srv/CheckIn.h"
#include "hive_srv/roundType.h"

class HiveController{
    ros::NodeHandle n;

    ros::ServiceClient CheckInClient;
    ros::ServiceClient roundTypeClient;

    static HiveController* s_instance;
    HiveController();

public:
    static HiveController* instance();

    void CheckIn(std::string robotName);
    bool roundType();

};

#endif
