import argparse
from datetime import datetime

import pandas as pd
from tqdm import tqdm

from EDFVD import test as test_edfvd
from set_generator import generate_random_task_set, generate_task_set_with_utilisation


def get_task_set_definition(task_set):
    task_set_definition = f"{len(task_set)}\n"
    for task in task_set:
        task_set_definition += f"{int(task['T'])} {int(task['D'])} {int(task['X'])+1}\n"
        task_set_definition += f"{int(task[0])} {int(task[1])}\n"
    return task_set_definition


def generate_per_n_tasks(
    task_sets_output=None,
    header_output=None,
    probability_of_HI=0.5,
    wcet_HI_ratio=2.5,
    min_period=6,
    max_period=24,
    max_wcet_LO=20,
    n_tasks_list=[2, 3, 4, 5],
    sets_per_amount=200,
):
    if not task_sets_output:
        task_sets_output = f"outputs/{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_statespace.txt"
    if not header_output:
        header_output = f"outputs/{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_statespace.csv"

    task_sets_header = pd.DataFrame()

    task_set_id = 0

    task_sets_definition = ""

    for n_tasks in n_tasks_list:
        generated_task_sets = set()
        for _ in tqdm(range(sets_per_amount), desc=f"n_tasks={n_tasks}"):
            while True:
                task_set = generate_random_task_set(
                    n_tasks=n_tasks,
                    probability_of_HI=probability_of_HI,
                    wcet_HI_ratio=wcet_HI_ratio,
                    max_wcet_LO=max_wcet_LO,
                    max_period=max_period,
                    min_period=min_period,
                )
                task_set_hash = task_set.get_hash()
                if task_set_hash not in generated_task_sets:
                    generated_task_sets.add(task_set_hash)
                    break
            task_sets_definition += get_task_set_definition(task_set)

            task_set_info = pd.Series(dtype=float)
            task_set_info["ts_id"] = task_set_id
            task_set_info["U"] = task_set.get_average_utilisation()
            task_set_info["nbt"] = len(task_set)
            task_set_info["EDFVD_test"] = int(test_edfvd(task_set))

            task_sets_header = pd.concat([task_sets_header, task_set_info.to_frame().T], ignore_index=True)

            task_set_id += 1

    task_sets_definition = f"{task_set_id}\n" + task_sets_definition
    task_sets_header.to_csv(header_output, index=False)
    with open(task_sets_output, "w") as text_file:
        text_file.write(task_sets_definition)


def generate_per_utilisation(
    task_sets_output=None,
    header_output=None,
    probability_of_HI=0.5,
    min_period=5,
    max_period=50,
    n_tasks=3,
    from_utilisation=89,
    to_utilisation=99,
    utilisation_step=1,
    sets_per_step=100,
):
    if not task_sets_output:
        task_sets_output = f"outputs/{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_scheduling.txt"
    if not header_output:
        header_output = f"outputs/{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_scheduling.csv"

    task_set_id = 0

    task_sets_header = pd.DataFrame()
    task_sets_definition = ""

    utilisations = [from_utilisation]
    while utilisations[-1] + utilisation_step <= to_utilisation:
        utilisations.append(utilisations[-1] + utilisation_step)

    for u in utilisations:
        u = u / 100
        generated_task_sets = set()
        for _ in tqdm(range(sets_per_step), desc=f"U={u*100:.0f}%"):
            while True:
                task_set = generate_task_set_with_utilisation(
                    n_tasks=n_tasks,
                    target_average_utilisation=u,
                    max_period=max_period,
                    probability_of_HI=probability_of_HI,
                    min_period=min_period,
                )
                task_set_hash = task_set.get_hash()
                if task_set_hash not in generated_task_sets:
                    generated_task_sets.add(task_set_hash)
                    break

            task_sets_definition += get_task_set_definition(task_set)

            task_set_info = pd.Series(dtype=float)
            task_set_info["ts_id"] = task_set_id
            task_set_info["U"] = u
            task_set_info["Uv"] = task_set.get_average_utilisation()
            task_set_info["nbt"] = len(task_set)
            task_set_info["EDFVD_test"] = int(test_edfvd(task_set))

            task_sets_header = pd.concat([task_sets_header, task_set_info.to_frame().T], ignore_index=True)

            task_set_id += 1

    task_sets_definition = f"{task_set_id}\n" + task_sets_definition

    task_sets_header.to_csv(header_output, index=False)
    with open(task_sets_output, "w") as text_file:
        text_file.write(task_sets_definition)


def read_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--type", "-t", help="type of set generation [n_tasks, utilisation]", type=str, required=True)
    parser.add_argument(
        "--task_sets_output",
        "-o",
        help="path to txt output with task sets description (.txt)",
        type=str,
        required=False,
        default=None,
    )
    parser.add_argument(
        "--header_output",
        "-c",
        help="path to csv output with task sets header (.csv)",
        type=str,
        required=False,
        default=None,
    )
    parser.add_argument(
        "--probability_of_HI",
        "-phi",
        help="probability of tasks being high-criticality",
        type=float,
        required=True,
    )
    parser.add_argument(
        "--wcet_HI_ratio",
        "-rhi",
        help="the maximum ratio between high- and low-criticality execution time",
        type=float,
        required=False,
    )
    parser.add_argument(
        "-tas",
        "--task_amounts",
        nargs="+",
        help="list of the number of tasks per task set",
        type=int,
        required=False,
    )
    parser.add_argument(
        "-s",
        "--set_per_amount",
        help="amout of task sets to generate for each number of tasks",
        required=False,
        type=int,
    )
    parser.add_argument(
        "-max_t",
        "--maximum_period",
        help="maximum period",
        required=True,
        type=int,
    )
    parser.add_argument(
        "-min_t",
        "--minimum_period",
        help="minimum period",
        required=True,
        type=int,
    )
    parser.add_argument(
        "-ta",
        "--task_amount",
        help="number of tasks per task set",
        type=int,
        required=False,
    )
    parser.add_argument(
        "-max_c_lo",
        "--max_wcet_LO",
        help="the maximum WCET for low-criticality execution time",
        type=float,
        required=False,
    )
    parser.add_argument(
        "-u",
        "--from_utilisation",
        help="from utilization",
        type=float,
        required=False,
    )
    parser.add_argument(
        "-U",
        "--to_utilisation",
        help="to utilization",
        type=float,
        required=False,
    )
    parser.add_argument(
        "-us",
        "--utilisation_step",
        help="step",
        type=float,
        required=False,
    )
    parser.add_argument(
        "-ss",
        "--sets_per_step",
        help="sets per step",
        type=int,
        required=False,
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = read_args()
    if args.type == "n_tasks":
        generate_per_n_tasks(
            args.task_sets_output,
            args.header_output,
            args.probability_of_HI,
            args.wcet_HI_ratio,
            args.minimum_period,
            args.maximum_period,
            args.max_wcet_LO,
            args.task_amounts,
            args.set_per_amount,
        )
    elif args.type == "utilisation":
        generate_per_utilisation(
            args.task_sets_output,
            args.header_output,
            args.probability_of_HI,
            args.minimum_period,
            args.maximum_period,
            args.task_amount,
            args.from_utilisation,
            args.to_utilisation,
            args.utilisation_step,
            args.sets_per_step,
        )
    else:
        print("unknown type")
