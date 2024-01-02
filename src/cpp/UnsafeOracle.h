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
};

#endif