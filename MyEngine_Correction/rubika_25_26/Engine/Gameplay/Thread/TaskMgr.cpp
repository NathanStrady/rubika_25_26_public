#include "TaskMgr.h"

#include <cassert>

#include "Engine/Globals.h"
#include "Engine/Debug/DebugMgr.h"

void TaskMgr::Init()
{
    int i = 0, j = 0;
    workerThreads.reserve(workerCount);
    while (i < workerCount){
        workerThreads.emplace_back(&TaskMgr::WorkerLoop, this);
        ++i;
    }
    
    syncThreads.reserve(syncCount);
    while (j < syncCount)
    {
        syncThreads.emplace_back(&TaskMgr::SyncLoop, this);
        ++j;
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
    switch (phase)
    {
        case ePhase::Worker:
            {
                std::unique_lock<std::mutex> queueLock(queueMutex);
                workerTaskQueue.emplace(task);
            }
            ++workerActiveTasks;
            cv.notify_one();
            break;
        case ePhase::Update:
            {
                std::unique_lock<std::mutex> queueLock(queueMutex);
                updateTaskQueue.emplace(task);
            }
            ++syncActiveTasks;
            syncCv.notify_one();
            break;
        case ePhase::Draw:
            {
                std::unique_lock<std::mutex> queueLock(queueMutex);
                drawTaskQueue.emplace(task);
            }
            ++syncActiveTasks;
            syncCv.notify_one();
            break;
        default: 
            assert(false);
    }
}

void TaskMgr::StartPhase(ePhase phase)
{
    CurrentPhase = phase;
    syncCv.notify_all();
}

void TaskMgr::WaitPhase()
{
    std::unique_lock<std::mutex> endTask(queueMutex);
    endSyncCv.wait(endTask, [this](){ return syncActiveTasks == 0; });
}

void TaskMgr::WorkerLoop()
{
    while (true)
    {
        std::unique_lock<std::mutex> notifyWorker(notifyQueueMutex);
        cv.wait(notifyWorker, [this]()
        {
            return gData.ExipApp || workerActiveTasks > 0;
        });

        if (gData.ExipApp)
        {
            return;
        }

        std::function<void()> task;
        {
            std::unique_lock<std::mutex> queueLock(queueMutex);
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

void TaskMgr::SyncLoop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> notifyWorker(notifyQueueMutex);
            syncCv.wait(notifyWorker, [this]() {
                return syncActiveTasks > 0 || gData.ExipApp;
            });
        }

        if (gData.ExipApp) return;

        switch (CurrentPhase)
        {
            case ePhase::Update: 
                UpdateThreadUpdate(); 
                break;
            case ePhase::Draw:   
                DrawUpdateThread(); 
                break;
            default: 
                break; 
        }
        
        if (syncActiveTasks == 0)
        {
            endSyncCv.notify_all();
        }
    }
}



void TaskMgr::UpdateThreadUpdate()
{
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> queueLock(queueMutex);
        if (!updateTaskQueue.empty())
        {
            task = updateTaskQueue.front();
            updateTaskQueue.pop();
        }
    }
    if (task)
    {
        task();
        --syncActiveTasks;
    }

}

void TaskMgr::DrawUpdateThread()
{
    
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> queueLock(queueMutex);
        if (!drawTaskQueue.empty())
        {
            task = drawTaskQueue.front();
            drawTaskQueue.pop();
        }
    }
    
    if (task)
    {
        task();
        --syncActiveTasks;
    }
}



