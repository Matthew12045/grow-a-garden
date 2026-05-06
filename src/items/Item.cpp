#include "Item.h"

#include <utility>

Item::Item(std::size_t id, std::string name, std::string description, double basePrice)
	: id_(id),
	  name_(std::move(name)),
	  description_(std::move(description)),
	  basePrice_(basePrice) {}

const std::string& Item::getName() const
{
	return name_;
}

const std::string& Item::getDescr() const
{
	return description_;
}

std::size_t Item::id() const
{
	return id_;
}
