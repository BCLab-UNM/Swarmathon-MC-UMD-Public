#include <ros/ros.h>

//ROS libraries
#include <tf/transform_datatypes.h>

//ROS messages
#include <std_msgs/Float32.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Quaternion.h>
#include <geometry_msgs/QuaternionStamped.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Range.h>
#include <std_msgs/UInt8.h>

//Package include
#include <usbSerial.h>

#include "DriveFix.h"

using namespace std;

//aBridge functions
void driveCommandHandler(const geometry_msgs::Twist::ConstPtr& message);
void fingerAngleHandler(const std_msgs::Float32::ConstPtr& angle);
void wristAngleHandler(const std_msgs::Float32::ConstPtr& angle);
void serialActivityTimer(const ros::TimerEvent& e);
void publishRosTopics();
void parseData(string data);
std::string getHumanFriendlyTime();

//Globals
nav_msgs::Odometry offsetOdom;
geometry_msgs::QuaternionStamped fingerAngle;
geometry_msgs::QuaternionStamped wristAngle;
sensor_msgs::Imu imu;
nav_msgs::Odometry odom;
sensor_msgs::Range sonarLeft;
sensor_msgs::Range sonarCenter;
sensor_msgs::Range sonarRight;
nav_msgs::Odometry odom_XY;
USBSerial usb;
const int baud = 115200;
char dataCmd[] = "d\n";
char moveCmd[16];
char host[128];
const float deltaTime = 0.1; //abridge's update interval
int currentMode = 0;
string publishedName;

// Allowing messages to be sent to the arduino too fast causes a disconnect
// This is the minimum time between messages to the arduino in microseconds.
// Only used with the gripper commands to fix a manual control bug.
unsigned int min_usb_send_delay = 100;

float heartbeat_publish_interval = 2;


//PID constants and arrays
const int histArrayLength = 1000;

float velFF = 0; //velocity feed forward
int stepV = 0; //keeps track of the point in the evArray for adding new error each update cycle.
float evArray[histArrayLength]; //history array of previous error for (arraySize/hz) seconds (error Velocity Array)
float velError[4] = {0,0,0,0}; //contains current velocity error and error 3 steps in the past.

int stepY = 0; //keeps track of the point in the eyArray for adding new error each update cycle.
float eyArray[histArrayLength]; //history array of previous error for (arraySize/hz) seconds (error Yaw Array)
float yawError[4] = {0,0,0,0}; //contains current yaw error and error 3 steps in the past.

float prevLin = 0;
float prevYaw = 0;

ros::Time prevDriveCommandUpdateTime;

//Publishers
ros::Publisher fingerAnglePublish;
ros::Publisher wristAnglePublish;
ros::Publisher imuPublish;
ros::Publisher odomPublish;
ros::Publisher sonarLeftPublish;
ros::Publisher sonarCenterPublish;
ros::Publisher sonarRightPublish;
ros::Publisher infoLogPublisher;
ros::Publisher heartbeatPublisher;

ros::Publisher odomXY;
ros::Publisher odomFilteredOffset;

ros::Publisher encoderPublisher;

//Subscribers
ros::Subscriber driveControlSubscriber;
ros::Subscriber fingerAngleSubscriber;
ros::Subscriber wristAngleSubscriber;
ros::Subscriber modeSubscriber;
ros::Subscriber IMUOffsetSubscriber;
ros::Subscriber offsetSubscriber;
ros::Subscriber odomFilteredSub;

//Timers
ros::Timer publishTimer;
ros::Timer publish_heartbeat_timer;

//Callback handlers
void publishHeartBeatTimerEventHandler(const ros::TimerEvent& event);
void modeHandler(const std_msgs::UInt8::ConstPtr& message);
void offsetHandler(const geometry_msgs::Quaternion& msg);
void odomHandler(const nav_msgs::Odometry::ConstPtr& msg);

int _left;
int _right;
bool _calcError = false;
float _initRoll = 0;
float _initPitch = 0;
float _initYaw = 0;

float _rollError = 0;
float _pitchError = 0;
float _yawError = 0;

int e_left = 0;
int e_right = 0;

int left_v;
int right_v;
int corrected_v_left;
int corrected_v_right;


float curr_odom_x = 0;
float curr_odom_y = 0;
float odom_offset_x = 0;
float odom_offset_y = 0;

tf::Quaternion offset_quat = tf::createIdentityQuaternion();
tf::Quaternion curr_quat = tf::createIdentityQuaternion();
tf::Quaternion set_quat = tf::createIdentityQuaternion();

DriveFix fix(&e_left, &e_right, &left_v, &right_v, &corrected_v_left, &corrected_v_right, 500);


int main(int argc, char **argv) {
    
    gethostname(host, sizeof (host));
    string hostname(host);
    ros::init(argc, argv, (hostname + "_ABRIDGE"));
    
    ros::NodeHandle param("~");
    string devicePath;
    param.param("device", devicePath, string("/dev/ttyUSB0"));
    usb.openUSBPort(devicePath, baud);

    
    sleep(5);
    
    ros::NodeHandle aNH;
    
    if (argc >= 2) {
        publishedName = argv[1];
        cout << "Welcome to the world of tomorrow " << publishedName << "!  ABridge module started." << endl;
    } else {
        publishedName = hostname;
        cout << "No Name Selected. Default is: " << publishedName << endl;
    }
    
    fingerAnglePublish = aNH.advertise<geometry_msgs::QuaternionStamped>((publishedName + "/fingerAngle/prev_cmd"), 10);
    wristAnglePublish = aNH.advertise<geometry_msgs::QuaternionStamped>((publishedName + "/wristAngle/prev_cmd"), 10);
    imuPublish = aNH.advertise<sensor_msgs::Imu>((publishedName + "/imu"), 10);
    odomPublish = aNH.advertise<nav_msgs::Odometry>((publishedName + "/odom"), 10);
    sonarLeftPublish = aNH.advertise<sensor_msgs::Range>((publishedName + "/sonarLeft"), 10);
    sonarCenterPublish = aNH.advertise<sensor_msgs::Range>((publishedName + "/sonarCenter"), 10);
    sonarRightPublish = aNH.advertise<sensor_msgs::Range>((publishedName + "/sonarRight"), 10);
    infoLogPublisher = aNH.advertise<std_msgs::String>("/infoLog", 1, true);
    heartbeatPublisher = aNH.advertise<std_msgs::String>((publishedName + "/abridge/heartbeat"), 1, true);
    encoderPublisher = aNH.advertise<geometry_msgs::Twist>((publishedName + "/encoders"), 10);
    odomFilteredOffset = aNH.advertise<nav_msgs::Odometry>((publishedName + "/odom/filteredOffset"), 10);

    odomXY= aNH.advertise<nav_msgs::Odometry>((publishedName + "/odomXY"), 10);

    driveControlSubscriber = aNH.subscribe((publishedName + "/driveControl"), 10, driveCommandHandler);
    fingerAngleSubscriber = aNH.subscribe((publishedName + "/fingerAngle/cmd"), 1, fingerAngleHandler);
    wristAngleSubscriber = aNH.subscribe((publishedName + "/wristAngle/cmd"), 1, wristAngleHandler);
    modeSubscriber = aNH.subscribe((publishedName + "/mode"), 1, modeHandler);
    odomFilteredSub = aNH.subscribe((publishedName + "/odom/filtered"), 1, odomHandler);

    offsetSubscriber = aNH.subscribe((publishedName + "/odom/Offsets"), 1, offsetHandler);


    
    publishTimer = aNH.createTimer(ros::Duration(deltaTime), serialActivityTimer);
    publish_heartbeat_timer = aNH.createTimer(ros::Duration(heartbeat_publish_interval), publishHeartBeatTimerEventHandler);
    
    imu.header.frame_id = publishedName+"/base_link";
    
    odom.header.frame_id = publishedName+"/odom";
    odom.child_frame_id = publishedName+"/base_link";

    for (int i = 0; i < histArrayLength; i++)
    {
    evArray[i] = 0;
    eyArray[i] = 0;
    }
    
    prevDriveCommandUpdateTime = ros::Time::now();

    ros::spin();
    
    return EXIT_SUCCESS;
}

//This command handler recives a linear velocity setpoint and a angular yaw error
//and produces a command output for the left and right motors of the robot.
//See the following paper for description of PID controllers.
//Bennett, Stuart (November 1984). "Nicholas Minorsky and the automatic steering of ships". IEEE Control Systems Magazine. 4 (4): 10–15. doi:10.1109/MCS.1984.1104827. ISSN 0272-1708.
void driveCommandHandler(const geometry_msgs::Twist::ConstPtr& message) {
   

  float left = (message->linear.x); //target linear velocity in meters per second
  float right = (message->angular.z); //angular error in radians

  left_v = left;
  right_v = right;

  _left = left;
  _right = right;

  fix.compute();

  left = corrected_v_left;
  right = corrected_v_right;

  cout<<"DRIVEFIX: e_left = "<<e_left << " e_right = " << e_right << endl;
  cout<<"DRIVEFIX: left = "<< corrected_v_left << " right = " << corrected_v_right << endl;

  // Cap motor commands at 120. Experimentally determined that high values (tested 180 and 255) can cause 
  // the hardware to fail when the robot moves itself too violently.
  int max_motor_cmd = 255;

  // Check that the resulting motor commands do not exceed the specified safe maximum value
  if (left > max_motor_cmd)
  {
    left = max_motor_cmd;
  }
  else if (left < -max_motor_cmd)
  {
    left = - max_motor_cmd;
  }

  if (right > max_motor_cmd)
  {
    right = max_motor_cmd;
  }
  else if (right < -max_motor_cmd)
  {
    right = -max_motor_cmd;
  }

  int leftInt = left;
  int rightInt = right;
    
  sprintf(moveCmd, "v,%d,%d\n", leftInt, rightInt); //format data for arduino into c string
  usb.sendData(moveCmd);                      //send movement command to arduino over usb
  memset(&moveCmd, '\0', sizeof (moveCmd));   //clear the movement command string
}


// The finger and wrist handlers receive gripper angle commands in floating point
// radians, write them to a string and send that to the arduino
// for processing.
void fingerAngleHandler(const std_msgs::Float32::ConstPtr& angle) {

  // To throttle the message rate so we don't lose connection to the arduino
  usleep(min_usb_send_delay);
  
  char cmd[16]={'\0'};

  // Avoid dealing with negative exponents which confuse the conversion to string by checking if the angle is small
  if (angle->data < 0.01) {
    // 'f' indicates this is a finger command to the arduino
    sprintf(cmd, "f,0\n");
  } else {
    sprintf(cmd, "f,%.4g\n", angle->data);
  }
  usb.sendData(cmd);
  memset(&cmd, '\0', sizeof (cmd));
}

void wristAngleHandler(const std_msgs::Float32::ConstPtr& angle) {
  // To throttle the message rate so we don't lose connection to the arduino
  usleep(min_usb_send_delay);
  
    char cmd[16]={'\0'};

    // Avoid dealing with negative exponents which confuse the conversion to string by checking if the angle is small
  if (angle->data < 0.01) {
    // 'w' indicates this is a wrist command to the arduino
    sprintf(cmd, "w,0\n");
  } else {
    sprintf(cmd, "w,%.4g\n", angle->data);
  }
  usb.sendData(cmd);
  memset(&cmd, '\0', sizeof (cmd));
}

void serialActivityTimer(const ros::TimerEvent& e) {
    usb.sendData(dataCmd);
    parseData(usb.readData());
    publishRosTopics();
}

void publishRosTopics() {
    fingerAnglePublish.publish(fingerAngle);
    wristAnglePublish.publish(wristAngle);
    imuPublish.publish(imu);
    odomPublish.publish(odom);
    sonarLeftPublish.publish(sonarLeft);
    sonarCenterPublish.publish(sonarCenter);
    sonarRightPublish.publish(sonarRight);
    odomFilteredOffset.publish(offsetOdom);
}

void parseData(string str) {
    istringstream oss(str);
    string sentence;
    
    while (getline(oss, sentence, '\n')) {
		istringstream wss(sentence);
		string word;

		vector<string> dataSet;
		while (getline(wss, word, ',')) {
			dataSet.push_back(word);
		}

		if (dataSet.size() >= 3 && dataSet.at(1) == "1") {

            if (dataSet.at(0) == "GRF") {
                fingerAngle.header.stamp = ros::Time::now();
                fingerAngle.quaternion = tf::createQuaternionMsgFromRollPitchYaw(atof(dataSet.at(2).c_str()), 0.0, 0.0);
            }
			else if (dataSet.at(0) == "GRW") {
				wristAngle.header.stamp = ros::Time::now();
				wristAngle.quaternion = tf::createQuaternionMsgFromRollPitchYaw(atof(dataSet.at(2).c_str()), 0.0, 0.0);
			}
			else if (dataSet.at(0) == "IMU") {
				imu.header.stamp = ros::Time::now();
				if(_left == _right && left != 0){
					if(!_calcError){
						_initRoll = atof(dataSet.at(8).c_str());
						_initPitch = atof(dataSet.at(9).c_str());
						_initYaw = atof(dataSet.at(10).c_str()) - _yawError;	
						_calcError = true;
					}
					_rollError = atof(dataSet.at(8).c_str()) - _initRoll;
					_pitchError = atof(dataSet.at(9).c_str()) - _initPitch;
					_yawError = atof(dataSet.at(10).c_str()) - _initYaw;
				} else{
					_calcError = false;
				}
				imu.linear_acceleration.x = atof(dataSet.at(2).c_str());
				imu.linear_acceleration.y = 0; //atof(dataSet.at(3).c_str());
				imu.linear_acceleration.z = atof(dataSet.at(4).c_str());
				imu.angular_velocity.x = atof(dataSet.at(5).c_str());
				imu.angular_velocity.y = atof(dataSet.at(6).c_str());
				imu.angular_velocity.z = atof(dataSet.at(7).c_str());
				imu.orientation = tf::createQuaternionMsgFromRollPitchYaw(atof(dataSet.at(8).c_str()), 
							atof(dataSet.at(9).c_str()), 
							atof(dataSet.at(10).c_str())- _yawError);
			}
			else if (dataSet.at(0) == "ODOM") {
				odom.header.stamp = ros::Time::now();
				odom.pose.pose.position.x += atof(dataSet.at(2).c_str()) / 100.0;
				odom.pose.pose.position.y += atof(dataSet.at(3).c_str()) / 100.0;
				odom.pose.pose.position.z = 0.0;
                odom.pose.pose.orientation = tf::createQuaternionMsgFromYaw(atof(dataSet.at(4).c_str()));

				odom.twist.twist.linear.x = atof(dataSet.at(5).c_str()) / 100.0;
				odom.twist.twist.linear.y = atof(dataSet.at(6).c_str()) / 100.0;
				odom.twist.twist.angular.z = atof(dataSet.at(7).c_str());

                odom_XY.pose.pose.position.x += (atof(dataSet.at(10).c_str()) / 100.0);
                odom_XY.pose.pose.position.y += (atof(dataSet.at(11).c_str()) / 100.0);
                odom_XY.pose.pose.position.z = 0.0;
                odom_XY.pose.pose.orientation = tf::createQuaternionMsgFromYaw(atof(dataSet.at(4).c_str()));
                odom_XY.twist.twist.linear.x = 0;
                odom_XY.twist.twist.linear.y = 0;
                odom_XY.twist.twist.angular.z = 0;

                odomXY.publish(odom_XY);

                geometry_msgs::Twist msg;
                e_left = atof(dataSet.at(8).c_str());
                e_right = atof(dataSet.at(9).c_str());

                msg.angular.x = e_left;
                msg.angular.y = e_right;

                encoderPublisher.publish(msg);
			}
			else if (dataSet.at(0) == "USL") {
				sonarLeft.header.stamp = ros::Time::now();
				sonarLeft.range = atof(dataSet.at(2).c_str()) / 100.0;
			}
			else if (dataSet.at(0) == "USC") {
				sonarCenter.header.stamp = ros::Time::now();
				sonarCenter.range = atof(dataSet.at(2).c_str()) / 100.0;
			}
			else if (dataSet.at(0) == "USR") {
				sonarRight.header.stamp = ros::Time::now();
				sonarRight.range = atof(dataSet.at(2).c_str()) / 100.0;
			}

		}
	}
}

void modeHandler(const std_msgs::UInt8::ConstPtr& message) {
	currentMode = message->data;
}

void publishHeartBeatTimerEventHandler(const ros::TimerEvent&) {
    std_msgs::String msg;
    msg.data = "";
    heartbeatPublisher.publish(msg);
}

void offsetHandler(const geometry_msgs::Quaternion& msg){
    odom_offset_x = msg.x;
    odom_offset_y = msg.y;



    tf::Quaternion q(0.0, 0.0, msg.z, msg.w);
    tf::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);
    set_quat = tf::createQuaternionFromRPY(roll, pitch, yaw);

    offset_quat = curr_quat*set_quat.inverse();
    offset_quat.normalize();
}

void odomHandler(const nav_msgs::Odometry::ConstPtr& msg){
    curr_odom_x = msg->pose.pose.position.x;
    curr_odom_y = msg->pose.pose.position.y;

    tf::quaternionMsgToTF(msg->pose.pose.orientation, curr_quat);



    offsetOdom.pose= msg->pose;
    offsetOdom.pose.pose.position.x -= odom_offset_x;
    offsetOdom.pose.pose.position.y -= odom_offset_y;

    //tf::quaternionTFToMsg((curr_quat*offset_quat.inverse()).normalize(), offsetOdom.pose.pose.orientation);
    tf::quaternionTFToMsg(set_quat, offsetOdom.pose.pose.orientation);


    offsetOdom.twist = msg->twist;
}























