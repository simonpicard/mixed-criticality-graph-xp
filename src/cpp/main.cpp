#include "Graph.h"
#include "Job.h"
#include "SafeOracle.h"
#include "Scheduler.h"
#include "State.h"
#include "UnsafeOracle.h"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    // Job* j = new Job(3, 3, 2, std::vector<int>{2, 3});
    // Job* j2 = new Job(3, 3, 1, std::vector<int>{1, 1});
    Job* j = new Job(5, 5, 2, std::vector<int>{1, 5});
    Job* j2 = new Job(5, 5, 1, std::vector<int>{4, 4});

    State* s = new State(std::vector<Job*>{j, j2});
    // State* s = new State(std::vector<Job*>{
    //     new Job(11, 11, 2, std::vector<int>{2, 3}),
    //     new Job(12, 12, 1, std::vector<int>{6, 6}),
    //     new Job(3, 3, 2, std::vector<int>{1, 2}),
    // });

    std::vector<std::function<bool(State*)>> safe_oracles{
        &SafeOracle::all_idle_hi};
    std::vector<std::function<bool(State*)>> unsafe_oracles{
        &UnsafeOracle::laxity};

    Graph g(s, &Scheduler::edfvd, "./test.dot", 2, safe_oracles,
            unsafe_oracles);
    // g.bfs();
    g.acbfs();

    return 0;
}