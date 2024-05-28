#pragma once

namespace geo {

    struct Coordinates {
        double lat; // Широта
        double lng; // Долгота
    };

    double ComputeDistance(Coordinates from, Coordinates to);

    template <typename Bus>
    inline double ComputeCurvature(const Bus* bus, double length_fact) {
        double length(0.0);
        for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
            auto from = bus->stops[i];
            auto to = bus->stops[i + 1];
            length += ComputeDistance(from->coordinates, to->coordinates);
        }
        return (length_fact / length);
    }

}  // namespace geo


