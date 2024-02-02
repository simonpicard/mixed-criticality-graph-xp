import argparse
from datetime import datetime

import numpy as np
import pandas as pd
from tqdm import tqdm

import EDFVD
import SetGenerator


def get_ts_str_cpp(ts):
    res = f"{len(ts)}\n"
    for e in ts:
        res += f"{int(e['T'])} {int(e['D'])} {int(e['X'])+1}\n"
        res += f"{int(e[0])} {int(e[1])}\n"
    return res


def generate_per_n_tasks(
    task_sets_output=None,
    header_output=None,
    p_HI=0.5,
    r_HI=2.5,
    max_T=24,
    max_C_LO=20,
    task_amounts=[2, 3, 4, 5],
    set_per_amount=200,
):
    if not task_sets_output:
        task_sets_output = f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_statespace.txt"
    if not header_output:
        header_output = f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_statespace.csv"

    df_c = pd.DataFrame()

    ts_id = 0

    ts_str_cpp = ""

    with tqdm(total=len(task_amounts) * set_per_amount) as pbar:
        for nb_t in task_amounts:
            created_ts = set()
            i = 0
            while i < set_per_amount:
                sg = SetGenerator.SetGenerator(p_HI, r_HI, max_C_LO, max_T, 0, nb_t)
                ts = sg.generateAnySetU()
                flat_ts = tuple([item for sublist in ts.tasks.values for item in sublist])
                if flat_ts not in created_ts:
                    created_ts.add(flat_ts)
                    ts_str_cpp += get_ts_str_cpp(ts)

                    res = pd.Series(dtype=float)
                    res["ts_id"] = ts_id
                    res["U"] = ts.getAverageUtilisation()
                    res["nbt"] = len(ts)
                    res["EDFVD_test"] = int(EDFVD.EDFVD(ts).test())

                    df_c = pd.concat([df_c, res.to_frame().T], ignore_index=True)

                    ts_id += 1
                    i += 1

                    pbar.update(1)

    ts_str_cpp = f"{ts_id}\n" + ts_str_cpp
    df_c.to_csv(header_output, index=False)
    with open(task_sets_output, "w") as text_file:
        text_file.write(ts_str_cpp)


def generate_per_utilisation(
    task_sets_output=None,
    header_output=None,
    p_HI=0.5,
    r_HI=2,
    max_T=50,
    task_amount=3,
    max_C_LO=20,
    from_u=0.89,
    to_u=0.99,
    step=0.01,
    sets_per_step=100,
):
    if not task_sets_output:
        task_sets_output = f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_scheduling.txt"
    if not header_output:
        header_output = f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets_scheduling.csv"

    ts_id = 0

    df_sc = pd.DataFrame()
    ts_str_cpp = ""

    generated_tasks = {}

    utilisations = [from_u + i * step for i in range(int((to_u - from_u) / step) + 1)]
    utilisations = list(map(lambda x: round(x, 2), utilisations))
    utilisations_np = np.array(utilisations)

    total_t = len(utilisations) * sets_per_step

    generated_tasks = {u: 0 for u in utilisations}
    pbars = {u: tqdm(total=sets_per_step, desc=f"TS:{u*100:.0f}%") for u in utilisations}

    sg = SetGenerator.SetGenerator(p_HI, r_HI, max_C_LO, max_T, 0, task_amount)
    current_t = 0
    while current_t < total_t:
        ts = sg.generateAnySetU()
        ts_u = ts.getAverageUtilisation()
        difference_array = np.absolute(utilisations_np - ts_u)
        index = difference_array.argmin()
        target_u = utilisations[index]
        u_error = difference_array[index]

        if ts_u >= from_u and ts_u <= to_u and u_error < 0.001:
            if generated_tasks[target_u] < sets_per_step:
                generated_tasks[target_u] += 1
                pbars[target_u].update(1)
                current_t += 1

                ts_str_cpp += get_ts_str_cpp(ts)

                res = pd.Series(dtype=float)
                res["ts_id"] = ts_id
                res["U"] = target_u
                res["Uv"] = ts_u
                res["nbt"] = len(ts)
                res["EDFVD_test"] = int(EDFVD.EDFVD(ts).test())

                df_sc = pd.concat([df_sc, res.to_frame().T], ignore_index=True)
                ts_id += 1

    for pct in pbars:
        pbars[pct].close()

    ts_str_cpp = f"{ts_id}\n" + ts_str_cpp

    df_sc.to_csv(header_output, index=False)
    with open(task_sets_output, "w") as text_file:
        text_file.write(ts_str_cpp)


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
        "--hi_probability",
        "-phi",
        help="probability of tasks being high-criticality",
        type=float,
        required=False,
        default=0.5,
    )
    parser.add_argument(
        "--hi_ratio",
        "-rhi",
        help="the maximum ratio between high- and low-criticality execution time",
        type=float,
        required=False,
        default=2.5,
    )
    parser.add_argument(
        "-tas",
        "--task_amounts",
        nargs="+",
        help="list of the number of tasks per task set",
        type=int,
        required=False,
        default=[2, 3, 4],
    )
    parser.add_argument(
        "-s",
        "--set_per_amount",
        help="amout of task sets to generate for each number of tasks",
        required=False,
        type=int,
        default=100,
    )
    parser.add_argument(
        "-max_t",
        "--maximum_period",
        help="maximum period",
        required=False,
        type=int,
        default=20,
    )
    parser.add_argument(
        "-ta",
        "--task_amount",
        help="number of tasks per task set",
        type=int,
        required=False,
        default=3,
    )
    parser.add_argument(
        "-max_c_lo",
        "--maximum_wcet_lo",
        help="the maximum WCET for low-criticality execution time",
        type=float,
        required=False,
        default=20,
    )
    parser.add_argument(
        "-u",
        "--from_u",
        help="from utilization",
        type=float,
        required=False,
        default=0.89,
    )
    parser.add_argument(
        "-U",
        "--to_u",
        help="to utilization",
        type=float,
        required=False,
        default=0.99,
    )
    parser.add_argument(
        "-us",
        "--utilization_step",
        help="step",
        type=float,
        required=False,
        default=0.01,
    )
    parser.add_argument(
        "-ss",
        "--sets_per_step",
        help="sets per step",
        type=int,
        required=False,
        default=100,
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = read_args()
    if args.type == "n_tasks":
        generate_per_n_tasks(
            args.task_sets_output,
            args.header_output,
            args.hi_probability,
            args.hi_ratio,
            args.maximum_period,
            args.maximum_wcet_lo,
            args.task_amounts,
            args.set_per_amount,
        )
    elif args.type == "utilisation":
        generate_per_utilisation(
            args.task_sets_output,
            args.header_output,
            args.hi_probability,
            args.hi_ratio,
            args.maximum_period,
            args.task_amount,
            args.maximum_wcet_lo,
            args.from_u,
            args.to_u,
            args.utilization_step,
            args.sets_per_step,
        )
    else:
        print("unknown type")
