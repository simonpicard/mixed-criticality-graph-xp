class EDFVD:
    def __init__(self, ts):
        self.ts = ts

    def test(self):
        res = self.ts.getUtilisationOfLevelAtLevel(0, 0)
        if self.ts.getUtilisationOfLevelAtLevel(1, 1) < 1:
            res += min(
                self.ts.getUtilisationOfLevelAtLevel(1, 1),
                self.ts.getUtilisationOfLevelAtLevel(1, 0) / (1 - self.ts.getUtilisationOfLevelAtLevel(1, 1)),
            )
        else:
            res += self.ts.getUtilisationOfLevelAtLevel(1, 1)
        return res <= 1
