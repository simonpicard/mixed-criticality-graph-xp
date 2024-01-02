#ifndef UNSAFEORACLE_H
#define UNSAFEORACLE_H
#include "State.h"
#include <iostream>
#include <vector>

#pragma once

class UnsafeOracle {
   public:
    static bool laxity(State* state);
    static bool worst_laxity(State* state);
    static bool interference(State* state);
    static bool interference_at_level(State* state, int crit);
    static bool all_interference(State* state);
};

#endif