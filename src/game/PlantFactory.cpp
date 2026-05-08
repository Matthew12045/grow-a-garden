#include "PlantFactory.h"

std::unique_ptr<Plant> makePlant(const std::string& seedName) {
    if (seedName == "Carrot Seed")
        return std::make_unique<Plant>( 1, "Carrot",     0, 5,  10,  0,  30.0, false, 0);
    if (seedName == "Blueberry Seed")
        return std::make_unique<Plant>( 2, "Blueberry",  0, 4,   8,  0,  45.0, true,  1);
    if (seedName == "Rose Seed")
        return std::make_unique<Plant>( 3, "Rose",       0, 6,  12,  0,  80.0, false, 0);
    if (seedName == "Bamboo Seed")
        return std::make_unique<Plant>( 4, "Bamboo",     0, 3,  15,  0,  35.0, true,  0);
    if (seedName == "Corn Seed")
        return std::make_unique<Plant>( 5, "Corn",       0, 5,  14,  0,  60.0, false, 0);
    if (seedName == "Tomato Seed")
        return std::make_unique<Plant>( 6, "Tomato",     0, 6,  10,  0,  65.0, true,  2);
    if (seedName == "Apple Seed")
        return std::make_unique<Plant>( 7, "Apple",      0, 8,  20,  0, 150.0, false, 0);
    if (seedName == "Cactus Seed")
        return std::make_unique<Plant>( 8, "Cactus",     0, 4,  25,  0,  90.0, false, 0);
    if (seedName == "Coconut Seed")
        return std::make_unique<Plant>( 9, "Coconut",    0, 7,  25,  0, 200.0, false, 0);
    if (seedName == "Beanstalk Seed")
        return std::make_unique<Plant>(10, "Beanstalk",  0, 8,  20,  0, 220.0, false, 0);
    if (seedName == "Cacao Seed")
        return std::make_unique<Plant>(11, "Cacao",      0, 6,  18,  0, 130.0, false, 0);
    // Fallback
    return std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
}