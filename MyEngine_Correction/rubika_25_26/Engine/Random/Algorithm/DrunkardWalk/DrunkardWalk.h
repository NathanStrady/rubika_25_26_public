#pragma once
#include <set>
#include <SFML/Graphics/Texture.hpp>

#include "Engine/Random/Algorithm/Grid/DrunkardGrid.h"
#include "Engine/Random/Algorithm/Grid/Grid.h"
#include "Engine/Random/Instances/RandomInstance.h"

enum DrunkardDirection : uint8_t
{
    North = 0,
    South = 1,
    East = 2,
    West = 3,
};

class DrunkardWalk 
{
public:
    DrunkardWalk();
    DrunkardWalk(int sizeX, int sizeY, int iteration, int distance, int spawnNumber);
    
    bool Generate(int randomId = 0);
    void GenerateTexture(sf::Texture& texture);

    bool DetermineDirection(sf::Vector2i& newIndex) const;
    std::vector<int> Walk() const;

private:
    int SizeX;
    int SizeY;
    int Iteration;
    int Distance;
    int SpawnNumber;
    RandomInstance* RandomInstance;
    DrunkardGrid* InternalGrid;
};
