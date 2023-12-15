#include "Graph.h"
#include "Job.h"
#include "Scheduler.h"
#include "State.h"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    std::cout << "Hello, World!" << std::endl;

    Job* j = new Job(3, 3, 2, std::vector<int>{2, 3});
    Job* j2 = new Job(3, 3, 1, std::vector<int>{1, 1});

    State* s = new State(std::vector<Job*>{j, j2});

    Graph g(s, "./test.dot");

    g.bfs(&Scheduler::edfvd);

    return 0;
}