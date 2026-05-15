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

TEST(RaccoonEventTest, DefaultStolenAmountIs2) {
    RaccoonEvent raccoon;  // default ctor stolenAmount=2
    Garden g(20, 20);
    Player player;
    fillGarden(g);

    raccoon.trigger(g, player);
    int remaining = countOccupied(g);

    EXPECT_GE(remaining, 400 - 2);
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
    RandomEventManager mgr;
    Garden g(20, 20);
    Player player;
    EXPECT_NO_THROW(mgr.update(1.0f, g, player));
}

TEST(RandomEventManagerTest, RespectsTriggerIntervalDoesNotFireEarly) {
    // 60 second trigger interval; call update with small deltas and verify no fire
    auto event = std::make_unique<CountingEvent>();
    CountingEvent* raw = event.get();

    RandomEventManager mgr(1.0f);
    mgr.registerEvent(std::move(event));

    Garden g(5, 5);
    Player player;

    // Call update with small deltas totaling less than 60 seconds
    for (int i = 0; i < 50; ++i) {
        mgr.update(1.0f, g, player);  // 50 * 1.0f = 50 seconds
    }
    EXPECT_EQ(raw->triggerCount, 0) << "Should not fire before 60 seconds";

    // Now push it just over 60 seconds
    mgr.update(10.1f, g, player);
    EXPECT_EQ(raw->triggerCount, 1) << "Should fire once after crossing 60 seconds";
}

TEST(RandomEventManagerTest, FiresMultipleTimesAfterIntervalElapses) {
    auto event = std::make_unique<CountingEvent>();
    CountingEvent* raw = event.get();

    RandomEventManager mgr(1.0f);
    mgr.registerEvent(std::move(event));

    Garden g(5, 5);
    Player player;

    // Accumulate 120+ seconds
    mgr.update(61.0f, g, player);
    int afterFirst = raw->triggerCount;
    EXPECT_EQ(afterFirst, 1) << "Should fire once after first 60 seconds";

    mgr.update(60.0f, g, player);
    EXPECT_EQ(raw->triggerCount, 2) << "Should fire again after next 60 seconds";
}

TEST(RandomEventManagerTest, AddNullEventIsIgnored) {
    RandomEventManager mgr;
    EXPECT_NO_THROW(mgr.registerEvent(nullptr));

    Garden g(5, 5);
    Player player;
    EXPECT_NO_THROW(mgr.update(1.0f, g, player));
}
