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

void ParseInput(std::istream& input) {
    json::Document queries(json::Load(input));
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    svg::Document map;
    request_handler::RequestHandler handler(catalogue, map_renderer);
    for (const auto& [key, value] : queries.GetRoot().AsMap()) {
        if (key == "base_requests") {
            handler.ParseBaseRequests(value);
        }
        else if (key == "render_settings") {
            handler.ParseRenderSettings(value);
        }
        else if (key == "stat_requests") {
            std::cout << Print(handler.ParseStatRequests(value, map)) << std::endl;
        }
    }
}
