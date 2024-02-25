#include "Graph.h"
#include "Job.h"
#include "SafeOracle.h"
#include "Scheduler.h"
#include "State.h"
#include "UnsafeOracle.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

void statespace_antichain_experiment(State* initial_state, int test_case_id, std::string output_path) {
    std::ofstream output_file;
    output_file.open(output_path, std::ios::in | std::ios::out | std::ios::ate);

    int64_t* search_result;
    Graph g(initial_state, &Scheduler::edfvd, "", -1, {}, {});

    std::stringstream search_result_csv_line;

    search_result = g.bfs();
    search_result_csv_line << test_case_id << ",BFS,EDF-VD,None,None," << search_result[0] << "," << search_result[1]
                           << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();
    search_result_csv_line.str("");

    search_result = g.acbfs();
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,None," << search_result[0] << "," << search_result[1]
                           << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    output_file.close();
};

void statespace_oracle_experiment(State* initial_state, int test_case_id, std::string output_path) {
    std::ofstream output_file;
    output_file.open(output_path, std::ios::in | std::ios::out | std::ios::ate);

    std::stringstream search_result_csv_line;
    int64_t* search_result;

    Graph g(initial_state, &Scheduler::edfvd, "", -1, {}, {});

    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,None," << search_result[0] << "," << search_result[1]
                           << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.set_safe_oracle(&SafeOracle::all_idle_hi);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,all_idle_hi,None," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_safe_oracle();
    g.set_safe_oracle(&SafeOracle::edf_carryoverjobs);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,edf_carryoverjobs,None," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_safe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::interference);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,interference," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_safe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::worst_interference);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,hi_interference," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_safe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::all_interference);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,all_interference," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_unsafe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::laxity);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,laxity," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_unsafe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::worst_laxity);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,worst_laxity," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_unsafe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::sum_sorted_laxities);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,sum_sorted_laxities," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    g.clear_unsafe_oracle();
    g.set_unsafe_oracle(&UnsafeOracle::sum_sorted_worst_laxities);
    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,None,sum_sorted_worst_laxities," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    output_file.close();
};

void make_graph_experiment(State* initial_state, int test_case_id, std::string output_path) {
    std::ofstream output_file;
    output_file.open(output_path, std::ios::in | std::ios::out | std::ios::ate);

    std::stringstream search_result_csv_line;
    int64_t* search_result;

    Graph g(initial_state, &Scheduler::edfvd, "./graph_experiment.dot", -1, {}, {});

    g.set_safe_oracle(&SafeOracle::edf_carryoverjobs);

    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,edf_carryoverjobs,None," << search_result[0] << ","
                           << search_result[1] << "," << search_result[2] << "," << search_result[3] << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    output_file.close();
};

void scheduling_performance_experiment(State* initial_state, int test_case_id, std::string output_path) {
    std::ofstream output_file;
    output_file.open(output_path, std::ios::in | std::ios::out | std::ios::ate);

    std::stringstream search_result_csv_line;
    int64_t* search_result;

    Graph g(new State(*initial_state), &Scheduler::edfvd, "", -1, {&SafeOracle::edf_carryoverjobs},
            {&UnsafeOracle::worst_interference});

    search_result = g.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,EDF-VD,edf_carryoverjobs,hi_interference," << search_result[0]
                           << "," << search_result[1] << "," << search_result[2] << "," << search_result[3]
                           << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    Graph g2(new State(*initial_state), &Scheduler::lwlf, "", -1, {&SafeOracle::edf_carryoverjobs},
             {&UnsafeOracle::worst_interference});

    search_result = g2.acbfs();
    search_result_csv_line.str("");
    search_result_csv_line << test_case_id << ",ACBFS,LWLF,edf_carryoverjobs,hi_interference," << search_result[0]
                           << "," << search_result[1] << "," << search_result[2] << "," << search_result[3]
                           << std::endl;
    std::cout << search_result_csv_line.str();
    output_file << search_result_csv_line.str();

    output_file.close();

    delete initial_state;
};

void read_task_sets(std::string const& input_path, std::string const& output_path,
                    std::function<void(State*, int, std::string)> experiment, int offset = 0, int n_experiments = -1) {
    std::ifstream input_file(input_path);
    if (!input_file.is_open()) {
        std::cerr << "Problem opening input file: " << input_path << std::endl;
        std::cerr << "Current directory: " << std::filesystem::current_path() << std::endl;

        exit(1);
    }

    int t;  // n test cases
    int n;  // n tasks in test case
    int T, D, X, c1, c2;
    std::vector<Job*> jobs;
    int end;

    input_file >> t;

    if (n_experiments == -1) {
        end = t;
    } else {
        end = offset + n_experiments;
    }

    for (int i = 0; i < end; i++) {
        input_file >> n;
        for (int j = 0; j < n; j++) {
            input_file >> T >> D >> X;
            input_file >> c1 >> c2;
            if (i >= offset) {
                Job* job = new Job(T, D, int2crit(X), std::vector<int>{c1, c2});
                jobs.push_back(job);
            }
        }
        if (i >= offset) {
            State* initial_state = new State(jobs);
            experiment(initial_state, i, output_path);
        }
        jobs.clear();
    }
};

void output_file_setup(std::string const& output_path) {
    std::ofstream output_file;
    output_file.open(output_path);

    output_file << "tid,search_type,scheduler,safe,unsafe,schedulable,depth,n_"
                   "visited,duration"
                << std::endl;

    output_file.close();
}

void dev_main() {
    // Job* j = new Job(3, 3, 2, std::vector<int>{2, 3});
    // Job* j2 = new Job(3, 3, 1, std::vector<int>{1, 1});
    // Job* j = new Job(5, 5, 2, std::vector<int>{1, 5});
    // Job* j2 = new Job(5, 5, 1, std::vector<int>{4, 4});

    // State* s = new State(std::vector<Job*>{j, j2});
    State* s = new State(std::vector<Job*>{
        new Job(11, 11, HI, std::vector<int>{2, 3}),
        new Job(12, 12, LO, std::vector<int>{6, 6}),
        new Job(3, 3, HI, std::vector<int>{1, 2}),
    });

    std::vector<std::function<bool(State*)>> safe_oracles, unsafe_oracles;

    safe_oracles = {&SafeOracle::all_idle_hi};
    unsafe_oracles = {&UnsafeOracle::worst_interference};

    Graph g(s, &Scheduler::reduce_interference, "./test.dot", 3, safe_oracles, unsafe_oracles);
    g.acbfs();
}

int main(int argc, char** argv) {
    if (argc == 1) {
        dev_main();
    } else if (argc >= 4) {
        std::string xp_type = argv[1];
        std::string input_path = argv[2];
        std::string output_path = argv[3];
        std::function<void(State*, int, std::string)> experiment;
        int offset = 0;
        int n_experiments = -1;
        if (argc >= 5) offset = std::atoi(argv[4]);
        if (argc >= 6) n_experiments = std::atoi(argv[5]);

        if (xp_type == "antichain") {
            experiment = statespace_antichain_experiment;
        } else if (xp_type == "scheduling") {
            experiment = scheduling_performance_experiment;
        } else if (xp_type == "oracle") {
            experiment = statespace_oracle_experiment;
        } else if (xp_type == "plot_graph") {
            experiment = make_graph_experiment;
        } else {
            std::cout << "Unknown experiment type: " << xp_type << std::endl;
            return 0;
        }

        output_file_setup(output_path);
        read_task_sets(input_path, output_path, experiment, offset, n_experiments);
    } else {
        std::cout << "Usage: " << argv[0] << " <xp_type> <input_path> <output_path>" << std::endl;
        return 0;
    }

    return 0;
}
