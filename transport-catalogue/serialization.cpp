#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <fstream>
#include <sstream>
#include "serialization.h"

void Serialization::SaveStops(transport_catalogue::TransportCatalogue& tc_) {
    for (const auto& stop_name : tc_.GetAllStopsNames()) {
        auto stop = tc_.GetStopByName(stop_name);
        transport::Stop add_stop;
        add_stop.set_name(stop.name);
        add_stop.mutable_coordinates()->set_lat(stop.coordinates.lat);
        add_stop.mutable_coordinates()->set_lng(stop.coordinates.lng);
        *catalogue_.add_stops() = std::move(add_stop);
    }
}

void Serialization::SaveStopsDistances(transport_catalogue::TransportCatalogue& tc_) {
    int i = 0;
    for (const auto& [stop_to_stop, distance] : tc_.GetAllStopToStopDistances()) {
        catalogue_.mutable_distances()->add_distance();
        catalogue_.mutable_distances()->mutable_distance(i)->set_from_stop(std::string(stop_to_stop.first));
        catalogue_.mutable_distances()->mutable_distance(i)->set_to_stop(std::string(stop_to_stop.second));
        catalogue_.mutable_distances()->mutable_distance(i)->set_distance(distance);
        ++i;
    }
}

void Serialization::SaveBuses(transport_catalogue::TransportCatalogue& tc_) {
    int i = 0;
    for (const auto& [name, bus] : tc_.GetAllBuses()) {
        catalogue_.add_buses();
        //transport::Bus add_bus;
        catalogue_.mutable_buses(i)->set_name(bus->name);
        catalogue_.mutable_buses(i)->set_is_rounded(bus->is_rounded);
        for (const auto& stop : bus->route) {
            catalogue_.mutable_buses(i)->add_route(stop->name);
        }
        ++i;
        //*catalogue_.add_buses() = std::move(add_bus);
    }
}

void Serialization::SaveRenderSettings(renderer::MapRenderer::MapSettings& map_settings) {
    catalogue_.mutable_map_settings()->set_width(map_settings.width);
    catalogue_.mutable_map_settings()->set_height(map_settings.height);
    catalogue_.mutable_map_settings()->set_padding(map_settings.padding);
    catalogue_.mutable_map_settings()->set_line_width(map_settings.line_width);
    catalogue_.mutable_map_settings()->set_stop_radius(map_settings.stop_radius);
    catalogue_.mutable_map_settings()->set_bus_label_font_size(map_settings.bus_label_font_size);
    catalogue_.mutable_map_settings()->mutable_bus_label_offset()->set_x(map_settings.bus_label_offset.x);
    catalogue_.mutable_map_settings()->mutable_bus_label_offset()->set_y(map_settings.bus_label_offset.y);
    catalogue_.mutable_map_settings()->set_stop_label_font_size(map_settings.stop_label_font_size);
    catalogue_.mutable_map_settings()->mutable_stop_label_offset()->set_x(map_settings.stop_label_offset.x);
    catalogue_.mutable_map_settings()->mutable_stop_label_offset()->set_y(map_settings.stop_label_offset.y);
    if (std::holds_alternative<std::monostate>(map_settings.underlayer_color)) {
        catalogue_.mutable_map_settings()->mutable_underlayer_color()->set_monostate(true);
    }
    else
        if (std::holds_alternative<std::string>(map_settings.underlayer_color)) {
            catalogue_.mutable_map_settings()->mutable_underlayer_color()->set_name(std::get<std::string>(map_settings.underlayer_color));
        }
        else
            if (std::holds_alternative<svg::Rgb>(map_settings.underlayer_color)) {
                svg::Rgb temp = std::get<svg::Rgb>(map_settings.underlayer_color);
                catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgb()->set_red(temp.red);
                catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgb()->set_green(temp.green);
                catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgb()->set_blue(temp.blue);
            }
            else
                if (std::holds_alternative<svg::Rgba>(map_settings.underlayer_color)) {
                    svg::Rgba temp = std::get<svg::Rgba>(map_settings.underlayer_color);
                    catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgba()->set_red(temp.red);
                    catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgba()->set_green(temp.green);
                    catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgba()->set_blue(temp.blue);
                    catalogue_.mutable_map_settings()->mutable_underlayer_color()->mutable_rgba()->set_opacity(temp.opacity);
                }
    catalogue_.mutable_map_settings()->set_underlayer_width(map_settings.underlayer_width);
    int i = 0;
    for (const auto& color : map_settings.color_palette) {
        catalogue_.mutable_map_settings()->add_color_palette();

        if (std::holds_alternative<std::monostate>(color)) {
            catalogue_.mutable_map_settings()->mutable_color_palette(i)->set_monostate(true);
        }
        else
            if (std::holds_alternative<std::string>(color)) {
                catalogue_.mutable_map_settings()->mutable_color_palette(i)->set_name(std::get<std::string>(color));
            }
            else
                if (std::holds_alternative<svg::Rgb>(color)) {
                    svg::Rgb temp = std::get<svg::Rgb>(color);
                    catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgb()->set_red(temp.red);
                    catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgb()->set_green(temp.green);
                    catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgb()->set_blue(temp.blue);
                }
                else
                    if (std::holds_alternative<svg::Rgba>(color)) {
                        svg::Rgba temp = std::get<svg::Rgba>(color);
                        catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgba()->set_red(temp.red);
                        catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgba()->set_green(temp.green);
                        catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgba()->set_blue(temp.blue);
                        catalogue_.mutable_map_settings()->mutable_color_palette(i)->mutable_rgba()->set_opacity(temp.opacity);
                    }
        ++i;
    }
}

void Serialization::SaveRouterSettings(std::pair<int, double> router_info) {
    catalogue_.mutable_router_info()->set_bus_wait_time(router_info.first);
    catalogue_.mutable_router_info()->set_bus_velocity(router_info.second);
}

void Serialization::CreateBase(transport_catalogue::TransportCatalogue& tc_, renderer::MapRenderer::MapSettings& map_settings, std::pair<int, double>& router_info) {
    std::ofstream out_file(file_name, std::ios::binary);
    SaveStops(tc_);
    SaveBuses(tc_);
    SaveStopsDistances(tc_);
    SaveRenderSettings(map_settings);
    SaveRouterSettings(router_info);
    catalogue_.SerializeToOstream(&out_file);
}

void Serialization::LoadStops(transport_catalogue::TransportCatalogue& tc_) {
    for (int i = 0; i < catalogue_.stops_size(); ++i) {
        auto stop = catalogue_.stops(i);
        std::string name = stop.name();
        double lat = stop.coordinates().lat();
        double lng = stop.coordinates().lng();
        tc_.AddStop(name, lat, lng);
    }
}

void Serialization::LoadStopsDistances(transport_catalogue::TransportCatalogue& tc_) {
    for (int i = 0; i < catalogue_.distances().distance_size(); ++i) {
        std::string from = catalogue_.distances().distance(i).from_stop();
        std::string to = catalogue_.distances().distance(i).to_stop();
        int distance = catalogue_.distances().distance(i).distance();
        tc_.AddStopToStopDistance(from, to, distance);
    }
}

void Serialization::LoadBuses(transport_catalogue::TransportCatalogue& tc_) {
    for (int i = 0; i < catalogue_.buses_size(); ++i) {
        auto bus = catalogue_.buses(i);
        std::string name = bus.name();
        bool is_rounded = bus.is_rounded();
        std::vector<std::string> route;
        for (auto stop : bus.route()) {
            route.push_back(stop);
        }
        tc_.AddBus(name, route, is_rounded);
    }
}

void Serialization::LoadRenderSettings(renderer::MapRenderer& renderer_) {
    renderer_.SetWidth(catalogue_.map_settings().width());
    renderer_.SetHeight(catalogue_.map_settings().height());
    renderer_.SetPadding(catalogue_.map_settings().padding());
    renderer_.SetLineWidth(catalogue_.map_settings().line_width());
    renderer_.SetStopRadius(catalogue_.map_settings().stop_radius());
    renderer_.SetBusLabelFontSize(catalogue_.map_settings().bus_label_font_size());
    renderer_.SetBusLabelOffset(catalogue_.map_settings().bus_label_offset().x(), catalogue_.map_settings().bus_label_offset().y());
    renderer_.SetStopLabelFontSize(catalogue_.map_settings().stop_label_font_size());
    renderer_.SetStopLabelOffset(catalogue_.map_settings().stop_label_offset().x(), catalogue_.map_settings().stop_label_offset().y());
    if (catalogue_.map_settings().underlayer_color().has_rgb()) {
        svg::Rgb color(catalogue_.map_settings().underlayer_color().rgb().red(), catalogue_.map_settings().underlayer_color().rgb().green(), catalogue_.map_settings().underlayer_color().rgb().blue());
        renderer_.SetUnderlayerColor(color);
    }
    else if (catalogue_.map_settings().underlayer_color().has_rgba()) {
        svg::Rgba color(catalogue_.map_settings().underlayer_color().rgba().red(), catalogue_.map_settings().underlayer_color().rgba().green(), catalogue_.map_settings().underlayer_color().rgba().blue(), catalogue_.map_settings().underlayer_color().rgba().opacity());
        renderer_.SetUnderlayerColor(color);
    }
    else if (catalogue_.map_settings().underlayer_color().name() != "") {
        renderer_.SetUnderlayerColor(catalogue_.map_settings().underlayer_color().name());
    }
    else {
        renderer_.SetUnderlayerColor(svg::Color());
    }
    renderer_.SetUnderlayerWidth(catalogue_.map_settings().underlayer_width());
    for (int i = 0; i < catalogue_.map_settings().color_palette_size(); ++i) {
        if (catalogue_.map_settings().color_palette(i).has_rgb()) {
            svg::Rgb color(catalogue_.map_settings().color_palette(i).rgb().red(), catalogue_.map_settings().color_palette(i).rgb().green(), catalogue_.map_settings().color_palette(i).rgb().blue());
            renderer_.AddColorToPalette(color);
        }
        else if (catalogue_.map_settings().color_palette(i).has_rgba()) {
            svg::Rgba color(catalogue_.map_settings().color_palette(i).rgba().red(), catalogue_.map_settings().color_palette(i).rgba().green(), catalogue_.map_settings().color_palette(i).rgba().blue(), catalogue_.map_settings().color_palette(i).rgba().opacity());
            renderer_.AddColorToPalette(color);
        }
        else if (catalogue_.map_settings().color_palette(i).name() != "") {
            renderer_.AddColorToPalette(catalogue_.map_settings().color_palette(i).name());
        }
        else {
            renderer_.AddColorToPalette(svg::Color());
        }
    }
}

std::pair<int, double> Serialization::LoadRouterSettings() {
    return std::make_pair(catalogue_.router_info().bus_wait_time(), catalogue_.router_info().bus_velocity());
}

std::pair<int, double> Serialization::LoadBase(transport_catalogue::TransportCatalogue& tc_, renderer::MapRenderer& renderer_) {
    std::ifstream in_file(file_name, std::ios::binary);
    catalogue_.ParseFromIstream(&in_file);
    LoadStops(tc_);
    LoadStopsDistances(tc_);
    LoadBuses(tc_);
    LoadRenderSettings(renderer_);
    return LoadRouterSettings();
}

void Serialization::MakeBase(std::istream& input) {
    JsonReader reader;
    auto result = reader.ParseInput(input);
    file_name = result.first;
    auto tc_ = result.second;
    auto map_settings = reader.GetMapSettings();
    CreateBase(tc_, map_settings, reader.GetBusRouterInfo());
}

void Serialization::ProcessRequests(std::istream& input) {
    JsonReader reader;
    auto result = reader.ParseStatInput(input);
    file_name = result.first;
    transport_catalogue::TransportCatalogue tc_;
    renderer::MapRenderer renderer_;
    std::pair<int, double> router_info = LoadBase(tc_, renderer_);
    reader.SetCatalogue(tc_);
    reader.SetRenderer(renderer_);
    reader.SetBusRouterInfo(router_info);
    reader.CalculateOutput(result.second);
}
