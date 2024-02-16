#include "SafeOracle.h"

#include "simulators/EDFSimulator.h"
#include <stdexcept>

/*
 * Based on:
\begin{equation}
  L = \sum_{i=1}^n \left\lceil\frac{L}{T_i}\right\rceil C_i
\end{equation}
We can compute this value in the same way as in Theorem of FTP schedulers:
\begin{align*}
  w_0     & = \sum_{i=1}^{n}Ci & (initialisation) \\
  w_{k+1} & = \sum_{i=1}^{n}\left\lceil \frac{w_k}{T_i}\right\rceil C_i & (iteration)
\end{align*}
 */
static int div_ceil_int(int numerator, int denominator) {
    const int result = numerator / denominator + ((numerator % denominator) == 0 ? 0 : 1);
    return result;
}

static int compute_L_HI(std::vector<Job*> const& jobs) {
    int w0 = 0;
    for (Job* job : jobs) {
        if (HI == job->get_X()) {
            w0 += job->get_C(HI);
        }
    }

    int wk = w0;
    bool converged = false;

    while (!converged) {
        int next_wk = 0;
        for (Job* job : jobs) {
            if (HI == job->get_X()) {
                next_wk += div_ceil_int(wk, job->get_T()) * job->get_C(HI);
            }
        }

        converged = (next_wk == wk);
        wk = next_wk;
    }

    return wk;
}

bool SafeOracle::edf_carryoverjobs(State* state) {
    if (HI != state->get_crit()) {
        return false;
    }

    std::vector<size_t> hi_task_ids = state->get_tasks_of_level(HI);

    rtsimulator::EDFSimulator simulator;
    int carryover_deadline_max = 0;

    for (Job *job: state->get_jobs()) {
        simulator.addTask( // task
            job->get_C(HI),
            job->get_D(),
            job->get_T()
        );
        if (job->get_rct() > 0) { // carry over job
            const int rct = job->get_rct();
            const int ttd = job->get_ttd();
            simulator.addAperiodicJob(rct, ttd);

            const int deadline = job->get_ttd();
            if (deadline > carryover_deadline_max) {
                carryover_deadline_max = deadline;
            }
        }
    }

    const int big_l = compute_L_HI(state->get_jobs());

    const int upper_bound = carryover_deadline_max + big_l;
    bool feasible = simulator.simulate(upper_bound);

    return feasible;
}
