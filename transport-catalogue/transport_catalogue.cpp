
#include "transport_catalogue.h"

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);
    }
    return nullptr;
}

void TransportCatalogue::AddStop(std::string stop, geo::Coordinates coordinates) {
    if (!stopname_to_stop_.count(stop)) {
        stops_.push_back({ std::move(stop), coordinates });
        stopname_to_stop_.insert({ stops_.back().stop_name, &stops_.back() });
        stops_to_buses_.insert({ &stops_.back(), {} });
    }
}

void TransportCatalogue::SetDistanceBetweenStops(std::string_view from_stop, std::string_view to_stop, Distance distance) {
    auto current_stop = FindStop(from_stop);
    auto stop_to = FindStop(to_stop);
    if (stop_to != nullptr) {
        StopPair pair_stops = {current_stop, stop_to};
        distances_.insert({pair_stops, distance});      
    }
}

const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
    if (busname_to_bus_.count(bus)) {
        return busname_to_bus_.at(bus);
    }
    return nullptr;
}

void TransportCatalogue::AddBus(std::string bus_name, const std::vector<std::string>& stops, TypeRoute type) {
    Bus bus(bus_name);
    if (!busname_to_bus_.count(bus_name)) {
        for (auto& stop : stops) {
            bus.stops.emplace_back(const_cast<Stop*>(FindStop(stop)));
        }
        bus.is_roundtrip = (std::move(type));
        buses_.push_back(std::move(bus));
        std::string_view bus_ptr_name = buses_.back().bus_name;
        busname_to_bus_.insert({ bus_ptr_name, &buses_.back() });
        for (auto& stop_ptr : buses_.back().stops) {
            std::string_view bus_ptr_name = buses_.back().bus_name;
            if (stops_to_buses_.count(stop_ptr)) {
                stops_to_buses_.at(stop_ptr).insert(busname_to_bus_.at(bus_ptr_name));
            }
        }
    }
}

Distance TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    StopPair key  = {from , to};
    StopPair rkey = {to , from};
    if (distances_.count(key))  return distances_.at(key);
    if (distances_.count(rkey)) return distances_.at(rkey);
    return Distance{ 0 };
}

const std::unordered_set<Bus*>* TransportCatalogue::FindBusesForStop(const std::string_view stop) const {
    auto stop_ptr = const_cast<Stop*>(FindStop(stop));
    if (stops_to_buses_.count(stop_ptr)) {
        return &stops_to_buses_.at(stop_ptr);
    }
    return nullptr;
}

const std::deque<Bus>* TransportCatalogue::GetAllBuses() const {
    return &buses_; 
}

