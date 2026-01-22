#pragma once
#include <SFML/Window/Keyboard.hpp>

#include "Grid.h"

enum CelluleType : uint8_t
{
    Dead = 0,
    Alive = 1,
};

inline const std::string to_string(CelluleType e)
{
    switch (e)
    {
        case Dead: return "Dead";
        case Alive: return "Alive";
        default: return "Unknown";
    }
}



class CellularGrid
{
public:
    CellularGrid();
    CellularGrid(int SizeX, int SizeY);
    ~CellularGrid();

    bool Generate();
    bool IsAlive(int i, int j) const;
    bool IsAlive(int index) const;
    int GetNeighbours(int i, int j) const;

    bool SetAlive(int i, int j) const;
    bool SetAlive(int index) const;
    void Clear();

    Grid<CelluleType>* cellularGrid;
  
    
};
