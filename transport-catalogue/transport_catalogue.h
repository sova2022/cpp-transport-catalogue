#pragma once

#include <deque>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
  
struct Stop {
  std::string stop_name;
  Coordinates coordinates;
};

struct Bus {
  Bus(std::string& bus_name);
  std::string bus_name;
  std::vector<Stop*> stops;
};

struct Route {
  size_t stops;
  size_t unique_stops;
  double lenght;
  double curvature;
};

struct Distance {
    int meters;
};

struct StopPairHasher {
    std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
        const void* p1 = static_cast<const void*>(pair.first);
        const void* p2 = static_cast<const void*>(pair.second);
        return std::hash<const void*>{}(p1) ^ std::hash<const void*>{}(p2);
    }
};

class TransportCatalogue {

using Stops = typename std::unordered_map<std::string_view, Stop*>;
using Buses = typename std::unordered_map<std::string_view, Bus*>;
using StopsToBuses = typename std::unordered_map<Stop*, std::set<std::string_view>>;
using StopPair = std::pair<const Stop*, const Stop*>;
using Distances = std::unordered_map<StopPair, const Distance, StopPairHasher>;
    
public:
    const Stop* FindStop(std::string_view stop_name) const;   
    void AddStop(std::string stop, Coordinates coordinates);
    void SetDistanceBetweenStops(std::string_view from_stop, std::string_view to_stop, Distance distance);
    const Bus* FindBus(std::string_view bus) const;
    void AddBus(std::string bus_name, const std::vector<std::string_view>& stops);
    Distance GetDistance(const Stop* from, const Stop* to) const;
    std::optional<Route> GetRouteInfo(std::string_view bus) const;
    std::set<std::string_view> FindStopsToBuses(std::string_view stop) const;

  
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    Stops stopname_to_stop_;
    Buses busname_to_bus_;
    StopsToBuses stops_to_buses_;
    Distances distances_;
};
