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

        CommandType ParseCommand(::std::string query);
        ::std::vector<::std::string> ParseRoundedRoute(::std::string query);
        ::std::vector<::std::string> ParseNotRoundedRoute(::std::string query);
        ::std::pair<::std::vector<::std::string>, bool> ParseRoute(::std::string query);
        BusInput ParseBusInput(::std::string query);
        StopInput ParseStop(::std::string query);
        StopDistancesInput ParseStopDistances(::std::string query);
        TransportCatalogue ParseInput(::std::istream& input);
    }
}
