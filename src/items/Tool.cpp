#include "Tool.h"

#include <utility>

Tool::Tool(std::size_t id,
		   std::string name,
		   std::string description,
		   double basePrice,
		   int durability)
	: Item(id, std::move(name), std::move(description), basePrice),
	  durability_(durability) {}

double Tool::getPrice() const
{
	return basePrice_;
}

int Tool::getDurability() const
{
	return durability_;
}
