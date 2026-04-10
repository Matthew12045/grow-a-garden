#include <string>
#include <vector>
#include "Mutation.h"
#include "HarvestedItem.h"

class Plant 
{
    private:
        double clacPrice();
    protected:
        int id_;
        std::string name_;
        int currentStage_;
        int maxStages_;
        std::size_t ticksPerStage_;
        std::size_t ticksElapsed;
        double sellPrice_;
        std::vector<Mutation> mutations_;
    public:
        void grow(std::size_t ticks);
        HarvestedItem harvest();
        bool isFullyGrown() const;
        int getTimeToGrowth() const;
        void applyWeatherEffect(Weather weather);
        void addMutation();
};