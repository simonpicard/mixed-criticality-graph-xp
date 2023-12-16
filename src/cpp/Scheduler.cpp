#include "Scheduler.h"

int Scheduler::edfvd(State* state) {
    std::vector<int> actives = state->get_actives();

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
            if (!first_set or dl < min_dl or dl == min_dl and i < j_id) {
                min_dl = dl;
                j_id = i;
                first_set = true;
            }
        }
    }
    return j_id;
}