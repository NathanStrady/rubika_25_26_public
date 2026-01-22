#include "PerlinGrid.h"

#include <iostream>
#include <__msvc_ostream.hpp>

#include "Engine/Random/Instances/RandomInstance.h"


PerlinGrid::PerlinGrid(int SizeX, int SizeY, int CellSize): normalizedVectorGrid(Grid<sf::Vector2f>(2, 2)),
                                                            perlinDebugGrid(Grid<float>(2, 2))
{
    this->CellSize = CellSize;
    this->GridSizeX = GetPerlinCoords(SizeX);
    this->GridSizeY = GetPerlinCoords(SizeY);
}


bool PerlinGrid::Generate(RandomInstance& random)
{
    if (!GenerateNormalizedVector(random))
    {
        return false;
    }
    
    return true; 
}

bool PerlinGrid::GenerateNormalizedVector(RandomInstance& random)
{
    normalizedVectorGrid = Grid<sf::Vector2f>(this->GridSizeX, this->GridSizeY);
    normalizedVectorGrid.Generate(sf::Vector2f(0, 0));
    for (int i = 0; i < normalizedVectorGrid.SizeX; i++)
    {
        for (int j = 0; j <  normalizedVectorGrid.SizeY; j++)
        {
            float x = static_cast<float>(random.RandDouble(-1.f, 1.f));
            float y = static_cast<float>(random.RandDouble(-1.f, 1.f));
            sf::Vector2f normVector = sf::Vector2f(x, y).normalized();
            normalizedVectorGrid.SetCell(i, j, normVector);
        }
    }
    
    return true;
}

float PerlinGrid::GetPerlinValue(float x, float y)
{
    float nx0, nx1;
    float ix0, ix1;
    float ny0, ny1;

    float sx = x - trunc(x);
    float sy = y - trunc(y);
    
    nx0 = DotGridGradient(x, y, x, y);
    nx1 = DotGridGradient(x+1, y, x, y);
    ix0 = Interpolate(nx0, nx1, sx);
    
    ny0 = DotGridGradient(x, y+1,  x, y);
    ny1 = DotGridGradient(x+1, y+1, x, y);
    ix1 = Interpolate(ny0, ny1, sx);
    
    return Interpolate(ix0, ix1, sy);
}

float PerlinGrid::SmoothStep(float w)
{
    if (w <= 0.0)
    {
        return 0;
    }

    if (w >= 1.0)
    {
        return 1.0;
    }

    return w * w * (3.0 - 2.0 * w);
}

float PerlinGrid::Interpolate(float a0, float a1, float w)
{
    return a0 + (a1 - a0) * SmoothStep(w);
}

int PerlinGrid::GetPerlinCoords(int i)
{
    return i / CellSize + 1;
}

float PerlinGrid::DotGridGradient(unsigned ix, unsigned iy, float& x, float& y)
{
    sf::Vector2f normVector;
    float dx = x - ix;
    float dy = y - iy;
    
    normalizedVectorGrid.GetCell(ix % GridSizeX, iy % GridSizeY, normVector);
    return dx * normVector.x + dy * normVector.y;
}








