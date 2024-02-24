#include "Scheduler.h"

int Scheduler::edfvd(State* state) {
    std::vector<size_t> actives = state->get_actives();

    if (actives.empty()) {
        return -1;
    }

    if (actives.size() == 1) {
        return actives[0];
    }

    bool first_set = false;
    float min_dl;
    float dl;
    int j_id;

    if (state->get_crit() == 2) {
        for (int i : actives) {
            dl = state->get_job(i)->get_ttd();
            if (!first_set or dl < min_dl) {
                min_dl = dl;
                j_id = i;
                first_set = true;
            }
        }
    } else if (state->get_crit() == 1) {
        for (int i : actives) {
            dl = state->get_job(i)->get_ttvd(state->get_relativity());
            if (!first_set or dl < min_dl or (dl == min_dl and i < j_id)) {
                min_dl = dl;
                j_id = i;
                first_set = true;
            }
        }
    }
    return j_id;
}
int Scheduler::lwlf(State* state) {
    std::vector<size_t> actives = state->get_actives();

    if (actives.empty()) {
        return -1;
    }

    if (actives.size() == 1) {
        return actives[0];
    }

    bool first_set = false;
    float min_w_lax;
    float w_lax;
    int j_id;

    for (int i : actives) {
        w_lax = state->get_job(i)->get_worst_laxity(state->get_crit());
        if (!first_set or w_lax < min_w_lax or (w_lax == min_w_lax and i < j_id)) {
            min_w_lax = w_lax;
            j_id = i;
            first_set = true;
        }
    }

    return j_id;
}

int Scheduler::reduce_interference(State* state) {
    // experimental
    std::vector<size_t> actives = state->get_actives();

    if (actives.empty()) {
        return -1;
    }

    if (actives.size() == 1) {
        return actives[0];
    }

    bool first_set = false;
    float min_wilf;
    float wilf;
    int min_ttd;
    int ttd;
    std::vector<int> candidates;
    // first find the task with the worst interference laxity
    // then, gather all tasks with a smaller or equal ttd
    // executing any of those will reduce its worst interference
    for (int current_crit = state->get_crit(); current_crit <= 2; current_crit++) {
        for (int i : actives) {
            if (!state->get_job(i)->is_discarded(current_crit)) {
                wilf = state->get_interference_laxity_float(current_crit, i);
                ttd = state->get_job(i)->get_ttd();

                if (first_set)
                    if (wilf >= min_wilf)
                        if (ttd >= min_ttd) continue;

                min_wilf = wilf;
                min_ttd = ttd;
                first_set = true;
                candidates.clear();
                int current_ttd = state->get_job(i)->get_ttd();
                for (int j : actives) {
                    if (!state->get_job(j)->is_discarded(current_crit))
                        if (state->get_job(j)->get_ttd() <= current_ttd) candidates.push_back(j);
                }
            }
        }
    }

    first_set = false;
    int max_crit;
    int j_crit;
    int min_wl;
    int wl;
    int j_id;
    // then we want to execute the job which will reduce the most the load
    // that is the one with the lowest ttd, the highest criticality and the
    // lowest worst laxity
    for (int i : candidates) {
        j_crit = state->get_job(i)->get_X();
        ttd = state->get_job(i)->get_ttd();
        wl = state->get_job(i)->get_worst_laxity(state->get_crit());
        if (first_set)
            if (ttd >= min_ttd)
                if (j_crit <= max_crit)
                    if (wl >= min_wl)
                        if (i >= j_id) continue;
        j_id = i;
        first_set = true;
        max_crit = j_crit;
        min_ttd = ttd;
        min_wl = wl;
    }

    return j_id;
}
