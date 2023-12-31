import pandas as pd

import SetGenerator
from sufficientCond import EDFVD


def get_ts_str_cpp(ts):
    res = f"{len(ts)}\n"
    for e in ts:
        res += f"{int(e['T'])} {int(e['D'])} {int(e['X'])+1}\n"
        res += f"{int(e[0])} {int(e[1])}\n"
    return res


def generate_for_complex():
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
    df_c.to_csv("task_cplx_header_v2.csv", index=False)
    with open("task_cplx_input_v2.txt", "w") as text_file:
        text_file.write(ts_str_cpp)


if __name__ == "__main__":
    generate_for_complex()
