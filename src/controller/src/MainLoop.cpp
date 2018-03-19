#include <ros/ros.h>
#include <sys/time.h>

// ROS libraries
#include <angles/angles.h>
#include <random_numbers/random_numbers.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

// ROS messages
#include <std_msgs/Float32.h>
#include <std_msgs/Int16.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/String.h>
#include <sensor_msgs/Joy.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <apriltags_ros/AprilTagDetectionArray.h>
#include <std_msgs/Float32MultiArray.h>
#include "swarmie_msgs/Waypoint.h"

#include <ros/ros.h>
#include <signal.h>

//using stack for the stack of behaviors
#include "SMACS.h"
//include this to init the handlers
#include "handlers/Handlers.h"
#include "behaviors/Behaviors.h"

#include "controllers/DriveController.h"
#include "controllers/ClawController.h"
#include "controllers/OffsetController.h"
#include "controllers/HiveController.h"

#include "message_filters/subscriber.h"

using namespace std;


int currentMode = 0;//mode of the robot. Manual or auto
char host[128];
string hostname(host);
string publishedName;




// Methods that handle
void sigintEventHandler(int signal);
void modeHandler(const std_msgs::UInt8::ConstPtr& message);
void joyCmdHandler(const sensor_msgs::Joy::ConstPtr& message);
void publishHeartBeatTimerEventHandler(const ros::TimerEvent& event);
void publishStatusTimerEventHandler(const ros::TimerEvent& event);
void tick(const ros::TimerEvent&); //main tick of the robot
long millis();



//Publishers
ros::Publisher nodeTest;
ros::Publisher driveControlPublish;
ros::Publisher heartbeatPublisher;
ros::Publisher status_publisher;
ros::Publisher fingerAnglePublish;
ros::Publisher wristAnglePublish;
ros::Publisher offsetPublish;

//Subscribers
ros::Subscriber modeSubscriber;
ros::Subscriber joySubscriber;
ros::Subscriber targetSubscriber;
ros::Subscriber leftSonarSubscriber;
ros::Subscriber centerSonarSubscriber;
ros::Subscriber rightSonarSubscriber;
ros::Subscriber odometrySubscriber;
ros::Subscriber encoderSubscriber;
ros::Subscriber IMUSubscriber;
ros::Subscriber odomFilteredFilter;


//Times for ticking the stack
ros::Timer stateMachineTimer;
ros::Timer publish_status_timer;
ros::Timer publish_heartbeat_timer;
const float behaviourLoopTimeStep = 0.1; // time between the behavior loop calls
const float status_publish_interval = 1;
const float heartbeat_publish_interval = 2;


//global variables that we are constantly use
//They are used a lot so they are global
geometry_msgs::Twist velocity;
bool stopped = true;

bool collisionEnabled = false;

bool init = false;

long initTime = 0;


int main(int argc, char **argv) {

    gethostname(host, sizeof (host));
    string hostname(host);

    if (argc >= 2) {
    publishedName = argv[1];
    cout << "Welcome to the world of tomorrow " << publishedName
         << "!  Controller turnDirectionule started." << endl;
    } else {
        publishedName = hostname;
        cout << "No Name Selected. Default is: " << publishedName << endl;
    }

    // NoSignalHandler so we can catch SIGINT ourselves and shutdown the node
    ros::init(argc, argv, (publishedName + "_CONTROLLER"), ros::init_options::NoSigintHandler);
    ros::NodeHandle nh;

    // Register the SIGINT event handler so the node can shutdown properly
    signal(SIGINT, sigintEventHandler);

    driveControlPublish = nh.advertise<geometry_msgs::Twist>((publishedName + "/driveControl"), 10);
    heartbeatPublisher = nh.advertise<std_msgs::String>((publishedName + "/controller/heartbeat"), 1, true);
    status_publisher = nh.advertise<std_msgs::String>((publishedName + "/status"), 1, true);
    nodeTest = nh.advertise<std_msgs::Int16>((publishedName + "/test"), 1, true);
    fingerAnglePublish = nh.advertise<std_msgs::Float32>((publishedName + "/fingerAngle/cmd"), 1, true);
    wristAnglePublish = nh.advertise<std_msgs::Float32>((publishedName + "/wristAngle/cmd"), 1, true);
    offsetPublish = nh.advertise<geometry_msgs::Quaternion>((publishedName + "/odom/Offsets"), 1);


    modeSubscriber = nh.subscribe((publishedName + "/mode"), 1, modeHandler);
    joySubscriber = nh.subscribe((publishedName + "/joystick"), 10, joyCmdHandler);
    leftSonarSubscriber = nh.subscribe((publishedName + "/sonarLeft"), 10, &SonarHandler::handleLeft, SonarHandler::instance());
    centerSonarSubscriber = nh.subscribe((publishedName + "/sonarCenter"), 10, &SonarHandler::handleCenter, SonarHandler::instance());
    rightSonarSubscriber = nh.subscribe((publishedName + "/sonarRight"), 10, &SonarHandler::handleRight, SonarHandler::instance());
    odometrySubscriber = nh.subscribe((publishedName + "/odom/filteredOffset"), 10, &OdometryHandler::handle, OdometryHandler::instance());
    targetSubscriber = nh.subscribe((publishedName + "/targets"), 10, &TargetHandler::handle, TargetHandler::instance());
    encoderSubscriber = nh.subscribe((publishedName + "/encoders"), 10, &EncoderHandler::handle, EncoderHandler::instance());
    IMUSubscriber = nh.subscribe((publishedName + "/imu"), 10, &IMUHandler::handle, IMUHandler::instance());

    //Timers to publish something.
    stateMachineTimer = nh.createTimer(ros::Duration(behaviourLoopTimeStep), tick);
    publish_status_timer = nh.createTimer(ros::Duration(status_publish_interval), publishStatusTimerEventHandler);
    publish_heartbeat_timer = nh.createTimer(ros::Duration(heartbeat_publish_interval), publishHeartBeatTimerEventHandler);

    //register controllers
    DriveController::instance()->registerDrivePublisher(driveControlPublish);
    ClawController::instance()->registerPublishers(fingerAnglePublish, wristAnglePublish);
    OffsetController::instance()->registerPublishers(offsetPublish);


    // Put the first behavior on stack

    //SMACS::instance()->push(new SearchBehavior());              TESTING PURPOSES ONLY
    //SMACS::instance()->push(new SearchAlgorithmBehavior());
    //SMACS::instance()->push(new SearchAlgorithmRelativeBehavior());

    // Disable the sonar because the robot is not doing anything yet
    SonarHandler::instance()-> setEnable(false);

    // Close fingers
    ClawController::instance()->fingerOpen();
    // Put wist up
    ClawController::instance()->wristUp();

    HiveController::instance()->CheckIn(publishedName);

    // Spin the node
    ros::spin();

    return EXIT_SUCCESS;
}

void tick(const ros::TimerEvent&) {
    // To print log "tail -f path/"name of log file".txt | grep "TAG""
    // If mode is auto
    if (currentMode == 2 || currentMode == 3) {
        if(!init){
            // Set heading and offset the position
            float theta = IMUHandler::instance()->theta;
            float x = 0 + (1 * cos(theta));
            float y = 0 + (1 * sin(theta));

            OffsetController::instance()->sendOffsets(-x, -y, IMUHandler::instance()->w, IMUHandler::instance()->z);
            OffsetController::instance()->centerX = 0;
            OffsetController::instance()->centerY = 0;
            OffsetController::instance()->centerTheta = theta;


            // Put the first behavior on stack
            SMACS::instance()->push(new SearchBehavior());
            SonarHandler::instance()->setEnable(true);


            // Get round type
            bool roundType = HiveController::instance()->roundType();
            cout <<"ROUNDTYPE: "<< roundType<<endl;

            init = true;
            initTime = millis();
            // Sleep to let offset reset
        }
    	// If sonar handler is not enables
        if(!collisionEnabled){
            //SonarHandler::instance()->setEnable(true);
            collisionEnabled = true;
        }


        if(millis() - initTime > 2000){
            // Tick the SMACS
            SMACS::instance()->tick();
        }


        //Flag that states that robot is in auto
        stopped = false;
    } else {    //manual
   		// If robot is not stopped
        if(!stopped){
        	// Stop robot
            DriveController::instance()->stop();
            stopped = true;
            if(!TargetHandler::instance()->getHasCube()){
                ClawController::instance()->fingerOpen();
                ClawController::instance()->wristUp();
            }
        }
    }
}


// This publishes a message that the the robot is online
void publishStatusTimerEventHandler(const ros::TimerEvent&) {
  std_msgs::String msg;
  //Added + because we will use custom arduino code
  msg.data = "MC+";
  status_publisher.publish(msg);
}

// This receives the current mode of the robot from the GUI
void modeHandler(const std_msgs::UInt8::ConstPtr& message) {
  currentMode = message->data;
}

//This receives the messages from the GUI about the controller 
void joyCmdHandler(const sensor_msgs::Joy::ConstPtr& message) {
  const int max_motor_cmd = 255;
  if (currentMode == 0 || currentMode == 1) {
    float linear  = abs(message->axes[4]) >= 0.1 ? message->axes[4]*max_motor_cmd : 0.0;
    float angular = abs(message->axes[3]) >= 0.1 ? message->axes[3]*max_motor_cmd : 0.0;

    float left = linear - angular;
    float right = linear + angular;

    if(left > max_motor_cmd) {
      left = max_motor_cmd;
    }
    else if(left < -max_motor_cmd) {
      left = -max_motor_cmd;
    }

    if(right > max_motor_cmd) {
      right = max_motor_cmd;
    }
    else if(right < -max_motor_cmd) {
      right = -max_motor_cmd;
    }

    DriveController::instance()->sendDriveCommand(left, right);
  }
}

// I think this is used to shut down the robot if it is out of range
void sigintEventHandler(int sig) {
  // All the default sigint handler does is call shutdown()
  ros::shutdown();
}

// This is currently not used but it could publish some message every 10th of a sec
void publishHeartBeatTimerEventHandler(const ros::TimerEvent&) {
  std_msgs::String msg;
  msg.data = "";
  heartbeatPublisher.publish(msg);
}


long millis(){
    struct timeval tp;
    gettimeofday(&tp, 0);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}



























