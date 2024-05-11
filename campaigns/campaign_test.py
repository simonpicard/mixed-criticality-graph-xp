#!/usr/bin/env python3

"""
Example of campaign script.
"""


from benchkit.campaign import CampaignSuite, CampaignCartesianProduct, CampaignIterateVariables
from benchkit.utils.dir import get_curdir

from benchmarks import MCSBench



def main() -> None:
    timeout_seconds = 3

    # Define the campaign, associated with the LevelDB benchmark
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

    taskset_files = ["outputs/20240511_094319-statespace-period-max.txt"]
    taskset_files = ["outputs/20240511_133342-statespace-utilisation.txt"]
    taskset_positions = range(5)
    taskset_positions = [0, 20, 40, 60]
    scheduler = "edfvd"
    safe_oracles = ["hi-idle-point"]
    unsafe_oracles = ["hi-over-demand"]

    varying_variables = [{
        "taskset_file": tf,
        "taskset_position": tp,
    } for tf in taskset_files for tp in taskset_positions]
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

    # Define the campaign suite and run the benchmarks in the suite
    campaigns = [campaign01]
    suite = CampaignSuite(campaigns=campaigns)
    suite.print_durations()
    suite.run_suite()

    # Generate a graph to visualize the results
    campaign01.generate_graph(
        plot_name="lineplot",
        # x="max_period",
        x="U",
        # y="visited_count",
        y="duration_ns",
        hue="use_case",
    )


if __name__ == "__main__":
    main()
