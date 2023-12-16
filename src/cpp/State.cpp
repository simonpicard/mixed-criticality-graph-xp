#include "State.h"
#include <iostream>
#include <numeric>
#include <sstream>

State::~State() {
    for (Job* p : jobs) {
        delete p;
    }
    jobs.clear();
}

State::State(const State& other)
    : crit(other.crit), relativity(other.relativity) {
    std::vector<Job*> jobs_;
    for (int i = 0; i < other.jobs.size(); ++i) {
        Job* clone = new Job(static_cast<Job*>(other.jobs[i]));
        jobs_.push_back(clone);
    }
    jobs = jobs_;

    utilisation_of_level_at_level = std::vector<std::vector<float>>{
        std::vector<float>{other.get_utilisation_of_level_at_level(1, 1),
                           other.get_utilisation_of_level_at_level(1, 2)},
        std::vector<float>{other.get_utilisation_of_level_at_level(2, 1),
                           other.get_utilisation_of_level_at_level(2, 2)}};
}

std::vector<int> State::get_actives() const {
    std::vector<int> vect;
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->is_active()) {
            vect.push_back(i);
        }
    }
    return vect;
}

std::vector<int> State::get_implicitly_completeds() const {
    std::vector<int> vect;
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->is_implicitly_completed(crit)) {
            vect.push_back(i);
        }
    }
    return vect;
}

std::vector<int> State::get_eligibles() {
    std::vector<int> vect;
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->is_eligible(crit)) {
            vect.push_back(i);
        }
    }
    return vect;
}

std::vector<int> State::get_tasks_of_level(int of_level) const {
    std::vector<int> vect;
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->get_X() == of_level) {
            vect.push_back(i);
        }
    }
    return vect;
}

bool State::is_fail() const {
    for (Job* job : jobs) {
        if (job->is_deadline_miss()) return true;
    }
    return false;
}

void State::run_tansition(int to_run_index = -1) {
    for (int i = 0; i < jobs.size(); ++i) {
        jobs[i]->execute(i == to_run_index);
    }
}

void State::completion_transition(int ran_index = -1,
                                  bool signals_completion = false) {
    if (ran_index == -1) return;

    if (jobs[ran_index]->is_implicitly_completed(crit) or signals_completion) {
        jobs[ran_index]->terminate();
    } else if (jobs[ran_index]->get_rct() == 0) {
        for (int i = 0; i < jobs.size(); ++i) {
            jobs[i]->critic(crit, crit + 1, i == ran_index);
        }
        crit = 2;
    }
}

void State::request_transition(std::vector<int> const& requestings) {
    for (int i : requestings) {
        jobs[i]->request(crit);
    }
}

void State::repr() const { std::cout << str() << std::endl; }

std::string State::str() const {
    std::stringstream ss;
    ss << crit;
    for (Job* job : jobs) {
        ss << " ";
        ss << job->str();
    }
    return ss.str();
}

std::string State::dot_node(std::string node_id,
                            std::string extra_node_arg) const {
    std::stringstream ss;

    ss << node_id << " [label=<";

    for (int i = 0; i < jobs.size(); ++i) {
        ss << jobs[i]->dot_node();
        if (i < jobs.size() - 1) ss << " ";
    }
    ss << ">,";
    if (this->crit == 1)
        ss << "fillcolor=lightcyan";
    else if (this->crit == 2)
        ss << "fillcolor=lightyellow";

    if (this->is_fail()) ss << ",color=orchid,penwidth=5";

    ss << extra_node_arg;

    ss << "]" << std::endl;

    return ss.str();
}

uint64_t State::get_hash() const {
    uint64_t hash = crit - 1;
    uint64_t factor = 2;

    for (Job* job : jobs) {
        hash = hash + job->get_hash() * factor;
        factor = factor * job->get_hash_factor();
    }

    return hash;
}

float State::compute_utilisation_of_level_at_level(int of_level,
                                                   int at_level) const {
    float utilisation = 0.0;
    if (at_level > of_level) {
        return utilisation;
    }
    for (Job* job : jobs) {
        if (job->get_X() == of_level) {
            utilisation += job->get_utilisation_at_level(at_level);
        }
    }
    return utilisation;
}

void State::initialize() {
    utilisation_of_level_at_level = std::vector<std::vector<float>>{
        std::vector<float>{compute_utilisation_of_level_at_level(1, 1),
                           compute_utilisation_of_level_at_level(1, 2)},
        std::vector<float>{compute_utilisation_of_level_at_level(2, 1),
                           compute_utilisation_of_level_at_level(2, 2)}};

    relativity = utilisation_of_level_at_level[1][0] /
                 (1.0 - utilisation_of_level_at_level[0][0]);
}