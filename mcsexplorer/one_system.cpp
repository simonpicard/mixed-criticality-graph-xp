#include "EdfInterference.h"
#include "Graph.h"
#include "SafeOracle.h"
#include "Scheduler.h"
#include "State.h"
#include "UnsafeOracle.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct {
    std::string inputfile_path;
    size_t taskset_position;
    bool use_idlesim = false;
    std::string scheduler = "edfvd";
    std::vector<std::string> safe_oracles;
    std::vector<std::string> unsafe_oracles;
    int log_level = -1;
} CONFIG;

State* read_task_set(std::string const& input_path, int offset = 0) {
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

    end = offset + 1;

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
            return initial_state;
        }
    }

    std::cerr << "Taskset of offset " << offset << " not found in given file: " << input_path << std::endl;
    exit(1);
}

void parse_args(int argc, char** argv) {
    int i = 1;
    while (i < argc) {
        std::string argument = argv[i];

        if ("--taskset-file" == argument) {
            i++;
            const std::string filepath = argv[i];
            if (not std::filesystem::is_regular_file(filepath)) {
                std::cerr << "Given taskset file is not a regular file: " << filepath << std::endl;
                std::cerr << "Current directory: " << std::filesystem::current_path() << std::endl;
                exit(1);
            }
            CONFIG.inputfile_path = filepath;
            i++;
        } else if ("--taskset-position" == argument) {
            i++;
            const std::string taskset_position_str = argv[i];
            const size_t taskset_position = static_cast<size_t>(std::stoi(taskset_position_str));
            CONFIG.taskset_position = taskset_position;
            i++;
        } else if ("--use-idlesim" == argument) {
            i++;
            CONFIG.use_idlesim = true;
        } else if ("--scheduler" == argument) {
            i++;
            const std::string scheduler = argv[i];
            CONFIG.scheduler = scheduler;
            i++;
        } else if ("--safe-oracles" == argument) {
            i++;
            const std::string safe_oracles = argv[i];

            std::string token;
            std::istringstream tokenStream(safe_oracles);
            while (std::getline(tokenStream, token, ',')) {
                CONFIG.safe_oracles.push_back(token);
            }
            i++;
        } else if ("--unsafe-oracles" == argument) {
            i++;
            const std::string unsafe_oracles = argv[i];

            std::string token;
            std::istringstream tokenStream(unsafe_oracles);
            while (std::getline(tokenStream, token, ',')) {
                CONFIG.unsafe_oracles.push_back(token);
            }
            i++;
        } else if ("--log-level" == argument) {
            i++;
            const std::string log_level_str = argv[i];
            const int log_level = static_cast<int>(std::stoi(log_level_str));
            CONFIG.log_level = log_level;
            i++;
        } else {
            i++;
            std::cerr << "Unknown argument: " << argument << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    parse_args(argc, argv);
    std::cout << "Arguments: " << std::endl;
    std::cout << "  input file: " << CONFIG.inputfile_path << std::endl;
    std::cout << "  taskset number: " << CONFIG.taskset_position << std::endl;
    std::cout << "  use idlesim: " << (CONFIG.use_idlesim ? "true" : "false") << std::endl;
    std::cout << "  scheduler: " << CONFIG.scheduler << std::endl;
    std::cout << "  safe-oracles: " << (CONFIG.safe_oracles.empty() ? "None" : "");
    for (auto oracle : CONFIG.safe_oracles) {
        std::cout << oracle << " ";
    }
    std::cout << std::endl;
    std::cout << "  unsafe-oracles: " << (CONFIG.unsafe_oracles.empty() ? "None" : "");
    for (auto oracle : CONFIG.unsafe_oracles) {
        std::cout << oracle << " ";
    }
    std::cout << std::endl;
    std::cout << "  log level: " << CONFIG.log_level << std::endl;

    State* start_state = read_task_set(CONFIG.inputfile_path, CONFIG.taskset_position);

    std::function<int(State*)> scheduler;
    if ("edfvd" == CONFIG.scheduler) {
        scheduler = Scheduler::edfvd;
    } else if ("lwlf" == CONFIG.scheduler) {
        scheduler = Scheduler::lwlf;
    } else {
        std::cerr << "Bad scheduler: " << CONFIG.scheduler << std::endl;
    }

    std::vector<std::function<bool(State*)>> safe_oracles, unsafe_oracles;

    for (std::string safe_oracle_str : CONFIG.safe_oracles) {
        if ("hi-idle-point" == safe_oracle_str) {
            safe_oracles.push_back(&SafeOracle::all_idle_hi);
        } else {
            std::cerr << "Bad safe oracle: " << safe_oracle_str << std::endl;
        }
    }

    for (std::string unsafe_oracle_str : CONFIG.unsafe_oracles) {
        if ("negative-laxity" == unsafe_oracle_str) {
            unsafe_oracles.push_back(&UnsafeOracle::laxity);
        } else if ("negative-worst-laxity" == unsafe_oracle_str) {
            unsafe_oracles.push_back(&UnsafeOracle::worst_laxity);
        } else if ("over-demand" == unsafe_oracle_str) {
            unsafe_oracles.push_back(&UnsafeOracle::over_demand);
        } else if ("hi-over-demand" == unsafe_oracle_str) {
            unsafe_oracles.push_back(&UnsafeOracle::hi_over_demand);
        } else {
            std::cerr << "Bad unsafe oracle: " << unsafe_oracle_str << std::endl;
        }
    }

    Graph graph(start_state, scheduler, "", CONFIG.log_level, safe_oracles, unsafe_oracles);

    int64_t* result;
    if (CONFIG.use_idlesim) {
        result = graph.acbfs();
    } else {
        result = graph.bfs();
    }

    std::cout << "Results:";
    std::cout << "is_safe=" << (result[0] == 1 ? "True" : "False") << ";";
    std::cout << "automaton_depth=" << result[1] << ";";
    std::cout << "visited_count=" << result[2] << ";";
    std::cout << "duration_ns=" << result[3];

    return 0;
}
