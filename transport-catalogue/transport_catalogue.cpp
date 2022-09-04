#include "transport_catalogue.h"

void transport_catalogue::TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude) {
    Coordinates coord;
    coord.lat = latitude;
    coord.lng = longitude;
    Stop new_stop;
    new_stop.name = name;
    new_stop.coordinates = coord;
    stops_.push_back(new_stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stopname_to_busname_[&stops_.back()];
}

void transport_catalogue::TransportCatalogue::AddBus(const std::string& name, std::vector<std::string> stops, bool rounded) {
    Bus new_bus;
    new_bus.name = name;
    for (const std::string stop_name : stops) {
        new_bus.route.push_back(stopname_to_stop_.at(stop_name));
    }
    new_bus.is_rounded = rounded;
    buses_.push_back(new_bus);
    busname_to_bus_[buses_.back().name] = &buses_.back();
    double distance_real = 0.0;
    double distance_ideal = 0.0;
    for (const auto stop : buses_.back().route) {
        stopname_to_busname_[stop].insert(buses_.back().name);
    }
    if (new_bus.is_rounded == true) {
        for (int i = 0; i < buses_.back().route.size() - 1; ++i) {
            if (stops_to_distance.count({ buses_.back().route[i], buses_.back().route[i + 1] }) == 0) {
                distance_real += stops_to_distance.at({ buses_.back().route[i + 1], buses_.back().route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ buses_.back().route[i], buses_.back().route[i + 1] });
            }
            distance_ideal += std::abs(ComputeDistance(buses_.back().route[i]->coordinates, buses_.back().route[i + 1]->coordinates));
        }
        for (int i = buses_.back().route.size() - 1; i >= 1; --i) {
            if (stops_to_distance.count({ buses_.back().route[i], buses_.back().route[i - 1] }) == 0) {
                distance_real += stops_to_distance.at({ buses_.back().route[i - 1], buses_.back().route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ buses_.back().route[i], buses_.back().route[i - 1] });
            }
        }
        distance_ideal = distance_ideal * 2;
    }
    else {
        for (int i = 0; i < buses_.back().route.size() - 1; ++i) {
            if (stops_to_distance.count({ buses_.back().route[i], buses_.back().route[i + 1] }) == 0) {
                distance_real += stops_to_distance.at({ buses_.back().route[i + 1], buses_.back().route[i] });
            }
            else {
                distance_real += stops_to_distance.at({ buses_.back().route[i], buses_.back().route[i + 1] });
            }
            distance_ideal += std::abs(ComputeDistance(buses_.back().route[i]->coordinates, buses_.back().route[i + 1]->coordinates));
        }
    }
    buses_.back().distance_real = distance_real;
    buses_.back().distance_ideal = distance_ideal;
}

std::pair<bool, std::vector<std::string>> transport_catalogue::TransportCatalogue::BusesOnStop(const std::string& stop_name) {
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

transport_catalogue::TransportCatalogue::Statistics transport_catalogue::TransportCatalogue::GetBusInfo(const std::string& bus) {
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
    result.distance = busname_to_bus_.at(bus)->distance_real;
    result.curvature = busname_to_bus_.at(bus)->distance_real / busname_to_bus_.at(bus)->distance_ideal;
    return result;
}

void transport_catalogue::TransportCatalogue::AddStopDistances(const std::string& stop_name, std::vector<std::pair<std::string, int>> stops_and_distances) {
    for (const auto info : stops_and_distances) {
        stops_to_distance[{stopname_to_stop_[stop_name], stopname_to_stop_[info.first]}] = info.second;
    }
}
