#include "ros/ros.h"
#include <vector>
#include "Robot.h"

#include "hive_srv/CheckIn.h"
#include "hive_srv/roundType.h"

ros::ServiceServer checkInServer;
ros::ServiceServer roundTypeServer;

bool CheckIn(hive_srv::CheckIn::Request &req, hive_srv::CheckIn::Response &res);
bool roundType(hive_srv::roundType::Request &req, hive_srv::roundType::Response &res);

using namespace std;

vector<Robot> robotList;

int main(int argc, char **argv){
    ros::init(argc, argv, "hive"); //initialize the server
    ros::NodeHandle n; //create a node handle

    checkInServer = n.advertiseService("CheckIn", CheckIn);
    roundTypeServer = n.advertiseService("roundType", roundType);


    cout<< "Server spininnnn brah"<<endl;
    ros::spin(); //spin the service as fast as you can
    return 0;
}

bool CheckIn(hive_srv::CheckIn::Request &req, hive_srv::CheckIn::Response &res){
    string robotName = req.robotName;
    // Loop through robots. If robot exist dont add
    for(int i=0; i < robotList.size(); i++){
        if(robotName == robotList[i].robotName){
            cout << "CHECKIN: Robot "<<robotName<<" exists. Not added."<<endl;
            return true;
        }
    }

    robotList.push_back(Robot(robotName));
    cout << "CHECKIN: Robot "<<robotName<<" added. Count: "<<robotList.size()<<endl;
    return true;
}

bool roundType(hive_srv::roundType::Request &req, hive_srv::roundType::Response &res){
    if(robotList.size()>3){
        cout<<"ROUNDTYPE: "<<" sending round type final "<<endl;
        res.isPrelim = false;
    } else {
        cout<<"ROUNDTYPE: "<<" sending round type prelim "<<endl;
        res.isPrelim = true;
    }
    return true;
}


























