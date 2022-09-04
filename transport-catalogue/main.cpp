#include <iostream>
#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transport_catalogue::TransportCatalogue transport_catalogue = transport_catalogue::input::ParseInput(std::cin);
    transport_catalogue::output::ParseOutput(::std::cin, transport_catalogue);
}
