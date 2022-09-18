#pragma once
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace renderer {

    bool IsZero(double value);

    class SphereProjector {
    public:
        SphereProjector() = default;
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct MapSettings {
        MapSettings() = default;
        double width = 0;
        double height = 0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset = { 0.0, 0.0 };
        int stop_label_font_size = 0;
        svg::Point stop_label_offset = { 0.0, 0.0 };
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        svg::Polyline MakeRoutePolilyne(std::vector<geo::Coordinates>& all_points, std::vector<geo::Coordinates>& current_points, int color_index) const;
        svg::Text MakeBusUnderlayer(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view bus_name) const;
        svg::Text MakeBusName(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, int color_index, std::string_view bus_name) const;
        svg::Circle MakeStop(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates) const;
        svg::Text MakeStopUnderlayer(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view stop_name) const;
        svg::Text MakeStopName(std::vector<geo::Coordinates>& all_points, geo::Coordinates coordinates, std::string_view stop_name) const;
        int GetColorCapacity() const;
        void SetWidth(double width);
        void SetHeight(double height);
        void SetPadding(double padding);
        void SetLineWidth(double line_width);
        void SetStopRadius(double stop_radius);
        void SetBusLabelFontSize(int bus_label_font_size);
        template <typename NodeContainer>
        void SetBusLabelOffset(const NodeContainer& bus_label_offset);
        void SetStopLabelFontSize(int stop_label_font_size);
        template <typename NodeContainer>
        void SetStopLabelOffset(const NodeContainer& stop_label_offset);
        template <typename NodeHolster>
        void SetUnderlayerColor(NodeHolster& node);
        void SetUnderlayerWidth(double underlayer_width);
        template <typename NodeHolster>
        void SetColorPalette(NodeHolster& node);
    private:
        MapSettings settings_;
        template <typename NodeHolster>
        svg::Color MakeColorOutOfNode(NodeHolster& node);
    };

    template <typename NodeHolster>
    svg::Color MapRenderer::MakeColorOutOfNode(NodeHolster& node) {
        if (node.IsString()) {
            return svg::Color(node.AsString());
        }
        else if (node.IsArray()) {
            if (node.AsArray().size() == 4) {
                return svg::Color(svg::Rgba(node.AsArray().at(0).AsInt(), node.AsArray().at(1).AsInt(), node.AsArray().at(2).AsInt(), node.AsArray().at(3).AsDouble()));
            }
            else {
                return svg::Color(svg::Rgb(node.AsArray().at(0).AsInt(), node.AsArray().at(1).AsInt(), node.AsArray().at(2).AsInt()));
            }
        }
        return svg::Color();
    }
    template <typename NodeHolster>
    void MapRenderer::SetColorPalette(NodeHolster& node) {
        for (const auto& color : node) {
            settings_.color_palette.push_back(MakeColorOutOfNode(color));
        }
    }
    template <typename NodeContainer>
    void MapRenderer::SetStopLabelOffset(const NodeContainer& stop_label_offset) {
        settings_.stop_label_offset.x = stop_label_offset[0].AsDouble();
        settings_.stop_label_offset.y = stop_label_offset[1].AsDouble();
    }
    template <typename NodeContainer>
    void MapRenderer::SetBusLabelOffset(const NodeContainer& bus_label_offset) {
        settings_.bus_label_offset.x = bus_label_offset[0].AsDouble();
        settings_.bus_label_offset.y = bus_label_offset[1].AsDouble();
    }
    template <typename NodeHolster>
    void MapRenderer::SetUnderlayerColor(NodeHolster& node) {
        settings_.underlayer_color = MakeColorOutOfNode(node);
    }

} // namespace renderer