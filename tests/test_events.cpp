#include <gtest/gtest.h>
#include "../src/systems/RaccoonEvent.h"
#include "../src/systems/RandomEventManager.h"
#include "../src/world/Garden.h"
#include "helpers/ConcreteClasses.h"
#include <memory>

// ─── Helpers ──────────────────────────────────────────────────────────────

// Fills every cell of a garden with a fresh ConcretePlant.
static void fillGarden(Garden& g) {
    for (int y = 0; y < g.getHeight(); ++y)
        for (int x = 0; x < g.getWidth(); ++x)
            g.plantCrop(x, y,
                std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
}

static int countOccupied(Garden& g) {
    int count = 0;
    for (int y = 0; y < g.getHeight(); ++y)
        for (int x = 0; x < g.getWidth(); ++x)
            if (!g.getCell(x, y).isEmpty())
                ++count;
    return count;
}

// ── RaccoonEvent ─────────────────────────────────────────────────────────

TEST(RaccoonEventTest, TriggerDoesNotCrashOnEmptyGarden) {
    RaccoonEvent raccoon(5);
    Garden g(20, 20);
    Player player;
    EXPECT_NO_THROW(raccoon.trigger(g, player));
}

TEST(RaccoonEventTest, TriggerStealsFromPopulatedGarden) {
    RaccoonEvent raccoon(5);
    Garden g(20, 20);
    Player player;
    fillGarden(g);

    int before = countOccupied(g);  // 400
    raccoon.trigger(g, player);
    int after = countOccupied(g);

    EXPECT_LT(after, before) << "Raccoon should have stolen at least one plant";
}

TEST(RaccoonEventTest, TriggerStealsAtMostStolenAmountPlants) {
    const int stolenAmount = 10;
    RaccoonEvent raccoon(stolenAmount);
    Garden g(20, 20);
    Player player;
    fillGarden(g);

    int before = countOccupied(g);
    raccoon.trigger(g, player);
    int after = countOccupied(g);

    EXPECT_GE(after, before - stolenAmount)
        << "Raccoon should not steal more than stolenAmount plants";
}

TEST(RaccoonEventTest, DefaultStolenAmountIs10) {
    RaccoonEvent raccoon;  // default ctor stolenAmount=10
    Garden g(20, 20);
    Player player;
    fillGarden(g);

    raccoon.trigger(g, player);
    int remaining = countOccupied(g);

    EXPECT_GE(remaining, 400 - 10);
}

TEST(RaccoonEventTest, TriggerWithZeroStolenAmountStealsNothing) {
    RaccoonEvent raccoon(0);
    Garden g(20, 20);
    Player player;
    fillGarden(g);

    raccoon.trigger(g, player);
    EXPECT_EQ(countOccupied(g), 400);
}

// ── RandomEventManager ────────────────────────────────────────────────────

// Counting event: tracks how many times it was triggered.
class CountingEvent : public RandomEvent {
public:
    int triggerCount = 0;
    void trigger(Garden& /*g*/, Player& /*p*/) override { ++triggerCount; }
};

TEST(RandomEventManagerTest, NoEventsNoCrash) {
    RandomEventManager mgr(1.0f, 0);
    Garden g(20, 20);
    Player player;
    EXPECT_NO_THROW(mgr.checkAndTrigger(1, g, player));
}

TEST(RandomEventManagerTest, RespectsCooldownDoesNotFireEarly) {
    // Cooldown = 100 ticks; fire at tick 1 then check at tick 50.
    auto event = std::make_unique<CountingEvent>();
    CountingEvent* raw = event.get();

    RandomEventManager mgr(1.0f, 100);
    mgr.addEvent(std::move(event));

    Garden g(5, 5);
    Player player;

    mgr.checkAndTrigger(0, g, player);   // fires at tick 0 (first opportunity)
    int firedAtZero = raw->triggerCount;

    mgr.checkAndTrigger(50, g, player);  // only 50 ticks passed → cooldown not done
    EXPECT_EQ(raw->triggerCount, firedAtZero) << "Should not fire during cooldown";
}

TEST(RandomEventManagerTest, FiresAfterCooldownExpires) {
    auto event = std::make_unique<CountingEvent>();
    CountingEvent* raw = event.get();

    RandomEventManager mgr(1.0f, 10);  // prob=1.0 → always fires when allowed
    mgr.addEvent(std::move(event));

    Garden g(5, 5);
    Player player;

    mgr.checkAndTrigger(0,  g, player);  // fires
    mgr.checkAndTrigger(10, g, player);  // cooldown expired → fires again
    EXPECT_EQ(raw->triggerCount, 2);
}

TEST(RandomEventManagerTest, ZeroProbabilityNeverFires) {
    auto event = std::make_unique<CountingEvent>();
    CountingEvent* raw = event.get();

    RandomEventManager mgr(0.0f, 0);  // prob=0 → never
    mgr.addEvent(std::move(event));

    Garden g(5, 5);
    Player player;

    for (int tick = 0; tick < 100; ++tick)
        mgr.checkAndTrigger(tick, g, player);

    EXPECT_EQ(raw->triggerCount, 0);
}

TEST(RandomEventManagerTest, AddNullEventIsIgnored) {
    RandomEventManager mgr(1.0f, 0);
    EXPECT_NO_THROW(mgr.addEvent(nullptr));

    Garden g(5, 5);
    Player player;
    EXPECT_NO_THROW(mgr.checkAndTrigger(0, g, player));
}
