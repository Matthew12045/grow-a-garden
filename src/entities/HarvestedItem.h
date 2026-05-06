#pragma once

#include <string>
#include <vector>

#include "../items/Item.h"
#include "Mutation.h"

class HarvestedItem : public Item {
private:
	std::vector<MutationType> mutations_;
	double finalSellPrice_;

public:
	HarvestedItem();
	HarvestedItem(double finalSellPrice, std::vector<MutationType> mutations);

	double getPrice() const override;
	const std::vector<MutationType>& getMutationList() const;
	std::string getMutations() const;
};
