#pragma once
#include "json_reader.h"
#include <transport_catalogue.pb.h>


class Serialization {
public:
    void MakeBase(std::istream& input);
    void ProcessRequests(std::istream& input);

private:
    mutable transport::TransportCatalogue catalogue_;
    std::string file_name;

    void SaveStops(transport_catalogue::TransportCatalogue& tc_);
    void SaveStopsDistances(transport_catalogue::TransportCatalogue& tc_);
    void SaveBuses(transport_catalogue::TransportCatalogue& tc_);
    void SaveRenderSettings(renderer::MapRenderer::MapSettings& map_settings);
    void SaveRouterSettings(std::pair<int, double> router_info);
    void CreateBase(transport_catalogue::TransportCatalogue& tc_, renderer::MapRenderer::MapSettings& map_settings, std::pair<int, double>& router_info);

    void LoadStops(transport_catalogue::TransportCatalogue& tc_);
    void LoadStopsDistances(transport_catalogue::TransportCatalogue& tc_);
    void LoadBuses(transport_catalogue::TransportCatalogue& tc_);
    void LoadRenderSettings(renderer::MapRenderer& renderer_);
    std::pair<int, double> LoadRouterSettings();
    std::pair<int, double> LoadBase(transport_catalogue::TransportCatalogue& tc_, renderer::MapRenderer& renderer_);
};
