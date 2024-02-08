import pandas as pd


class TaskSet:
    def __init__(self, tasks=[]):
        self.tasks = pd.DataFrame([t.task for t in tasks])

    def add_task(self, task):
        self.tasks = pd.concat([self.tasks, task.task.to_frame().T], ignore_index=True)
        self.tasks = self.tasks.sort_values(
            by=self.tasks.columns.to_list(), ignore_index=True
        )  # to get consistent sets

    def pop(self):
        self.tasks = self.tasks.iloc[:-1]

    def clear(self):
        self.tasks = self.tasks.iloc[0:0]

    def get_df(self):
        return self.tasks

    def get_size(self):
        return len(self.tasks)

    def get_task(self, i):
        return self.tasks.iloc[i]

    def get_utilisation_of_level_at_level(self, K, l):
        if len(self.tasks) == 0:
            return 0
        scope = self.tasks["X"] == K
        return self.tasks.loc[scope, f"U{int(l)}"].sum()

    def get_utilisation_of_level(self, K):
        if len(self.tasks) == 0:
            return 0
        scope = self.tasks["X"] >= K
        return self.tasks.loc[scope, f"U{int(K)}"].sum()

    def get_average_utilisation(self):
        if len(self) == 0:
            return 0
        return sum([self.get_utilisation_of_level(i) for i in range(int(self.tasks["X"].max() + 1))]) / (
            self.tasks["X"].max() + 1
        )

    def __repr__(self):
        return str(self.tasks)

    def __getitem__(self, index):
        return self.get_task(index)

    def __hash__(self):
        return hash(self.tasks)

    def copy(self):
        return TaskSet(self.tasks)

    def __eq__(self, other):
        return self.tasks == other.tasks

    def __len__(self):
        return self.get_size()

    def get_hash(self):
        hash_cols = ["T", "D", "X", 0, 1]
        data = tuple(self.tasks[hash_cols].values.flatten())
        return hash(data)
