#pragma once

#include <memory>
#include "Plant.h"

class Cell 
{
    private:
        std::unique_ptr<Plant> plant_;
    public:
        bool isEmpty() const;
        bool setPlant(std::unique_ptr<Plant> plant);
        void clearPlant();
        Plant* getPlant() const;
};