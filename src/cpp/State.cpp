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

bool State::is_fail() const {
    for (Job* job : jobs) {
        if (job->is_deadline_miss()) return true;
    }
    return false;
}

void State::run_tansition(std::vector<int> to_run) {
    for (int i = 0; i < jobs.size(); ++i) {
        if (std::find(to_run.begin(), to_run.end(), i) != to_run.end()) {
            jobs[i]->execute(true);
        } else {
            jobs[i]->execute(false);
        }
    }
}

void State::completion_transition() {
    // TODO
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

std::string State::dot_node(std::string node_id) const {
    std::stringstream ss;

    ss << node_id << " [label=<";
    for (Job* job : jobs) {
        ss << job->dot_node();
        ss << " ";
    }
    ss << crit;
    ss << ">,";
    if (this->crit == 1)
        ss << "fillcolor=lightcyan";
    else if (this->crit == 2)
        ss << "fillcolor=lightyellow";

    if (this->is_fail()) ss << ",color=orchid,penwidth=5";

    ss << "]" << std::endl;

    return ss.str();
}