#pragma once

class TaskManager
{
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Task      = std::function<void()>;

    struct TaskEntry
    {
        TimePoint endTime;
        Task      task;

        bool operator>(const TaskEntry& other) const
        {
            return endTime > other.endTime;
        }
    };

    static TaskManager& GetSingleton()
    {
        static TaskManager instance;
        return instance;
    }

    void AddTask(Task task, std::chrono::milliseconds delay);
    void Update();

private:
    TaskManager()  = default;
    ~TaskManager() = default;

    std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> tasks;
};
