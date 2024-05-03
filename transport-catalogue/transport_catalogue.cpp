#include "transport_catalogue.h"

namespace detail {
    
Bus::Bus(const std::string& bus_name)
      : bus_name(std::move(bus_name)) {          
}

} // namespace detail

const detail::Stop* TransportCatalogue::FindStop(const std::string_view& stop_name) const {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);     
    } 
    return nullptr;
}

void TransportCatalogue::AddStop(const std::string& stop, const Coordinates& coordinates) {  
    if (!stopname_to_stop_.count(stop)) {
        stops_.push_back({std::move(stop), std::move(coordinates)});
        std::string_view stop_ptr_name = stops_.back().stop_name;
        stopname_to_stop_.insert({stop_ptr_name, std::move(&stops_.back())});       
        stops_to_buses_.insert({std::move(const_cast<detail::Stop*>(FindStop(stop))), {}});
    }  
}

const detail::Bus* TransportCatalogue::FindBus(const std::string_view& bus) const {
    if (busname_to_bus_.count(bus)) {
        return busname_to_bus_.at(bus);
    }
    return nullptr;
}

void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops) {
    detail::Bus bus(bus_name);
    if (!busname_to_bus_.count(bus_name)) {
        for (auto& stop : stops) {
            bus.stops.push_back(std::move(const_cast<detail::Stop*>(FindStop(stop))));
        }
        buses_.push_back(std::move(bus));
        std::string_view bus_ptr_name = buses_.back().bus_name;
        busname_to_bus_.insert({std::move(bus_ptr_name), std::move(&buses_.back())});
        for (auto& stop : stops) {
            std::string_view bus_ptr_name = buses_.back().bus_name;
            stops_to_buses_.at(const_cast<detail::Stop*>(FindStop(stop))).insert(std::move(bus_ptr_name));
        }
    }   
}

detail::Route TransportCatalogue::GetRouteInfo(const std::string_view& bus) const {
    std::unordered_set<std::string_view> unique;
    std::vector<Coordinates> coordinates;
    for (auto& stop : FindBus(bus)->stops) {
        unique.insert(stop->stop_name);
        coordinates.push_back(stop->coordinates);
    }
    double leight(0.0);
    for (size_t i = 0; i < coordinates.size() - 1; ++i){
            leight += ComputeDistance(coordinates[i], coordinates[i + 1]);
    }
    return {FindBus(bus)->stops.size(), unique.size(), leight};
}

const std::set<std::string_view> TransportCatalogue::FindStopsToBuses(const std::string_view& stop) const {
    if(stops_to_buses_.count(const_cast<detail::Stop*>(FindStop(stop)))) {
        return stops_to_buses_.at(const_cast<detail::Stop*>(FindStop(stop)));
    }
    return {};
}
