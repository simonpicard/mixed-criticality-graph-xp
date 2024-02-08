#include "simulators/EDFSimulator.h"
#include <iostream>

bool test01() {
    rtsimulator::EDFSimulator simulator;

    // Adding tasks
    simulator.addTask(1, 4, 5);
    simulator.addTask(2, 6, 10);

    // Adding aperiodic jobs
    simulator.addAperiodicJob(2, 7);
    simulator.addAperiodicJob(3, 8);

    bool simulation_result = simulator.simulate(20);

    if (simulation_result) {
        std::cout << "All jobs completed without deadline misses." << std::endl;
    } else {
        std::cout << "Some jobs missed their deadlines." << std::endl;
    }

    return simulation_result;
}

bool test02() {
    rtsimulator::EDFSimulator simulator;

    // Adding tasks
    simulator.addTask(2, 2, 2);

    // Adding aperiodic jobs
    simulator.addAperiodicJob(1, 1);

    bool simulation_result = simulator.simulate(20);

    if (simulation_result) {
        std::cout << "All jobs completed without deadline misses." << std::endl;
    } else {
        std::cout << "Some jobs missed their deadlines." << std::endl;
    }

    return simulation_result;
}

int main() {
    test01();
    test02();

    return 0;
}
