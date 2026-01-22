#include "DrunkardGrid.h"

#include <iostream>

#include "CellularGrid.h"
#include "Engine/Random/Instances/RandomInstance.h"

DrunkardGrid::DrunkardGrid()
{
    drunkardGrid = new Grid(256, 256);
}

DrunkardGrid::DrunkardGrid(int SizeX, int SizeY)
{
    drunkardGrid = new Grid(SizeX, SizeY);
}

DrunkardGrid::~DrunkardGrid()
{
    delete drunkardGrid;
    drunkardGrid = nullptr;
}

bool DrunkardGrid::Generate()
{
    if (!drunkardGrid)
    {
        std::cout << "Drunkard Grid Not Initialized" << std::endl;
        return false;
    }
    
    drunkardGrid->Generate(0);

    return true;
}
