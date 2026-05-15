#pragma once

#include <string>

#include "Item.h"

class Cell;
class Player;

class Tool : public Item {
protected:
	int durability_;
	int maxDurability_;

protected:
	Tool(std::size_t id,
		 std::string name,
		 std::string description,
		 double basePrice,
		 int durability);
	~Tool() override = default;

public:
	double getPrice() const override;
	int getDurability() const;
	int getMaxDurability() const;
	bool isBroken() const;
	void resetDurability();
	void setDurability(int durability);

	virtual void use(Cell& cell, Player& player) = 0;

protected:
	bool consumeDurability(int amount = 1);
};
