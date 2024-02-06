import pandas as pd


class Task:
    def __init__(self, O, T, D, X, C):
        self.task = pd.Series([O, T, D, X] + list(C), ["O", "T", "D", "X"] + list(range(len(C))))
        U = pd.Series(
            (self.task[range(len(C))] / self.task["T"]).values,
            index=["U" + str(i) for i in range(len(C))],
        )
        if X == 0:
            U["U1"] = 0

        self.task = pd.concat([self.task, U])
