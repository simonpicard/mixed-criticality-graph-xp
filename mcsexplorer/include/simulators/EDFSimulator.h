#ifndef EDFSIMULATOR_H
#define EDFSIMULATOR_H

#include <vector>
#include <queue>
#include <map>

namespace rtsimulator {

class EDFSimulator {
   public:
    struct Task {
        int C;  // Execution time
        int D;  // Relative deadline
        int T;  // Period
    };

    struct Job {
        int taskId;         // Originating Task Index (position in tasks vector)
        int remainingTime;  // Remaining execution time
        int deadline;       // Absolute deadline (from task start time)
    };

    struct JobComparator {
        bool operator()(const Job& a, const Job& b) const {
            if (a.deadline == b.deadline)
                return a.taskId > b.taskId;
            return a.deadline > b.deadline;
        }
    };

   private:
    std::vector<Task> tasks;
    std::vector<Job> aperiodicJobs;
    std::map<int, std::vector<int>> taskReleases;

    using JobQueue = std::priority_queue<Job, std::vector<Job>, JobComparator>;

    JobQueue jobQueue;

   public:
    explicit EDFSimulator();

    void addTask(int C, int D, int T);
    void addAperiodicJob(int execution_time, int absolute_deadline);
    bool simulate(int time_bound);

   private:
    void initializeTaskReleases(int time_bound);
};

}

#endif /* EDFSIMULATOR_H */
