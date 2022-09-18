#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "json.h"
#include "request_handler.h"

enum CommandType {
    Bus,
    Stop,
    StopWithDistance
};

struct BusInput {
    std::string name;
    std::vector<std::string> route;
    bool is_rounded;
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

std::string Print(const json::Node& node);
json::Node MakeJsonOutputBus(json::Node node, RequestHandler& handler);
json::Node MakeJsonOutputStop(json::Node node, RequestHandler& handler);
json::Node MakeJsonOutputMap(json::Node node, RequestHandler& handler, svg::Document& map);
BusInput ParseBusInput(json::Node node);
StopInput ParseStopInput(json::Node node);
StopDistancesInput ParseStopWithDistanceInput(json::Node node);
void ParseInput(std::istream& input);
