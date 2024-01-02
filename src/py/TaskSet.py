import pandas as pd


class TaskSet:
    def __init__(self, tasks=[]):
        self.tasks = pd.DataFrame([t.task for t in tasks])

    def addTask(self, task):
        # self.tasks = self.tasks.append(task.task, ignore_index=True)
        self.tasks = pd.concat([self.tasks, task.task.to_frame().T], ignore_index=True)

    def pop(self):
        self.tasks = self.tasks.iloc[:-1]

    def clear(self):
        self.tasks = self.tasks.iloc[0:0]

    def get_df(self):
        return self.tasks

    def getSize(self):
        return len(self.tasks)

    def getTask(self, i):
        return self.tasks.iloc[i]

    def getUtilisationOfLevelAtLevel(self, K, l):
        if len(self.tasks) == 0:
            return 0
        scope = self.tasks["X"] == K
        return self.tasks.loc[scope, f"U{int(l)}"].sum()

    def getUtilisationOfLevel(self, K):
        if len(self.tasks) == 0:
            return 0
        scope = self.tasks["X"] >= K
        return self.tasks.loc[scope, f"U{int(K)}"].sum()

    def getAverageUtilisation(self):
        if len(self) == 0:
            return 0
        return sum([self.getUtilisationOfLevel(i) for i in range(int(self.tasks["X"].max() + 1))]) / (
            self.tasks["X"].max() + 1
        )

    def __repr__(self):
        return str(self.tasks)

    def __getitem__(self, index):
        return self.getTask(index)

    def __hash__(self):
        return hash(self.tasks)

    def copy(self):
        return TaskSet(self.tasks)

    def __eq__(self, other):
        return self.tasks == other.tasks

    def __len__(self):
        return self.getSize()

    def getWorstC(self, i):
        t = self.getTask(i)
        return t[1]

    def getD(self):
        return self.tasks["D"]

    def getT(self):
        return self.tasks["T"]

    def getK(self):
        return int(self.tasks["X"].max())
