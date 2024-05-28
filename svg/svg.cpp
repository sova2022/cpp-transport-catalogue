#include "svg.h"

namespace svg {

    using namespace std::literals;

    #define INT(expr) static_cast<int>(expr)

    std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
        out << "rgb(" << INT(rgb.red)  << "," << INT(rgb.green);
        out << ","    << INT(rgb.blue) << ")";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Rgba& rgba) {
        out << "rgba(" << INT(rgba.red)  << "," << INT(rgba.green);
        out << ","     << INT(rgba.blue) << "," << rgba.opacity << ")";
        return out;
    }

    #define ALT std::holds_alternative

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        if (ALT<std::monostate>(color)) out << "none";
        if (ALT<std::string>(color))    out << std::get<std::string>(color);
        if (ALT<Rgb>(color))            out << std::get<Rgb>(color);
        if (ALT<Rgba>(color))           out << std::get<Rgba>(color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
        switch (cap) {
        case StrokeLineCap::BUTT:   return out << "butt";
        case StrokeLineCap::ROUND:  return out << "round";
        case StrokeLineCap::SQUARE: return out << "square";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
        switch (join) {
        case StrokeLineJoin::ARCS:       return out << "arcs";
        case StrokeLineJoin::BEVEL:      return out << "bevel";
        case StrokeLineJoin::MITER:      return out << "miter";
        case StrokeLineJoin::MITER_CLIP: return out << "miter-clip";
        case StrokeLineJoin::ROUND:      return out << "round";
        }
        return out;
    }

    void HtmlEncodeString(std::ostream& out, std::string_view data) {
        for (char c : data) {
            switch (c) {
            case '"':
                out << "&quot;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            default:
                out.put(c);
            }
        }
        out << "</text>"sv;
    }

    // RenderContext

    RenderContext::RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    // Circle

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // Polyline

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_first = true;
        std::string delim;
        for (auto point : points_) {
            is_first ? delim = "" : delim = " ";
            out << delim << point.x << ","sv << point.y;
            is_first = false;
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // Text

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv    << position_.x << "\" y=\""sv << position_.y;
        out << "\" dx=\""sv << offset_.x   << "\" dy=\""sv << offset_.y;
        out << "\" font-size=\""sv << font_size_ << "\""sv;
        font_family_.empty() ? out << ""sv : out << " font-family=\""sv << font_family_ << "\""sv;
        font_weight_.empty() ? out << ""sv : out << " font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv;
        HtmlEncodeString(out, data_);
        
    }

    // Document

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        svg::RenderContext ctx(out, 2, 2);
        for (const auto& object : objects_) {
            object->Render(ctx);
        }
        out << "</svg>"sv << std::endl;
    }

}  // namespace svg