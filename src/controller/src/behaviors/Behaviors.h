#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include "ros/ros.h"
//#include "../controllers/Controllers.h"
#include "std_msgs/Float32.h"

/**
 * @brief The Behavor class - abstract class that represents a behavior.
 * Has only one method that has to be implemented. The rest is up to the user
 */
class Behavior{
    public:
        virtual bool tick() = 0;    // =0 makes it a pure virtual method, meaning you do not have to implement it in the scope of Behavior
};



/**
 * @brief The SimpleBehavior class - Simple testing behavior that probably wont work
 */
class SimpleBehavior: public Behavior{
    ros::Publisher test;

    public:
        SimpleBehavior(ros::Publisher&);
        bool tick();
};

#endif