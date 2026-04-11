#include <string>
#include <vector>
#include "Mutation.h"
#include "HarvestedItem.h"
#include "../world/Cell.h"
#include "../world/WeatherSystem.h"
#include <utility>
#include <algorithm>

class Plant 
{
    private:
        double calcPrice();
    protected:
        int id_;
        std::string name_;
        int currentStage_;
        int maxStages_;
        std::size_t baseTicksPerStage_;
        std::size_t currentTicksPerStage_;
        std::size_t ticksElapsed_;
        double sellPrice_;
        std::vector<Mutation> mutations_;
        bool regrowsAfterHarvest_;
        bool consumedOnHarvest_;
        int regrowStage_;
    public:
        Plant(int id,
              std::string name,
              int currentStage,
              int maxStages,
              std::size_t ticksPerStage,
              std::size_t ticksElapsed,
              double sellPrice,
              bool regrowsAfterHarvest,
              int regrowStage);
        void grow(std::size_t ticks);
        HarvestedItem harvest();
        bool isFullyGrown() const;
        std::size_t getTimeToGrowth() const;
        void applyWeatherEffect(WeatherType weatherType);
        void addMutation(Mutation newMutation);
        std::vector<MutationType> getMutations() const;
};