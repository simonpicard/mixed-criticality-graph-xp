#include "simulators/EDFSimulator.h"
#include <iostream>

namespace rtsimulator {

EDFSimulator::EDFSimulator() = default;

void EDFSimulator::addTask(int execution_time, int relative_deadline, int period) {
    Task task = {
        .C = execution_time,
        .D = relative_deadline,
        .T = period,
    };
    tasks.push_back(task);
}

void EDFSimulator::addAperiodicJob(int execution_time, int absolute_deadline) {
    Job job = {
        .taskId = -1,  // aperiodic jobs are not tight to any task
        .remainingTime = execution_time,
        .deadline = absolute_deadline,
    };
    aperiodicJobs.push_back(job);
}

void EDFSimulator::initializeTaskReleases(int time_bound) {
    for (int i = 0; i < tasks.size(); ++i) {
        for (int release_time = 0; release_time <= time_bound; release_time += tasks[i].T) {
            taskReleases[release_time].push_back(i);
        }
    }
}

bool EDFSimulator::simulate(int time_bound) {
    initializeTaskReleases(time_bound);

    // Add aperiodic jobs to the queue
    for (const auto& job : aperiodicJobs) {
        jobQueue.push(job);
    }

    for (int current_time = 0; current_time <= time_bound; ++current_time) {
        if (taskReleases.contains(current_time)) {
            for (int task_index : taskReleases[current_time]) {
                const Task& task = tasks[task_index];
                Job new_job = {
                    .taskId = task_index,
                    .remainingTime = task.C,
                    .deadline = current_time + task.D,
                };
                jobQueue.push(new_job);
            }
        }

        if (!jobQueue.empty()) {
            Job current_job = jobQueue.top();
            jobQueue.pop();

            // Check for deadline miss
            if (current_time >= current_job.deadline) {
                std::cout << "Deadline miss detected for job from Task Index: " << current_job.taskId << " at time: " << current_time << std::endl;
                return false; // Indicate scheduling failure due to deadline miss
            }

            current_job.remainingTime--; // Simulate job execution for one time unit

            if (current_job.remainingTime > 0) {
                jobQueue.push(current_job); // Re-queue the job if it's not completed
            }
        }
    }

    return true;
}

}
