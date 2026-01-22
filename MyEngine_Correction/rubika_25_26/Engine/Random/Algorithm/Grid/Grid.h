#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>

template <typename T = uint8_t>
class Grid 
{
public:
    int SizeX;
    int SizeY;
    
    Grid(int sizeX, int sizeY);
    ~Grid() = default;
    
    void Generate(T d);
    bool GetCell(int i, int j, T& value) const;
    bool GetCell(int index, T& value) const;
    bool SetCell(int i, int j, T value);
    bool SetCell(int index, T value);

    bool IsInGrid(int i, int j) const;
    int To1D(int i, int j) const;
    sf::Vector2i To2D(int index) const;

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const Grid& grid)
    {
        return os << grid.toString();
    }
    
    void Clear();
private:
    std::vector<T> data;
};



template <typename T>
std::string Grid<T>::toString() const
{
    std::string s = " ";
    for (int i = 0; i < SizeX; ++i)
    {
        for (int j = 0; j < SizeY; ++j) 
        {
            if (T cell; GetCell(i, j, cell))
            {
                s += std::to_string(cell) + " ";
            }
        }
        s += "\n";
    }
    return s; 
}

template <typename T>
void Grid<T>::Clear()
{
    data.clear();
}


template <typename T>
Grid<T>::Grid(int sizeX, int sizeY)
{
    this->SizeX = sizeX;
    this->SizeY = sizeY;
}

template <typename T>
void Grid<T>::Generate(T d)
{
    data.assign(static_cast<size_t>(SizeX) * static_cast<size_t>(SizeY), d);
}

template <typename T>
bool Grid<T>::GetCell(int i, int j, T& value) const
{
    int idx = To1D(i, j);
    if (IsInGrid(i, j))
    {
        value = data[idx];
        return true;
    }

    return false;    
}

template <typename T>
bool Grid<T>::GetCell(int index, T& value) const
{
    sf::Vector2i idx = To2D(index);
    if (IsInGrid(idx.x, idx.x))
    {
        value = data[index];
        return true;
    }

    return false;
}

template <typename T>
bool Grid<T>::SetCell(int index, T value)
{
    sf::Vector2i idx = To2D(index);
    if (IsInGrid(idx.x, idx.y))
    {
        data[index] = value;
        return true;
    }

    return false;
}

template <typename T>
bool Grid<T>::SetCell(int i, int j, T value)
{
    int idx = To1D(i, j);
    if (IsInGrid(i, j))
    {
        data[idx] = value;
        return true;
    }

    return false;
}

template <typename T>
bool Grid<T>::IsInGrid(int i, int j) const
{
    return i >= 0 && i < SizeX && j >= 0 && j < SizeY;
}

template <typename T>
int Grid<T>::To1D(int i, int j) const
{
    return j * SizeX + i;
}

template <typename T>
sf::Vector2i Grid<T>::To2D(int index) const
{
    return { index % SizeX, index / SizeX };
}

