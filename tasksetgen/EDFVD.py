def test(task_set):
    utilisation_of_LO_at_LO = task_set.getUtilisationOfLevelAtLevel(0, 0)
    utilisation_of_HI_at_LO = task_set.getUtilisationOfLevelAtLevel(1, 0)
    utilisation_of_HI_at_HI = task_set.getUtilisationOfLevelAtLevel(1, 1)

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
