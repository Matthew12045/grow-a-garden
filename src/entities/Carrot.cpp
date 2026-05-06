#include "Carrot.h"

Carrot::Carrot()
    : Plant(/*id*/        1,
            /*name*/      "Carrot",
            /*stage*/     0,
            /*maxStages*/ 5,
            /*ticksPerStage*/ 10,
            /*ticksElapsed*/  0,
            /*sellPrice*/ 30.0,
            /*regrows*/   false,
            /*regrowStage*/ 0) {}

Carrot::Carrot(int         id,
               int         currentStage,
               int         maxStages,
               std::size_t ticksPerStage,
               std::size_t ticksElapsed,
               double      sellPrice,
               bool        regrows,
               int         regrowStage)
    : Plant(id, "Carrot", currentStage, maxStages,
            ticksPerStage, ticksElapsed, sellPrice, regrows, regrowStage) {}
