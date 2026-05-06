#pragma once

#include <memory>
#include <vector>

#include "Cell.h"

class Garden 
{
    private:
        std::vector<Cell> cells_;
        int width_;
        int height_;
    
    public:
        //constructor
        Garden(int width, int height);

        int getWidth() const;
        int getHeight() const;

        // Read and Write
        Cell& getCell(int x, int y);
        // Read only
        const Cell& getCell(int x, int y) const;
        bool plantCrop(int x, int y, std::unique_ptr<Plant> crop);
};