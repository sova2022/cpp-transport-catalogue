#pragma once

#include <string>
#include <vector>

#include "geo.h"

struct Stop {
    std::string stop_name;
    geo::Coordinates coordinates;
};

using TypeRoute = std::pair<bool, const Stop*>;

struct Bus {
    Bus(std::string& bus_name);
    std::string bus_name;
    std::vector<Stop*> stops;
    TypeRoute is_roundtrip;
};

struct BusStat {
    size_t stops;
    size_t unique_stops;
    double length;
    double curvature;
};

struct Distance {
    Distance() = default;
    Distance(int value);
    int meters;
};

struct StopPairHasher {
    std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
};