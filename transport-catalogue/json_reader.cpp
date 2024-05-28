#include "json_reader.h"

inline std::set<std::string> SortBuses(const std::unordered_set<Bus*>* buses) {
    std::set<std::string> sorted;
    for (auto bus : *buses) sorted.emplace(bus->bus_name);
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
        return document_->GetRoot().AsMap().at("base_requests").AsArray();
    }

    const std::vector<Node>& JsonReader::GetStatRequests() const {
        return document_->GetRoot().AsMap().at("stat_requests").AsArray();
    }

    const Dict& JsonReader::GetRenderSetting() const {
        return document_->GetRoot().AsMap().at("render_settings").AsMap();
    }

    void JsonReader::AddStopsDataToCatalogue() const {
        for (auto node : GetBaseRequests()) {
            if (node.AsMap().at("type").AsString() == "Stop") {

                std::string name = node.AsMap().at("name").AsString();
                double latitude  = node.AsMap().at("latitude").AsDouble();
                double longitude = node.AsMap().at("longitude").AsDouble();

                const_cast<TransportCatalogue&>(handler_->GetDataBase())
                                                         .AddStop(name, { latitude , longitude });

            }
        }
        for (auto node : GetBaseRequests()) {
            if (node.AsMap().at("type").AsString() == "Stop") {
                if (!node.AsMap().at("road_distances").AsMap().empty()) {
                    std::string name = node.AsMap().at("name").AsString();
                    for (auto stop : node.AsMap().at("road_distances").AsMap()) {                      
                        std::string to_stop = stop.first;
                        Distance dist(node.AsMap().at("road_distances").AsMap().at(to_stop).AsInt());
                        const_cast<TransportCatalogue&>(handler_->GetDataBase())
                            .SetDistanceBetweenStops(name, to_stop, dist);
                    }
                }

            }
        }
    }

    void JsonReader::AddBusesDataToCatalogue() const {
        for (auto node : GetBaseRequests()) {
            if (node.AsMap().at("type").AsString() == "Bus") {
                std::string name = node.AsMap().at("name").AsString();
                std::vector<std::string> stops;
                auto stops_node = node.AsMap().at("stops").AsArray();           
                for (auto stop : stops_node) {
                    stops.emplace_back(stop.AsString());
                }
                TypeRoute type = { node.AsMap().at("is_roundtrip").AsBool(), handler_->GetDataBase().FindStop(stops.back()) };
                if (!node.AsMap().at("is_roundtrip").AsBool()) {
                    std::vector<std::string> temp = stops;
                    stops.insert(stops.end(), std::next(temp.rbegin()), temp.rend());
                    
                }
                const_cast<TransportCatalogue&>(handler_->GetDataBase())
                                                          .AddBus(name, stops, type);
            }
        }
    }
    
    svg::ColorType JsonReader::ParseColorType(Node value) const {
        if (value.IsString()) {
            return svg::ColorType::STRING;
        }
        else if (value.IsArray()) {
            if (value.AsArray().size() == 3) {
                return svg::ColorType::RGB;
            }
            else if (value.AsArray().size() == 4) {
                return svg::ColorType::RGBA;
            }
        }     
        return svg::ColorType::NONE;
    }
    
#define UINT(expr) static_cast<uint8_t>(expr)

    void JsonReader::ParseRenderSettings(renderer::MapRenderer& renderer) const {
        Dict dict = GetRenderSetting();
        renderer.width                = dict.at("width").AsDouble();
        renderer.height               = dict.at("height").AsDouble();
        renderer.padding              = dict.at("padding").AsDouble();
        renderer.line_width           = dict.at("line_width").AsDouble();
        renderer.stop_radius          = dict.at("stop_radius").AsDouble();
        renderer.bus_label_font_size  = dict.at("bus_label_font_size").AsInt();
        renderer.stop_label_font_size = dict.at("stop_label_font_size").AsInt();
        renderer.underlayer_width     = dict.at("underlayer_width").AsDouble();

        renderer.bus_label_offset     = { dict.at("bus_label_offset").AsArray()[0].AsDouble()
                                        , dict.at("bus_label_offset").AsArray()[1].AsDouble() }; 
        renderer.stop_label_offset    = { dict.at("stop_label_offset").AsArray()[0].AsDouble()
                                        , dict.at("stop_label_offset").AsArray()[1].AsDouble() };
  
        switch (ParseColorType(dict.at("underlayer_color"))) {
            case svg::ColorType::STRING: {
                renderer.underlayer_color = dict.at("underlayer_color").AsString();
                break;
            }
            case svg::ColorType::RGB: {
                const auto& color_array   = dict.at("underlayer_color").AsArray();
                renderer.underlayer_color = svg::Rgb{ UINT(color_array[0].AsInt()), UINT(color_array[1].AsInt())
                                                    , UINT(color_array[2].AsInt()) };
                break;
            }
            case svg::ColorType::RGBA: {
                const auto& color_array   = dict.at("underlayer_color").AsArray();
                renderer.underlayer_color = svg::Rgba{ UINT(color_array[0].AsInt()), UINT(color_array[1].AsInt())
                                                     , UINT(color_array[2].AsInt()), color_array[3].AsDouble() };
                break;
            }
            case svg::ColorType::NONE:
                renderer.underlayer_color = svg::NoneColor;
        }
        const auto& color_palette = dict.at("color_palette").AsArray();
        for (const auto& color : color_palette) {
            switch (ParseColorType(color)) {
                case svg::ColorType::STRING: {
                    renderer.color_palette.push_back(color.AsString());
                    break;
                }
                case svg::ColorType::RGB: {
                    const auto& color_array = color.AsArray();
                    renderer.color_palette.emplace_back(svg::Rgb{ UINT(color_array[0].AsInt()), UINT(color_array[1].AsInt())
                                                             , UINT(color_array[2].AsInt()) });
                    break;
                }
                case svg::ColorType::RGBA: {
                    const auto& color_array = color.AsArray();
                    renderer.color_palette.emplace_back(svg::Rgba{ UINT(color_array[0].AsInt()), UINT(color_array[1].AsInt())
                                                              , UINT(color_array[2].AsInt()), color_array[3].AsDouble() });
                    break;
                }
                case svg::ColorType::NONE: {
                    renderer.color_palette.push_back(svg::NoneColor);
                    break;
                }
            }
        }
        
    }

    Node JsonReader::GetStatForBusRequest(const std::string_view name, int request_id) {
        Dict answer;
        auto stat = handler_->GetBusStat(name);
        if (stat) {
            answer.emplace("curvature",         Node{ stat->curvature });
            answer.emplace("request_id",        Node{ request_id });
            answer.emplace("route_length",      Node{ stat->length });
            answer.emplace("stop_count",        Node{ static_cast<int>(stat->stops) });
            answer.emplace("unique_stop_count", Node{ static_cast<int>(stat->unique_stops) });
        } else {
            std::string error("not found");
            answer.emplace("error_message",     Node{ error });
            answer.emplace("request_id",        Node{ request_id });
        }      
        return Node{ answer };   
    }

    Node JsonReader::GetStatForStopRequest(const std::string_view name, int request_id) {
        Array buses;
        Dict answer;
        if (handler_->GetDataBase().FindStop(name) == nullptr) {
            std::string error("not found");
            answer.emplace("error_message", Node{ error });
            answer.emplace("request_id",    Node{ request_id });
            return Node{ answer };
        }
        auto buses_container = handler_->GetBusesByStop(name);
        for (auto bus_name : SortBuses(buses_container)) {
            buses.emplace_back(Node{ bus_name });
        }
        answer.emplace("buses",      Node{ buses });
        answer.emplace("request_id", Node{ request_id });
        return Node{ answer };       
    }
    
    Node JsonReader::GetMapScheme(RequestHandler& handler, int request_id) {
        Dict answer;
        std::ostringstream out;
        handler.RenderMap().Render(out);
        answer.emplace("request_id", Node{ request_id });
        answer.emplace("map", Node{ out.str() });
        return Node{ answer };
    }

    void JsonReader::ParseAndPrintStat([[maybe_unused]] RequestHandler& handler, std::ostream& out) {
        Array answer;
        for (auto node : GetStatRequests()) {
            int request_id = node.AsMap().at("id").AsInt();      
            if (node.AsMap().at("type").AsString() == "Bus") {
                const std::string_view name = node.AsMap().at("name").AsString();
                answer.emplace_back(GetStatForBusRequest(name, request_id));
            }
            if (node.AsMap().at("type").AsString() == "Stop") {
                const std::string_view name = node.AsMap().at("name").AsString();
                answer.emplace_back(GetStatForStopRequest(name, request_id));
            }
            if (node.AsMap().at("type").AsString() == "Map") {
                answer.emplace_back(GetMapScheme(handler, request_id));
            }
        }
        Print(Document{ answer }, out);
        
    }

} // namespace json