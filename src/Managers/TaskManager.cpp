#include "Managers/TaskManager.h"

void TaskManager::AddTask(Task task, std::chrono::milliseconds delay)
{
    auto now = std::chrono::steady_clock::now();
    tasks.emplace(TaskEntry{ now + delay, std::move(task) });
}

void TaskManager::Update()
{
    auto now = std::chrono::steady_clock::now();
    while (!tasks.empty() && tasks.top().endTime <= now) {
        auto taskEntry = tasks.top();
        tasks.pop();

        if (taskEntry.task) {
            taskEntry.task();
        }
    }
}
