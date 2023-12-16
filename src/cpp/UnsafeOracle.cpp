#include "UnsafeOracle.h"

bool UnsafeOracle::laxity(State* state) {
    for (Job* job : state->get_jobs()) {
        if (job->get_laxity() < 0) return true;
    }
    return false;
}