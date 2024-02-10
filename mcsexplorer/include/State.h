#ifndef STATE_H
#define STATE_H
#include "Job.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#pragma once

class State {
   public:
    State() = default;
    ~State();

    explicit State(std::vector<Job*> const& jobs)
        : jobs(std::move(jobs)), crit(LO) {
        initialize();
    };

    State(const State& other);

    std::vector<size_t> get_actives() const;
    std::vector<size_t> get_implicitly_completeds() const;
    std::vector<size_t> get_eligibles();
    std::vector<size_t> get_tasks_of_level(int of_level) const;

    bool is_fail() const;

    void run_tansition(int to_run_index);
    void completion_transition(int ran_index, bool signals_completion);
    void request_transition(std::vector<int> const& requestings);

    size_t n() const { return jobs.size(); }
    Criticality get_crit() const { return crit; };
    void set_crit(Criticality criticality) { crit = criticality; };
    Job* get_job(int i) { return jobs[i]; };
    std::vector<Job*> get_jobs() { return jobs; };
    int get_size() const { return jobs.size(); };
    float get_relativity() const { return relativity; };

    int get_interference_laxity(int target_crit, size_t i) const;
    float get_interference_laxity_float(int target_crit, size_t i) const;

    void repr() const;
    std::string str() const;
    std::string str_tasks() const;
    std::string dot_node(std::string node_id,
                         std::string extra_node_arg = "") const;

    uint64_t get_hash() const;
    uint64_t get_hash_idle() const;
    std::vector<int> get_ordered_idle_nats() const;
    std::pair<u_int64_t, std::vector<int>> get_idle_nats_pair() const;
    std::string get_node_id() const { return std::to_string(get_hash()); };
    std::string get_node_idle_id() const;

    float get_utilisation_of_level_at_level(int of_level, int at_level) const {
        return utilisation_of_level_at_level[of_level - 1][at_level - 1];
    };

   private:
    std::vector<Job*> jobs;
    Criticality crit;
    int max_crit;

    float compute_utilisation_of_level_at_level(int of_level,
                                                int at_level) const;

    std::vector<std::vector<float>> utilisation_of_level_at_level;

    float relativity;

    void initialize();
};

#endif
