#pragma once
#include <string>
#include <vector>
#include "geo.h"

namespace domain {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        bool is_rounded;
        std::vector<const Stop*> route;
        double distance_real;
        double distance_ideal;
    };

    struct Statistics {
        bool found;
        bool is_rounded;
        size_t stops_count;
        size_t unique_stops_count;
        double distance;
        double curvature;
    };
} // namespace domain
