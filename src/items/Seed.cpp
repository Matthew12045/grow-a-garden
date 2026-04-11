#include "Seed.h"

#include <utility>

Seed::Seed(std::size_t id,
		   std::string name,
		   std::string description,
		   double basePrice,
		   std::string cropType)
	: Item(id, std::move(name), std::move(description), basePrice),
	  cropType_(std::move(cropType)) {}

double Seed::getPrice() const
{
	return basePrice_;
}

const std::string& Seed::getCropType() const
{
	return cropType_;
}
