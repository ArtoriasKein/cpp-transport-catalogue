#include <string>
#include <iomanip>
#include "stat_reader.h"

std::ostream& transport_catalogue::output::operator<<(std::ostream& out, transport_catalogue::TransportCatalogue::Statistics statistics) {
    out << statistics.stops_count << " stops on route, " << statistics.unique_stops_count << " unique stops, " << std::setprecision(6) << statistics.distance << " route length" << ", " << statistics.curvature << " curvature" << std::endl;
    return out;
}

std::ostream& transport_catalogue::output::ParseOutput(std::ostream& out, std::istream& input, transport_catalogue::TransportCatalogue& catalogue) {
    int requests_count;
    input >> requests_count;
    input.ignore(256, '\n');
    std::vector<std::string> results;
    for (int i = 0; i < requests_count; ++i) {
        std::string query;
        std::getline(input, query);
        results.push_back(query);
    }
    for (const std::string& output : results) {
        std::string command = output.substr(0, output.find(' '));
        std::string query = output.substr(output.find(' ') + 1, output.size());
        if (command == "Bus") {
            auto result = catalogue.GetBusInfo(query);
            if (result.found == false) {
                out << "Bus " << query << ": not found" << std::endl;
                continue;
            }
            out << "Bus " << query << ": " << result;
        }
        else {
            auto result = catalogue.BusesOnStop(query);
            if (result.first == false) {
                out << "Stop " << query << ": not found" << std::endl;
            }
            else if (result.second.size() == 0) {
                out << "Stop " << query << ": no buses" << std::endl;
            }
            else {
                out << "Stop " << query << ": buses ";
                bool is_first = true;
                for (const std::string& bus_name : result.second) {
                    if (is_first) {
                        is_first = false;
                        out << bus_name;
                    }
                    else {
                        out << " " << bus_name;
                    }
                }
                out << std::endl;
            }
        }
    }
    return out;
}
