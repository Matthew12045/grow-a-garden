#include <gtest/gtest.h>
#include "../src/world/Cell.h"
#include "../src/world/Garden.h"
#include "helpers/ConcreteClasses.h"
#include <memory>

// ── Cell ──────────────────────────────────────────────────────────────────

TEST(CellTest, NewCellIsEmpty) {
    Cell c;
    EXPECT_TRUE(c.isEmpty());
}

TEST(CellTest, GetPlantReturnsNullOnEmptyCell) {
    Cell c;
    EXPECT_EQ(c.getPlant(), nullptr);
}

TEST(CellTest, SetPlantWithNullptrReturnsFalse) {
    Cell c;
    EXPECT_FALSE(c.setPlant(nullptr));
}

TEST(CellTest, SetPlantSucceedsOnEmptyCell) {
    Cell c;
    auto plant = std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0);
    EXPECT_TRUE(c.setPlant(std::move(plant)));
}

TEST(CellTest, CellIsNotEmptyAfterSetPlant) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    EXPECT_FALSE(c.isEmpty());
}

TEST(CellTest, GetPlantReturnsPlantAfterSetPlant) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    EXPECT_NE(c.getPlant(), nullptr);
}

TEST(CellTest, SetPlantOnOccupiedCellReturnsFalse) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    auto second = std::make_unique<ConcretePlant>(2, "Carrot", 0, 3, 8, 0, 50.0);
    EXPECT_FALSE(c.setPlant(std::move(second)));
}

TEST(CellTest, SetPlantOnOccupiedCellDoesNotReplaceExistingPlant) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    Plant* originalPtr = c.getPlant();
    c.setPlant(std::make_unique<ConcretePlant>(2, "Carrot", 0, 3, 8, 0, 50.0));
    EXPECT_EQ(c.getPlant(), originalPtr);  // original plant unchanged
}

TEST(CellTest, ClearPlantEmptiesCell) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    c.clearPlant();
    EXPECT_TRUE(c.isEmpty());
}

TEST(CellTest, ClearPlantOnEmptyCellDoesNotCrash) {
    Cell c;
    EXPECT_NO_THROW(c.clearPlant());
}

TEST(CellTest, CanReplantAfterClear) {
    Cell c;
    c.setPlant(std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    c.clearPlant();
    EXPECT_TRUE(c.setPlant(std::make_unique<ConcretePlant>(2, "Carrot", 0, 3, 8, 0, 50.0)));
}

// ── Garden ────────────────────────────────────────────────────────────────

TEST(GardenTest, GetWidthReturnsCorrectValue) {
    Garden g(20, 20);
    EXPECT_EQ(g.getWidth(), 20);
}

TEST(GardenTest, GetHeightReturnsCorrectValue) {
    Garden g(20, 20);
    EXPECT_EQ(g.getHeight(), 20);
}

TEST(GardenTest, AllCellsStartEmpty) {
    Garden g(20, 20);
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x)
            EXPECT_TRUE(g.getCell(x, y).isEmpty())
                << "Cell (" << x << "," << y << ") should be empty";
}

TEST(GardenTest, PlantCropPlacesPlantInCorrectCell) {
    Garden g(20, 20);
    auto plant = std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0);
    bool placed = g.plantCrop(3, 7, std::move(plant));
    EXPECT_TRUE(placed);
    EXPECT_FALSE(g.getCell(3, 7).isEmpty());
}

TEST(GardenTest, PlantCropDoesNotAffectOtherCells) {
    Garden g(20, 20);
    g.plantCrop(3, 7, std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    // Neighbours should still be empty
    EXPECT_TRUE(g.getCell(4, 7).isEmpty());
    EXPECT_TRUE(g.getCell(3, 8).isEmpty());
    EXPECT_TRUE(g.getCell(2, 7).isEmpty());
}

// Critical: verifies the index formula is (y * WIDTH + x), not (y * height + x).
// On a non-square garden this would corrupt cell lookups.
TEST(GardenTest, IndexFormulaIsCorrectOnNonSquareGarden) {
    // 10 columns (width), 5 rows (height)
    Garden g(10, 5);
    g.plantCrop(9, 4, std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    // Bottom-right corner should be occupied
    EXPECT_FALSE(g.getCell(9, 4).isEmpty());
    // Top-left and any other corner must be empty
    EXPECT_TRUE(g.getCell(0, 0).isEmpty());
    EXPECT_TRUE(g.getCell(9, 0).isEmpty());
    EXPECT_TRUE(g.getCell(0, 4).isEmpty());
}

TEST(GardenTest, DifferentCoordinatesMapToDifferentCells) {
    Garden g(20, 20);
    // Plant at (5, 3) and verify (3, 5) is a different cell
    g.plantCrop(5, 3, std::make_unique<ConcretePlant>(1, "A", 0, 5, 10, 0, 100.0));
    EXPECT_TRUE(g.getCell(3, 5).isEmpty());
}

TEST(GardenTest, ConstGetCellWorks) {
    Garden g(20, 20);
    g.plantCrop(1, 1, std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));
    const Garden& cg = g;
    EXPECT_FALSE(cg.getCell(1, 1).isEmpty());
    EXPECT_TRUE(cg.getCell(0, 0).isEmpty());
}

TEST(GardenTest, PlantCropReturnsFalseWhenCellOccupied) {
    Garden g(20, 20);
    g.plantCrop(0, 0, std::make_unique<ConcretePlant>(1, "A", 0, 5, 10, 0, 100.0));
    bool result = g.plantCrop(0, 0, std::make_unique<ConcretePlant>(2, "B", 0, 5, 10, 0, 50.0));
    EXPECT_FALSE(result);
}
