#include "HiveController.h"
HiveController* HiveController::s_instance = 0;

HiveController* HiveController::instance(){
    if(!s_instance)
        s_instance = new HiveController;
    return s_instance;
}

HiveController::HiveController(){
    CheckInClient = n.serviceClient<hive_srv::CheckIn>("CheckIn");
    roundTypeClient = n.serviceClient<hive_srv::roundType>("roundType");
}

void HiveController::CheckIn(std::string robotName){
    hive_srv::CheckIn srv;
    srv.request.robotName = robotName;
    CheckInClient.call(srv);
}

bool HiveController::roundType(){
    hive_srv::roundType srv;
    roundTypeClient.call(srv);
    return srv.response.isPrelim;
}
