#pragma once
#include <iostream>
#include <string>
#include "json.h"
#include "request_handler.h"


class JsonReader {
public:
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

    JsonReader();
    std::string Print(const json::Node& node);
    void ParseInput(std::istream& input);
    BusInput ParseBusInput(const json::Node& node);
    StopInput ParseStopInput(const json::Node& node);
    StopDistancesInput ParseStopWithDistanceInput(const json::Node& array);
    void ParseBaseRequests(const json::Node& array);
    void ParseRenderSettings(const json::Node& node);
private:
    transport_catalogue::TransportCatalogue catalogue_;
    renderer::MapRenderer map_renderer_;
    request_handler::RequestHandler handler;
};
