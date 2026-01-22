#include "RandomInstance.h"

RandomInstance::RandomInstance()
{
    RandomEng.seed(Seed);
}

RandomInstance::RandomInstance(const uint32_t seed)
{
    SetSeed(seed);
    RandomEng.seed(Seed);
}

unsigned RandomInstance::RandUInt(unsigned min, unsigned max)
{
    std::uniform_int_distribution<unsigned> dist(min, max);
    return dist(RandomEng);
}

int RandomInstance::RandInt32(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(RandomEng);
}

double RandomInstance::RandDouble(double min, double max)
{
    std::uniform_real_distribution<double> dist(min, max);
    return dist(RandomEng);
}

double RandomInstance::RandNormalDouble(double center, double disp)
{
    std::normal_distribution<double> dist(center, disp);
    return dist(RandomEng);
}

void RandomInstance::SetSeed(uint32_t seed)
{
    Seed = seed;
}
