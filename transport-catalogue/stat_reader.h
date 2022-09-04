#pragma once
#include <iostream>
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace output {
        std::ostream& operator<<(std::ostream& out, transport_catalogue::TransportCatalogue::Statistics statistics);
        std::ostream& ParseOutput(std::ostream& out, std::istream& input, transport_catalogue::TransportCatalogue& catalogue);
    }
}
