#include "TaskMgr.h"

#include <cassert>
#include <iostream>

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
    
    syncCv.notify_all();
    for (auto& thread : syncThreads)
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
            ++updateActiveTasks;
            syncCv.notify_one();
            break;
        case ePhase::Draw:
            {
                std::unique_lock<std::mutex> queueLock(queueMutex);
                drawTaskQueue.emplace(task);
            }
            ++drawActiveTasks;
            syncCv.notify_one();
            break;
        default:
            break;
    }
}

void TaskMgr::StartPhase(ePhase phase)
{
    CurrentPhase = phase;
    syncCv.notify_all();
}

void TaskMgr::WaitPhase()
{
    std::unique_lock<std::mutex> endTask(notifySyncEnd);
    endSyncCv.wait(endTask, [this]()
    {
        return (CurrentPhase == ePhase::Update && updateActiveTasks == 0) || (CurrentPhase == ePhase::Draw && drawActiveTasks == 0);
    });
}

void TaskMgr::WorkerLoop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> notifyWorker(notifyWorkerRegister);
            cv.wait(notifyWorker, [this]()
            {
                return gData.ExipApp || workerActiveTasks > 0;
            });
        }

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
        
        if (task)
        {
            task();
            --workerActiveTasks;
        }
    }
}

void TaskMgr::SyncLoop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> notifySync(notifySyncRegister);
            syncCv.wait(notifySync, [this]() {
                return ((updateActiveTasks > 0 && CurrentPhase == ePhase::Update || drawActiveTasks > 0 && CurrentPhase == ePhase::Draw) && CurrentPhase != ePhase::None || CurrentPhase != ePhase::Worker) || gData.ExipApp;
            });
        }
        if (gData.ExipApp) return;
        
        std::function<void()> task;
        switch (CurrentPhase)
        {
            case ePhase::Update:
                {
                    std::unique_lock<std::mutex> queueLock(queueMutex);
                    if (!updateTaskQueue.empty())
                    {
                        task = updateTaskQueue.front();
                        updateTaskQueue.pop();
                    }
                }
                break;
            case ePhase::Draw:   
                {
                    std::unique_lock<std::mutex> queueLock(queueMutex);
                    if (!drawTaskQueue.empty())
                    {
                        task = drawTaskQueue.front();
                        drawTaskQueue.pop();
                    }
                }
                break;
            default: 
                break; 
        }
        
        if (task)
        {
            task();
            
            if (CurrentPhase == ePhase::Draw)
                --drawActiveTasks;
            else
                --updateActiveTasks;
            
            endSyncCv.notify_all();
        }
    }
}



