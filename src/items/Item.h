#pragma once

#include <cstddef>
#include <string>

class Item {
protected:
    std::size_t id_;
    std::string name_;
    std::string description_;
    double basePrice_;

public:
    Item(std::size_t id, std::string name, std::string description, double basePrice);
    virtual ~Item() = default;

    virtual double getPrice() const = 0;

    const std::string& getName() const;
    const std::string& getDescr() const;
    std::size_t id() const;
};