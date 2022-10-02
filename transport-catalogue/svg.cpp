#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

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
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool is_first = true;
        out << "<polyline points=\""sv;
        for (const Point& point : points_) {
            if (is_first) {
                is_first = false;
                out << point.x << "," << point.y;
            }
            else {
                out << " "s << point.x << "," << point.y;
            }
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }


    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        position_.x = pos.x;
        position_.y = pos.y;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_.x = offset.x;
        offset_.y = offset.y;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    std::string Text::ParseData(std::string& text) {
        std::string result = text;
        int pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('&', pos) != ::std::string::npos) {
                pos = result.find('&');
                result.replace(pos, 1, "&amp;");
                ++pos;
            }
            else {
                break;
            }
        }
        while (result.find('<') != std::string::npos) {
            result.replace(result.find('<'), 1, "&lt;");
        }
        while (result.find('>') != std::string::npos) {
            result.replace(result.find('>'), 1, "&gt;");
        }
        while (result.find('\'') != std::string::npos) {
            result.replace(result.find('\''), 1, "&apos;");
        }
        while (result.find('\"') != std::string::npos) {
            result.replace(result.find('\"'), 1, "&quot;");
        }
        return result;
    }

    Text& Text::SetData(std::string data) {
        data_ = ParseData(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\" ";
        if (font_family_ != "") {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        if (font_weight_ != "") {
            out << "font-weight=\""sv << font_weight_ << "\""sv;;
        }
        RenderAttrs(context.out);
        out << ">"sv << data_ << "</text>";
    }

    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }


    void Document::Render(std::ostream& out) const {
        RenderContext context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        out << "<svg xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n";
        for (const auto& object : objects_) {
            object->Render(context);
        }
        out << "</svg>";
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:   out << "arcs"sv; return out;
        case StrokeLineJoin::BEVEL:   out << "bevel"sv; return out;
        case StrokeLineJoin::MITER: out << "miter"sv; return out;
        case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; return out;
        case StrokeLineJoin::ROUND: out << "round"sv; return out;
        }
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:   out << "butt"sv; return out;
        case StrokeLineCap::ROUND:   out << "round"sv; return out;
        case StrokeLineCap::SQUARE: out << "square"sv; return out;
        }
        return out;
    }
    void ColorPrinter::operator()(std::monostate) const {
        out << "none";
    }
    void ColorPrinter::operator()(std::string color) const {
        out << color;
    }
    void ColorPrinter::operator()(Rgb color) const {
        out << "rgb" << "(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green) << "," << static_cast<int>(color.blue) << ")";
    }
    void ColorPrinter::operator()(Rgba color) const {
        out << "rgba" << "(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green) << "," << static_cast<int>(color.blue) << "," << color.opacity << ")";
    }
    std::ostream& operator<<(std::ostream& out, const Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }
}  // namespace svg
