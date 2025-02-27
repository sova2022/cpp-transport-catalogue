#include "json_reader.h"

inline std::set<std::string> SortBuses(const std::unordered_set<Bus*>* buses) {
    std::set<std::string> sorted;
    for (Bus* bus : *buses) sorted.emplace(bus->bus_name);
    return sorted;
}

namespace json {
    void JsonReader::LoadHandler(RequestHandler handler) {
        handler_ = std::move(std::make_unique<RequestHandler>(handler));
    }

    void JsonReader::LoadJson(std::istream& input) {
        document_ = std::move(std::make_unique<Document>(Load(input)));
    }

    const std::vector<Node>& JsonReader::GetBaseRequests() const {
        return document_->GetRoot().AsDict().at("base_requests").AsArray();
    }

    const std::vector<Node>& JsonReader::GetStatRequests() const {
        return document_->GetRoot().AsDict().at("stat_requests").AsArray();
    }

    const Dict& JsonReader::GetRenderSetting() const {
        return document_->GetRoot().AsDict().at("render_settings").AsDict();
    }

    const Dict& JsonReader::GetRoutingSetting() const {
        return document_->GetRoot().AsDict().at("routing_settings").AsDict();
    }

    void JsonReader::AddStopsDataToCatalogue() const {
        for (const Node& node : GetBaseRequests()) {
            if (node.AsDict().at("type").AsString() == "Stop") {

                std::string name = node.AsDict().at("name").AsString();
                double latitude = node.AsDict().at("latitude").AsDouble();
                double longitude = node.AsDict().at("longitude").AsDouble();

                const_cast<TransportCatalogue&>(handler_->GetDataBase())
                    .AddStop(name, { latitude , longitude });

            }
        }
        for (const Node& node : GetBaseRequests()) {
            if (node.AsDict().at("type").AsString() == "Stop") {
                if (!node.AsDict().at("road_distances").AsDict().empty()) {
                    std::string name = node.AsDict().at("name").AsString();
                    for (const std::pair<const std::string, const Node>& stop : node.AsDict().at("road_distances").AsDict()) {
                        std::string to_stop = stop.first;
                        Distance dist(node.AsDict().at("road_distances").AsDict().at(to_stop).AsInt());
                        const_cast<TransportCatalogue&>(handler_->GetDataBase())
                            .SetDistanceBetweenStops(name, to_stop, dist);
                    }
                }

            }
        }
    }

    void JsonReader::AddBusesDataToCatalogue() const {
        for (const Node& node : GetBaseRequests()) {
            if (node.AsDict().at("type").AsString() == "Bus") {
                std::string name = node.AsDict().at("name").AsString();
                std::vector<std::string> stops;
                auto stops_node = node.AsDict().at("stops").AsArray();
                for (const Node& stop : stops_node) {
                    stops.emplace_back(stop.AsString());
                }
                TypeRoute type = { node.AsDict().at("is_roundtrip").AsBool(), handler_->GetDataBase().FindStop(stops.back()) };
                if (!node.AsDict().at("is_roundtrip").AsBool()) {
                    std::vector<std::string> temp = stops;
                    stops.insert(stops.end(), std::next(temp.rbegin()), temp.rend());

                }
                const_cast<TransportCatalogue&>(handler_->GetDataBase())
                    .AddBus(name, stops, type);
            }
        }
    }

    void JsonReader::AddRoutingSetting() const {
        RoutingSettings settings(GetRoutingSetting().at("bus_wait_time").AsDouble(), GetRoutingSetting().at("bus_velocity").AsDouble());
        const_cast<transport_router::TransportRouter&>(handler_->GetRouter()).SetRoutingSettings(settings);
    }

    svg::Color JsonReader::HandlingColor(const Node& value) const {
        if (value.IsString()) {
            return value.AsString();
        }
        if (value.IsArray()) {
            if (value.AsArray().size() == 3) {

                return svg::Rgb{ static_cast<uint8_t>(value.AsArray()[0].AsInt())
                               , static_cast<uint8_t>(value.AsArray()[1].AsInt())
                               , static_cast<uint8_t>(value.AsArray()[2].AsInt()) };;
            }
            if (value.AsArray().size() == 4) {
                return svg::Rgba{ static_cast<uint8_t>(value.AsArray()[0].AsInt())
                                , static_cast<uint8_t>(value.AsArray()[1].AsInt())
                                , static_cast<uint8_t>(value.AsArray()[2].AsInt())
                                , value.AsArray()[3].AsDouble() };
            }
        }
        return svg::NoneColor;
    }

    void JsonReader::ParseRenderSettings(renderer::MapRenderer& renderer) const {
        Dict dict = GetRenderSetting();
        renderer.width = dict.at("width").AsDouble();
        renderer.height = dict.at("height").AsDouble();
        renderer.padding = dict.at("padding").AsDouble();
        renderer.line_width = dict.at("line_width").AsDouble();
        renderer.stop_radius = dict.at("stop_radius").AsDouble();
        renderer.bus_label_font_size = dict.at("bus_label_font_size").AsInt();
        renderer.stop_label_font_size = dict.at("stop_label_font_size").AsInt();
        renderer.underlayer_width = dict.at("underlayer_width").AsDouble();

        renderer.bus_label_offset = { dict.at("bus_label_offset").AsArray()[0].AsDouble()
                                        , dict.at("bus_label_offset").AsArray()[1].AsDouble() };
        renderer.stop_label_offset = { dict.at("stop_label_offset").AsArray()[0].AsDouble()
                                        , dict.at("stop_label_offset").AsArray()[1].AsDouble() };

        renderer.underlayer_color = HandlingColor(dict.at("underlayer_color"));
        const auto& color_palette = dict.at("color_palette").AsArray();
        for (const Node& color : color_palette) {
            renderer.color_palette.emplace_back(HandlingColor(color));
        }
    }

    Node JsonReader::GetStatForBusRequest(const std::string_view name, int request_id) {
        Builder answer;
        answer.StartDict();
        auto stat = handler_->GetBusStat(name);
        if (stat) {
            answer.Key("curvature").Value(stat->curvature)
                .Key("request_id").Value(request_id)
                .Key("route_length").Value(stat->length)
                .Key("stop_count").Value(static_cast<int>(stat->stops))
                .Key("unique_stop_count").Value(static_cast<int>(stat->unique_stops));
        }
        else {
            std::string error("not found");
            answer.Key("error_message").Value(error)
                .Key("request_id").Value(request_id);
        }
        answer.EndDict();
        return answer.Build();
    }

    Node JsonReader::GetStatForStopRequest(const std::string_view name, int request_id) {
        Array buses;
        Builder answer;
        answer.StartDict();
        if (handler_->GetDataBase().FindStop(name) == nullptr) {
            std::string error("not found");
            answer.Key("error_message").Value(error)
                .Key("request_id").Value(request_id)
                .EndDict();
            return answer.Build();
        }
        auto buses_container = handler_->GetBusesByStop(name);
        for (std::string bus_name : SortBuses(buses_container)) {
            buses.emplace_back(Node{ bus_name });
        }
        answer.Key("buses").Value(buses)
            .Key("request_id").Value(request_id)
            .EndDict();
        return answer.Build();
    }

    Node JsonReader::GetMapScheme(RequestHandler& handler, int request_id) {
        Builder answer;
        std::ostringstream out;
        handler.RenderMap().Render(out);
        answer.StartDict()
            .Key("request_id").Value(request_id)
            .Key("map").Value(out.str())
            .EndDict();
        return answer.Build();
    }

    Node JsonReader::GetRouteInfo(const std::string_view from, const std::string_view to, int request_id) {
        Builder answer;
        transport_router::RouteData route_data = handler_->GetRouter().CalculateRoute(from, to);

        if (!route_data.founded) {
            answer.StartDict()
                .Key("request_id").Value(request_id)
                .Key("error_message").Value("not found")
                .EndDict();
            return answer.Build();
        }

        Array items;
        for (const transport_router::RouteItem& item : route_data.items) {
            Dict items_map;
            if (item.type == EdgeType::TRAVEL) {
                items_map["type"] = std::string("Bus");
                items_map["bus"] = item.edge_name;
                items_map["span_count"] = item.span_count;
            }
            else if (item.type == EdgeType::WAIT) {
                items_map["type"] = std::string("Wait");
                items_map["stop_name"] = item.edge_name;
            }
            items_map["time"] = item.time;
            items.push_back(items_map);
        }
        answer.StartDict()
            .Key("request_id").Value(request_id)
            .Key("total_time").Value(route_data.total_time)
            .Key("items").Value(items)
            .EndDict();
        return answer.Build();
    }

    void JsonReader::ParseAndPrintStat(RequestHandler& handler, std::ostream& out) {
        Builder answer;
        answer.StartArray();
        for (const Node& node : GetStatRequests()) {
            int request_id = node.AsDict().at("id").AsInt();
            if (node.AsDict().at("type").AsString() == "Bus") {
                const std::string_view name = node.AsDict().at("name").AsString();
                answer.Value(GetStatForBusRequest(name, request_id).GetValue());
            }
            if (node.AsDict().at("type").AsString() == "Stop") {
                const std::string_view name = node.AsDict().at("name").AsString();
                answer.Value(GetStatForStopRequest(name, request_id).GetValue());
            }
            if (node.AsDict().at("type").AsString() == "Map") {
                answer.Value(GetMapScheme(handler, request_id).GetValue());
            }
            if (node.AsDict().at("type").AsString() == "Route") {
                answer.Value(GetRouteInfo(node.AsDict().at("from").AsString(),
                    node.AsDict().at("to").AsString(), request_id).GetValue());
            }

        }
        answer.EndArray();
        Print(Document{ answer.Build() }, out);
    }

} // namespace json