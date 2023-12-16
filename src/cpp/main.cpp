#include "Graph.h"
#include "Job.h"
#include "Scheduler.h"
#include "State.h"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    Job* j = new Job(3, 3, 2, std::vector<int>{2, 3});
    Job* j2 = new Job(3, 3, 1, std::vector<int>{1, 1});

    State* s = new State(std::vector<Job*>{j, j2});

    std::cout << s->get_relativity() << std::endl;

    Graph g(s, "./test.dot", 2);

    g.bfs(&Scheduler::edfvd);

    return 0;
}