#include "Job.h"

void Job::initialize() {
    nat = 0;
    rct = 0;
}

void Job::execute(bool run) {
    if (run) {
        rct--;
    }
    if (is_active()) {
        nat--;
    } else {
        nat = std::max(nat - 1, 0);
    }
}

void Job::request(int crit) {
    rct = C[crit - 1];
    nat = T;
}

void Job::critic(int current_crit, int next_crit) {
    if (current_crit == next_crit) {
        return;
    }
    if (X < next_crit) {
        terminate();
    } else {
        if (is_active()) {
            rct = rct + C[next_crit - 1] - C[current_crit - 1];
        }
    }
}

void Job::repr() const { std::cout << str() << std::endl; }

std::string Job::str() const {
    std::stringstream ss;
    ss << "(" << nat << ", " << rct << ")";
    return ss.str();
}

std::string Job::dot_node() const {
    std::stringstream ss;
    ss << "(" << nat << ", " << rct << ")";
    return ss.str();
}
