#ifndef SEARCH_ALGORITHM_RELATIVE_BEHAVIOR_H
#define SEARCH_ALGORITHM_RELATIVE_BEHAVIOR_H

#include "BehaviorInterface.h"
#include "../controllers/DriveController.h"
#include <cmath>
#include <math.h>

class SearchAlgorithmRelativeBehavior: public Behavior{
    bool initialize = true;
    bool first_checking=true;
    bool initialDrive = true;
    bool initialCheck = true;
    bool AlgorithmA = false;
    bool AlgorithmB = false;
    bool AlgorithmC = false;
    bool initial = true;
    bool first = true;
    bool second = true;
    bool third = true;

    float initial_theta;

    float theta;
    float distance;

    //These are distance for trigonometric sides
    float distance1;
    float distance2;

    float left;
    float center;
    float right;

    // intended algorithm coordinates
    float xi; 
    float yi;
    // intended algorithm coordinates corrected through rotation
    float x;
    float y;
    // x and y iterator
    float xiterator;
    float yiterator;

    float iterator;

    int msleep(unsigned long milisec)
    { //Funvtion to stop the program for miliseconds
        struct timespec req={0};
        time_t sec=(int)(milisec/1000);
        milisec=milisec-(sec*1000);
        req.tv_sec=sec;
        req.tv_nsec=milisec*1000000L;
        while(nanosleep(&req,&req)==-1)
            continue;
        return 1;
    }


    int iterCount = 0;

    public:
        SearchAlgorithmRelativeBehavior() : Behavior(SEARCH_ALGORITHM_RELATIVE_BEHAVIOR_TYPE){
            xiterator = 0.0;
            yiterator = 0.5;
            iterator = 1.0;
        }
        bool tick();
        void determineRovers();
};



#endif
