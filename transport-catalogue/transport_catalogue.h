#pragma once
#include "geo.h"
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        struct Stop {
            ::std::string name;
            Coordinates coordinates;
        };

        struct Bus {
            ::std::string name;
            bool is_rounded;
            ::std::vector<const Stop*> route;
            double distance_real;
            double distance_ideal;
        };

        void AddStop(const ::std::string& name, double latitude, double longitude);
        void AddBus(const ::std::string& name, const ::std::vector<::std::string>& stops, bool rounded);
        void AddStopDistances(const ::std::string& stop_name, const ::std::vector<::std::pair<::std::string, int>>& stops_and_distances);
        struct Statistics {
            bool found;
            int stops_count;
            int unique_stops_count;
            double distance;
            double curvature;
        };
        ::std::pair<bool, ::std::vector<::std::string>> BusesOnStop(const ::std::string& stop_name);
        Statistics GetBusInfo(const ::std::string& bus);
    private:
        ::std::deque<Bus> buses_;
        ::std::deque<Stop> stops_;
        ::std::unordered_map<::std::string_view, const Stop*> stopname_to_stop_;
        ::std::unordered_map<const Stop*, ::std::set<std::string_view>> stopname_to_busname_;
        ::std::unordered_map<::std::string_view, const Bus*> busname_to_bus_;
        struct StopsHasher {
            size_t operator()(::std::pair<const Stop*, const Stop*> stops) const {
                ::std::hash<const void*> ptr_hasher;
                size_t result = ptr_hasher(stops.first) + ptr_hasher(stops.second) * 37;
                return result;
            }
        };
        ::std::unordered_map<::std::pair<const Stop*, const Stop*>, int, StopsHasher> stops_to_distance;
    };
}
