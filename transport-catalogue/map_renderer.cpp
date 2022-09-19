#include "map_renderer.h"
#include <set>

namespace renderer {

    inline const double EPSILON = 1e-6;

    bool renderer::IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Polyline MapRenderer::MakeRoutePolilyne(std::vector<geo::Coordinates>& current_points, int color_index) const {
        svg::Polyline result;
        for (const auto& coordinate : current_points) {
            result.AddPoint(proj(coordinate));
        }
        return result.SetFillColor(svg::Color())
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeColor(settings_.color_palette[color_index]);
    }
    svg::Text MapRenderer::MakeBusUnderlayer(geo::Coordinates coordinates, std::string_view bus_name) const {
        svg::Text result;
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
    svg::Text MapRenderer::MakeBusName(geo::Coordinates coordinates, int color_index, std::string_view bus_name) const {
        svg::Text result;
        return result.SetPosition(proj(coordinates))
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(bus_name))
            .SetFillColor(settings_.color_palette[color_index]);
    }
    svg::Circle MapRenderer::MakeStop(geo::Coordinates coordinates) const {
        svg::Circle result;
        return result.SetCenter(proj(coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white");
    }
    svg::Text MapRenderer::MakeStopUnderlayer(geo::Coordinates coordinates, std::string_view stop_name) const {
        svg::Text result;
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
    svg::Text MapRenderer::MakeStopName(geo::Coordinates coordinates, std::string_view stop_name) const {
        svg::Text result;
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

    void MapRenderer::SetShpereProjector(std::vector<geo::Coordinates> all_coordinates) {
        SphereProjector new_one(all_coordinates.begin(), all_coordinates.end(), settings_.width, settings_.height, settings_.padding);
        std::swap(proj, new_one);
    }

    void MapRenderer::RenderMap(request_handler::RequestHandler& handler, svg::Document& doc) {
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

} //namespace renderer
