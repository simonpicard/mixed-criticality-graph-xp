#include "UnsafeOracle.h"

bool UnsafeOracle::laxity(State* state) {
    for (Job* job : state->get_jobs()) {
        if (job->is_active())
            if (job->get_laxity() < 0) return true;
    }
    return false;
}

bool UnsafeOracle::worst_laxity(State* state) {
    for (Job* job : state->get_jobs()) {
        if (job->is_active())
            if (job->get_worst_laxity(state->get_crit()) < 0) return true;
    }
    return false;
}

bool UnsafeOracle::interference_at_level(State* state, int crit) {
    for (int i = 0; i < state->get_jobs().size(); i++) {
        if (state->get_interference_laxity(crit, i) < 0) {
            return true;
        }
    }

    return false;
}

bool UnsafeOracle::interference(State* state) {
    return interference_at_level(state, state->get_crit());
}

bool UnsafeOracle::all_interference(State* state) {
    if (interference_at_level(state, 1)) return true;
    if (interference_at_level(state, 2)) return true;
    return false;
}

bool UnsafeOracle::worst_interference(State* state) {
    return interference_at_level(state, 2);
}

bool UnsafeOracle::sum_sorted_laxities(State* state) {
    auto jobs = state->get_jobs();
    size_t n = jobs.size();
    std::vector<int> laxities;
    laxities.reserve(n);

    for (Job *job: jobs) {
        laxities.push_back(job->get_laxity());
    }

    std::sort(laxities.begin(), laxities.end());

    int l_k = 0;
    for (int k = 1; k <= laxities.size(); ++k) {
        l_k += laxities[k - 1];

        if (l_k <= k - 2) {
            return true; // Necessary condition violated, so the scheduling is unsafe
        }
    }

    return false; // Necessary condition satisfied, we don't know if it is safe
}
