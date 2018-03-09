#include "ros/ros.h"

int main(int argc, char **argv){
    ros::init(argc, argv, "hive"); //initialize the server
    ros::NodeHandle n; //create a node handle


    ros::spin(); //spin the service as fast as you can
    return 0;
}
