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

    if (state->get_crit() == HI) {
        for (int i : actives) {
            dl = state->get_job(i)->get_ttd();
            if (!first_set or dl < min_dl) {
                min_dl = dl;
                j_id = i;
                first_set = true;
            }
        }
    } else if (state->get_crit() == LO) {
        for (int i : actives) {
            dl = state->get_job(i)->get_ttvd(state->get_relativity());
            if (!first_set or dl < min_dl or (dl == min_dl and i < j_id)) {
                min_dl = dl;
                j_id = i;
                first_set = true;
            }
        }
    } else {
        std::cout << "error: wrong crit" << std::endl;
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
