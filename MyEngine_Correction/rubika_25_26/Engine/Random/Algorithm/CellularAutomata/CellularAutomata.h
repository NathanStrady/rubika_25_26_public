#pragma once
#include <SFML/Graphics/Texture.hpp>

#include "Engine/Random/Algorithm/Grid/CellularGrid.h"
#include "Engine/Random/Instances/RandomInstance.h"


class CellularAutomata
{
public:
    CellularAutomata();
    CellularAutomata(unsigned sizeX, unsigned sizeY, double spawnPercent, unsigned threshold, unsigned iteration);
    ~CellularAutomata();

    bool Generate(int randomId);
    void GenerateTexture(sf::Texture& texture);

    void GenerateAliveCells() const;
    void CellularWalk();

    bool BirthNewCell(int i, int j) const;

private:

    unsigned SizeX;
    unsigned SizeY;
    double SpawnPercent;
    unsigned NeightborThreshold;
    unsigned Iteration;
    RandomInstance* RandomInstance;
    CellularGrid* CurrentGrid;
    CellularGrid* NextGrid;
};

