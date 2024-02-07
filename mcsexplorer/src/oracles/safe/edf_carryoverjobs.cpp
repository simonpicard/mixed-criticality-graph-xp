#include "SafeOracle.h"

#include "simulators/EDFSimulator.h"

bool SafeOracle::edf_carryoverjobs(State* state) {
    if (LO == state->get_crit()) {
        return false;
    }

    std::vector<Task*> hi_tasks = state->get_tasks_of_level(HI);//TODO check level == criticaltiy
    const size_t hyperperiod = 10; // TODO

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
            auto ttd = job->get_ttd();
            simulator.addAperiodicJob(rct, ttd);
            study_bound += rct;
        }
    }

    bool result = simulator.simulate(study_bound);

    return result;
}
