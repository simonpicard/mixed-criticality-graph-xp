#ifndef EDFINTERFERENCE_HPP
#define EDFINTERFERENCE_HPP

#include "State.h"
#include <stdexcept>
#include <format>
#include <functional>


typedef std::function<size_t(int, State*)> interference_function;


size_t edf_interference(size_t i, State* state) {
    const auto n = state->n();
    if (i >= n) {
        auto message = std::format(
            "Task index i must be 0 <= i < n = {}. i = {}.",
            n,
            i
        );
        throw std::invalid_argument(message);
    }

    Job* tau_i = state->get_job(i);
    auto ttd_i = tau_i->get_ttd();

    auto prio_higher_than_i = [tau_i, ttd_i, i, state](size_t j) -> bool {
        Job* tau_j = state->get_job(j);
        auto ttd_j = tau_j->get_ttd();
        return ttd_j < ttd_i || (ttd_j == ttd_i && j < i);
    };

    auto nb_future_jobs_interfering_before_i_deadline = [i, ttd_i, state](size_t j) -> int {
        Job* tau_j = state->get_job(j);
        auto nat_j = tau_j->get_nat();
        auto D_j = tau_j->get_D();
        auto T_j = tau_j->get_T();

        if (nat_j + D_j > ttd_i) {
            return 0;
        }
        if (nat_j + D_j == ttd_i) {
            return (j < i) ? 1 : 0;
        }

        int q = (ttd_i - nat_j - D_j) / T_j;
        int n = q + 1;

        auto last_j_deadline_before_i_deadline = nat_j + (n * T_j) + D_j;
        int result;
        if (last_j_deadline_before_i_deadline < ttd_i) {
            result = n;
        } else if (last_j_deadline_before_i_deadline == ttd_i) {
            result = (j < i) ? n : n - 1;
        } else {
            throw std::invalid_argument("Impossible case.");
        }

        return result;
    };

    auto interference_of_one_task = [state, &nb_future_jobs_interfering_before_i_deadline](int j) -> int {
        Job* tau_j = state->get_job(j);
        auto rct_j = tau_j->get_rct();
        auto crit = state->get_crit();
        auto C_j = tau_j->get_C(crit);

        int n = nb_future_jobs_interfering_before_i_deadline(j);
        int result = rct_j + (n * C_j);
        return result;
    };

    size_t I_i = 0;
    for (size_t k = 0; k < n; ++k) {
        if (k != i && prio_higher_than_i(k)) {
            I_i += interference_of_one_task(k);
        }
    }
    return I_i;
}


bool _unsafe_edf_task_interference(
    int i,
    State* state,
    interference_function interference
) {
    if (interference(i, state) == 0) return false;//TODO
    return true;//TODO
}


bool unsafe_edf_system_interference(State* state) {
    const size_t n = state->get_jobs().size();
    for (size_t i = 0; i < n; ++i) {
        if (_unsafe_edf_task_interference(i, state, edf_interference)) {
            return true;
        }
    }
    return false;
}

#endif /* EDFINTERFERENCE_HPP */
