#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "State.h"
#include "Types.h"
#include <iostream>
#include <vector>

#pragma once

class Scheduler {
   public:
    static int edfvd(State* state);
    static int lwlf(State* state);
    static int reduce_interference(State* state);
};

#endif
