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
    static bool sum_sorted_laxities(State* state);
    static bool sum_sorted_worst_laxities(State* state);
    static bool over_demand(State* state);
    static bool hi_over_demand(State* state);
};

#endif
