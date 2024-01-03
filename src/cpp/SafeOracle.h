#ifndef SAFEORACLE_H
#define SAFEORACLE_H
#include "State.h"
#include "UnsafeOracle.h"
#include <iostream>
#include <vector>

#pragma once

class SafeOracle {
   public:
    static bool all_idle_hi(State* state);
    static bool hi_inteference(State* state);
};

#endif