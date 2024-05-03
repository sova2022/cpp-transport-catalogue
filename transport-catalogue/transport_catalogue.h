#pragma once

#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace detail {
    
struct Stop {
  std::string stop_name;
  Coordinates coordinates;
};

struct Bus {
  Bus(const std::string& bus_name);
  std::string bus_name;
  std::vector<Stop*> stops;
};

struct Route {
  size_t stops;
  size_t unique_stops;
  double leight;
};

} // namespace detail

class TransportCatalogue {

using Stops = typename std::unordered_map<std::string_view, detail::Stop*>;
using Buses = typename std::unordered_map<std::string_view, detail::Bus*>;
using StopsToBuses = typename std::unordered_map<detail::Stop*, std::set<std::string_view>>;
    
public:

const detail::Stop* FindStop(const std::string_view& stop_name) const;   
void AddStop(const std::string& stop, const Coordinates& coordinates);
const detail::Bus* FindBus(const std::string_view& bus) const;
void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops);
detail::Route GetRouteInfo(const std::string_view& bus) const;
const std::set<std::string_view> FindStopsToBuses(const std::string_view& stop) const;
  
private:
    std::deque<detail::Stop> stops_;
    std::deque<detail::Bus> buses_;
    Stops stopname_to_stop_;
    Buses busname_to_bus_;
    StopsToBuses stops_to_buses_;
};
