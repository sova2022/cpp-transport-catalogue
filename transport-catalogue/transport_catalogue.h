#pragma once

#include <deque>
#include <iostream>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"
  
class TransportCatalogue {

using Stops = typename std::unordered_map<std::string_view, Stop*>;
using Buses = typename std::unordered_map<std::string_view, Bus*>;
using StopsToBuses = typename std::unordered_map<Stop*, std::unordered_set<Bus*>>;
using StopPair = std::pair<const Stop*, const Stop*>;
using Distances = std::unordered_map<StopPair, const Distance, StopPairHasher>;
    
public:
    const Stop* FindStop(std::string_view stop_name) const;   
    void AddStop(std::string stop, geo::Coordinates coordinates);
    void SetDistanceBetweenStops(std::string_view from_stop, std::string_view to_stop, Distance distance);
    const Bus* FindBus(std::string_view bus) const;
    void AddBus(std::string bus_name, const std::vector<std::string>& stops, TypeRoute type);
    Distance GetDistance(const Stop* from, const Stop* to) const;
    const std::unordered_set<Bus*>* FindBusesForStop(const std::string_view stop) const;
    const std::deque<Bus>& GetAllBuses() const;
    const Stops& GetAllStops() const;
    size_t GetAllStopsCount() const;
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    Stops stopname_to_stop_;
    Buses busname_to_bus_;
    StopsToBuses stops_to_buses_;
    Distances distances_;
};
