class EDFVD:
    def __init__(self, ts):
        self.ts = ts

    def test(self):
        utilisation_of_LO_at_LO = self.ts.getUtilisationOfLevelAtLevel(0, 0)
        utilisation_of_HI_at_LO = self.ts.getUtilisationOfLevelAtLevel(1, 0)
        utilisation_of_HI_at_HI = self.ts.getUtilisationOfLevelAtLevel(1, 1)

        utilisation_at_LO = utilisation_of_LO_at_LO + utilisation_of_HI_at_LO
        utilisation_at_HI = utilisation_of_HI_at_HI

        if utilisation_at_LO > 1:
            return False
        if utilisation_at_HI > 1:
            return False

        utilisation_EDFVD = utilisation_of_LO_at_LO
        if utilisation_of_HI_at_HI == 1:
            utilisation_EDFVD += utilisation_of_HI_at_HI
        else:
            utilisation_EDFVD += min(
                utilisation_of_HI_at_HI,
                utilisation_of_HI_at_LO / (1 - utilisation_of_HI_at_HI),
            )

        return utilisation_EDFVD <= 1
