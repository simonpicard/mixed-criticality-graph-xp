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

bool UnsafeOracle::interference(State* state) {
    for (int i = 0; i < state->get_jobs().size(); i++) {
        Job* job = state->get_jobs()[i];
        if (job->is_active()) {
            int ttd = job->get_ttd();
            int total_rct = job->get_rct();
            for (int j = 0; j < state->get_jobs().size(); j++) {
                if (i == j) continue;
                Job* other_job = state->get_jobs()[j];
                if (other_job->is_discarded(state->get_crit())) continue;
                int other_ttd = other_job->get_ttd();
                if (other_ttd <= ttd) {
                    total_rct += other_job->get_rct();
                    other_ttd += other_job->get_T();
                }
                while (other_ttd <= ttd) {
                    total_rct += other_job->get_C()[state->get_crit() - 1];
                    other_ttd += other_job->get_T();
                }
                if (total_rct > ttd) return true;
            }
        }
    }

    return false;
}

bool UnsafeOracle::interference_at_level(State* state, int crit) {
    if (state->get_crit() > crit) return false;

    for (int i = 0; i < state->get_jobs().size(); i++) {
        Job* job = state->get_jobs()[i];
        if (job->is_active() && job->get_X() >= crit) {
            int ttd = job->get_ttd();
            int total_rct =
                job->get_rct() +
                (job->get_C()[crit - 1] - job->get_C()[state->get_crit() - 1]);
            ;
            for (int j = 0; j < state->get_jobs().size(); j++) {
                if (i == j) continue;
                Job* other_job = state->get_jobs()[j];
                if (other_job->is_discarded(crit)) continue;
                int other_ttd = other_job->get_ttd();
                if (other_ttd <= ttd) {
                    if (other_job->is_active())
                        total_rct +=
                            other_job->get_rct() +
                            (other_job->get_C()[crit - 1] -
                             other_job->get_C()[state->get_crit() - 1]);
                    other_ttd += other_job->get_T();
                }
                while (other_ttd <= ttd) {
                    total_rct += other_job->get_C()[crit - 1];
                    other_ttd += other_job->get_T();
                }
                if (total_rct > ttd) return true;
            }
        }
    }

    return false;
}

bool UnsafeOracle::all_interference(State* state) {
    if (interference_at_level(state, 1)) return true;
    if (interference_at_level(state, 2)) return true;
    return false;
}