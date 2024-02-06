from random import random, randrange

import Task
import TaskSet


class SetGenerator:
    def __init__(self, pHI, rHI, CmaxLO, Tmax, U, nbT=0):
        self.pHI = pHI
        self.rHI = rHI
        self.CmaxLO = CmaxLO
        self.Tmax = Tmax
        self.U = U
        self.nbT = nbT

    def generateTaskUniform(self):
        O = 0
        C = [randrange(1, self.CmaxLO + 1)] * 2

        if random() <= self.pHI:
            X = 1
            C[1] = randrange(C[0], min(int(round(self.rHI * C[0])), self.Tmax) + 1)
        else:
            X = 0

        T = randrange(C[X], self.Tmax + 1)
        D = T

        return Task.Task(O, T, D, X, C)

    def generateAnySetU(self):
        ts = TaskSet.TaskSet()

        done = False
        while not done:
            ts.clear()
            while len(ts) < self.nbT:
                ts.addTask(self.generateTaskUniform())
                if ts.getUtilisationOfLevel(0) > 1 or ts.getUtilisationOfLevel(1) > 1:
                    break
            else:
                if not ((ts.tasks["X"] == ts.tasks.loc[0, "X"]).all()):
                    done = True
        return ts
