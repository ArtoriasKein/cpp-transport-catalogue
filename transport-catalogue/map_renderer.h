#pragma once
#include "geo.h"
#include "svg.h"
#include "request_handler.h"

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

    class MapRenderer {
    public:

        struct BusRender {
            std::string_view name;
            std::vector<std::string_view> route;
            int color_index = 0;
            bool is_rounded = false;
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

        void RenderMap(request_handler::RequestHandler& handler, svg::Document& doc);
        svg::Polyline MakeRoutePolilyne(std::vector<geo::Coordinates>& current_points, int color_index) const;
        svg::Text MakeBusUnderlayer(geo::Coordinates coordinates, std::string_view bus_name) const;
        svg::Text MakeBusName(geo::Coordinates coordinates, int color_index, std::string_view bus_name) const;
        svg::Circle MakeStop(geo::Coordinates coordinates) const;
        svg::Text MakeStopUnderlayer(geo::Coordinates coordinates, std::string_view stop_name) const;
        svg::Text MakeStopName(geo::Coordinates coordinates, std::string_view stop_name) const;
        int GetColorCapacity() const;
        void SetWidth(double width);
        void SetHeight(double height);
        void SetPadding(double padding);
        void SetLineWidth(double line_width);
        void SetStopRadius(double stop_radius);
        void SetBusLabelFontSize(int bus_label_font_size);
        template <typename OffsetNodeType>
        void SetBusLabelOffset(const OffsetNodeType& bus_label_offset);
        void SetStopLabelFontSize(int stop_label_font_size);
        template <typename OffsetNodeType>
        void SetStopLabelOffset(const OffsetNodeType& stop_label_offset);
        template <typename ColorNodeType>
        void SetUnderlayerColor(ColorNodeType& node);
        void SetUnderlayerWidth(double underlayer_width);
        template <typename ColorNodeType>
        void SetColorPalette(ColorNodeType& node);
        void SetShpereProjector(std::vector<geo::Coordinates> all_coordinates);
    private:
        MapSettings settings_;
        SphereProjector proj;
        template <typename ColorNodeType>
        svg::Color MakeColorOutOfNode(ColorNodeType& node);
    };

    template <typename ColorNodeType>
    svg::Color MapRenderer::MakeColorOutOfNode(ColorNodeType& node) {
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
    template <typename ColorNodeType>
    void MapRenderer::SetColorPalette(ColorNodeType& node) {
        for (const auto& color : node) {
            settings_.color_palette.push_back(MakeColorOutOfNode(color));
        }
    }
    template <typename OffsetNodeType>
    void MapRenderer::SetStopLabelOffset(const OffsetNodeType& stop_label_offset) {
        settings_.stop_label_offset.x = stop_label_offset[0].AsDouble();
        settings_.stop_label_offset.y = stop_label_offset[1].AsDouble();
    }
    template <typename OffsetNodeType>
    void MapRenderer::SetBusLabelOffset(const OffsetNodeType& bus_label_offset) {
        settings_.bus_label_offset.x = bus_label_offset[0].AsDouble();
        settings_.bus_label_offset.y = bus_label_offset[1].AsDouble();
    }
    template <typename ColorNodeType>
    void MapRenderer::SetUnderlayerColor(ColorNodeType& node) {
        settings_.underlayer_color = MakeColorOutOfNode(node);
    }

} // namespace renderer
