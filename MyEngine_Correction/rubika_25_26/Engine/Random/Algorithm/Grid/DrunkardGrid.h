#pragma once
#include "Grid.h"

class DrunkardGrid
{
public:
    DrunkardGrid();
    DrunkardGrid(int SizeX, int SizeY);
    ~DrunkardGrid();

    bool Generate();
    
    Grid<>* drunkardGrid;

};
