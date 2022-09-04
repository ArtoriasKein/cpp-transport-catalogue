#pragma once
#include <vector>
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace input {
        enum CommandType {
            Bus,
            Stop,
            StopWithDistance
        };

        struct BusInput {
            ::std::string name;
            ::std::vector<std::string> route;
            bool is_rounded;
        };

        struct StopInput {
            ::std::string name;
            double latitude;
            double longitude;
        };

        struct StopDistancesInput {
            ::std::string name;
            ::std::vector<::std::pair<::std::string, int>> stop_to_distance;
        };

        CommandType ParseCommand(const ::std::string& query);
        ::std::vector<::std::string> ParseRoundedRoute(const ::std::string& query);
        ::std::vector<::std::string> ParseNotRoundedRoute(const ::std::string& query);
        ::std::pair<::std::vector<::std::string>, bool> ParseRoute(const ::std::string& query);
        BusInput ParseBusInput(const ::std::string& query);
        StopInput ParseStop(const ::std::string& query);
        StopDistancesInput ParseStopDistances(const ::std::string& query);
        TransportCatalogue ParseInput(::std::istream& input);
    }
}
