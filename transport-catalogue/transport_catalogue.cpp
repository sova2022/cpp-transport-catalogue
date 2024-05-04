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

void TransportCatalogue::AddStop(std::string& stop, Coordinates coordinates) {  
    if (!stopname_to_stop_.count(stop)) {
        stops_.push_back({std::move(stop), coordinates});
        stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});       
        stops_to_buses_.insert({&stops_.back(), {}});
    }  
}

const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
    if (busname_to_bus_.count(bus)) {
        return busname_to_bus_.at(bus);
    }
    return nullptr;
}

void TransportCatalogue::AddBus(std::string& bus_name, const std::vector<std::string_view>& stops) {
    Bus bus(bus_name);
    if (!busname_to_bus_.count(bus_name)) {
        for (auto& stop : stops) {
            bus.stops.push_back(const_cast<Stop*>(FindStop(stop)));
        }
        buses_.push_back(std::move(bus));
        std::string_view bus_ptr_name = buses_.back().bus_name;
        busname_to_bus_.insert({bus_ptr_name, &buses_.back()});
        for (auto& stop : stops) {
            std::string_view bus_ptr_name = buses_.back().bus_name;
            auto stop_ptr = const_cast<Stop*>(FindStop(stop));
            if (stops_to_buses_.count(stop_ptr)) {             
                stops_to_buses_.at(stop_ptr).insert(bus_ptr_name);
            }
        }
    }   
}

Route TransportCatalogue::GetRouteInfo(std::string_view bus) const {
    if (FindBus(bus) == nullptr) {
        Route route;
        return route;
    }
    std::unordered_set<std::string_view> unique;
    std::vector<Coordinates> coordinates;
    for (auto& stop : FindBus(bus)->stops) {
        unique.insert(stop->stop_name);
        coordinates.push_back(stop->coordinates);
    }
    double lenght(0.0);
    for (size_t i = 0; i < coordinates.size() - 1; ++i){
            lenght += ComputeDistance(coordinates[i], coordinates[i + 1]);
    }
    return {FindBus(bus)->stops.size(), unique.size(), lenght};
}

std::set<std::string_view> TransportCatalogue::FindStopsToBuses(std::string_view stop) const {
    auto stop_ptr = const_cast<Stop*>(FindStop(stop));
    if(stops_to_buses_.count(stop_ptr)) {
        return stops_to_buses_.at(stop_ptr);
    }
    return {};
}
