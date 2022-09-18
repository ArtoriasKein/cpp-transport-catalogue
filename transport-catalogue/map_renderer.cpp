#include "map_renderer.h"

namespace renderer {

    inline const double EPSILON = 1e-6;

    bool renderer::IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Polyline MapRenderer::MakeRoutePolilyne(std::vector<geo::Coordinates>& all_points, std::vector<geo::Coordinates>& current_points, int color_index) const {
        svg::Polyline result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        for (const auto& coordinate : current_points) {
            result.AddPoint(proj(coordinate));
        }
        return result.SetFillColor(svg::Color())
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeColor(settings_.color_palette[color_index]);
    }
    svg::Text MapRenderer::MakeBusUnderlayer(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view bus_name) const {
        svg::Text result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        return result.SetPosition(proj(coordinates))
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(bus_name))
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
    svg::Text MapRenderer::MakeBusName(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, int color_index, std::string_view bus_name) const {
        svg::Text result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        return result.SetPosition(proj(coordinates))
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(bus_name))
            .SetFillColor(settings_.color_palette[color_index]);
    }
    svg::Circle MapRenderer::MakeStop(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates) const {
        svg::Circle result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        return result.SetCenter(proj(coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white");
    }
    svg::Text MapRenderer::MakeStopUnderlayer(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view stop_name) const {
        svg::Text result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        return result.SetPosition(proj(coordinates))
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetData(std::string(stop_name));
    }
    svg::Text MapRenderer::MakeStopName(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view stop_name) const {
        svg::Text result;
        const SphereProjector proj(all_points.begin(), all_points.end(), settings_.width, settings_.height, settings_.padding);
        return result.SetPosition(proj(coordinates))
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(std::string(stop_name))
            .SetFillColor("black");
    }
    int MapRenderer::GetColorCapacity() const {
        return settings_.color_palette.size();
    }
    void MapRenderer::SetWidth(double width) {
        settings_.width = width;
    }
    void MapRenderer::SetHeight(double height) {
        settings_.height = height;
    }
    void MapRenderer::SetPadding(double padding) {
        settings_.padding = padding;
    }
    void MapRenderer::SetLineWidth(double line_width) {
        settings_.line_width = line_width;
    }
    void MapRenderer::SetStopRadius(double stop_radius) {
        settings_.stop_radius = stop_radius;
    }
    void MapRenderer::SetBusLabelFontSize(int bus_label_font_size) {
        settings_.bus_label_font_size = bus_label_font_size;
    }
    void MapRenderer::SetStopLabelFontSize(int stop_label_font_size) {
        settings_.stop_label_font_size = stop_label_font_size;
    }
    void MapRenderer::SetUnderlayerWidth(double underlayer_width) {
        settings_.underlayer_width = underlayer_width;
    }

} //namespace renderer
