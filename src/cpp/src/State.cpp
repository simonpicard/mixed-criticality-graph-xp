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
        crit = HI;
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

std::string State::str_tasks() const {
    std::stringstream ss;
    int i = 0;
    for (Job* job : jobs) {
        ss << "J" << i << " -> ";
        ss << job->str_task() << std::endl;
        ++i;
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

uint64_t State::get_hash() const {
    uint64_t hash = crit - 1;
    uint64_t factor = 2;

    for (Job* job : jobs) {
        hash = hash + job->get_hash() * factor;
        factor = factor * job->get_hash_factor();
    }

    return hash;
}

uint64_t State::get_hash_idle() const {
    uint64_t hash = crit - 1;
    uint64_t factor = 2;

    for (Job* job : jobs) {
        hash = hash + job->get_hash_idle() * factor;
        factor = factor * job->get_hash_factor();
    }

    return hash;
}

std::vector<int> State::get_ordered_idle_nats() const {
    std::vector<int> nats;

    for (Job* job : jobs) {
        if (not job->is_active()) nats.push_back(job->get_nat());
    }

    return nats;
}

std::pair<u_int64_t, std::vector<int>> State::get_idle_nats_pair() const {
    std::vector<int> nats;
    uint64_t hash = 0;
    uint64_t factor = 1;

    for (Job* job : jobs) {
        if (not job->is_active()) {
            nats.push_back(job->get_nat());
            factor *= job->get_T();
            hash += job->get_nat() * factor;
        }
    }

    return std::pair<u_int64_t, std::vector<int>>(hash, nats);
}

std::string State::get_node_idle_id() const {
    std::stringstream ss;
    ss << "n_" << this->get_hash_idle();
    for (int nat : this->get_ordered_idle_nats()) {
        ss << "_" << nat;
    }
    return ss.str();
}

int State::get_interference_laxity(int target_crit, int i) const {
    if (target_crit < crit) return 999999999;

    Job* job = jobs[i];
    if (!job->is_active() || job->is_discarded(target_crit)) return 999999999;

    int ttd = job->get_ttd();
    int total_rct = job->get_rct() +
                    (job->get_C()[target_crit - 1] - job->get_C()[crit - 1]);
    for (int j = 0; j < jobs.size(); j++) {
        if (i == j) continue;
        Job* other_job = jobs[j];
        if (other_job->is_discarded(target_crit)) continue;
        // this need to be reviewed
        // this need to be tested for D < T
        int other_ttd = other_job->get_ttd();
        if (other_ttd <= ttd) {
            if (other_job->is_active()) {
                total_rct += other_job->get_rct() +
                             (other_job->get_C()[target_crit - 1] -
                              other_job->get_C()[crit - 1]);
                other_ttd += other_job->get_T();
            } else {
                int other_nat = other_job->get_nat();
                if (other_nat == 0) {
                    other_nat = 1;  // other job will emit at next tick earliest
                }
                other_ttd = other_nat + other_job->get_D();
            }

            if (other_ttd <= ttd) {
                int n = 1;
                n +=
                    (ttd - other_ttd) / other_job->get_T();  // integer division
                total_rct += other_job->get_C()[target_crit - 1] * n;
            }

            // same as above if using a while loop
            // while (other_ttd <= ttd) {
            //     total_rct += other_job->get_C()[target_crit - 1];
            //     other_ttd += other_job->get_T();
            // }
        }
    }
    return ttd - total_rct;
}

float State::get_interference_laxity_float(int target_crit, int i) const {
    // experimental
    if (target_crit < crit) return 999999999;

    Job* job = jobs[i];
    if (!job->is_active() || job->is_discarded(target_crit)) return 999999999;

    float ttd = job->get_ttd();
    float total_rct = job->get_rct() +
                      (job->get_C()[target_crit - 1] - job->get_C()[crit - 1]);
    for (int j = 0; j < jobs.size(); j++) {
        if (i == j) continue;
        Job* other_job = jobs[j];
        if (other_job->is_discarded(target_crit)) continue;
        // this need to be reviewed
        // this need to be tested for D < T
        float other_ttd = other_job->get_ttd();
        if (other_ttd <= ttd) {
            if (other_job->is_active()) {
                total_rct += other_job->get_rct() +
                             (other_job->get_C()[target_crit - 1] -
                              other_job->get_C()[crit - 1]);
                other_ttd += other_job->get_T();
            } else {
                int other_nat = other_job->get_nat();
                if (other_nat == 0) {
                    other_nat = 1;  // other job will emit at next tick earliest
                }
                other_ttd = other_nat + other_job->get_D();
            }

            if (other_ttd <= ttd) {
                float n = 1;
                n += (ttd - other_ttd) / other_job->get_T();  // float division
                total_rct += other_job->get_C()[target_crit - 1] * n;
            }
        }
    }
    return ttd - total_rct;
}
