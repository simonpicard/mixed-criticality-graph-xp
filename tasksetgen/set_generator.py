from random import randint, random, randrange, uniform

from drs import drs

from Task import Task
from TaskSet import TaskSet

U_MIN = 0
U_MAX = 1


def generate_task_uniform(probability_of_HI, wcet_HI_ratio, max_wcet_LO, max_period):
    offset = 0
    wcet = [randrange(1, max_wcet_LO + 1)] * 2

    if random() <= probability_of_HI:
        criticality_level = 1
        wcet[1] = randrange(wcet[0], min(int(round(wcet_HI_ratio * wcet[0])), max_period) + 1)
    else:
        criticality_level = 0

    period = randrange(wcet[criticality_level], max_period + 1)
    deadline = period  # implicit deadline

    return Task(offset, period, deadline, criticality_level, wcet)


def generate_random_task_set(n_tasks, probability_of_HI, wcet_HI_ratio, max_wcet_LO, max_period):
    ts = TaskSet()

    done = False
    while not done:
        ts.clear()
        while len(ts) < n_tasks:
            ts.add_task(generate_task_uniform(probability_of_HI, wcet_HI_ratio, max_wcet_LO, max_period))
            if ts.get_utilisation_of_level(0) > 1 or ts.get_utilisation_of_level(1) > 1:
                break
        else:
            if not ((ts.tasks["X"] == ts.tasks.loc[0, "X"]).all()):
                done = True
    return ts


def generate_task_set_with_utilisation(
    n_tasks, target_average_utilisation, max_period, probability_of_HI, tolerance=0.005, verbose=False
):
    assert n_tasks > 1, "n_tasks must be at least 2"
    assert target_average_utilisation >= U_MIN, f"u_target must be greater than {U_MIN}"
    assert target_average_utilisation <= U_MAX, f"u_target must be less than {U_MAX}"

    while True:
        # draw number of HI
        n_HI = randint(1, n_tasks - 1)
        # infer number of LO
        n_LO = n_tasks - n_HI

        # select HI tasks
        tasks_HI = [i for i in range(n_tasks) if random() <= probability_of_HI]
        if len(tasks_HI) == 0:
            if verbose:
                print("no HI tasks")
            continue
        if len(tasks_HI) == n_tasks:
            if verbose:
                print("all HI tasks")
            continue

        # compute possible range for utilisation in LO and HI based on target average utilisation
        range_to_u_max = U_MAX - target_average_utilisation
        range_to_u_min = target_average_utilisation
        random_range = min(range_to_u_max, range_to_u_min)

        # draw utilisation of HI from possible range
        u_HI_upper_bound = min(U_MAX, target_average_utilisation + random_range)
        u_HI_lower_bound = max(U_MIN, target_average_utilisation - random_range)
        u_HI = uniform(u_HI_lower_bound, u_HI_upper_bound)

        # infer utilisation of LO to match target average utilisation
        u_LO = 2 * target_average_utilisation - u_HI

        u_avg = (u_HI + u_LO) / 2  # should always be == u_target
        if u_avg != target_average_utilisation:
            if verbose:
                print(f"u_avg != u_target: {u_avg} != {target_average_utilisation}")
            continue

        # draw periods before hand
        periods = [randrange(1, max_period + 1) for i in range(n_tasks)]

        # tasks must have a min wcet of 1, this inferieng what is the min utilisation based on the period
        u_min_in_LO = [1 / p for p in periods]

        # for DRS, lower bounds must sum to less than max utilisation
        if sum(u_min_in_LO) > u_LO:
            if verbose:
                print(f"sum(u_min_in_LO) > u_LO: {sum(u_min_in_LO)} > {u_LO}")
            continue

        # using DRS to draw task level utilisation in LO
        try:
            u_LO_tasks = drs(n_tasks, u_LO, [U_MAX] * n_tasks, u_min_in_LO)
        except ZeroDivisionError:
            if verbose:
                print("ZeroDivisionError in DRS utilisation in LO")
            continue

        # defining lower and upper bounds for all tasks in HI
        u_min_in_HI = []
        u_max_in_HI = []
        for i in range(n_tasks):
            if i in tasks_HI:
                # minimum utilisation for HI tasks in HI is their utilisation in LO
                u_min_in_HI.append(u_LO_tasks[i])
                u_max_in_HI.append(U_MAX)
            else:
                # if task is LO, utilisation in HI must be 0
                # thus set DRS range to [0,0] hence only option is 0
                u_min_in_HI.append(0)
                u_max_in_HI.append(0)

        # for DRS, lower bounds must sum to less than max utilisation
        if sum(u_min_in_HI) > u_HI:
            if verbose:
                print(f"sum(u_min_in_HI) > u_HI: {sum(u_min_in_HI)} > {u_HI}")
            continue

        # using DRS to draw task level utilisation of HI tasks in HI
        try:
            u_HI_tasks = drs(n_tasks, u_HI, u_max_in_HI, u_min_in_HI)
        except ZeroDivisionError:
            if verbose:
                print("ZeroDivisionError in DRS utilisation in HI")
            continue

        task_set = TaskSet()

        for period, u_LO, u_HI in zip(periods, u_LO_tasks, u_HI_tasks):
            wcet_LO = max(1, round(period * u_LO))
            wcet = [wcet_LO] * 2
            criticality_level = 0
            if i in tasks_HI:
                wcet[1] = round(period * u_HI)
                criticality_level = 1

            offset = 0
            deadline = period  # implicit deadline
            task = Task(offset, period, deadline, criticality_level, wcet)

            task_set.add_task(task)

        u_LO_generated = task_set.get_utilisation_of_level(0)
        u_HI_generated = task_set.get_utilisation_of_level(1)
        generated_average_utilisation = (u_LO_generated + u_HI_generated) / 2

        delta_u_avg = generated_average_utilisation - target_average_utilisation

        # Utilisation cannot be above 1
        if u_LO_generated > 1:
            if verbose:
                print(f"u_LO_generated>1: {u_LO_generated}")
            continue
        if u_HI_generated > 1:
            if verbose:
                print(f"u_HI_generated>1: {u_HI_generated}")
            continue
        # Tolerance check
        if abs(delta_u_avg) > tolerance:
            if verbose:
                print(f"abs(delta_u_avg) > tolerance: {abs(delta_u_avg)} > {tolerance}")
            continue

        recap_str = f"""
        Generating a set of {n_tasks} with target actual utilisation of {target_average_utilisation}.

        Randomly drawn number of HI tasks = {n_HI}
        Hence, number of LO tasks = {n_LO}

        HI tasks have ids: {tasks_HI}

        Utilisation of LO tasks = {u_LO}
        Utilisation of HI tasks = {u_HI}
        -> Average utilisation = {u_avg}

        Loads of tasks in mode LO = {u_LO_tasks}
        Loads of tasks in mode HI = {u_HI_tasks}
        """
        if verbose:
            print(recap_str)
            print(task_set)
            print(f"Utilisation in LO = {u_LO_generated:.4f}, delta to target = {u_LO_generated - u_LO:.4f}")
            print(f"Utilisation in HI = {u_HI_generated:.4f}, delta to target = {u_HI_generated - u_HI:.4f}")
            print(
                f"Average utilisation = {generated_average_utilisation:.4f}, delta to target = {generated_average_utilisation - u_avg:.4f}"
            )

        return task_set
