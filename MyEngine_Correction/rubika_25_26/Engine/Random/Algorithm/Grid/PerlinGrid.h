#pragma once
#include "Grid.h"
#include "Engine/Random/Instances/RandomInstance.h"

class PerlinGrid
{
public:
    int CellSize;
    int GridSizeX;
    int GridSizeY;
    
    
    PerlinGrid(int SizeX, int SizeY, int CellSize);
    ~PerlinGrid() = default;

    bool Generate(RandomInstance& random);
    bool GenerateNormalizedVector(RandomInstance& random);
    
    int GetPerlinCoords(int i);
    float DotGridGradient(unsigned ix, unsigned iy, float& x, float& y);
    
    float GetPerlinValue(float x, float y);

    // Interp algorithm
    float SmoothStep(float w);
    float Interpolate(float a0, float a1, float w);
    
    Grid<sf::Vector2f> normalizedVectorGrid;
    Grid<float> perlinDebugGrid;


};
