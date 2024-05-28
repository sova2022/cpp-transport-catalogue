#include <algorithm>

#include "request_handler.h"

#define UNCONST const_cast<std::deque<Bus>*>

inline void SortBusesInDeque(const std::deque<Bus>* buses) {
    std::sort(UNCONST(buses)->begin(), UNCONST(buses)->end(), 
        [](auto lhs, auto rhs) { return lhs.bus_name < rhs.bus_name; });
}

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
    : db_(db)
    , renderer_(renderer) {
}

const TransportCatalogue& RequestHandler::GetDataBase() {
    return db_; 
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    auto bus_ptr = db_.FindBus(bus_name);
    if (bus_ptr == nullptr) {
        return std::nullopt;
    }

    std::unordered_set<std::string_view> unique;
    for (Stop* stop : bus_ptr->stops) {
        unique.emplace(stop->stop_name);
    }

    double length(0.0);
    for (size_t i = 0; i < bus_ptr->stops.size() - 1; ++i) {
        auto from = bus_ptr->stops[i];
        auto to   = bus_ptr->stops[i + 1];
        length   += db_.GetDistance(from, to).meters;
    }

    double curvature(0.0);
    curvature += geo::ComputeCurvature(bus_ptr, length);
    BusStat stat = { bus_ptr->stops.size(), unique.size(), length, curvature };
    return std::make_optional(stat);
}

const std::unordered_set<Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.FindBusesForStop(stop_name);
}

const renderer::SphereProjector RequestHandler::GetProjector(const std::deque<Bus>* buses) const {
    std::vector<geo::Coordinates> coords;
    for (auto bus : *buses) {
        for (auto stop : bus.stops)
            coords.emplace_back(stop->coordinates);
    }
    renderer::SphereProjector projector(coords.begin(), coords.end()
                                       , renderer_.width, renderer_.height, renderer_.padding);
    return projector;
}

svg::Document RequestHandler::RenderMap() const {    
    svg::Document doc;
    std::vector<geo::Coordinates> coords;
    renderer::SphereProjector projector = GetProjector(db_.GetAllBuses());
    auto all_buses_ptr = db_.GetAllBuses();
    SortBusesInDeque(all_buses_ptr);
    std::vector<std::pair<svg::Text, svg::Text>> labels;
    std::map<std::string, Stop*> unique_sort_stops;
    int bus_number = 0;
    for (auto bus : *all_buses_ptr) {
        auto route_color = renderer_.color_palette[bus_number % renderer_.color_palette.size()];
        renderer_.RenderRouteLine(bus, projector, doc, route_color, unique_sort_stops);

        labels.emplace_back(renderer_.RenderTextLabels(projector(bus.stops.front()->coordinates),
            renderer_.bus_label_offset, bus.bus_name, route_color, renderer_.bus_label_font_size, "bold"));

        if (!bus.is_roundtrip.first && bus.stops.front() != bus.is_roundtrip.second) {
            labels.emplace_back(renderer_.RenderTextLabels(projector(bus.is_roundtrip.second->coordinates),
                renderer_.bus_label_offset, bus.bus_name, route_color, renderer_.bus_label_font_size, "bold"));
        }
        ++bus_number;
    }

    for (auto label : labels) {
        doc.Add(label.first);
        doc.Add(label.second);
    }

    renderer_.RenderStopsSymbols(unique_sort_stops, projector, doc);
    for (auto [name, stop] : unique_sort_stops) {
        auto label = renderer_.RenderTextLabels(projector(stop->coordinates),
            renderer_.stop_label_offset, name, "black", renderer_.stop_label_font_size, "");

        doc.Add(label.first);
        doc.Add(label.second);
    }


    return doc;
}

