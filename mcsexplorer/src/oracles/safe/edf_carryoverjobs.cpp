#include "SafeOracle.h"

#include "simulators/EDFSimulator.h"
#include <stdexcept>

bool SafeOracle::edf_carryoverjobs(State* state) {
    if (HI != state->get_crit()) {
        return false;
    }

    std::vector<size_t> hi_task_ids = state->get_tasks_of_level(HI); // TODO check level == criticality

    throw std::runtime_error("not fully implemented");

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

            const int deadline = job->get_D(); // TODO define this with answer of Joel
            if (deadline > carryover_deadline_max) {
                carryover_deadline_max = deadline;
            }
        }
    }

    const int big_l = 0; // TODO define with answer of Joel

    const int upper_bound = carryover_deadline_max + big_l;
    bool feasible = simulator.simulate(upper_bound);

    return feasible;
}
