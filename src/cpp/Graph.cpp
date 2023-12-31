#include "Graph.h"
#include "util.cpp"

Graph::Graph() = default;

Graph::~Graph() { delete initial_state; }

bool Graph::is_fail(std::vector<State*> const& states) {
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

void Graph::run_tansition(State* state, int to_run) {
    state->run_tansition(to_run);
}

std::vector<State*> Graph::completion_transition(State* state, int to_run) {
    State* state_signals = new State(*state);
    state->completion_transition(to_run, false);
    state_signals->completion_transition(to_run, true);
    if (state->get_hash() != state_signals->get_hash()) {
        return std::vector<State*>{state, state_signals};
    }
    delete state_signals;
    return std::vector<State*>{state};
}

std::vector<State*> Graph::request_transition(
    std::vector<State*> const& states) {
    std::vector<State*> new_states;

    for (State* current_state : states) {
        std::vector<int> eligibles_candidates = current_state->get_eligibles();
        std::vector<std::vector<int>> all_eligibles =
            power_set(eligibles_candidates);

        for (std::vector<int> const& current_eligibles : all_eligibles) {
            State* request_state = new State(*current_state);
            request_state->request_transition(current_eligibles);
            new_states.push_back(request_state);
        }
        delete current_state;
    }

    return new_states;
}

bool Graph::has_unsafe(std::vector<State*> const& states) {
    for (std::function<bool(State*)> unsafe_oracle : unsafe_oracles) {
        for (State* current_state : states) {
            if (unsafe_oracle(current_state)) {
                log_unsafe(current_state);
                return true;
            }
        }
    }
    return false;
}

void Graph::handle_safe(std::vector<State*>& states) {
    for (std::function<bool(State*)> safe_oracle : safe_oracles) {
        if (verbose >= 2) {
            for (State* current_state : states) {
                if (safe_oracle(current_state)) {
                    log_safe(current_state);
                }
            }
        }

        states.erase(std::remove_if(states.begin(), states.end(), safe_oracle),
                     states.end());
    }
}

void Graph::handle_run_tansition(State* state, int to_run, bool is_last_leaf) {
    if (to_run > -1) {
        run_tansition(state, to_run);
        log_run(state, is_last_leaf);
    }
}

std::vector<State*> Graph::handle_completion_transition(State* state,
                                                        int to_run,
                                                        bool is_last_leaf) {
    if (to_run > -1) {
        u_int64_t original_hash = state->get_hash();
        std::vector<State*> completion_states =
            completion_transition(state, to_run);

        if (completion_states.size() > 1) {
            for (State* new_state : completion_states) {
                log_completion(new_state, is_last_leaf);
            }
        }
        return completion_states;
    }
    return std::vector<State*>{state};
}

std::vector<State*> Graph::handle_request_transition(
    std::vector<State*> const& states, bool is_last_leaf) {
    std::vector<State*> request_states = request_transition(states);

    for (int i = 0; i < request_states.size(); ++i) {
        State* request_state = request_states[i];
        log_request(request_state, is_last_leaf,
                    i == request_states.size() - 1);
    }

    return request_states;
}

std::vector<State*> Graph::get_neighbors(
    std::vector<State*> const& leaf_states) {
    std::vector<State*> new_states;

    for (int leaf_i = 0; leaf_i < leaf_states.size(); ++leaf_i) {
        State* current_state = leaf_states[leaf_i];
        State* original_leaf_state = new State(*current_state);

        // verbose setup
        bool is_last_leaf = leaf_i == leaf_states.size() - 1;
        log_start(current_state, is_last_leaf);

        // apply all three transitions
        int to_run = schedule(current_state);
        handle_run_tansition(current_state, to_run, is_last_leaf);
        std::vector<State*> states_for_request =
            handle_completion_transition(current_state, to_run, is_last_leaf);
        std::vector<State*> request_state =
            handle_request_transition(states_for_request, is_last_leaf);

        connect_neighbors_graphviz(original_leaf_state, request_state);

        delete original_leaf_state;

        // add new states
        new_states.insert(new_states.end(), request_state.begin(),
                          request_state.end());
    }

    return new_states;
}

void Graph::initialize_search(bool use_idle_antichain_current) {
    automaton_is_safe = true;
    use_idle_antichain = use_idle_antichain_current;
    visited_count = 0;
    automaton_depth = 0;
    start = std::chrono::high_resolution_clock::now();
}

int64_t* Graph::bfs() {
    initialize_search(false);

    graphiz_setup(graph_output_path);
    log_start_search();

    std::vector<State*> leaf_states{new State(*initial_state)};
    std::vector<State*> neighbors;

    std::unordered_set<uint64_t> visited_hashes;
    visited_hashes.insert(leaf_states[0]->get_hash());

    while (!leaf_states.empty()) {
        log_step(leaf_states.size());

        automaton_is_safe =
            not(is_fail(leaf_states) or has_unsafe(leaf_states));
        if (not automaton_is_safe) break;

        handle_safe(leaf_states);

        neighbors = get_neighbors(leaf_states);

        automaton_depth++;
        visited_count = visited_count + leaf_states.size();
        leaf_states.clear();

        for (State* neighbor : neighbors) {
            uint64_t state_hash = neighbor->get_hash();
            if (visited_hashes.find(state_hash) == visited_hashes.end()) {
                visited_hashes.insert(state_hash);
                leaf_states.push_back(neighbor);
            } else {
                delete neighbor;
            }
        }
    }

    if (!automaton_is_safe)
        for (State* elem : leaf_states) delete elem;

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    graphiz_teardown(graph_output_path);
    log_end_search(duration);

    static int64_t arr[4];

    arr[0] = int64_t(automaton_is_safe);
    arr[1] = automaton_depth;
    arr[2] = visited_count;
    arr[3] = duration.count();
    return arr;
}

bool pairwise_smaller_all(std::vector<int> a, std::vector<int> b) {
    if (a.size() != b.size()) {
        // if this happens there is a bug in the code about antichain max set
        std::cout << "a" << std::endl;
        for (int i : a) {
            std::cout << i << std::endl;
        }
        std::cout << "b" << std::endl;
        for (int i : b) {
            std::cout << i << std::endl;
        }
    }
    for (unsigned int i = 0; i < a.size(); i++) {
        if (a[i] > b[i]) return false;
    }
    return true;
}

int64_t* Graph::acbfs() {
    initialize_search(true);

    graphiz_setup(graph_output_path);
    log_start_search();

    std::vector<State*> leaf_states{new State(*initial_state)};
    std::vector<State*> neighbors;

    std::unordered_map<uint64_t, std::unordered_map<uint64_t, std::vector<int>>>
        visited_hashes;

    uint64_t initial_state_hash = initial_state->get_hash_idle();
    std::pair<uint64_t, std::vector<int>> initial_state_idle_nats_pair =
        initial_state->get_idle_nats_pair();

    visited_hashes[initial_state_hash] =
        std::unordered_map<uint64_t, std::vector<int>>(
            {{initial_state_idle_nats_pair.first,
              initial_state_idle_nats_pair.second}});

    while (!leaf_states.empty()) {
        log_step(leaf_states.size());

        automaton_is_safe =
            not(is_fail(leaf_states) or has_unsafe(leaf_states));
        if (not automaton_is_safe) break;

        handle_safe(leaf_states);

        neighbors = get_neighbors(leaf_states);

        automaton_depth++;
        visited_count = visited_count + leaf_states.size();
        leaf_states.clear();

        for (State* neighbor : neighbors) {
            uint64_t neighbor_hash = neighbor->get_hash_idle();

            std::pair<uint64_t, std::vector<int>> neighbor_idle_nats_pair =
                neighbor->get_idle_nats_pair();

            uint64_t& neighbor_idle_nats_hash = neighbor_idle_nats_pair.first;
            std::vector<int>& neighbor_idle_nats_vector =
                neighbor_idle_nats_pair.second;

            if (visited_hashes.find(neighbor_hash) == visited_hashes.end()) {
                // this arrangement of active jobs and their respecting rct has
                // not been visited earlier so no visited state can simulate the
                // neighbor

                visited_hashes[neighbor_hash] =
                    std::unordered_map<uint64_t, std::vector<int>>(
                        {{neighbor_idle_nats_hash, neighbor_idle_nats_vector}});
                leaf_states.push_back(neighbor);

            } else {
                // this arrangement of active jobs and their respecting rct has
                // been visited earlier and wether this neighbor is simulated or
                // not depends on the value of its nats
                bool neighbor_is_simulated = false;
                std::vector<u_int64_t> visited_is_simulated_hashes;

                if (visited_hashes[neighbor_hash].find(
                        neighbor_idle_nats_hash) !=
                    visited_hashes[neighbor_hash].end()) {
                    // this exact state has already been visited and we do not
                    // need to explore it
                    delete neighbor;
                } else {
                    for (const auto& [visited_idle_nat_hash,
                                      visited_idle_nat_vector] :
                         visited_hashes[neighbor_hash]) {
                        if (pairwise_smaller_all(visited_idle_nat_vector,
                                                 neighbor_idle_nats_vector)) {
                            neighbor_is_simulated = true;
                            simulate_neighbor_graphviz(neighbor,
                                                       visited_idle_nat_vector);
                            break;
                        }
                        if (pairwise_smaller_all(neighbor_idle_nats_vector,
                                                 visited_idle_nat_vector)) {
                            // this neighbor simulates some previously visited
                            // states which can safely be removed from memory
                            visited_is_simulated_hashes.push_back(
                                visited_idle_nat_hash);
                        }
                    }
                    for (u_int64_t const& key : visited_is_simulated_hashes) {
                        visited_hashes[neighbor_hash].erase(key);
                    }
                    if (!neighbor_is_simulated) {
                        visited_hashes[neighbor_hash][neighbor_idle_nats_hash] =
                            neighbor_idle_nats_vector;
                        leaf_states.push_back(neighbor);
                    } else {
                        log_simulated(neighbor);
                        delete neighbor;
                    }
                }
            }
        }
    }

    if (!automaton_is_safe)
        for (State* elem : leaf_states) delete elem;

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    graphiz_teardown(graph_output_path);
    log_end_search(duration);

    static int64_t arr[4];
    arr[0] = int64_t(automaton_is_safe);
    arr[1] = automaton_depth;
    arr[2] = visited_count;
    arr[3] = duration.count();

    return arr;
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

    std::stringstream legend;
    legend << "legend_nat_rct "
           << "[label=<rct,nat>,fillcolor=white]" << std::endl;
    legend << "legend_lo "
           << "[label=<LO>,fillcolor=lightcyan]" << std::endl;
    legend << "legend_hi "
           << "[label=<HI>,fillcolor=lightyellow]" << std::endl;
    legend << "legend_fail "
           << "[label=<fail>,color=orchid,fillcolor=white,penwidth=5]"
           << std::endl;
    legend << "legend_simulated "
           << "[label=<simulated>,color=blue,fillcolor=white,penwidth=5]"
           << std::endl;
    legend << "legend_safe "
           << "[label=<safe>,color=green,fillcolor=white,penwidth=5]"
           << std::endl;
    legend << "legend_unsafe "
           << "[label=<unsafe>,color=red,fillcolor=white,penwidth=5]"
           << std::endl;
    append_to_file(graph_output_path, legend.str());
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
    if (not use_idle_antichain) {
        from_node_id = from->get_node_id();
        to_node_id = to->get_node_id();
    } else {
        from_node_id = from->get_node_idle_id();
        to_node_id = to->get_node_idle_id();
    }

    std::string to_node_arg = "";
    for (std::function<bool(State*)> safe_oracle : safe_oracles) {
        if (safe_oracle(to)) {
            to_node_arg = ",color=green,penwidth=5";
            break;
        }
    }
    for (std::function<bool(State*)> unsafe_oracle : unsafe_oracles) {
        if (unsafe_oracle(to)) {
            to_node_arg = ",color=red,penwidth=5";
            break;
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

void Graph::connect_neighbors_graphviz(State* from,
                                       std::vector<State*> to_list) const {
    if (!plot_graph) {
        return;
    }
    for (State* to : to_list) {
        connect_neighbor_graphviz(from, to);
    }
}

void Graph::simulate_neighbor_graphviz(State* neighbor,
                                       std::vector<int> nats) const {
    std::stringstream simulator_hash;
    simulator_hash << "n_";
    simulator_hash << neighbor->get_hash_idle();
    for (int nat : nats) {
        simulator_hash << "_" << nat;
    }

    std::stringstream set_simulated_state_border;
    set_simulated_state_border << neighbor->get_node_idle_id()
                               << " [color=blue,penwidth=5]" << std::endl;
    append_to_file(graph_output_path, set_simulated_state_border.str());

    std::stringstream connect_simulated_state;
    connect_simulated_state << neighbor->get_node_idle_id() << " -> "
                            << simulator_hash.str() << " [style=\"dashed\"]"
                            << std::endl;
    // append_to_file(graph_output_path, connect_simulated_state.str());
}

// LOGGING FUNCTIONS

void Graph::repr(std::vector<State*> states) {
    for (int i = 0; i < states.size(); ++i) {
        std::cout << "S" << i << "-> " << states[i]->str() << std::endl;
    }
    std::cout << std::endl;
}

void Graph::log_start_search() {
    if (verbose >= 0)
        std::cout << "┌██ Start " << (use_idle_antichain ? "Antichain " : "")
                  << "Breadth First Search" << std::endl;
}

void Graph::log_end_search(std::chrono::milliseconds duration) {
    if (verbose >= 0)
        std::cout << "└██ Automaton is "
                  << (automaton_is_safe ? "SAFE" : "UNSAFE") << " | visited "
                  << visited_count << " states | depth " << automaton_depth
                  << " | time " << duration.count() << " ms" << std::endl;
}

void Graph::log_step(int leaf_states_size) {
    if (verbose >= 1)
        std::cout << "├" << (verbose >= 2 ? "┬" : "")
                  << " Depth: " << automaton_depth
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

std::string Graph::get_second_hiearchy_char(bool is_last_leaf) {
    return is_last_leaf ? std::string(" ") : std::string("│");
}

std::string Graph::get_third_hiearchy_char(bool is_last_request) {
    return is_last_request ? std::string("└") : std::string("├");
}

void Graph::log_start(State* state, bool is_last_leaf) {
    std::string second_hiearchy_char =
        is_last_leaf ? std::string("└") : std::string("├");
    if (verbose >= 2) {
        std::cout << "│" << second_hiearchy_char << "┬ ";
        std::cout << "START " << state->str() << std::endl;
    }
}

void Graph::log_run(State* state, bool is_last_leaf) {
    if (verbose >= 2) {
        std::cout << "│" << get_second_hiearchy_char(is_last_leaf) << "├ ";
        std::cout << "RUN " << state->str() << std::endl;
    }
}

void Graph::log_completion(State* state, bool is_last_leaf) {
    if (verbose >= 2) {
        std::cout << "│" << get_second_hiearchy_char(is_last_leaf) << "├ ";
        std::cout << "COMPLETION " << state->str() << std::endl;
    }
}

void Graph::log_request(State* state, bool is_last_leaf, bool is_last_request) {
    std::string third_hiearchy_char =
        is_last_request ? std::string("└") : std::string("├");
    if (verbose >= 2) {
        std::cout << "│" << get_second_hiearchy_char(is_last_leaf)
                  << get_third_hiearchy_char(is_last_request);
        std::cout << " REQUEST " << state->str() << std::endl;
    }
}

void Graph::log_simulated(State* simulated_state) {
    if (verbose >= 2) {
        std::cout << "├ SIMULATED " << simulated_state->str() << std::endl;
    }
}