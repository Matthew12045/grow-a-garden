#include <gtest/gtest.h>
#include "../src/systems/Inventory.h"
#include "../src/systems/Shop.h"
#include "../src/items/Seed.h"
#include "helpers/ConcreteClasses.h"
#include <memory>

// ─── Helpers ──────────────────────────────────────────────────────────────

static std::unique_ptr<Seed> makeSeed(const std::string& name, double price = 5.0) {
    return std::make_unique<Seed>(1, name, "desc", price, "CropType");
}

// ── Inventory: addItem ────────────────────────────────────────────────────

TEST(InventoryTest, AddItemSucceeds) {
    Inventory inv(10);
    EXPECT_TRUE(inv.addItem(makeSeed("Carrot Seed"), 1));
}

TEST(InventoryTest, AddItemWithZeroQtyFails) {
    Inventory inv(10);
    EXPECT_FALSE(inv.addItem(makeSeed("Carrot Seed"), 0));
}

TEST(InventoryTest, AddItemWithNegativeQtyFails) {
    Inventory inv(10);
    EXPECT_FALSE(inv.addItem(makeSeed("Carrot Seed"), -5));
}

TEST(InventoryTest, AddSameItemAccumulatesQuantity) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 3);
    inv.addItem(makeSeed("Carrot Seed"), 4);  // same name → same slot
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 7);
}

TEST(InventoryTest, AddDifferentItemsUsesSeparateSlots) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"),    2);
    inv.addItem(makeSeed("Tomato Seed"),    3);
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 2);
    EXPECT_EQ(inv.getQuantity("Tomato Seed"), 3);
}

TEST(InventoryTest, AddItemFailsWhenFull) {
    Inventory inv(2);
    inv.addItem(makeSeed("A"), 1);
    inv.addItem(makeSeed("B"), 1);
    EXPECT_FALSE(inv.addItem(makeSeed("C"), 1));  // 3rd unique item over cap
}

TEST(InventoryTest, AddExistingItemWhenFullSucceeds) {
    // Adding more of an already-slotted item should work even at capacity.
    Inventory inv(1);
    inv.addItem(makeSeed("A"), 1);
    EXPECT_TRUE(inv.addItem(makeSeed("A"), 5));
}

// ── Inventory: removeItem ─────────────────────────────────────────────────

TEST(InventoryTest, RemoveItemReducesQuantity) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 5);
    EXPECT_TRUE(inv.removeItem("Carrot Seed", 2));
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 3);
}

TEST(InventoryTest, RemoveItemAllQuantityClearsSlot) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 3);
    inv.removeItem("Carrot Seed", 3);
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 0);
}

TEST(InventoryTest, RemoveItemWhenSlotFreedAllowsNewItem) {
    Inventory inv(1);
    inv.addItem(makeSeed("A"), 1);
    inv.removeItem("A", 1);
    EXPECT_TRUE(inv.addItem(makeSeed("B"), 1));
}

TEST(InventoryTest, RemoveMoreThanAvailableFails) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 2);
    EXPECT_FALSE(inv.removeItem("Carrot Seed", 5));
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 2);  // unchanged
}

TEST(InventoryTest, RemoveUnknownItemFails) {
    Inventory inv(10);
    EXPECT_FALSE(inv.removeItem("Ghost Item", 1));
}

TEST(InventoryTest, RemoveWithZeroQtyFails) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 3);
    EXPECT_FALSE(inv.removeItem("Carrot Seed", 0));
}

// ── Inventory: getQuantity ────────────────────────────────────────────────

TEST(InventoryTest, GetQuantityReturnsZeroForUnknownItem) {
    Inventory inv(10);
    EXPECT_EQ(inv.getQuantity("Ghost"), 0);
}

TEST(InventoryTest, GetQuantityReturnsCorrectCount) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 7);
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 7);
}

// ── Inventory: isFull ─────────────────────────────────────────────────────

TEST(InventoryTest, NotFullWhenEmpty) {
    Inventory inv(5);
    EXPECT_FALSE(inv.isFull());
}

TEST(InventoryTest, IsFullWhenMaxSlotsReached) {
    Inventory inv(2);
    inv.addItem(makeSeed("A"), 1);
    inv.addItem(makeSeed("B"), 1);
    EXPECT_TRUE(inv.isFull());
}

TEST(InventoryTest, NotFullAfterSlotFreed) {
    Inventory inv(1);
    inv.addItem(makeSeed("A"), 1);
    inv.removeItem("A", 1);
    EXPECT_FALSE(inv.isFull());
}

// ── Inventory: getItems ───────────────────────────────────────────────────

TEST(InventoryTest, GetItemsReturnsAllPrototypes) {
    Inventory inv(10);
    inv.addItem(makeSeed("Carrot Seed"), 2);
    inv.addItem(makeSeed("Tomato Seed"), 3);
    EXPECT_EQ(inv.getItems().size(), 2u);
}

TEST(InventoryTest, GetItemsIsEmptyInitially) {
    Inventory inv(10);
    EXPECT_TRUE(inv.getItems().empty());
}

// ── Shop ─────────────────────────────────────────────────────────────────

TEST(ShopTest, GetAvailableItemsEmptyInitially) {
    Shop shop;
    EXPECT_TRUE(shop.getAvailableItems().empty());
}

TEST(ShopTest, AddAvailableItemStoresItem) {
    Shop shop;
    shop.addAvailableItem(makeSeed("Carrot Seed"));
    EXPECT_EQ(shop.getAvailableItems().size(), 1u);
}

TEST(ShopTest, AddMultipleItemsStoresAll) {
    Shop shop;
    shop.addAvailableItem(makeSeed("A"));
    shop.addAvailableItem(makeSeed("B"));
    shop.addAvailableItem(makeSeed("C"));
    EXPECT_EQ(shop.getAvailableItems().size(), 3u);
}

TEST(ShopTest, AddNullptrIsIgnored) {
    Shop shop;
    shop.addAvailableItem(nullptr);  // should not crash or add
    EXPECT_TRUE(shop.getAvailableItems().empty());
}

TEST(ShopTest, ProcessPurchaseReturnsTrueForValidArgs) {
    Shop shop;
    auto seed = makeSeed("Carrot Seed");
    Player player;
    player.addSheckles(100.0); // Give player enough sheckles to buy the item
    EXPECT_TRUE(shop.processPurchase(seed.get(), &player));
}

TEST(ShopTest, ProcessPurchaseReturnsFalseForNullItem) {
    Shop shop;
    Player player;
    EXPECT_FALSE(shop.processPurchase(nullptr, &player));
}

TEST(ShopTest, ProcessPurchaseReturnsFalseForNullPlayer) {
    Shop shop;
    auto seed = makeSeed("Carrot Seed");
    EXPECT_FALSE(shop.processPurchase(seed.get(), nullptr));
}

TEST(ShopTest, ProcessSaleReturnsTrueForValidArgs) {
    Shop shop;
    Player player;
    EXPECT_TRUE(shop.processSale(makeSeed("Carrot Seed"), &player));
}

TEST(ShopTest, ProcessSaleReturnsFalseForNullItem) {
    Shop shop;
    Player player;
    EXPECT_FALSE(shop.processSale(nullptr, &player));
}

TEST(ShopTest, ProcessSaleReturnsFalseForNullPlayer) {
    Shop shop;
    EXPECT_FALSE(shop.processSale(makeSeed("Carrot Seed"), nullptr));
}
