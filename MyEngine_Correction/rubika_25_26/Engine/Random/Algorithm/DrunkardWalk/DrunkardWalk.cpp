#include "DrunkardWalk.h"

#include <array>
#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include "Engine/Globals.h"
#include "Engine/Random/RandomMgr.h"
#include "Engine/Random/Algorithm/Grid/Grid.h"
#include "Engine/Random/Instances/RandomInstance.h"
#include "Engine/Render/Ressource/TextureMgr.h"

DrunkardWalk::DrunkardWalk(): RandomInstance(nullptr), InternalGrid(nullptr)
{
    this->SizeX = 256;
    this->SizeY = 256;
    this->Iteration = 10;
    this->Distance = 1000;
    this->SpawnNumber = 1;
}

DrunkardWalk::DrunkardWalk(int sizeX, int sizeY, int iteration, int distance, int spawnNumber): RandomInstance(nullptr),
    InternalGrid(nullptr)
{
    this->SizeX = sizeX;
    this->SizeY = sizeY;
    this->Iteration = iteration;
    this->Distance = distance;
    this->SpawnNumber = spawnNumber;
}

bool DrunkardWalk::Generate(int randomId)
{
    InternalGrid = new DrunkardGrid(SizeX, SizeY);
    if (!InternalGrid->Generate())
    {
        return false;
    }
    
    if (!gData.RandomMgr->GetInstance(randomId, RandomInstance))
    {
        std::cout << "Instance doesn't exist." << std::endl;
        return false; 
    }
    
    return true;
}

void DrunkardWalk::GenerateTexture(sf::Texture& texture)
{
    sf::Image image = sf::Image(sf::Vector2u(SizeX, SizeY));
    std::vector<int> drunk = Walk();
    
    for (auto& index : drunk)
    {
        sf::Vector2i coords = InternalGrid->drunkardGrid->To2D(index);
        image.setPixel(sf::Vector2u(coords.x, coords.y), sf::Color::White);
    }
    
    if (!texture.loadFromImage(image))
    {
        std::cout << "Failed to load texture." << std::endl;
    }
}

// If the current directions is not valid, we chose another one randomly
bool DrunkardWalk::DetermineDirection(sf::Vector2i& newIndex) const
{
    std::vector<std::pair<int, int>> directions;
    directions.reserve(directions.size());
    directions = {{
        {-1, 0},
        { 1, 0},
        { 0, 1},
        { 0,-1}
    }};


    bool bFind = false;
    while (!directions.empty() && !bFind)
    {
        const int r = RandomInstance->RandInt32(0, static_cast<int>(directions.size() - 1));
        const std::pair<int, int> direction = directions[r];
        if (InternalGrid->drunkardGrid->IsInGrid(newIndex.x + direction.first, newIndex.y + direction.second))
        {
            newIndex.x += direction.first;
            newIndex.y += direction.second;
            bFind = true;
        } else
        {
            directions.erase(directions.begin() + r);
        }
    }

    return bFind;
}

std::vector<int> DrunkardWalk::Walk() const
{
    std::vector<int> visitedIndex;
    std::vector<bool> visitedCells = std::vector<bool>(SizeX * SizeY, false);
    
    int currentSpawn = 0;
    while (currentSpawn < SpawnNumber)
    {
        int currentAgent = 0;
        int currentIndex = RandomInstance->RandInt32(0, SizeX * SizeY);
    
        while (currentAgent < Iteration)
        {
            // Get starting point
            if (currentAgent > 0)
            {
                int r = RandomInstance->RandInt32(0, static_cast<int>(visitedIndex.size() - 1));
                currentIndex = visitedIndex[r];
            } else
            {
                visitedCells[currentIndex] = true;
                visitedIndex.push_back(currentIndex);
            }

            // Move the walker
            int currentStep = 0;
            while (currentStep < Distance)
            {
                // Determine direction
                sf::Vector2i coords = InternalGrid->drunkardGrid->To2D(currentIndex);
                if (DetermineDirection(coords))
                {
                    currentIndex = InternalGrid->drunkardGrid->To1D(coords.x, coords.y);
                    // Mark it as visited if not
                    if (!visitedCells[currentIndex])
                    {
                        visitedCells[currentIndex] = true;
                        visitedIndex.push_back(currentIndex);
                    }
                }
                else
                {
                    std::cout << "Can't move" << std::endl;
                }
            
                currentStep++;
            }
            currentAgent++;
        }
        currentSpawn++;
    }
    
    return visitedIndex;
}

