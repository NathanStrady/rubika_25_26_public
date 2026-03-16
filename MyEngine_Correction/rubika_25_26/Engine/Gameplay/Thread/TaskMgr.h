#pragma once
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

class TaskMgr
{
public:
    const int workerCount = 4;
    const int syncCount = 4;
    
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
    void SyncLoop();

    void UpdateThreadUpdate();
    void DrawUpdateThread();

private:
    std::atomic<ePhase> CurrentPhase;

    std::queue<std::function<void()>> workerTaskQueue;
    std::queue<std::function<void()>> updateTaskQueue;
    std::queue<std::function<void()>> drawTaskQueue;
    
    std::vector<std::thread> workerThreads;
    std::vector<std::thread> syncThreads;
    
    std::condition_variable cv;
    std::condition_variable syncCv;
    std::condition_variable endSyncCv;
    
    std::atomic<int> workerActiveTasks = 0;
    std::atomic<int> syncActiveTasks = 0;
    
    std::mutex queueMutex;
    std::mutex notifyQueueMutex;
    std::mutex notifyEndTask;

    
};
