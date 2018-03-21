#include "SMACS.h"
// Initialize the instance
SMACS* SMACS::s_instance = 0;

SMACS::SMACS(){}

SMACS* SMACS::instance(){
    if(!s_instance)
        s_instance = new SMACS;
    return s_instance;
}


void SMACS::pushWithMutex(Behavior* b){
    // Check if behavior is null. If it is then do not put anything on stack
    if(b == NULL){
        cout << "STACK: " << "NULL pointer passed to stack" << endl;
        return;
    }

    //lock the stack from being used while we are doing something with it
    std::lock_guard<std::mutex> guard(stackMutex);
    //cout << "STACK: " << "Stack locked"<< endl;

    //if stack is not empty, check if behavior is stackable
    if(!behaviorStack.empty()){
        Behavior *top = behaviorStack.top();
        cout << "STACK: " << "Got top"<< endl;
        if(b->getType() == top->getType()){
            if(!b->isStackable()){
                cout << "STACK: " << "Behavior is not stackable. Was not pushed to stack"<< endl;
                return;
            }
        }
        //if the top behavior is pickup and we are trying to avoid center
        if(top->getType() == PICKUP_BEHAVIOR_TYPE && b->getType() == AVOID_CENTER_BEHAVIOR_TYPE){
            //pop pickup
            behaviorStack.pop();
            ClawController::instance()->wristUp();
        }

    }

    //stop the robot
    DriveController::instance()->stop();

    //push new behavior
    behaviorStack.push(b);
    cout << "STACK: " << "Pushed to stack"<< endl;

    //cout << "STACK: " << "Stack unlocked"<< endl;
}


void SMACS::push(Behavior* b){
    // Check if behavior is null. If it is then do not put anything on stack
    if(b == NULL){
        cout << "STACK: " << "NULL pointer passed to stack" << endl;
        return;
    }

    //lock the stack from being used while we are doing something with it
    cout << "STACK: " << "Pushing with no mutex"<< endl;

    //if stack is not empty, check if behavior is stackable
    if(!behaviorStack.empty()){
        Behavior *top = behaviorStack.top();
        //cout << "STACK: " << "Got top"<< endl;
        if(b->getType() == top->getType()){
            if(!b->isStackable()){
                //cout << "STACK: " << "Behavior is not stackable. Was not pushed to stack"<< endl;
                return;
            }
        }
        //if the top behavior is pickup and we are trying to avoid center
        if(top->getType() == PICKUP_BEHAVIOR_TYPE && b->getType() == AVOID_CENTER_BEHAVIOR_TYPE){
            //pop pickup
            behaviorStack.pop();
            ClawController::instance()->wristUp();
        }
    }



    //stop the robot
    DriveController::instance()->stop();

    //push new behavior
    behaviorStack.push(b);
    cout << "STACK: " << "Pushed to stack"<< endl;

    //cout << "STACK: " << "Stack unlocked"<< endl;
}

void SMACS::pop(){
    if(behaviorStack.empty()){
        cout << "STACK: " << "Behavior stack is already empty."<< endl;
        return;
    }

    //lock stack
    std::lock_guard<std::mutex> guard(stackMutex);
    //cout << "STACK: " << "Stack locked"<< endl;

    DriveController::instance()->stop();
    cout << "STACK: " << "Popped from stack"<< endl;

    //pop element
    behaviorStack.pop();

    //cout << "STACK: " << "Stack unlocked"<< endl;
}

void SMACS::pushNext(Behavior *b){
    if(b == NULL){
        cout << "STACK: " << "NULL pointer passed to stack"<< endl;
        return;
    }
   // cout << "STACK: " << "Stack not locked for push next"<< endl;

    // if stack is not empty, then save top element and pop it to get to the next element
    if(!behaviorStack.empty()){
        Behavior *hold = behaviorStack.top();
        behaviorStack.pop();

        //if stack is not empty, check if second elements is stackable
        if(!behaviorStack.empty()){
            Behavior *top = behaviorStack.top();
            if(b->getType() == top->getType()){
                if(!b->isStackable()){
                    cout << "STACK: " << "Behavior is not stackable. Was not pushed to stack"<< endl;
                    behaviorStack.push(hold);
                    return;
                }
            }
        }

        behaviorStack.push(b);
        behaviorStack.push(hold);
    } else {
        behaviorStack.push(b);
    }

    cout << "STACK: " << "Pushed next behavior to stack."<<endl;
}


bool SMACS::isEmpty(){return behaviorStack.empty();}

// This is a tick method
bool SMACS::tick(){
    // Lock stack
    std::lock_guard<std::mutex> guard(stackMutex);

    // If stack is not empty
    if(!behaviorStack.empty()){
        cout << "STACK: " << "Ticking behvior: "<< behaviorStack.top()->getCurrentBehavior()<< endl;
        int b = behaviorStack.top()->getCurrentBehavior();

        //if behavior changed update time
        if(b == PICKUP_BEHAVIOR_TYPE || b == DROP_BEHAVIOR_TYPE || b == DEFAULT_BEHAVIOR_TYPE || b == DRIVE_BEHAVIOR_TYPE ||
                b == SEARCH_FOR_DROP_BEHAVIOR_TYPE || b==AVOID_CENTER_BEHAVIOR_TYPE){
            setTime();
        }

        //if 3 min passed reset
        if(millis() - timeSinceLastUpdate >= 180000){
            TargetHandler::instance()->setEnabled(true);
            TargetHandler::instance()->setHasCube(false);
            SonarHandler::instance()->setEnable(true);
            ClawController::instance()->fingerOpen();
            ClawController::instance()->wristUp();
            behaviorStack = std::stack <Behavior*>();
            behaviorStack.push(new RandomSearchBehavior());
            setTime();
        }

        if(behaviorStack.top()->tick()){
            DriveController::instance()->stop();
            cout << "STACK: " << "Popped from stack"<< endl;
            //pop element
            behaviorStack.pop();
            //cout << "STACK: " << "Stack popped from tick"<< endl;
            return true;
        }
        //cout << "STACK: " << "Stack unlocked for tick"<< endl;
    }
    return false;
}






















