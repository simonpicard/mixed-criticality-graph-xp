#include "SafeOracle.h"

bool SafeOracle::all_idle_hi(State* state) {
    if (state->get_crit() == 1) return false;
    for (Job* job : state->get_jobs()) {
        if (job->get_rct() > 0) return false;
    }
    return true;
}