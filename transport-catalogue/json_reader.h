#pragma once

#include <memory>
#include <sstream>

#include "../json/json_builder.h"
#include "request_handler.h"

namespace json {

    class JsonReader {

    public:
        JsonReader() = default;

        void LoadHandler(RequestHandler handler);
        void LoadJson(std::istream& input);
        const Array& GetBaseRequests()  const;
        const Array& GetStatRequests()  const;
        const Dict& GetRenderSetting()  const;
        const Dict& GetRoutingSetting() const;
        void AddStopsDataToCatalogue()  const;
        void AddBusesDataToCatalogue()  const;
        void AddRoutingSetting()        const;
        svg::Color HandlingColor(const Node& value) const;
        void ParseRenderSettings(renderer::MapRenderer& renderer) const;
        Node GetStatForBusRequest(const std::string_view name, int request_id);
        Node GetStatForStopRequest(const std::string_view name, int request_id);
        Node GetMapScheme(RequestHandler& handler, int request_id);
        Node GetRouteInfo(const std::string_view from, const std::string_view to, int request_id);
        void ParseAndPrintStat(RequestHandler& handler, std::ostream& out);

    private:
        std::unique_ptr<Document> document_;
        std::unique_ptr<RequestHandler> handler_;
    };

} //namespace json