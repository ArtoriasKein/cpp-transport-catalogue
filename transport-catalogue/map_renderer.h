#pragma once
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>

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
        template <typename Request_Handler>
        void RenderMap(Request_Handler& handler, svg::Document& doc);
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
        void SetBusLabelOffset(const double x, const double y);
        void SetStopLabelFontSize(int stop_label_font_size);
        template <typename OffsetNodeType>
        void SetStopLabelOffset(const OffsetNodeType& stop_label_offset);
        void SetStopLabelOffset(const double x, const double y);
        void SetUnderlayerColor(const std::string& name);
        void SetUnderlayerColor(const svg::Rgb& rgb);
        void SetUnderlayerColor(const svg::Rgba& rgba);
        void SetUnderlayerColor(const svg::Color& color);
        void SetUnderlayerWidth(double underlayer_width);
        void SetColorPalette(std::vector<svg::Color>& color_palette);
        void AddColorToPalette(const std::string& name);
        void AddColorToPalette(const svg::Rgb& rgb);
        void AddColorToPalette(const svg::Rgba& rgba);
        void AddColorToPalette(const svg::Color& color);
        void SetShpereProjector(std::vector<geo::Coordinates> all_coordinates);
        const MapSettings GetSettings() const;
    private:
        MapSettings settings_;
        SphereProjector proj;
        template <typename ColorNodeType>
        svg::Color MakeColorOutOfNode(ColorNodeType& node);
    };

    template <typename Request_Handler>
    void MapRenderer::RenderMap(Request_Handler& handler, svg::Document& doc) {
        int color_index = 0;
        int color_capacity = GetColorCapacity();
        std::vector<BusRender> all_buses;
        std::set<std::string_view> all_stops;
        SetShpereProjector(handler.GetAllStopCoordinates());
        for (const std::string_view& bus_name : handler.GetAllBusesNames()) { //Проход по всем маршрутам и их отрисовка + заполнение массивов всех маршрутов и остановок для дальнейшей отрисовки
            BusRender new_bus;
            new_bus.name = bus_name;
            new_bus.color_index = color_index;
            std::vector<geo::Coordinates> current_stops_coordinates;
            auto stat = handler.GetBusStat(bus_name);
            if (stat.has_value()) {
                if (stat.value().is_rounded) {
                    new_bus.is_rounded = true;
                    for (const auto& stop_name : handler.GetBusRoute(bus_name)) {
                        new_bus.route.push_back(stop_name);
                        all_stops.insert(stop_name);
                        current_stops_coordinates.push_back(handler.GetStopCoordinates(stop_name));
                    }
                }
                else {
                    new_bus.is_rounded = false;
                    auto bus_stops = handler.GetBusRoute(bus_name);
                    for (const auto& stop_name : bus_stops) {
                        new_bus.route.push_back(stop_name);
                        all_stops.insert(stop_name);
                        current_stops_coordinates.push_back(handler.GetStopCoordinates(stop_name));
                    }
                    std::reverse(bus_stops.begin(), bus_stops.end());
                    bool is_first = true;
                    for (const auto& stop_name : bus_stops) {
                        if (is_first) {
                            is_first = false;
                            continue;
                        }
                        current_stops_coordinates.push_back(handler.GetStopCoordinates(stop_name));
                    }
                }
            }

            doc.Add(MakeRoutePolilyne(current_stops_coordinates, color_index));
            if (color_index + 1 == color_capacity) {
                color_index = 0;
            }
            else {
                ++color_index;
            }
            all_buses.push_back(new_bus);
        }
        for (const BusRender& bus : all_buses) { // Отрисовка названий маршрутов
            if (bus.is_rounded) {
                doc.Add(MakeBusUnderlayer(handler.GetStopCoordinates(*bus.route.begin()), bus.name));
                doc.Add(MakeBusName(handler.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
            }
            else {
                if (*(bus.route.begin()) == *(bus.route.end() - 1)) {
                    doc.Add(MakeBusUnderlayer(handler.GetStopCoordinates(*bus.route.begin()), bus.name));
                    doc.Add(MakeBusName(handler.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
                }
                else {
                    doc.Add(MakeBusUnderlayer(handler.GetStopCoordinates(*bus.route.begin()), bus.name));
                    doc.Add(MakeBusName(handler.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
                    doc.Add(MakeBusUnderlayer(handler.GetStopCoordinates(*(bus.route.end() - 1)), bus.name));
                    doc.Add(MakeBusName(handler.GetStopCoordinates(*(bus.route.end() - 1)), bus.color_index, bus.name));
                }
            }
        }
        for (const std::string_view& stop_name : all_stops) { // Отрисовка кружков остановок
            doc.Add(MakeStop(handler.GetStopCoordinates(stop_name)));
        }
        for (const std::string_view& stop_name : all_stops) { // Отрисовка названий остановок
            doc.Add(MakeStopUnderlayer(handler.GetStopCoordinates(stop_name), stop_name));
            doc.Add(MakeStopName(handler.GetStopCoordinates(stop_name), stop_name));
        }
    }

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

} // namespace renderer
