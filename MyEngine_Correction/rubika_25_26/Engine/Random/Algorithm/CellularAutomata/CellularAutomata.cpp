#include "CellularAutomata.h"

#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include "Engine/Globals.h"
#include "Engine/Random/RandomMgr.h"
#include "Engine/Random/Algorithm/Grid/Grid.h"

CellularAutomata::CellularAutomata(): RandomInstance(), CurrentGrid(nullptr), NextGrid(nullptr)
{
    this->SizeX = 256;
    this->SizeY = 256;
    this->SpawnPercent = 0.6;
    this->NeightborThreshold = 5;
    this->Iteration = 5;
}

CellularAutomata::CellularAutomata(unsigned sizeX, unsigned sizeY, double spawnPercent, unsigned threshold,
                                   unsigned iteration): RandomInstance(nullptr), CurrentGrid(nullptr), NextGrid(nullptr)
{
    this->SizeX = sizeX;
    this->SizeY = sizeY;
    this->SpawnPercent = spawnPercent;
    this->NeightborThreshold = threshold;
    this->Iteration = iteration;
}

CellularAutomata::~CellularAutomata()
{
    RandomInstance = nullptr;
    
    delete CurrentGrid;
    CurrentGrid = nullptr;
    
    delete NextGrid;
    NextGrid = nullptr;
}

bool CellularAutomata::Generate(int randomId)
{
    CurrentGrid = new CellularGrid(static_cast<int>(SizeX), static_cast<int>(SizeY));
    NextGrid = new CellularGrid(static_cast<int>(SizeX), static_cast<int>(SizeY));
    
    if (!gData.RandomMgr->GetInstance(randomId, RandomInstance))
    {
        std::cout << "Instance doesn't exist." << std::endl;
        return false; 
    }
    
    if (!CurrentGrid->Generate())
    {
        std::cout << "Can't generate the grid." << std::endl;
        return false; 
    }

    if (!NextGrid->Generate())
    {
        std::cout << "Can't generate the grid." << std::endl;
        return false;
    }
    
    return true;
}

void CellularAutomata::GenerateTexture(sf::Texture& texture)
{
    sf::Image image = sf::Image(sf::Vector2u(SizeX, SizeY));
    
    CellularWalk();
    for (int i = 0; i < static_cast<int>(SizeX); i++)
    {
        for (int j = 0; j < static_cast<int>(SizeY); j++)
        {
            if (CurrentGrid->IsAlive(i, j))
            {
                image.setPixel(sf::Vector2u(i, j), sf::Color::White);
            }
        }
    }

    if (!texture.loadFromImage(image))
    {
        std::cout << "Failed to load texture." << std::endl;
    }
}

void CellularAutomata::GenerateAliveCells() const
{
    for (int i = 0; i < static_cast<int>(SizeX); i++)
    {
        for (int j = 0; j < static_cast<int>(SizeY); j++)
        {
            if (RandomInstance->RandDouble(0.f, 1.f) < SpawnPercent)
            {
                if (!CurrentGrid->SetAlive(i, j))
                {
                    
                }
            }
        }
    }
}

void CellularAutomata::CellularWalk() 
{
    GenerateAliveCells();
    
    int currIteration = 0;
    while (currIteration < static_cast<int>(Iteration))
    {
        for (int i = 0; i < static_cast<int>(SizeX); i++)
        {
            for (int j = 0; j < static_cast<int>(SizeY); j++)
            {
                if (!BirthNewCell(i, j))
                {
                    //std::cout << "Can't birth a new cell." << std::endl;
                }
            }
        }
        
        std::swap(CurrentGrid, NextGrid);
        currIteration++;
    }

}

bool CellularAutomata::BirthNewCell(int i, int j) const
{
    if (CurrentGrid->GetNeighbours(i, j) < static_cast<int>(NeightborThreshold))
    {
        return false;
    }
    
    if (!NextGrid->SetAlive(i, j))
    {
        return false;
    }
    
    return true; 
}


