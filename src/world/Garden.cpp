#include "Garden.h"

Garden::Garden(int width, int height) 
    : width_(width), 
      height_(height), 
      cells_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {}

int Garden::getWidth() const
{
    return width_;
}

int Garden::getHeight() const
{
    return height_;
}

Cell& Garden::getCell(int x, int y)
{
    return cells_[(static_cast<std::size_t>(y) * height_) + static_cast<std::size_t>(x)];
}

const Cell& Garden::getCell(int x, int y) const
{
    return cells_[(static_cast<std::size_t>(y) * height_) + static_cast<std::size_t>(x)];
}
bool Garden::plantCrop(int x, int y, std::unique_ptr<Plant> crop)
{
    return getCell(x, y).setPlant(std::move(crop));
}