#!/usr/bin/env python3

"""
Example of campaign script.
"""

import concurrent.futures
import datetime
import pathlib
import random

import pandas as pd
import tqdm
from benchkit.campaign import (
    Campaign,
    CampaignCartesianProduct,
    CampaignIterateVariables,
    CampaignSuite,
)
from benchkit.utils.types import PathType
from benchmarks import MCSBench

import os
from typing import Tuple

# timeout_seconds = 300
timeout_seconds = 36000


def nb_systems(tasksystems_path: PathType) -> int:
    path = pathlib.Path("..") / tasksystems_path
    with open(path) as tasksystems_file:
        value_read = tasksystems_file.readline().split()[0]
    if not value_read.isdigit():
        raise ValueError(f'Cannot read number of systems in: "{tasksystems_path}"')
    result = int(value_read)
    return result


def get_taskset_filename(experiment: str, benchmark: MCSBench) -> pathlib.Path:
    print(experiment)
    root_dir = pathlib.Path(__file__).parent.parent.resolve()
    outputs_dir = root_dir / "outputs"
    files = sorted(
        [f for f in os.listdir(outputs_dir) if f.endswith(f"-{experiment}.csv")]
    )

    if len(files) == 0:
        benchmark.make_taskset(experiment=experiment)
        # recursive call, but this time we have a taskset:
        result = get_taskset_filename(experiment=experiment, benchmark=benchmark)
    else:
        result = (outputs_dir / files[-1]).resolve()

    return result


def get_both_taskset_filenames(experiment: str, benchmark: MCSBench) -> Tuple[str, str]:
    taskset_csv_path = get_taskset_filename(experiment=experiment, benchmark=benchmark)
    filename_csv = str(taskset_csv_path.name)
    filename_txt = filename_csv[:-4] + ".txt"
    return f"outputs/{filename_txt}", f"outputs/{filename_csv}"


def taskset2filename(experiment: str, benchmark: MCSBench) -> str:
    period_tasket_filenames = get_both_taskset_filenames(
        experiment=experiment,
        benchmark=benchmark,
    )
    period_tasket_filename = period_tasket_filenames[0]
    return period_tasket_filename


def campaign_state_space():
    benchmark = MCSBench(timeout_seconds=timeout_seconds)

    taskset_files = [taskset2filename(f, benchmark) for f in [
        "statespace-rtss-utilisation",
        "statespace-rtss-period-max",
        "statespace-rtss-n-tasks",
    ]]

    varying_variables = [
        {
            "taskset_file": tf,
            "taskset_position": tp,
        }
        for tf in taskset_files
        for tp in range(nb_systems(tasksystems_path=tf))
    ]

    base_config = {
        "scheduler": "edfvd",
        "safe_oracles": [],
    }

    use_cases = [
        {
            **base_config,
            "use_case": "ACBFS, no oracle",
            "use_idlesim": True,
            "unsafe_oracles": [],
        },
        {
            **base_config,
            "use_case": "ACBFS, oracles",
            "use_idlesim": True,
            "unsafe_oracles": ["hi-over-demand"],
        },
    ]
    variables = [
        use_case | other_variables
        for use_case in use_cases
        for other_variables in varying_variables
    ]

    campaign01 = CampaignIterateVariables(
        name="mcs_scale01",
        benchmark=benchmark,
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


def campaign_state_space_bfs():
    benchmark = MCSBench(timeout_seconds=timeout_seconds)

    taskset_files = [taskset2filename(f, benchmark) for f in [
        "statespace-rtss-bfs",
    ]]

    varying_variables = [
        {
            "taskset_file": tf,
            "taskset_position": tp,
        }
        for tf in taskset_files
        for tp in range(nb_systems(tasksystems_path=tf))
    ]

    base_config = {
        "scheduler": "edfvd",
        "safe_oracles": [],
        "unsafe_oracles": [],
    }

    use_cases = [
        {
            **base_config,
            "use_case": "BFS, no oracle",
            "use_idlesim": False,
        },
        {
            **base_config,
            "use_case": "ACBFS, no oracle",
            "use_idlesim": True,
        },
    ]
    variables = [
        use_case | other_variables
        for use_case in use_cases
        for other_variables in varying_variables
    ]

    campaign01 = CampaignIterateVariables(
        name="mcs_scale_bfs",
        benchmark=benchmark,
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

def campaign_schedulability():
    # make generate-set-scheduling-rtss
    taskset_files = [
        "outputs/20240511_133342-scheduling-rtss.txt"
    ]  # TODO function get the last


    varying_variables = [
        {
            "taskset_file": tf,
            "taskset_position": tp,
        }
        for tf in taskset_files
        for tp in range(nb_systems(tasksystems_path=tf))
    ]

    base_config = {
        "use_idlesim": True,
        "safe_oracles": [],
        "unsafe_oracles": ["hi-over-demand"],
    }

    use_cases = [
        {
            **base_config,
            "use_case": "EDF-VD (exact)",
            "scheduler": "edfvd",
        },
        {
            **base_config,
            "use_case": "LWLF (exact)",
            "scheduler": "lwlf",
        },
    ]
    variables = [
        use_case | other_variables
        for use_case in use_cases
        for other_variables in varying_variables
    ]

    campaign01 = CampaignIterateVariables(
        name="mcs_schedulability",
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


def campaign_oracles():
    # make generate-set-oracles-rtss
    taskset_files = [
        "outputs/20240511_133342-oracles-rtss.txt"
    ]  # TODO function get the last

    varying_variables = [
        {
            "taskset_file": tf,
            "taskset_position": tp,
        }
        for tf in taskset_files
        for tp in range(nb_systems(tasksystems_path=tf))
    ]

    base_config = {
        "scheduler": "edfvd",
        "use_idlesim": True,
    }

    use_cases = [
        {
            **base_config,
            "use_case": "baseline",
            "safe_oracles": [],
            "unsafe_oracles": [],
        },
        {
            **base_config,
            "use_case": "HI idle point",
            "safe_oracles": ["hi-idle-point"],
            "unsafe_oracles": [],
        },
        {
            **base_config,
            "use_case": "Negative laxity",
            "safe_oracles": [],
            "unsafe_oracles": ["negative-laxity"],
        },
        {
            **base_config,
            "use_case": "Negative wost laxity",
            "safe_oracles": [],
            "unsafe_oracles": ["negative-worst-laxity"],
        },
        {
            **base_config,
            "use_case": "Over demand",
            "safe_oracles": [],
            "unsafe_oracles": ["over-demand"],
        },
        {
            **base_config,
            "use_case": "HI over demand",
            "safe_oracles": [],
            "unsafe_oracles": ["hi-over-demand"],
        },
    ]
    variables = [
        use_case | other_variables
        for use_case in use_cases
        for other_variables in varying_variables
    ]

    campaign01 = CampaignIterateVariables(
        name="mcs_oracles",
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


def campaign_compression_table():
    # make generate-set-oracles-rtss --- the same as campaign_oracles()
    taskset_files = [
        "outputs/20240511_133342-oracles-rtss.txt"
    ]  # TODO function get the last

    varying_variables = [
        {
            "taskset_file": tf,
            "taskset_position": tp,
        }
        for tf in taskset_files
        for tp in range(nb_systems(tasksystems_path=tf))
    ]

    base_config = {
        "scheduler": "edfvd",
    }

    use_cases = [
        {
            **base_config,
            "use_case": "BFS",
            "safe_oracles": [],
            "unsafe_oracles": [],
            "use_idlesim": False,
        },
        {
            **base_config,
            "use_case": "ACBFS",
            "safe_oracles": [],
            "unsafe_oracles": [],
            "use_idlesim": True,
        },
        {
            **base_config,
            "use_case": "BFS, HI over demand",
            "safe_oracles": [],
            "unsafe_oracles": ["hi-over-demand"],
            "use_idlesim": False,
        },
        {
            **base_config,
            "use_case": "ACBFS, HI over demand",
            "safe_oracles": [],
            "unsafe_oracles": ["hi-over-demand"],
            "use_idlesim": True,
        },
    ]
    variables = [
        use_case | other_variables
        for use_case in use_cases
        for other_variables in varying_variables
    ]

    campaign01 = CampaignIterateVariables(
        name="mcs_compression_table",
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


def run_campaign_state_space():
    campaign01 = campaign_state_space()

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
    raw_output = benchmark.single_run(benchmark_duration_seconds=None, **record)
    output = benchmark.parse_output_to_results(
        command_output=raw_output, run_variables=record
    )
    return record | output


def parallel_runner(campaign: Campaign, nb_cpus: int) -> None:
    benchmark = campaign.parameters["benchmark"]
    records = campaign.parameters["variables"]
    benchmark.prebuild_bench()
    benchmark.build_bench()

    random.shuffle(records)
    name = campaign.parameters["experiment_name"]
    d = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    path = pathlib.Path(f"/tmp/results-{name}-{d}.csv")

    with concurrent.futures.ThreadPoolExecutor(
        max_workers=nb_cpus
    ) as executor, tqdm.tqdm(total=len(records)) as pbar:
        futures = [executor.submit(run_record, benchmark, record) for record in records]

        results = []
        for future in concurrent.futures.as_completed(futures):
            try:
                result = future.result()
                results.append(result)
                print(f"Result collected: {result}")
                result_keys = [k for k in result]
                result_values = [result[k] for k in result_keys]
                if not path.is_file():
                    with open(path, "a") as f:
                        f.write(";".join(map(str, result_keys)) + "\n")
                with open(path, "a") as f:
                    f.write(";".join(map(str, result_values)) + "\n")
                pbar.update(1)
            except Exception as e:
                print(f"An error occurred: {e}")

    df = pd.DataFrame(results)
    df.to_csv("/tmp/results.csv", sep=";", index=False)


def main() -> None:
    # parallel_runner(campaign=campaign_state_space_bfs(), nb_cpus=8)
    parallel_runner(campaign=campaign_state_space(), nb_cpus=8)


if __name__ == "__main__":
    main()
