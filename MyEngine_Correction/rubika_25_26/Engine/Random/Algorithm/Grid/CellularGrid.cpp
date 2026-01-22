#include "CellularGrid.h"

#include <iostream>
#include <__msvc_ostream.hpp>

CellularGrid::CellularGrid()
{
    cellularGrid = new Grid<CelluleType>(256, 256);
}

CellularGrid::CellularGrid(int SizeX, int SizeY)
{
    cellularGrid = new Grid<CelluleType>(SizeX, SizeY);
}

CellularGrid::~CellularGrid()
{
    delete cellularGrid;
    cellularGrid = nullptr;
}

bool CellularGrid::Generate()
{
    cellularGrid->Generate(Dead);

    // Generate Alive for the borders
    // North and South borders
    for (int j = 0; j < cellularGrid->SizeX; j++)
    {
        cellularGrid->SetCell(0, j, Alive);
        cellularGrid->SetCell(cellularGrid->SizeX - 1, j, Alive);
    }
    
    // East and west borders
    for (int i = 1; i < cellularGrid->SizeX -1; i++)
    {
        cellularGrid->SetCell(i, 0, Alive);
        cellularGrid->SetCell(i, cellularGrid->SizeY - 1, Alive);
    }
    
    return true;
}

// Check if a cell is alive
bool CellularGrid::IsAlive(int i, int j) const
{
    CelluleType cell;
    if (!cellularGrid->GetCell(i, j, cell))
    {
        //std::cout << "Can't get the cell at (" << i << "," << j << ")" << std::endl;
        return false; 
    }
    
    return cell == Alive;
}

bool CellularGrid::IsAlive(int index) const
{
    CelluleType cell;
    if (cellularGrid->GetCell(index, cell))
    {
        return false; 
    }

    return true;
}

// Return the number and a vector of neighbours of a given type cells. 
int CellularGrid::GetNeighbours(int i, int j) const
{
    int cptAlive = 0;
    const int dirs[8][2] = {
        { -1,  0 },
        {  1,  0 }, 
        {  0, -1 },
        {  0,  1 }, 
        { -1, -1 },
        { -1,  1 }, 
        {  1, -1 }, 
        {  1,  1 } 
    };

    for (auto& dir : dirs)
    {
        int next_i = i + dir[0];
        int next_j = j + dir[1];
        
        if (IsAlive(next_i, next_j))
        {
            cptAlive++;
        }
    }
  
    return cptAlive;
}

bool CellularGrid::SetAlive(int i, int j) const
{
    if (IsAlive(i, j))
    {
        return false;
    }
    
    if (!cellularGrid->SetCell(i, j,  Alive))
    {
        return false;
    }

    return true;
}

bool CellularGrid::SetAlive(int index) const
{
    if (IsAlive(index))
    {
        return false;
    }
    
    if (!cellularGrid->SetCell(index, Alive))
    {
        return false;
    }

    return true;
}

void CellularGrid::Clear()
{
    cellularGrid->Generate(Dead);
}

