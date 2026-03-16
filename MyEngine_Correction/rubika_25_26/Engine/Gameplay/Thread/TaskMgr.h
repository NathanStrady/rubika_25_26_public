#pragma once
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

class TaskMgr
{
public:
    const int workerCount = 4;
    
    void Init();
    void Shut();

    enum class ePhase
    {
        None,
        Worker,
        Update,
        Draw,
    };

    void RegisterTask(std::function<void()> task, ePhase phase);
    void StartPhase(ePhase phase);
    void WaitPhase();

    void WorkerLoop();
    void SyncWorkerLoop();

    void WorkerThreadUpdate();
    void UpdateThreadUpdate();
    void DrawUpdateThread();

private:
    ePhase CurrentPhase;

    std::queue<std::function<void()>> workerTaskQueue;
    std::queue<std::function<void()>> updateTaskQueue;
    std::queue<std::function<void()>> drawTaskQueue;
    
    std::vector<std::thread> workerThreads;
    std::condition_variable cv;
    
    std::atomic<int> workerActiveTasks = 0;
    std::atomic<int> updateActiveTasks = 0;
    std::atomic<int> drawActiveTasks = 0;
    
    std::mutex workerQueueMutex;
    std::mutex notifyWorkerQueueMutex;

    
};
