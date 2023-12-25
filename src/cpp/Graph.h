#ifndef GRAPH_H
#define GRAPH_H
#include "Job.h"
#include "State.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_set>

#pragma once

class Graph {
   public:
    Graph();
    ~Graph();

    Graph(State* initial_state_, std::string graph_output_path_ = "",
          int verbose_ = 0,
          std::vector<std::function<bool(State*)>> safe_oracles_ = {},
          std::vector<std::function<bool(State*)>> unsafe_oracles_ = {})
        : initial_state(initial_state_),
          graph_output_path(graph_output_path_),
          plot_graph(graph_output_path_ != ""),
          verbose(verbose_),
          safe_oracles(safe_oracles_),
          unsafe_oracles(unsafe_oracles_){};

    int64_t* bfs(int (*schedule)(State*), std::string fail_condtion = "DM");

    std::vector<State*> get_neighbors(std::vector<State*> leaf_states,
                                      int (*schedule)(State*));

    bool is_fail(std::vector<State*> const& states, std::string fail_condtion);

    static void repr(std::vector<State*> states);

    void graphiz_setup(std::string path);
    void graphiz_teardown(std::string path);
    void connect_neighbor_graphviz(State* from, State* to) const;

    void log_start();
    void log_end(bool res, int64_t visited_count, int step_i,
                 std::chrono::milliseconds duration);
    void log_step(int step_i, int64_t visited_count, int leaf_states_size);
    void log_unsafe(State* unsafe_state);
    void log_safe(State* safe_state);
    void log_start(State* state, std::string second_hiearchy_char);
    void log_run(State* state, std::string second_hiearchy_char);
    void log_completion(State* state, std::string second_hiearchy_char);
    void log_request(State* state, std::string second_hiearchy_char,
                     std::string third_hiearchy_char);

   protected:
    State* initial_state;
    std::string graph_output_path;
    bool plot_graph;
    int verbose;

    std::vector<std::function<bool(State*)>> safe_oracles;
    std::vector<std::function<bool(State*)>> unsafe_oracles;
};

#endif