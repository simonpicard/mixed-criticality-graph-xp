#include "EdfInterference.h"
#include "SafeOracle.h"
#include "State.h"
#include "UnsafeOracle.h"

int main() {
    std::vector<Job*> jobs = {
        new Job(11, 11, HI, {2, 3}),
        new Job(12, 12, LO, {5, 5}),
        new Job(3, 3, HI, {1, 2}),
    };

    State state = State(jobs);

    const bool is_unsafe = unsafe_edf_system_interference(&state);
    std::cout << "EDF is " << (is_unsafe ? "unsafe" : "not unsafe") << " on that system." << std::endl;

    const bool sum_min_lax_unsafe = UnsafeOracle::sum_sorted_laxities(&state);
    std::cout << "sum_min_lax oracle says " << (sum_min_lax_unsafe ? "unsafe" : "not unsafe") << " on that system."
              << std::endl;

    const bool sum_min_worst_lax_unsafe = UnsafeOracle::sum_sorted_worst_laxities(&state);
    std::cout << "sum_min_worst_lax_unsafe oracle says " << (sum_min_worst_lax_unsafe ? "unsafe" : "not unsafe")
              << " on that system." << std::endl;

    state.set_crit(HI);
    const bool edf_carryoverjobs = SafeOracle::edf_carryoverjobs(&state);
    std::cout << "edf_carryoverjobs oracle says " << (edf_carryoverjobs ? "unsafe" : "not unsafe") << " on that system."
              << std::endl;

    for (Job* job : jobs) {
        delete job;
    }

    return 0;
}
