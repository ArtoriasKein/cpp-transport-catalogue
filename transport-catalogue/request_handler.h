#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace request_handler {

    struct BusRender {
        std::string_view name;
        std::vector<std::string_view> route;
        int color_index = 0;
        bool is_rounded = false;
    };

    struct StopInput {
        std::string name;
        double latitude;
        double longitude;
    };

    struct StopDistancesInput {
        std::string name;
        std::vector<std::pair<std::string, int>> stop_to_distance;
    };

    struct BusInput {
        std::string name;
        std::vector<std::string> route;
        bool is_rounded;
    };

    class RequestHandler {
    public:
        RequestHandler(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);
        std::optional<domain::Statistics> GetBusStat(const std::string_view& bus_name) const;
        const std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;
        void RenderMap(svg::Document& doc) const;
        void ParseBaseRequests(const json::Node& array);
        void ParseRenderSettings(const json::Node& node);
        json::Array ParseStatRequests(const json::Node& node, svg::Document& map);
    private:
        transport_catalogue::TransportCatalogue& db_;
        renderer::MapRenderer& renderer_;
        json::Node MakeJsonOutputBus(const json::Node& node);
        json::Node MakeJsonOutputStop(const json::Node& node);
        json::Node MakeJsonOutputMap(const json::Node& node, svg::Document& map);
        BusInput ParseBusInput(const json::Node& node);
        StopInput ParseStopInput(const json::Node& node);
        StopDistancesInput ParseStopWithDistanceInput(const json::Node& array);
    };

} // namespace request_handler
