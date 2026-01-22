#include "Perlin.h"

#include <iostream>
#include <SFML/Graphics/Image.hpp>

#include "Engine/Globals.h"
#include "Engine/Random/RandomMgr.h"

Perlin::Perlin(): RandomInstance(nullptr), Grid(nullptr)
{
    this->SizeX = 256;
    this->SizeY = 256;
    this->CellSize = 16;
    this->Octaves = 4;

}

Perlin::Perlin(unsigned sizeX, unsigned sizeY, unsigned cellSize, unsigned octaves): RandomInstance(nullptr),
    Grid(nullptr)
{
    SizeX = sizeX;
    SizeY = sizeY;
    CellSize = cellSize;
    Octaves = octaves;
}

Perlin::~Perlin()
{
    RandomInstance = nullptr;
    
    delete Grid;
    Grid = nullptr;
}

bool Perlin::Generate(int randomId)
{
    Grid = new PerlinGrid(static_cast<int>(SizeX), static_cast<int>(SizeY), CellSize);
    if (!gData.RandomMgr->GetInstance(randomId, RandomInstance))
    {
        std::cout << "Instance doesn't exist." << std::endl;
        return false; 
    }

    if (!Grid->Generate(*RandomInstance))
    {
        std::cout << "Can't Generated Perlin Grid." << std::endl;
        return false;
    }
    
    return true;
}

void Perlin::GenerateTexture(sf::Texture& texture) const
{
    sf::Image image = sf::Image(sf::Vector2u(SizeX, SizeY));
    
    for (int i = 0; i < static_cast<int>(SizeX); i++)
    {
        for (int j = 0; j < static_cast<int>(SizeY); j++)
        {
            
            float v = 0; 
            unsigned iter = 0;
            float frequency = 1;
            float amplitude = 1;
            while (iter < Octaves)
            {
                v += Grid->GetPerlinValue(static_cast<float>(i) * frequency / static_cast<float>(Grid->GridSizeX),
                    static_cast<float>(j) * frequency / static_cast<float>(Grid->GridSizeY)) * amplitude;
           
                frequency *= 2;
                amplitude *= .5f;
                iter++;
            }
            uint8_t grey = (v + 1.0f) * 0.5f * 255.0f;
            image.setPixel(sf::Vector2u(i, j), sf::Color(grey, grey, grey, 255));
        }
    }

    if (!texture.loadFromImage(image))
    {
        std::cout << "Failed to load texture from image." << std::endl;
    }
    
}


