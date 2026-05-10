#pragma once

#include "../items/Tool.h"

class FertilizerTool : public Tool {
public:
    static constexpr int GROWTH_BOOST_TICKS = 20;

    FertilizerTool();

    using Tool::getPrice;
    using Tool::getDurability;
    using Item::getName;
    using Item::getDescr;
    using Item::id;

    std::unique_ptr<Item> clone() const override;

    void use(Cell& cell, Player& player) override;
};
