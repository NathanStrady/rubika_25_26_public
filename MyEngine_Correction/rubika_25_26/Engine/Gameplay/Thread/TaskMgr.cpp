#include "TaskMgr.h"

#include "Engine/Globals.h"
#include "Engine/Debug/DebugMgr.h"

void TaskMgr::Init()
{
    int i = 0;
    workerThreads.reserve(workerCount);
    while (i < workerCount){
        workerThreads.emplace_back(&TaskMgr::SyncWorkerLoop, this);
        ++i;
    }
}

void TaskMgr::Shut()
{
    cv.notify_all();
    for (auto& thread : workerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void TaskMgr::RegisterTask(std::function<void()> task, ePhase phase)
{
    {
        std::unique_lock<std::mutex> queueLock(workerQueueMutex);
        workerTaskQueue.emplace(task);
    }
    ++workerActiveTasks;
    cv.notify_one();
    StartPhase(phase);
}

void TaskMgr::StartPhase(ePhase phase)
{
    CurrentPhase = phase;
    cv.notify_all();
}

void TaskMgr::WaitPhase()
{
    std::unique_lock<std::mutex> notifySyncWorker(notifyWorkerQueueMutex);
    cv.wait(notifySyncWorker, [this]()
    {
        return gData.ExipApp || workerActiveTasks > 0 || drawActiveTasks > 0 || updateActiveTasks > 0;
    });
}

void TaskMgr::WorkerLoop()
{
    while (true)
    {
        std::unique_lock<std::mutex> notifySyncWorker(notifyWorkerQueueMutex);
        cv.wait(notifySyncWorker, [this]()
        {
            return gData.ExipApp || workerActiveTasks > 0;
        });

        if (gData.ExipApp)
        {
            return;
        }

        std::function<void()> task;
        {
            std::unique_lock<std::mutex> queueLock(workerQueueMutex);
            if (!workerTaskQueue.empty())
            {
                task = workerTaskQueue.front();
                workerTaskQueue.pop();
            }
        }
        --workerActiveTasks;
        task();
    }
}

void TaskMgr::SyncWorkerLoop()
{
    while (true)
    {
        switch (CurrentPhase)
        {
        case ePhase::Worker:
            WorkerThreadUpdate();
            break;

        case ePhase::Update:
            UpdateThreadUpdate();
            break;

        case ePhase::Draw:
            DrawUpdateThread();
            break;

        case ePhase::None:
            break;
            
        default:
            StartPhase(ePhase::Worker);
            break;
        }
    }
}

void TaskMgr::WorkerThreadUpdate()
{
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> queueLock(workerQueueMutex);
        if (!workerTaskQueue.empty())
        {
            task = workerTaskQueue.front();
            workerTaskQueue.pop();
        }
    }
    --workerActiveTasks;
    task();
}

void TaskMgr::UpdateThreadUpdate()
{
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> queueLock(workerQueueMutex);
        if (!updateTaskQueue.empty())
        {
            task = updateTaskQueue.front();
            updateTaskQueue.pop();
        }
    }
    --workerActiveTasks;
    task();
}

void TaskMgr::DrawUpdateThread()
{
    
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> queueLock(workerQueueMutex);
        if (!drawTaskQueue.empty())
        {
            task = drawTaskQueue.front();
            drawTaskQueue.pop();
        }
    }
    --workerActiveTasks;
    task();
}



