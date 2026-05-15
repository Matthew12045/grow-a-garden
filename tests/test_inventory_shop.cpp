#include <gtest/gtest.h>
#include "../src/systems/Inventory.h"
#include "../src/systems/Shop.h"
#include "../src/core/Player.h"
#include "../src/entities/Plant.h"
#include "../src/items/Seed.h"
#include "../src/items/Tool.h"
#include "../src/items/WateringCan.h"
#include "../src/world/Cell.h"
#include "helpers/ConcreteClasses.h"
#include <memory>
#include <string>

// ─── Helpers ──────────────────────────────────────────────────────────────

static std::unique_ptr<Seed> makeSeed(const std::string& name, double price = 5.0) {
    return std::make_unique<Seed>(1, name, "desc", price, "CropType");
}

class OneDurabilityTool : public Tool {
public:
    OneDurabilityTool()
        : Tool(99, "One Durability Tool", "Breaks after one valid use", 1.0, 1) {}

    std::unique_ptr<Item> clone() const override {
        return std::make_unique<OneDurabilityTool>(*this);
    }

    void use(Cell& cell, Player& /*player*/) override {
        if (isBroken() || cell.getPlant() == nullptr) {
            return;
        }

        consumeDurability();
    }
};

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

TEST(InventoryTest, GetItemPrototypeFindsOwnedPrototypeByName) {
    Inventory inv(10);
    ASSERT_TRUE(inv.addItem(std::make_unique<WateringCan>(), 1));

    Item* item = inv.getItemPrototype("Watering Can");

    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getName(), "Watering Can");
    EXPECT_NE(dynamic_cast<Tool*>(item), nullptr);
}

TEST(InventoryToolUseTest, SuccessfulUseLeavesToolQuantityUnchanged) {
    Player player;
    Inventory& inv = player.getInventory();
    Cell cell;
    ASSERT_TRUE(cell.setPlant(std::make_unique<Plant>(
        1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    ASSERT_TRUE(inv.addItem(std::make_unique<WateringCan>(), 1));

    Tool* tool = dynamic_cast<Tool*>(inv.getItemPrototype("Watering Can"));
    ASSERT_NE(tool, nullptr);
    tool->use(cell, player);
    if (tool->isBroken()) {
        inv.removeItem("Watering Can", 1);
    }

    EXPECT_EQ(inv.getQuantity("Watering Can"), 1);
    tool = dynamic_cast<Tool*>(inv.getItemPrototype("Watering Can"));
    ASSERT_NE(tool, nullptr);
    EXPECT_EQ(tool->getDurability(), 49);
}

TEST(InventoryToolUseTest, BrokenSingleToolRemovesExactlyOneQuantity) {
    Player player;
    Inventory& inv = player.getInventory();
    Cell cell;
    ASSERT_TRUE(cell.setPlant(std::make_unique<Plant>(
        1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    ASSERT_TRUE(inv.addItem(std::make_unique<OneDurabilityTool>(), 1));

    Tool* tool = dynamic_cast<Tool*>(inv.getItemPrototype("One Durability Tool"));
    ASSERT_NE(tool, nullptr);
    tool->use(cell, player);
    if (tool->isBroken()) {
        inv.removeItem("One Durability Tool", 1);
    }

    EXPECT_EQ(inv.getQuantity("One Durability Tool"), 0);
    EXPECT_EQ(inv.getItemPrototype("One Durability Tool"), nullptr);
}

TEST(InventoryToolUseTest, BreakingOneToolInStackResetsNextToolDurability) {
    Player player;
    Inventory& inv = player.getInventory();
    Cell cell;
    ASSERT_TRUE(cell.setPlant(std::make_unique<Plant>(
        1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    ASSERT_TRUE(inv.addItem(std::make_unique<OneDurabilityTool>(), 2));

    Tool* tool = dynamic_cast<Tool*>(inv.getItemPrototype("One Durability Tool"));
    ASSERT_NE(tool, nullptr);
    tool->use(cell, player);
    ASSERT_TRUE(tool->isBroken());

    inv.removeItem("One Durability Tool", 1);
    Tool* nextTool = dynamic_cast<Tool*>(inv.getItemPrototype("One Durability Tool"));
    ASSERT_NE(nextTool, nullptr);
    nextTool->resetDurability();

    EXPECT_EQ(inv.getQuantity("One Durability Tool"), 1);
    EXPECT_EQ(nextTool->getDurability(), nextTool->getMaxDurability());
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
    EXPECT_FLOAT_EQ(player.getSheckles(), 95.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 1);
}

TEST(ShopTest, ProcessPurchaseLeavesShopItemAvailable) {
    Shop shop;
    shop.addAvailableItem(makeSeed("Carrot Seed", 10.0));
    Player player;
    player.addSheckles(100.0f);

    Item* shopItem = shop.getAvailableItems().front().get();
    ASSERT_NE(shopItem, nullptr);
    EXPECT_TRUE(shop.processPurchase(shopItem, &player));

    ASSERT_EQ(shop.getAvailableItems().size(), 1u);
    ASSERT_NE(shop.getAvailableItems().front(), nullptr);
    EXPECT_EQ(shop.getAvailableItems().front()->getName(), "Carrot Seed");
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 1);
}

TEST(ShopTest, ProcessPurchaseReturnsFalseWhenPlayerCannotAffordItem) {
    Shop shop;
    auto seed = makeSeed("Carrot Seed", 10.0);
    Player player;
    player.addSheckles(5.0f);

    EXPECT_FALSE(shop.processPurchase(seed.get(), &player));
    EXPECT_FLOAT_EQ(player.getSheckles(), 5.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 0);
}

TEST(ShopTest, ProcessPurchaseReturnsFalseWhenInventoryIsFull) {
    Shop shop;
    auto seed = makeSeed("Overflow Seed", 10.0);
    Player player;
    player.addSheckles(100.0f);

    for (int i = 0; i < 20; ++i) {
        ASSERT_TRUE(player.getInventory().addItem(makeSeed("Seed " + std::to_string(i)), 1));
    }

    EXPECT_FALSE(shop.processPurchase(seed.get(), &player));
    EXPECT_FLOAT_EQ(player.getSheckles(), 100.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Overflow Seed"), 0);
}

TEST(ShopTest, ProcessPurchaseStoresToolInInventory) {
    Shop shop;
    WateringCan can;
    Player player;
    player.addSheckles(100.0f);

    EXPECT_TRUE(shop.processPurchase(&can, &player));
    EXPECT_FLOAT_EQ(player.getSheckles(), 75.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Watering Can"), 1);
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
    EXPECT_FLOAT_EQ(player.getSheckles(), 5.0f);
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

TEST(PlayerShopTest, PlayerBuyAddsItemAndDeductsSheckles) {
    Shop shop;
    shop.addAvailableItem(makeSeed("Carrot Seed", 10.0));
    Player player;
    player.addSheckles(100.0f);

    ASSERT_TRUE(player.buy(shop.getAvailableItems().front().get(), &shop));
    EXPECT_FLOAT_EQ(player.getSheckles(), 90.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 1);
}

TEST(PlayerEconomyTest, AddShecklesRejectsNegativeAmount) {
    Player player;
    player.addSheckles(25.0f);
    player.addSheckles(-10.0f);

    EXPECT_FLOAT_EQ(player.getSheckles(), 25.0f);
}

TEST(PlayerEconomyTest, DeductShecklesRejectsNegativeAmount) {
    Player player;
    player.addSheckles(25.0f);

    EXPECT_FALSE(player.deductSheckles(-10.0f));
    EXPECT_FLOAT_EQ(player.getSheckles(), 25.0f);
}

TEST(PlayerShopTest, PlayerSellRemovesOwnedItemAndAddsSheckles) {
    Shop shop;
    auto seed = makeSeed("Carrot Seed", 10.0);
    Player player;
    ASSERT_TRUE(player.getInventory().addItem(seed->clone(), 2));

    EXPECT_TRUE(player.sell(seed.get(), &shop));
    EXPECT_FLOAT_EQ(player.getSheckles(), 10.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 1);
}

TEST(PlayerShopTest, PlayerSellFailsForUnownedItem) {
    Shop shop;
    auto seed = makeSeed("Carrot Seed", 10.0);
    Player player;

    EXPECT_FALSE(player.sell(seed.get(), &shop));
    EXPECT_FLOAT_EQ(player.getSheckles(), 0.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 0);
}
