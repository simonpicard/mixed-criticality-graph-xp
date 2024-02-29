import os
from pathlib import Path
from typing import List

import pandas as pd


def merge_datasets(dataset_directory: str, unsorted_filenames: List[str], prefix: str):
    list_filenames = [f for f in unsorted_filenames]
    filenames = []
    file_nb = 0
    while len(list_filenames) > 0:
        forged_filename = f"{prefix}_{file_nb}.csv"
        if forged_filename in list_filenames:
            list_filenames.remove(forged_filename)
            filenames.append(forged_filename)
        else:
            break
        file_nb += 1

    pdd = Path(dataset_directory)
    output_filename = pdd / f"{prefix}.csv"

    if output_filename.exists():
        return

    first_line = True
    all_lines = []
    for filename in filenames:
        with open(pdd / filename, "r") as f:
            lines = f.readlines()
        if first_line:
            all_lines.extend(lines)
            first_line = False
        else:
            all_lines.extend(lines[1:])
    with open(output_filename, "w") as output_file:
        output_file.writelines(all_lines)


def potentially_merge_datasets(dataset_directory: str | Path, csv_filenames: List[str]):
    for csv_filename in csv_filenames:
        key = "_0.csv"
        if csv_filename.endswith(key):
            csv_prefix = csv_filename.split(key)[0]
            merge_datasets(
                dataset_directory=dataset_directory,
                unsorted_filenames=[
                    f for f in csv_filenames if csv_filename.startswith(csv_prefix)
                ],
                prefix=csv_prefix,
            )


def prepare_df(
    default_header_path=None,
    default_exploration_path=None,
    xp_identifier=None,
    output_directory="../outputs/",
):
    header_path = default_header_path or os.getenv("MCS_HEADER_PATH")
    exploration_path = default_exploration_path or os.getenv("MCS_SIMULATION_PATH")

    if not (header_path or exploration_path):
        dataset_directory = Path(output_directory)

        csv_filenames = sorted(
            name for name in os.listdir(dataset_directory) if name.endswith(".csv")
        )

        assert len(csv_filenames) >= 2, f"No dataset found in {dataset_directory}"

        potentially_merge_datasets(dataset_directory, csv_filenames)

        # reload (TODO function to avoid duplicate)
        csv_filenames = sorted(
            name for name in os.listdir(dataset_directory) if name.endswith(".csv")
        )

        # use the latest dataset produced
        header = [
            f for f in csv_filenames if f.endswith(f"_{xp_identifier}_header.csv")
        ][-1]
        explo = [f for f in csv_filenames if f.endswith(f"_{xp_identifier}_explo.csv")][
            -1
        ]

        print(f"CSV files used by the notebook: \n- header: {header}\n- explo: {explo}")

        header_path = (dataset_directory / header).resolve()
        exploration_path = (dataset_directory / explo).resolve()

    assert os.path.exists(header_path), f"Header path {header_path} does not exist"
    assert os.path.exists(
        exploration_path
    ), f"Simulation path {exploration_path} does not exist"

    df_header = pd.read_csv(header_path)
    df_explo = pd.read_csv(exploration_path)

    df_explo["schedulable"] = df_explo["schedulable"].astype(bool)
    df_explo["duration_s"] = df_explo["duration"] / 10**9

    df = df_explo.merge(df_header, left_on="tid", right_on="ts_id")

    df["n_task"] = df["nbt"]

    same_exploration_result = df.groupby(["tid", "scheduler"])["schedulable"].apply(
        lambda x: (x.iloc[0] == x).all()
    )

    if not same_exploration_result.all():
        print(
            "WARNING: Found at least one task set with different schedulability output for a given scheduler"
        )

    # some renaming
    df["Number of tasks"] = df["nbt"]
    df["Schedulable"] = df["schedulable"]

    df["oracle"] = df["safe"].combine_first(df["unsafe"])
    df["oracle"] = df["oracle"].fillna("none")

    total_min = df["duration_s"].sum() / 60
    total_hours = total_min / 60

    print(
        f"The total time taken by the simulations is {total_min:.2f} min, i.e {total_hours:.2f} hours."
    )

    n_schedulable_df = df.groupby(["tid", "scheduler"], as_index=False)[
        "schedulable"
    ].first()

    schedulers = df["scheduler"].unique()
    for scheduler in schedulers:
        s = n_schedulable_df["scheduler"] == scheduler
        n_schedulable = n_schedulable_df[s]["schedulable"].value_counts()
        print(
            f"For scheduler: {scheduler}, {n_schedulable[True]} tasks are schedulable and {n_schedulable[False]} tasks are not schedulable."
        )

    return df


if __name__ == "__main__":
    df = prepare_df(xp_identifier="oracles")
    print(df.head)
