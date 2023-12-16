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

    std::vector<State*> get_neighbors(std::vector<State*> leaf_states,
                                      int (*schedule)(State*));

    int64_t* bfs(int (*schedule)(State*), std::string fail_condtion = "DM");

    bool is_fail(std::vector<State*> const& states, std::string fail_condtion);

    static void repr(std::vector<State*> states);

    void graphiz_setup(std::string path);
    void graphiz_teardown(std::string path);

    void connect_neighbor_graphviz(State* from, State* to) const;

   protected:
    State* initial_state;
    std::string graph_output_path;
    bool plot_graph;
    int verbose;

    std::vector<std::function<bool(State*)>> safe_oracles;
    std::vector<std::function<bool(State*)>> unsafe_oracles;
};

#endif