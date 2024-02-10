#include "SafeOracle.h"

#include "simulators/EDFSimulator.h"
#include <stdexcept>

bool SafeOracle::edf_carryoverjobs(State* state) {
    if (LO == state->get_crit()) {
        return false;
    }

    std::vector<size_t> hi_task_ids = state->get_tasks_of_level(HI);//TODO check level == criticaltiy
    const size_t hyperperiod = 10; // TODO
    throw std::runtime_error("not fully implemented");

    size_t study_bound = hyperperiod;

    rtsimulator::EDFSimulator simulator;

    for (Job *job: state->get_jobs()) {
        simulator.addTask( // task
            job->get_C(HI),
            job->get_D(),
            job->get_T()
        );
        if (job->get_rct() > 0) { // carry over job
            int rct = job->get_rct();
            int ttd = job->get_ttd();
            simulator.addAperiodicJob(rct, ttd);
            study_bound += rct;
        }
    }

    bool result = simulator.simulate(study_bound);

    return result;
}
