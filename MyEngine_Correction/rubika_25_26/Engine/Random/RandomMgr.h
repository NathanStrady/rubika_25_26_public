#pragma once
#include <cstdint>
#include <map>
#include <utility>

#include "Algorithm/CellularAutomata/CellularAutomata.h"
#include "Algorithm/DrunkardWalk/DrunkardWalk.h"
#include "Algorithm/Perlin/Perlin.h"
#include "Engine/Debug/DebugMgr.h"

struct DebugValues
{
    unsigned u_linear;
    int i_linear;
    double d_linear;
    double d_normal;
};

class RandomInstance;

class RandomMgr : public IDebugable
{
public:
    using InstanceId = unsigned;
    bool d_init = true;
    int currentId = -1;
    


    void Init();
    void Shut();

    InstanceId CreateInstance();
    InstanceId CreateInstance(InstanceId instanceId);
    void DestroyInstance(InstanceId instanceId);
    bool GetInstance(InstanceId instanceId, RandomInstance*& random_instance) const;

    virtual void DrawDebug() override;

    static uint32_t GenerateRandomSeed();

private:
    const int MAX_RANDOM_INSTANCES = 10;
    std::map<InstanceId, RandomInstance*> instances;
    std::unique_ptr<DrunkardWalk> drunkardWalk;
    std::unique_ptr<CellularAutomata> cellularAutomata;
    std::unique_ptr<Perlin> perlinNoise;
    std::map<InstanceId, DebugValues> debug; 

};
