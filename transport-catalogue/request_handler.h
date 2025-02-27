#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class RequestHandler {
public:  
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer,
        transport_router::TransportRouter& router);

    const TransportCatalogue& GetDataBase();
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    const std::unordered_set<Bus*>* GetBusesByStop(const std::string_view& stop_name) const;
    const renderer::SphereProjector GetProjector(const std::deque<Bus>* buses) const;
    transport_router::TransportRouter& GetRouter();
    svg::Document RenderMap() const;
    
private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    transport_router::TransportRouter& router_;
};

