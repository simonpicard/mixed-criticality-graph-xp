#ifndef GRAPH_H
#define GRAPH_H
#include "Job.h"
#include "State.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#pragma once

class Graph {
   public:
    Graph();
    ~Graph();

    Graph(State* initial_state_, std::function<int(State*)> schedule_,
          std::string graph_output_path_ = "", int verbose_ = 0,
          std::vector<std::function<bool(State*)>> safe_oracles_ = {},
          std::vector<std::function<bool(State*)>> unsafe_oracles_ = {})
        : initial_state(initial_state_),
          schedule(schedule_),
          graph_output_path(graph_output_path_),
          plot_graph(graph_output_path_ != ""),
          verbose(verbose_),
          safe_oracles(safe_oracles_),
          unsafe_oracles(unsafe_oracles_){};

    bool is_fail(std::vector<State*> const& states);
    void run_tansition(State* state, int to_run);
    std::vector<State*> completion_transition(State* state, int to_run);
    std::vector<State*> request_transition(std::vector<State*> const& states);

    bool has_unsafe(std::vector<State*> const& states);
    void handle_safe(std::vector<State*>& states);

    void handle_run_tansition(State* state, int to_run, bool is_last_leaf);
    std::vector<State*> handle_completion_transition(State* state, int to_run,
                                                     bool is_last_leaf);
    std::vector<State*> handle_request_transition(
        std::vector<State*> const& states, bool is_last_leaf);

    std::vector<State*> get_neighbors(std::vector<State*> const& leaf_states);

    int64_t* bfs();
    int64_t* acbfs();

    void graphiz_setup(std::string path);
    void graphiz_teardown(std::string path);
    void connect_neighbors_graphviz(std::vector<State*> from_list,
                                    State* to) const;
    void simulate_neighbor_graphviz(State* neighbor,
                                    std::vector<int> nats) const;

    static void repr(std::vector<State*> states);
    void log_start_search();
    void log_end_search(bool res, int64_t visited_count, int step_i,
                        std::chrono::milliseconds duration);
    void log_step(int step_i, int64_t visited_count, int leaf_states_size);
    void log_unsafe(State* unsafe_state);
    void log_safe(State* safe_state);
    void log_start(State* state, bool is_last_leaf);
    void log_run(State* state, bool is_last_leaf);
    void log_completion(State* state, bool is_last_leaf);
    void log_request(State* state, bool is_last_leaf, bool is_last_request);
    void log_simulated(State* state);
    std::string get_second_hiearchy_char(bool is_last_leaf);
    std::string get_third_hiearchy_char(bool is_last_request);

   protected:
    State* initial_state;
    std::string graph_output_path;
    bool plot_graph;
    int verbose;
    bool use_graphviz_idle_id;

    std::function<int(State*)> schedule;
    std::vector<std::function<bool(State*)>> safe_oracles;
    std::vector<std::function<bool(State*)>> unsafe_oracles;
};

#endif