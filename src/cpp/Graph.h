#ifndef GRAPH_H
#define GRAPH_H
#include "Job.h"
#include "State.h"
#include <algorithm>
#include <chrono>
#include <functional>
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

    void initialize_search(bool use_idle_antichain_current);
    int64_t* finalize_search();

    int64_t* bfs();
    int64_t* acbfs();

    void set_safe_oracle(std::function<bool(State*)> safe_oracle) {
        safe_oracles = {safe_oracle};
    }
    void set_unsafe_oracle(std::function<bool(State*)> unsafe_oracle) {
        unsafe_oracles = {unsafe_oracle};
    }
    void clear_safe_oracle() { safe_oracles.clear(); }
    void clear_unsafe_oracle() { unsafe_oracles.clear(); }

    void graphiz_setup();
    void graphiz_teardown();
    void connect_neighbor_graphviz(State* from_, State* to) const;
    void connect_neighbors_graphviz(State* from,
                                    std::vector<State*> to_list) const;
    void simulate_neighbor_graphviz(State* neighbor,
                                    std::vector<int> nats) const;

    static void repr(std::vector<State*> states);
    void log_start_search();
    void log_end_search();
    void log_step(int leaf_states_size);
    void log_unsafe(State* unsafe_state);
    void log_safe(State* safe_state);
    void log_start(State* state, bool is_last_leaf);
    void log_run(State* state, bool is_last_leaf);
    void log_completion(State* state, bool is_last_leaf);
    void log_request(State* state, bool is_last_leaf, bool is_last_request);
    void log_simulated(State* state);
    void log_visited(State* state);
    std::string get_second_hiearchy_char(bool is_last_leaf);
    std::string get_third_hiearchy_char(bool is_last_request);

   protected:
    State* initial_state;
    std::string graph_output_path;
    bool plot_graph;
    int verbose;

    std::function<int(State*)> schedule;
    std::vector<std::function<bool(State*)>> safe_oracles;
    std::vector<std::function<bool(State*)>> unsafe_oracles;

    bool automaton_is_safe;
    bool use_idle_antichain;
    int automaton_depth;
    u_int64_t visited_count;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::milliseconds duration;
};

#endif
