#include "transport_catalogue.h"


Bus::Bus(std::string& bus_name)
    : bus_name(std::move(bus_name)) {
}

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);
    }
    return nullptr;
}

void TransportCatalogue::AddStop(std::string stop, Coordinates coordinates) {
    if (!stopname_to_stop_.count(stop)) {
        stops_.push_back({ std::move(stop), coordinates });
        stopname_to_stop_.insert({ stops_.back().stop_name, &stops_.back() });
        stops_to_buses_.insert({ &stops_.back(), {} });
    }
}

void TransportCatalogue::AddStopDistance(std::string from_stop, const std::vector<std::pair<std::string_view, Distance>>& to_stops) {
    auto current_stop = FindStop(from_stop);
    for (auto& [stop, dist] : to_stops) {
        auto stop_to = FindStop(stop);
        if (stop_to != nullptr) {
            StopPair pair_stops = { current_stop, stop_to };
            distances_.insert({ pair_stops, dist });
        }
    }
}

const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
    if (busname_to_bus_.count(bus)) {
        return busname_to_bus_.at(bus);
    }
    return nullptr;
}

void TransportCatalogue::AddBus(std::string bus_name, const std::vector<std::string_view>& stops) {
    Bus bus(bus_name);
    if (!busname_to_bus_.count(bus_name)) {
        for (auto& stop : stops) {
            bus.stops.push_back(const_cast<Stop*>(FindStop(stop)));
        }
        buses_.push_back(std::move(bus));
        std::string_view bus_ptr_name = buses_.back().bus_name;
        busname_to_bus_.insert({ bus_ptr_name, &buses_.back() });
        for (auto& stop_ptr : buses_.back().stops) {
            std::string_view bus_ptr_name = buses_.back().bus_name;
            if (stops_to_buses_.count(stop_ptr)) {
                stops_to_buses_.at(stop_ptr).insert(bus_ptr_name);
            }
        }
    }
}

const Distance TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    StopPair key = { from , to };
    StopPair rkey = { to , from };
    if (!distances_.count(key)) {
        return distances_.at(rkey);
    }
    return distances_.at(key);
}



std::optional<Route> TransportCatalogue::GetRouteInfo(std::string_view bus) const {
    auto bus_ptr = FindBus(bus);
    if (bus_ptr == nullptr) {
        return std::nullopt;
    }
    std::unordered_set<std::string_view> unique;
    std::vector<Coordinates> coordinates;
    for (auto& stop : bus_ptr->stops) {
        unique.insert(stop->stop_name);
        coordinates.push_back(stop->coordinates);
    }

    double lenght(0.0);
    for (size_t i = 0; i < bus_ptr->stops.size() - 1; ++i) {
        auto from = bus_ptr->stops[i];
        auto to = bus_ptr->stops[i + 1];
        lenght += GetDistance(from, to).meters;
    }

    double curvature(0.0);
    curvature += ComputeCurvature(bus_ptr, lenght);
    Route route = { bus_ptr->stops.size(), unique.size(), lenght, curvature };
    return std::make_optional(route);
}

std::set<std::string_view> TransportCatalogue::FindStopsToBuses(std::string_view stop) const {
    auto stop_ptr = const_cast<Stop*>(FindStop(stop));
    if (stops_to_buses_.count(stop_ptr)) {
        return stops_to_buses_.at(stop_ptr);
    }
    return {};
}

