#!/usr/bin/env python3

"""
Example of campaign script.
"""
import pandas as pd

from benchkit.campaign import CampaignSuite, CampaignCartesianProduct, CampaignIterateVariables
from benchkit.utils.types import PathType

from benchmarks import MCSBench
import concurrent.futures
import pathlib
import random


timeout_seconds = 300


def campaign_test():
    campaign00 = CampaignCartesianProduct(
        name="mcs_scale00",
        benchmark=MCSBench(timeout_seconds=timeout_seconds),
        nb_runs=20,
        variables={
            "taskset_file": ["outputs/20240511_094319-statespace-period-max.txt"],
            "taskset_position": [5],
            "use_idlesim": [True],
            "scheduler": ["edfvd"],
            "safe_oracles": [["hi-idle-point"]],
            "unsafe_oracles": [["hi-over-demand"]],
        },
        constants={},
        debug=False,
        gdb=False,
        enable_data_dir=True,
        continuing=False,
        benchmark_duration_seconds=None,
    )


def nb_systems(tasksystems_path: PathType) -> int:
    path = pathlib.Path('..')/tasksystems_path
    with open(path) as tasksystems_file:
        value_read = tasksystems_file.readline().split()[0]
    if not value_read.isdigit():
        raise ValueError(f'Cannot read number of systems in: "{tasksystems_path}"')
    result = int(value_read)
    return result


def campaign_first():
    taskset_files = ["outputs/20240511_094319-statespace-period-max.txt"]
    taskset_files = ["outputs/20240511_133342-statespace-utilisation.txt"]
    # taskset_positions = range(nb_systems(taskset_files[0]))
    # taskset_positions = [0, 20, 40, 60]
    scheduler = "edfvd"
    safe_oracles = ["hi-idle-point"]
    unsafe_oracles = ["hi-over-demand"]

    varying_variables = [{
        "taskset_file": tf,
        "taskset_position": tp,
    } for tf in taskset_files for tp in range(nb_systems(tasksystems_path=tf))]
    use_cases = [
        {
            "use_case": "BFS, no oracle",
            "scheduler": scheduler,
            "use_idlesim": False,
            "safe_oracles": [],
            "unsafe_oracles": [],
        },
        {
            "use_case": "ACBFS, no oracle",
            "scheduler": scheduler,
            "use_idlesim": True,
            "safe_oracles": [],
            "unsafe_oracles": [],
        },
        {
            "use_case": "ACBFS, oracles",
            "scheduler": scheduler,
            "use_idlesim": True,
            "safe_oracles": [],
            "unsafe_oracles": unsafe_oracles,
        },
    ]
    variables = [use_case | other_variables for use_case in use_cases for other_variables in varying_variables]

    campaign01 = CampaignIterateVariables(
        name="mcs_scale01",
        benchmark=MCSBench(timeout_seconds=timeout_seconds),
        nb_runs=1,
        variables=variables,
        constants={},
        debug=False,
        gdb=False,
        enable_data_dir=True,
        continuing=False,
        benchmark_duration_seconds=None,
    )

    return campaign01


def run_campaign_first():
    campaign01 = campaign_first()

    campaigns = [campaign01]
    suite = CampaignSuite(campaigns=campaigns)
    suite.print_durations()
    suite.run_suite()

    campaign01.generate_graph(
        plot_name="lineplot",
        # x="max_period",
        x="U",
        # y="visited_count",
        y="duration_ns",
        hue="use_case",
    )

    # TODO check coherence dataset; i.e. groupby "ts_id" and schedulable should have the same values


def run_record(benchmark, record):
    print(f"Running record with variables: {record}")
    # process = benchmark.single_run(benchmark_duration_seconds=None, **record)
    # process.wait()
    # raw_output = process.output()
    raw_output = benchmark.single_run(benchmark_duration_seconds=None, **record)
    output = benchmark.parse_output_to_results(command_output=raw_output, run_variables=record)
    return record | output


def parallel_runner():
    campaign01 = campaign_first()
    benchmark = campaign01.parameters["benchmark"]
    records = campaign01.parameters["variables"]
    nb_cpus = benchmark.platform.nb_cpus()
    benchmark.build_bench()

    # Example asynchronous run for a single process:
    # record = records[0]
    # process = benchmark.single_run(benchmark_duration_seconds=None, **record)
    # process.wait()
    # raw_output = process.output()
    # raw_output = benchmark.single_run(benchmark_duration_seconds=None, **record)
    # output = benchmark.parse_output_to_results(command_output=raw_output, run_variables=record)
    # print(record | output)

    # exit(0)
    # I would like to extend this to run all records (in the records List) in parallel, but limit the parallelism to the number of availables cpus (nb_cpus)
    # What would you propose?

    random.shuffle(records)

    with concurrent.futures.ThreadPoolExecutor(max_workers=nb_cpus) as executor:
        futures = [executor.submit(run_record, benchmark, record) for record in records]

        results = []
        for future in concurrent.futures.as_completed(futures):
            try:
                result = future.result()
                results.append(result)
                print(f"Result collected: {result}")
                path = pathlib.Path("/tmp/results2.csv")
                result_keys = [k for k in result]
                result_values = [result[k] for k in result_keys]
                if not path.is_file():
                    with open(path, "a") as f:
                        f.write(";".join(map(str, result_keys)) + "\n")
                with open(path, "a") as f:
                    f.write(";".join(map(str, result_values)) + "\n")
            except Exception as e:
                print(f"An error occurred: {e}")

    df = pd.DataFrame(results)
    df.to_csv("/tmp/results.csv", sep=";", index=False)


def main() -> None:
    parallel_runner()


if __name__ == "__main__":
    main()
