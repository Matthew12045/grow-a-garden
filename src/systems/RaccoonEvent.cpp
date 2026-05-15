#include "RaccoonEvent.h"
#include "../core/Player.h"
#include "../world/Garden.h"
#include "../world/Cell.h"

RaccoonEvent::RaccoonEvent(int stolenAmount)
    : stolenAmount_(stolenAmount),
      rng_(std::random_device{}())
{}

void RaccoonEvent::trigger(Garden& garden, Player& player) {
    int plantsStolen = 0;
    int maxAttempts = 100; // Prevent infinite loop if garden mostly empty
    int attempts = 0;
    std::uniform_int_distribution<int> xDist(0, garden.getWidth()  - 1);
    std::uniform_int_distribution<int> yDist(0, garden.getHeight() - 1);

    while (plantsStolen < stolenAmount_ && attempts < maxAttempts) {
        int x = xDist(rng_);
        int y = yDist(rng_);

        Cell& cell = garden.getCell(x, y);
        if (!cell.isEmpty()) {
            cell.clearPlant();
            plantsStolen++;
        }
        attempts++;
    }
}
