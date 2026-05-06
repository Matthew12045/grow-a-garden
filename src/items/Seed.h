#pragma once

#include <string>

#include "Item.h"

class Seed : public Item {
private:
	std::string cropType_;

public:
	Seed(std::size_t id,
		 std::string name,
		 std::string description,
		 double basePrice,
		 std::string cropType);

	double getPrice() const override;
	const std::string& getCropType() const;
};
