#include "EdfInterference.h"
#include "UnsafeOracle.h"
#include "State.h"

int main(int argc, char** argv) {
    std::vector<Job*> jobs = {
        new Job(11, 11, HI, {2, 3}),
        new Job(12, 12, LO, {5, 5}),
        new Job(3, 3, HI, {1,2}),
    };

    State state = State(jobs);

    const bool is_unsafe = unsafe_edf_system_interference(&state);
    std::cout << "EDF is " << (is_unsafe ? "unsafe" : "not unsafe") << " on that system." << std::endl;

    const bool sum_min_lax_unsafe = UnsafeOracle::sum_sorted_laxities(&state);
    std::cout << "sum_min_lax oracle says " << (sum_min_lax_unsafe ? "unsafe" : "not unsafe") << " on that system." << std::endl;

    return 0;
}