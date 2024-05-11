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

bool UnsafeOracle::sum_sorted_laxities(State* state) {
    auto jobs = state->get_jobs();
    size_t n = jobs.size();
    std::vector<int> laxities;
    laxities.reserve(n);

    for (Job* job : jobs) {
        if (job->is_active()) {
            laxities.push_back(job->get_laxity());
        }
    }

    std::sort(laxities.begin(), laxities.end());

    int l_k = 0;
    const int size = laxities.size();
    for (int k = 1; k <= size; ++k) {
        l_k += laxities[k - 1];

        if (l_k <= k - 2) {
            return true;  // Necessary condition violated, so the scheduling is unsafe
        }
    }

    return false;  // Necessary condition satisfied, we don't know if it is safe
}

bool UnsafeOracle::sum_sorted_worst_laxities(State* state) {
    // TODO same structure as sum_sorted_laxities; maybe factor out later.
    // TODO check if we can get rid of it, we do it cross laxities

    auto cri = state->get_crit();

    auto jobs = state->get_jobs();
    size_t n = jobs.size();
    std::vector<int> worst_laxities;
    worst_laxities.reserve(n);

    for (Job* job : jobs) {
        if (job->is_active() and job->get_X() >= cri) {
            worst_laxities.push_back(job->get_worst_laxity(cri));
        }
    }

    std::sort(worst_laxities.begin(), worst_laxities.end());

    int l_k = 0;
    const int size = worst_laxities.size();
    for (int k = 1; k <= size; ++k) {
        l_k += worst_laxities[k - 1];

        if (l_k <= k - 2) {
            return true;  // Necessary condition violated, so the scheduling is unsafe
        }
    }

    return false;  // Necessary condition satisfied, we don't know if it is safe
}

bool UnsafeOracle::over_demand(State* state) {
    for (int i : state->get_actives()) {
        Job* job = state->get_jobs()[i];
        int ttd = job->get_ttd();
        int demand_bound = state->get_demand_bound(ttd, state->get_crit());
        if (demand_bound > ttd) return true;
    }
    return false;
}

bool UnsafeOracle::hi_over_demand(State* state) {
    for (int i : state->get_actives()) {
        Job* job = state->get_jobs()[i];
        int ttd = job->get_ttd();
        int demand_bound = state->get_demand_bound(ttd, HI);
        if (demand_bound > ttd) return true;
    }
    return false;
}
