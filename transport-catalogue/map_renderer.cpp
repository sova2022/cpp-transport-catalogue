#include "map_renderer.h"

namespace renderer {

    std::pair<svg::Text, svg::Text> MapRenderer::RenderTextLabels(svg::Point point, svg::Point offset, std::string data,
        svg::Color color, double font_size, std::string font_weight) const {

        svg::Text backlayer;
        backlayer.SetPosition(point)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight(font_weight)
            .SetData(data)
            .SetFillColor(underlayer_color)
            .SetStrokeColor(underlayer_color)
            .SetStrokeWidth(underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg::Text text;
        text.SetPosition(point)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight(font_weight)
            .SetData(data)
            .SetFillColor(color);
        return { backlayer, text };
    }

    void MapRenderer::RenderStopsSymbols(const std::map<std::string, Stop*> stops,
        const SphereProjector& projector, svg::Document& doc) const {

        for (auto [name, stop] : stops) {
            svg::Circle circle;
            circle.SetCenter(projector(stop->coordinates))
                  .SetRadius(stop_radius)
                  .SetFillColor("white");
            doc.Add(circle);
        }
    }

    void MapRenderer::RenderRouteLine(const Bus& bus, const SphereProjector& projector, svg::Document& doc,
        svg::Color route_color, std::map<std::string, Stop*>& unique_sort_stops) const {

        svg::Polyline polyline;
        polyline.SetStrokeColor(route_color)
                .SetStrokeWidth(line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetFillColor(svg::NoneColor);

        for (const auto& stop : bus.stops) {
            polyline.AddPoint(projector(stop->coordinates));
            unique_sort_stops[stop->stop_name] = stop;
        }
        doc.Add(polyline);
    }

} // namespace renderer