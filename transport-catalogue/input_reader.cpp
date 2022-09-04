#include <iostream>
#include "input_reader.h"

using namespace transport_catalogue;

input::CommandType input::ParseCommand(const ::std::string& query) {
    ::std::string command = query.substr(0, query.find(' '));
    if (command == "Bus") {
        return input::CommandType::Bus;
    }
    if (command == "Stop" && query.find('m') != std::string::npos) {
        return input::CommandType::StopWithDistance;
    }
    return input::CommandType::Stop;
}

::std::vector<::std::string> input::ParseRoundedRoute(const ::std::string& query) {
    ::std::vector<::std::string> result;
    ::std::string line = query;
    while (line.find('-') != ::std::string::npos) {
        ::std::string stop_name = line.substr(0, line.find('-') - 1);
        line.erase(0, stop_name.size() + 3);
        result.push_back(stop_name);
    }
    result.push_back(line);
    return result;
}

::std::vector<::std::string> input::ParseNotRoundedRoute(const ::std::string& query) {
    ::std::vector<::std::string> result;
    ::std::string line = query;
    while (line.find('>') != ::std::string::npos) {
        ::std::string stop_name = line.substr(0, line.find('>') - 1);
        line.erase(0, stop_name.size() + 3);
        result.push_back(stop_name);
    }
    result.push_back(line);
    return result;
}

::std::pair<::std::vector<::std::string>, bool> input::ParseRoute(const ::std::string& query) {
    bool is_rounded = false;
    ::std::vector<::std::string> result;
    if (query.find('>') == std::string::npos) {
        is_rounded = true;
        result = ParseRoundedRoute(query);
    }
    else {
        result = ParseNotRoundedRoute(query);
    }
    return { result, is_rounded };
}

input::BusInput input::ParseBusInput(const ::std::string& query) {
    input::BusInput result;
    ::std::string bus_name = query.substr(query.find(' ') + 1, query.size());
    bus_name = bus_name.substr(0, bus_name.find(':'));
    ::std::string route_query = query.substr(query.find(":") + 2, query.size());
    ::std::pair<std::vector<std::string>, bool> route_result = ParseRoute(route_query);
    result.name = bus_name;
    result.is_rounded = route_result.second;
    result.route = route_result.first;
    return result;
}

input::StopInput input::ParseStop(const ::std::string& query) {
    input::StopInput result;
    ::std::string stop_name = query.substr(query.find(' ') + 1, query.size());
    stop_name = stop_name.substr(0, stop_name.find(':'));
    ::std::string coordinates_holster = query.substr(query.find(":") + 2, query.size());
    double latitude, longitude;
    latitude = std::stod(coordinates_holster.substr(0, coordinates_holster.find(',')));
    coordinates_holster = coordinates_holster.substr(coordinates_holster.find(',') + 2, coordinates_holster.size());
    if (coordinates_holster.find(',') != std::string::npos) {
        coordinates_holster = coordinates_holster.substr(0, coordinates_holster.find(','));
    }
    longitude = std::stod(coordinates_holster);
    result.name = stop_name;
    result.latitude = latitude;
    result.longitude = longitude;
    return result;
}

input::StopDistancesInput input::ParseStopDistances(const ::std::string& query) {
    input::StopDistancesInput result;
    ::std::string stop_name = query.substr(query.find(' ') + 1, query.size());
    stop_name = stop_name.substr(0, stop_name.find(':'));
    ::std::string distances_and_stops = query.substr(query.find(':') + 2, query.size());
    distances_and_stops = distances_and_stops.substr(distances_and_stops.find(',') + 2, distances_and_stops.size());
    distances_and_stops = distances_and_stops.substr(distances_and_stops.find(',') + 2, distances_and_stops.size());
    ::std::vector<::std::pair<::std::string, int>> stop_and_distance;
    while (distances_and_stops.find(',') != ::std::string::npos) {
        int distance = std::stoi(distances_and_stops.substr(0, distances_and_stops.find('m')));
        distances_and_stops = distances_and_stops.substr(distances_and_stops.find('m') + 5, distances_and_stops.size());
        ::std::string stop = distances_and_stops.substr(0, distances_and_stops.find(','));
        distances_and_stops.erase(0, stop.size() + 2);
        stop_and_distance.push_back({ stop, distance });
    }
    int distance = std::stoi(distances_and_stops.substr(0, distances_and_stops.find('m')));
    distances_and_stops = distances_and_stops.substr(distances_and_stops.find('m') + 5, distances_and_stops.size());
    ::std::string stop = distances_and_stops.substr(0, distances_and_stops.size());
    stop_and_distance.push_back({ stop, distance });
    result.name = stop_name;
    result.stop_to_distance = stop_and_distance;
    return result;
}

TransportCatalogue input::ParseInput(::std::istream& input) {
    int create_commands_count;
    input >> create_commands_count;
    input.ignore(256, '\n');
    ::std::vector<input::StopDistancesInput> distances_input;
    ::std::vector<input::BusInput> buses_input;
    TransportCatalogue result;
    for (int i = 0; i < create_commands_count; ++i) {
        ::std::string query;
        ::std::getline(input, query);
        switch (ParseCommand(query)) {
        case(input::CommandType::Bus):
            buses_input.push_back(ParseBusInput(query));
            break;
        case(input::CommandType::Stop):
        {
            input::StopInput input = ParseStop(query);
            result.AddStop(input.name, input.latitude, input.longitude);
            break;
        }
        case(input::CommandType::StopWithDistance):
        {
            input::StopInput input = ParseStop(query);
            result.AddStop(input.name, input.latitude, input.longitude);
            distances_input.push_back(ParseStopDistances(query));
            break;
        }
        }
    }
    for (const auto stop_with_distances : distances_input) {
        result.AddStopDistances(stop_with_distances.name, stop_with_distances.stop_to_distance);
    }
    for (const auto bus : buses_input) {
        result.AddBus(bus.name, bus.route, bus.is_rounded);
    }
    return result;
}
