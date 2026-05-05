#include "RaccoonEvent.h"
#include "../core/Player.h"
#include "../world/Garden.h"
#include "../world/Cell.h"
#include <cstdlib>

RaccoonEvent::RaccoonEvent(int stolenAmount) : stolenAmount_(stolenAmount) {}

void RaccoonEvent::trigger(Garden& garden, Player& player) {
    int plantsStolen = 0;
    int maxAttempts = 100; // Prevent infinite loop if garden mostly empty
    int attempts = 0;

    while (plantsStolen < stolenAmount_ && attempts < maxAttempts) {
        int x = rand() % garden.getWidth();
        int y = rand() % garden.getHeight();

        Cell& cell = garden.getCell(x, y);
        if (!cell.isEmpty()) {
            cell.clearPlant();
            plantsStolen++;
        }
        attempts++;
    }
}