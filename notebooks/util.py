import os

import pandas as pd


def prepare_df(default_header_path=None, default_exploration_path=None):
    header_path = os.getenv("MCS_HEADER_PATH", default_header_path)
    exploration_path = os.getenv("MCS_SIMULATION_PATH", default_exploration_path)

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
        print("WARNING: Found at least one task set with different schedulability output for a given scheduler")

    return df
