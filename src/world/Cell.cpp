#include "Cell.h"

bool Cell::isEmpty() const 
{
    return plant_ == nullptr;
}
bool Cell::setPlant(std::unique_ptr<Plant> newPlant)
{
    if (!newPlant || !isEmpty())
    {
        return false;
    }
    plant_ = std::move(newPlant);
    return true;
}
void Cell::clearPlant()
{
    plant_.reset();
}
Plant* Cell::getPlant() const
{
    return plant_.get();
}