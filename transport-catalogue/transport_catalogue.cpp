#include "transport_catalogue.h"

void transport_catalogue::TransportCatalogue::AddStop(std::string name, double latitude, double longitude) {
    Coordinates coord;
    coord.lat = latitude;
    coord.lng = longitude;
    Stop test;
    test.name = name;
    test.coordinates = coord;
    stops_.push_back(test);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stopname_to_busname_[&stops_.back()];
}

void transport_catalogue::TransportCatalogue::AddBus(std::string name, std::vector<std::string> stops, bool rounded) {
    Bus test;
    test.name = name;
    for (const std::string stop_name : stops) {
        test.route.push_back(stopname_to_stop_.at(stop_name));
    }
    test.is_rounded = rounded;
    buses_.push_back(test);
    busname_to_bus_[buses_.back().name] = &buses_.back();
    for (const auto stop : test.route) {
        stopname_to_busname_[stop].insert(buses_.back().name);
    }
}

std::pair<bool, std::vector<std::string>> transport_catalogue::TransportCatalogue::BusesOnStop(std::string stop_name) {
    std::vector<std::string> result;
    bool found = false;
    if (stopname_to_stop_.count(stop_name) == 0) {
        return { found, result };
    }
    if (stopname_to_busname_[stopname_to_stop_.at(stop_name)].size() == 0) {
        found = true;
        return { found, result };
    }
    for (const auto stop : stopname_to_busname_.at(stopname_to_stop_.at(stop_name))) {
        result.push_back(std::string(stop));
    }
    found = true;
    return { found, result };
}

transport_catalogue::TransportCatalogue::Statistics transport_catalogue::TransportCatalogue::GetBusInfo(std::string bus) {
    Statistics result;
    if (busname_to_bus_.count(bus) == 0) {
        result.found = false;
        return result;
    }
    result.found = true;
    if (busname_to_bus_.at(bus)->is_rounded == true) {
        result.stops_count = busname_to_bus_.at(bus)->route.size();
        result.stops_count = (result.stops_count * 2) - 1;
    }
    else {
        result.stops_count = busname_to_bus_.at(bus)->route.size();
    }
    int unique_count = 0;
    std::unordered_set<std::string_view> unique_stops_names;
    for (const auto stop : busname_to_bus_.at(bus)->route) {
        if (unique_stops_names.count(stop->name) == 0) {
            unique_stops_names.insert(stop->name);
            ++unique_count;
        }
    }
    result.unique_stops_count = unique_count;
    double distance_real = 0.0;
    double distance_ideal = 0.0;
    if (busname_to_bus_.at(bus)->is_rounded == true) {
        for (int i = 0; i < busname_to_bus_.at(bus)->route.size() - 1; ++i) {
            if (stops_to_distance.count({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i + 1] }) == 0) {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i + 1], busname_to_bus_.at(bus)->route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i + 1] });
            }
            distance_ideal += std::abs(ComputeDistance(busname_to_bus_.at(bus)->route[i]->coordinates, busname_to_bus_.at(bus)->route[i + 1]->coordinates));
        }
        for (int i = busname_to_bus_.at(bus)->route.size() - 1; i >= 1; --i) {
            if (stops_to_distance.count({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i - 1] }) == 0) {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i - 1], busname_to_bus_.at(bus)->route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i - 1] });
            }
        }
        distance_ideal = distance_ideal * 2;
    }
    else {
        for (int i = 0; i < busname_to_bus_.at(bus)->route.size() - 1; ++i) {
            if (stops_to_distance.count({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i + 1] }) == 0) {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i + 1], busname_to_bus_.at(bus)->route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ busname_to_bus_.at(bus)->route[i], busname_to_bus_.at(bus)->route[i + 1] });
            }
            distance_ideal += std::abs(ComputeDistance(busname_to_bus_.at(bus)->route[i]->coordinates, busname_to_bus_.at(bus)->route[i + 1]->coordinates));
        }
    }
    result.distance = distance_real;
    result.curvature = distance_real / distance_ideal;
    return result;
}

void transport_catalogue::TransportCatalogue::AddStopDistances(std::string stop_name, std::vector<std::pair<std::string, int>> stops_and_distances) {
    for (const auto info : stops_and_distances) {
        stops_to_distance[{stopname_to_stop_[stop_name], stopname_to_stop_[info.first]}] = info.second;
    }
}
