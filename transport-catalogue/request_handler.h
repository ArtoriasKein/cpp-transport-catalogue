#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

struct BusRender {
    std::string_view name;
    std::vector<std::string_view> route;
    int color_index = 0;
    bool is_rounded = false;
};

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);
    std::optional<domain::Statistics> GetBusStat(const std::string_view& bus_name) const;
    const std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;
    void RenderMap(svg::Document& doc) const;
private:
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
