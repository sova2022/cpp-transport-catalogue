#pragma once

#include <optional>
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

struct RoutingSettings {
    RoutingSettings() = default;
    RoutingSettings(double bus_wait_time, double bus_velocity);

    double bus_wait_time = 0.0;
    double bus_velocity  = 0.0;
};

struct StopPairHasher {
    std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
};