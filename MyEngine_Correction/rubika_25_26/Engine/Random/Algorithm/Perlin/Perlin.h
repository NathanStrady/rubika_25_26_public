#pragma once
#include <SFML/Graphics/Texture.hpp>

#include "Engine/Random/Algorithm/Grid/Grid.h"
#include "Engine/Random/Algorithm/Grid/PerlinGrid.h"
#include "Engine/Random/Instances/RandomInstance.h"

class Perlin
{
public:
    Perlin();
    Perlin(unsigned sizeX, unsigned sizeY, unsigned cellSize, unsigned octaves);
    ~Perlin();

    bool Generate(int randomId);
    void GenerateTexture(sf::Texture& texture) const;

private:
    unsigned SizeX;
    unsigned SizeY;
    unsigned CellSize;
    unsigned Octaves;
    RandomInstance* RandomInstance;
    PerlinGrid* Grid;
};
