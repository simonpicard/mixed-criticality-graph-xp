#include "Graph.h"
#include "util.cpp"

Graph::Graph() = default;

Graph::~Graph() {}

void Graph::repr(std::vector<State*> states) {
    for (int i = 0; i < states.size(); ++i) {
        std::cout << "S" << i << "-> " << states[i]->str() << std::endl;
    }
    std::cout << std::endl;
}

bool Graph::is_fail(std::vector<State*> const& states,
                    std::string fail_condition) {
    if (fail_condition == "DM") {
        for (State* state : states) {
            if (state->is_fail()) {
                if (verbose >= 2) {
                    std::cout << "│└ FAIL  " << state->str() << std::endl;
                }
                return true;
            }
        }
        return false;
    }
    std::cout << "Unknown fail condition" << std::endl;
    return true;
}

int64_t* Graph::bfs(int (*schedule)(State*), std::string fail_condtion) {
    int64_t visited_count = 0;
    static int64_t arr[4];

    int step_i = 0;
    std::vector<State*> leaf_states{initial_state};
    std::vector<State*> neighbors;
    std::unordered_set<uint64_t> visited_hash;

    bool res = true;

    graphiz_setup(graph_output_path);

    visited_hash.insert(leaf_states[0]->get_hash());

    auto start = std::chrono::high_resolution_clock::now();

    log_start();

    while (!leaf_states.empty()) {
        log_step(step_i, visited_count, leaf_states.size());

        if (is_fail(leaf_states, fail_condtion)) {
            res = false;
            break;
        }

        bool unsafe_found = false;
        for (std::function<bool(State*)> unsafe_oracle : unsafe_oracles) {
            for (int i = 0; i < leaf_states.size(); ++i) {
                if (unsafe_oracle(leaf_states[i])) {
                    log_unsafe(leaf_states[i]);
                    res = false;
                    unsafe_found = true;
                    break;
                }
            }
            if (unsafe_found) break;
        }
        if (unsafe_found) break;

        for (std::function<bool(State*)> safe_oracle : safe_oracles) {
            if (verbose >= 2) {
                for (int i = leaf_states.size() - 1; i >= 0; --i) {
                    if (safe_oracle(leaf_states[i])) {
                        log_safe(leaf_states[i]);
                    }
                }
            }

            leaf_states.erase(std::remove_if(leaf_states.begin(),
                                             leaf_states.end(), safe_oracle),
                              leaf_states.end());
        }

        visited_count = visited_count + leaf_states.size();

        neighbors = get_neighbors(leaf_states, schedule);
        step_i++;
        leaf_states.clear();

        for (State* neighbor : neighbors) {
            uint64_t state_hash = neighbor->get_hash();
            if (visited_hash.find(state_hash) == visited_hash.end()) {
                visited_hash.insert(state_hash);
                leaf_states.push_back(neighbor);
            } else {
                delete neighbor;
            }
        }
    }

    visited_count = visited_count + leaf_states.size();

    graphiz_teardown(graph_output_path);

    if (!res)
        for (State* elem : leaf_states) delete elem;

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    log_end(res, visited_count, step_i, duration);

    arr[0] = int64_t(res);
    arr[1] = visited_count;
    arr[2] = duration.count();
    arr[3] = step_i;

    return arr;
}

std::vector<State*> Graph::get_neighbors(std::vector<State*> leaf_states,
                                         int (*schedule)(State*)) {
    std::vector<State*> new_states;

    for (int leaf_i = 0; leaf_i < leaf_states.size(); ++leaf_i) {
        State* current_state = leaf_states[leaf_i];
        State* current_state_bkp = new State(*current_state);

        std::vector<State*> states_for_request;

        // verbose char
        bool is_last_leaf = leaf_i == leaf_states.size() - 1;
        std::string leaf_hiearchy_start_char =
            is_last_leaf ? std::string("└") : std::string("├");
        std::string leaf_hiearchy_other_char =
            is_last_leaf ? std::string(" ") : std::string("│");

        log_start(current_state, leaf_hiearchy_start_char);

        // execution transition
        int to_run = schedule(current_state);
        current_state->run_tansition(to_run);
        log_run(current_state, leaf_hiearchy_other_char);

        if (to_run == -1)
            states_for_request.push_back(current_state);
        else {
            // completion transitions
            State* current_state_signals = new State(*current_state);

            current_state->completion_transition(to_run, false);
            log_completion(current_state, leaf_hiearchy_other_char);
            states_for_request.push_back(current_state);

            current_state_signals->completion_transition(to_run, true);
            if (current_state->get_hash() !=
                current_state_signals->get_hash()) {
                states_for_request.push_back(current_state_signals);
                log_completion(current_state_signals, leaf_hiearchy_other_char);
            }
        }

        for (int i = 0; i < states_for_request.size(); ++i) {
            State* state_for_request = states_for_request[i];

            std::vector<int> eligibles_candidates =
                state_for_request->get_eligibles();
            std::vector<std::vector<int>> all_eligibles =
                power_set(eligibles_candidates);

            for (int j = 0; j < all_eligibles.size(); ++j) {
                std::vector<int> const& current_eligibles = all_eligibles[j];

                State* submit_state = new State(*state_for_request);
                submit_state->request_transition(current_eligibles);
                new_states.push_back(submit_state);

                bool is_last_request = i == states_for_request.size() - 1 and
                                       j == all_eligibles.size() - 1;
                std::string third_hiearchy_char =
                    is_last_request ? std::string("└") : std::string("├");
                log_request(submit_state, leaf_hiearchy_other_char,
                            third_hiearchy_char);
                connect_neighbor_graphviz(current_state_bkp, submit_state);
            }
            delete state_for_request;
        }

        delete current_state_bkp;
    }

    return new_states;
}

// GRAPHIZ FUNCTIONS

void Graph::graphiz_setup(std::string path) {
    if (!plot_graph) {
        return;
    }

    std::ofstream o_file;
    o_file.open(path);
    o_file << "digraph G "
              "{\n"
              "node[shape=\"box\",style=\"rounded,filled\", "
              "fontname=\"helvetica\"]"
              "\n";

    // initial_state
    o_file << "tasks_info [ shape=\"plaintext\",style=\"\",label = <\n"
              " <table bgcolor=\"black\" color=\"white\">\n"
              "  <tr>\n"
              "    <td bgcolor=\"white\">i</td>\n"
              "    <td bgcolor=\"white\">T</td>\n"
              "    <td bgcolor=\"white\">D</td>\n"
              "    <td bgcolor=\"white\">X</td>\n"
              "    <td bgcolor=\"white\">C1</td>\n"
              "    <td bgcolor=\"white\">C2</td>\n"
              "  </tr>\n";

    for (int i = 0; i < initial_state->get_size(); ++i) {
        Job* j = initial_state->get_job(i);
        o_file << "  <tr>\n";
        o_file << "    <td bgcolor=\"white\">" << i << "</td>\n";
        o_file << "    <td bgcolor=\"white\">" << j->get_T() << "</td>\n";
        o_file << "    <td bgcolor=\"white\">" << j->get_D() << "</td>\n";
        o_file << "    <td bgcolor=\"white\">" << j->get_X() << "</td>\n";
        o_file << "    <td bgcolor=\"white\">" << j->get_C()[0] << "</td>\n";
        o_file << "    <td bgcolor=\"white\">" << j->get_C()[1] << "</td>\n";
        o_file << "  </tr>\n";
    }
    o_file << " </table>\n"
              "> ]\n\n";

    o_file.close();
}

void Graph::graphiz_teardown(std::string path) {
    if (!plot_graph) {
        return;
    }
    append_to_file(path, "\n}");
}

void Graph::connect_neighbor_graphviz(State* from, State* to) const {
    if (!plot_graph) {
        return;
    }

    std::string from_node_id;
    std::string to_node_id;

    from_node_id = from->get_node_id();
    to_node_id = to->get_node_id();

    std::string to_node_arg = "";
    for (std::function<bool(State*)> safe_oracle : safe_oracles) {
        if (safe_oracle(to)) {
            to_node_arg = ",color=green,penwidth=5";
        }
    }
    for (std::function<bool(State*)> unsafe_oracle : unsafe_oracles) {
        if (unsafe_oracle(to)) {
            to_node_arg = ",color=red,penwidth=5";
        }
    }

    std::string from_node_desc = from->dot_node(from_node_id);
    std::string to_node_desc = to->dot_node(to_node_id, to_node_arg);

    std::stringstream edge_desc;

    edge_desc << from_node_id << " -> " << to_node_id << std::endl;

    append_to_file(graph_output_path, from_node_desc);
    append_to_file(graph_output_path, to_node_desc);
    append_to_file(graph_output_path, edge_desc.str());
};

// LOGGING FUNCTIONS

void Graph::log_start() {
    if (verbose >= 0)
        std::cout << "╒══ Start Breadth First Search ═══" << std::endl;
}

void Graph::log_end(bool res, int64_t visited_count, int step_i,
                    std::chrono::milliseconds duration) {
    if (verbose >= 0)
        std::cout << "╘══ Automaton is " << (res ? "SAFE" : "UNSAFE")
                  << " | visited " << visited_count << " states | depth "
                  << step_i << " | time " << duration.count() << " ms ═══"
                  << std::endl;
}

void Graph::log_step(int step_i, int64_t visited_count, int leaf_states_size) {
    if (verbose >= 1)
        std::cout << "├" << (verbose >= 2 ? "┬" : "") << " Depth: " << step_i
                  << ", visited: " << visited_count
                  << ", leaf state size: " << leaf_states_size << std::endl;
}

void Graph::log_unsafe(State* unsafe_state) {
    if (verbose >= 2) {
        std::cout << "│└ UNSAFE  " << unsafe_state->str() << std::endl;
    }
}

void Graph::log_safe(State* safe_state) {
    if (verbose >= 2) {
        std::cout << "│├ SAFE  " << safe_state->str() << std::endl;
    }
}

void Graph::log_start(State* state, std::string second_hiearchy_char) {
    if (verbose >= 2) {
        std::cout << "│" << second_hiearchy_char << "┬ ";
        std::cout << "START " << state->str() << std::endl;
    }
}

void Graph::log_run(State* state, std::string second_hiearchy_char) {
    if (verbose >= 2) {
        std::cout << "│" << second_hiearchy_char << "├ ";
        std::cout << "RUN " << state->str() << std::endl;
    }
}

void Graph::log_completion(State* state, std::string second_hiearchy_char) {
    if (verbose >= 2) {
        std::cout << "│" << second_hiearchy_char << "├ ";
        std::cout << "COMPLETION " << state->str() << std::endl;
    }
}

void Graph::log_request(State* state, std::string second_hiearchy_char,
                        std::string third_hiearchy_char) {
    if (verbose >= 2) {
        std::cout << "│" << second_hiearchy_char << third_hiearchy_char;
        std::cout << " REQUEST " << state->str() << std::endl;
    }
}