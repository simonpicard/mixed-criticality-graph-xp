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

    explicit State(std::vector<Job*> const& jobs_)
        : jobs(std::move(jobs_)), crit(1) {
        initialize();
    };

    State(const State& other);

    std::vector<int> get_actives() const;
    std::vector<int> get_implicitly_completeds() const;
    std::vector<int> get_eligibles();
    std::vector<int> get_tasks_of_level(int of_level) const;

    bool is_fail() const;

    void run_tansition(int to_run_index);
    void completion_transition(int ran_index, bool signals_completion);
    void request_transition(std::vector<int> const& requestings);

    int get_crit() const { return crit; };
    void set_crit(int crit_) { crit = crit_; };
    Job* get_job(int i) { return jobs[i]; };
    std::vector<Job*> get_jobs() { return jobs; };
    int get_size() const { return jobs.size(); };
    float get_relativity() const { return relativity; };

    void repr() const;
    std::string str() const;
    std::string dot_node(std::string node_id,
                         std::string extra_node_arg = "") const;

    uint64_t get_hash() const;
    std::string get_node_id() const { return std::to_string(get_hash()); };

    float get_utilisation_of_level_at_level(int of_level, int at_level) const {
        return utilisation_of_level_at_level[of_level - 1][at_level - 1];
    };

   private:
    std::vector<Job*> jobs;
    int crit;
    int max_crit;

    float compute_utilisation_of_level_at_level(int of_level,
                                                int at_level) const;

    std::vector<std::vector<float>> utilisation_of_level_at_level;

    float relativity;

    void initialize();
};

#endif