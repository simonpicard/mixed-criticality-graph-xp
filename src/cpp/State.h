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
        : jobs(std::move(jobs_)), crit(1){};

    State(const State& other) : crit(other.crit){};

    std::vector<int> get_actives() const;
    std::vector<int> get_implicitly_completeds() const;
    std::vector<int> get_eligibles();

    bool is_fail() const;

    void run_tansition(std::vector<int> to_run);
    void completion_transition();
    void request_transition(std::vector<int> const& requestings);

    int get_crit() const { return crit; };
    void set_crit(int crit_) { crit = crit_; };
    Job* get_job(int i) { return jobs[i]; };
    int get_size() const { return jobs.size(); };

    void repr() const;
    std::string str() const;
    std::string dot_node(std::string node_id) const;

   private:
    std::vector<Job*> jobs;
    int crit;
    int max_crit;
};

#endif