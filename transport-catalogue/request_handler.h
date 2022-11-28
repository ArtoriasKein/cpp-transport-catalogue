#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

namespace request_handler {

    class RequestHandler {
    public:
        RequestHandler(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);
        std::optional<domain::Statistics> GetBusStat(const std::string_view& bus_name) const;
        const std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;
        const std::vector<geo::Coordinates> GetAllStopCoordinates() const;
        const std::set<std::string_view> GetAllBusesNames() const;
        const std::vector<std::string_view> GetBusRoute(const std::string_view bus_name) const;
        const geo::Coordinates GetStopCoordinates(const std::string_view stop_name) const;
        void AddStopToCatalogue(std::string stop_name, double latitude, double longitude);
        void AddStopDistancesToCatalogue(std::string& stop_name, std::vector<std::pair<std::string, int>> stop_to_distance);
        void AddBusToCatalogue(std::string& bus_name, std::vector<std::string>& route, bool is_rounded);
        void SetWidthToRenderer(double width);
        void SetHeightToRenderer(double height);
        void SetPaddingToRenderer(double padding);
        void SetLineWidthToRenderer(double line_width);
        void SetStopRadiusToRenderer(double stop_radius);
        void SetBusLabelFontSizeToRenderer(int bus_label_font_size);
        void SetBusLabelOffsetToRenderer(const json::Array& as_array);
        void SetStopLabelFontSizeToRenderer(int stop_label_font_size);
        void SetStopLabelOffsetToRenderer(const json::Array& as_array);
        void SetUnderlayerColorToRenderer(svg::Color color);
        void SetUnderlayerWidthToRenderer(double underlayer_width);
        void SetColorPaletteToRenderer(std::vector<svg::Color> color_palette);
        void RenderMap(svg::Document& doc);
        void SetBusVelocity(int velocity);
        void SetBusWaitTime(int time);
        double GetBusVelocity() const;
        int GetBusWaitTime() const;
        void BuildGraph();
        json::Array ParseStatRequests(const json::Node& node);
        void SetCatalogue(transport_catalogue::TransportCatalogue& catalogue);
        void SetRenderer(renderer::MapRenderer& renderer);
    private:
        transport_catalogue::TransportCatalogue& db_;
        renderer::MapRenderer& renderer_;
        transport_router::TransportRouter router_;
        json::Node MakeJsonOutputBus(const json::Node& node);
        json::Node MakeJsonOutputStop(const json::Node& node);
        json::Node MakeJsonOutputMap(const json::Node& node, svg::Document& map);
        json::Node MakeJsonOutputRoute(const json::Node& node, std::optional<std::vector<transport_router::EdgeInfo>> info);
    };

} // namespace request
