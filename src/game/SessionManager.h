#pragma once

#include <vector>

#include "../core/Game.h"
#include "HarvestBasket.h"
#include "ShopData.h"

class SessionManager {
public:
    SessionManager(Game& game,
                   std::vector<BasketEntry>& harvestBasket,
                   const std::vector<ShopItemDef>& catalogue);
    ~SessionManager();

    void initOrLoad();
    void save();

    bool needsStarterRescue() const;
    void addStartingItems();

private:
    bool isInitializedSave() const;
    void loadHarvestBasket();

    Game& game_;
    std::vector<BasketEntry>& harvestBasket_;
    const std::vector<ShopItemDef>& catalogue_;

    static constexpr const char* SAVE_FILE = "save.json";
};
