#pragma once
#include "domain.h"
#include <deque>
#include <unordered_map>
#include <map>
#include <set>
#include <optional>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        void AddStop(const ::std::string& name, double latitude, double longitude);
        void AddBus(const ::std::string& name, const ::std::vector<::std::string>& stops, bool rounded);
        void AddStopDistances(const ::std::string& stop_name, const ::std::vector<::std::pair<::std::string, int>>& stops_and_distances);
        void AddStopToStopDistance(const ::std::string& from_stop, const ::std::string& to_stop, int distance);
        const ::std::optional<::std::set<std::string_view>> BusesOnStop(const ::std::string_view& stop_name) const;
        const ::std::optional<domain::Statistics> GetBusInfo(const ::std::string_view& bus) const;
        const ::std::set<std::string_view> GetAllBusesNames() const;
        const ::std::vector<std::string_view> GetBusRoute(const ::std::string_view& bus_name) const;
        const std::vector<geo::Coordinates> GetAllStopCoordinates() const;
        const domain::Stop& GetStopByName(const ::std::string_view& stop_name) const;
        size_t GetAllStopsCount() const;
        int GetStopToStopDistance(const std::string_view& from, const std::string_view& to) const;
        bool IsRoundBus(const ::std::string_view& bus_name) const;
        bool CheckStopValidity(const ::std::string_view& stop_name) const;
        const geo::Coordinates GetStopCoordinates(const std::string_view& stop_name) const;
        const ::std::unordered_map<::std::string_view, const domain::Bus*> GetAllBuses() const;
        const ::std::vector<std::string_view> GetAllStopsNames() const;
        const ::std::map<std::pair<std::string, std::string>, int> GetAllStopToStopDistances() const;
    private:
        ::std::deque<domain::Bus> buses_;
        ::std::deque<domain::Stop> stops_;
        ::std::unordered_map<::std::string_view, const domain::Stop*> stopname_to_stop_;
        ::std::unordered_map<const domain::Stop*, ::std::set<std::string_view>> stopname_to_busname_;
        ::std::unordered_map<::std::string_view, const domain::Bus*> busname_to_bus_;
        struct StopsHasher {
            size_t operator()(::std::pair<const domain::Stop*, const domain::Stop*> stops) const {
                ::std::hash<const void*> ptr_hasher;
                size_t result = ptr_hasher(stops.first) + ptr_hasher(stops.second) * 37;
                return result;
            }
        };
        ::std::unordered_map<::std::pair<const domain::Stop*, const domain::Stop*>, int, StopsHasher> stops_to_distance;
        ::std::pair<double, double> ComputeDistanceBetweenStops();
    };
}
