import argparse
from datetime import datetime

import pandas as pd

import EDFVD
import SetGenerator


def get_ts_str_cpp(ts):
    res = f"{len(ts)}\n"
    for e in ts:
        res += f"{int(e['T'])} {int(e['D'])} {int(e['X'])+1}\n"
        res += f"{int(e[0])} {int(e[1])}\n"
    return res


def generate_for_complex(task_sets_output, header_output):
    pHI = 0.5  # probability HI task
    rHI = 2.5  # if task is HI, then C_HI in [C_LO, rHI * C_LO]
    Tmax = 12
    nbTs = [2, 3, 4]

    df_c = pd.DataFrame()

    ts_id = 0

    ts_str_cpp = ""

    n_ts_per_nbt = 100

    for nb_t in nbTs:
        created_ts = set()
        i = 0
        while i < n_ts_per_nbt:
            print(nb_t, i)
            sg = SetGenerator.SetGenerator(pHI, rHI, -1, Tmax, 0, nb_t)
            ts = sg.generateSetPerformance(nb_t)
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
            else:
                print("duplicated")

    ts_str_cpp = f"{ts_id}\n" + ts_str_cpp
    df_c.to_csv(header_output, index=False)
    with open(task_sets_output, "w") as text_file:
        text_file.write(ts_str_cpp)


def read_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--type", "-t", help="type of set generation [statespace, performance]", type=str, required=True
    )
    parser.add_argument(
        "--task_sets_output",
        "-o",
        help="path to txt output with task sets description (.txt)",
        type=str,
        required=False,
        default=f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets.txt",
    )
    parser.add_argument(
        "--header_output",
        "-c",
        help="path to csv output with task sets header (.csv)",
        type=str,
        required=False,
        default=f"{datetime.now().strftime('%Y%m%d%H%M%S')}_task_sets.csv",
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = read_args()
    if args.type == "statespace":
        generate_for_complex(args.task_sets_output, args.header_output)
    else:
        print("unknown type")
