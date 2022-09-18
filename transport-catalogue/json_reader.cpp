#include "json_reader.h"
#include <iomanip>
#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

std::string Print(const json::Node& node) {
    std::ostringstream out;
    Print(json::Document{ node }, out);
    return out.str();
}

BusInput ParseBusInput(json::Node node) {
    BusInput result;
    result.name = node.AsMap().at("name").AsString();
    result.is_rounded = node.AsMap().at("is_roundtrip").AsBool();
    for (const auto& stop_name : node.AsMap().at("stops").AsArray()) {
        result.route.push_back(stop_name.AsString());
    }
    return result;
}

StopInput ParseStopInput(json::Node node) {
    StopInput result;
    result.name = node.AsMap().at("name").AsString();
    result.latitude = node.AsMap().at("latitude").AsDouble();
    result.longitude = node.AsMap().at("longitude").AsDouble();
    return result;
}

StopDistancesInput ParseStopWithDistanceInput(json::Node node) {
    StopDistancesInput result;
    result.name = node.AsMap().at("name").AsString();
    for (const auto& [key, value] : node.AsMap().at("road_distances").AsMap()) {
        result.stop_to_distance.push_back({ key, value.AsInt() });
    }
    return result;
}

json::Node MakeJsonOutputBus(json::Node node, RequestHandler& handler) {
    auto request_result = handler.GetBusStat(node.AsMap().at("name").AsString());
    if (request_result) {
        return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"stop_count", static_cast<int>(request_result.value().stops_count)}, {"unique_stop_count", static_cast<int>(request_result.value().unique_stops_count)}, {"route_length", request_result.value().distance}, {"curvature", request_result.value().curvature}});
    }
    return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"error_message", std::string("not found")} });
}

json::Node MakeJsonOutputMap(json::Node node, RequestHandler& handler, svg::Document& map) {
    handler.RenderMap(map);
    std::ostringstream strm;
    map.Render(strm);
    return json::Node(json::Dict{ { "request_id", node.AsMap().at("id").AsInt() }, {"map", json::Node(strm.str())} });
}

json::Node MakeJsonOutputStop(json::Node node, RequestHandler& handler) {
    auto request_result = handler.GetBusesByStop(node.AsMap().at("name").AsString());
    if (!request_result) {
        return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"error_message", std::string("not found")} });
    }
    json::Array buses;
    for (const auto& bus : request_result.value()) {
        buses.push_back(json::Node(std::string(bus)));
    }
    return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"buses", buses}});
}

void ParseInput(std::istream& input) {
    json::Document queries(json::Load(input));
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    svg::Document map;
    RequestHandler handler(catalogue, map_renderer);
    for (const auto& [key, value] : queries.GetRoot().AsMap()) {
        if (key == "base_requests") {
            std::vector<BusInput> bus_inputs;
            std::vector<StopDistancesInput> stop_distances_inputs;
            for (const json::Node& node : value.AsArray()) {
                if (node.AsMap().at("type").AsString() == "Bus") {
                    bus_inputs.push_back(ParseBusInput(node));
                }
                if (node.AsMap().at("type").AsString() == "Stop") {
                    if (node.AsMap().count("road_distances") != 0) {
                        StopInput stop_input = ParseStopInput(node);
                        catalogue.AddStop(stop_input.name, stop_input.latitude, stop_input.longitude);
                        stop_distances_inputs.push_back(ParseStopWithDistanceInput(node));
                    }
                    else {
                        StopInput stop_input = ParseStopInput(node);
                        catalogue.AddStop(stop_input.name, stop_input.latitude, stop_input.longitude);

                    }
                }
            }
            for (const auto& stop : stop_distances_inputs) {
                catalogue.AddStopDistances(stop.name, stop.stop_to_distance);
            }
            for (const auto& bus : bus_inputs) {
                catalogue.AddBus(bus.name, bus.route, bus.is_rounded);
            }
        }
        else if (key == "render_settings") {
            map_renderer.SetWidth(value.AsMap().at("width").AsDouble());
            map_renderer.SetHeight(value.AsMap().at("height").AsDouble());
            map_renderer.SetPadding(value.AsMap().at("padding").AsDouble());
            map_renderer.SetLineWidth(value.AsMap().at("line_width").AsDouble());
            map_renderer.SetStopRadius(value.AsMap().at("stop_radius").AsDouble());
            map_renderer.SetBusLabelFontSize(value.AsMap().at("bus_label_font_size").AsInt());
            map_renderer.SetBusLabelOffset(value.AsMap().at("bus_label_offset").AsArray());
            map_renderer.SetStopLabelFontSize(value.AsMap().at("stop_label_font_size").AsInt());
            map_renderer.SetStopLabelOffset(value.AsMap().at("stop_label_offset").AsArray());
            map_renderer.SetUnderlayerColor(value.AsMap().at("underlayer_color"));
            map_renderer.SetUnderlayerWidth(value.AsMap().at("underlayer_width").AsDouble());
            map_renderer.SetColorPalette(value.AsMap().at("color_palette").AsArray());
        }
        else if (key == "stat_requests") {
            json::Array requests;
            for (const json::Node& node : value.AsArray()) {
                if (node.AsMap().at("type").AsString() == "Bus") {
                    requests.push_back(MakeJsonOutputBus(node, handler));
                }
                else if (node.AsMap().at("type").AsString() == "Stop") {
                    requests.push_back(MakeJsonOutputStop(node, handler));
                }
                else if (node.AsMap().at("type").AsString() == "Map") {
                    requests.push_back(MakeJsonOutputMap(node, handler, map));
                }
            }
            std::cout << Print(requests) << std::endl;
        }
    }
}
