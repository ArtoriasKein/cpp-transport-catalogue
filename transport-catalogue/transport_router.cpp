#include "transport_router.h"

using namespace transport_router;
using namespace std::literals;

void TransportRouter::SetBusWaitTime(int bus_wait_time) {
	bus_wait_time_ = bus_wait_time;
}

void TransportRouter::SetBusVelocity(double bus_velocity) {
	bus_velocity_ = bus_velocity;
	mph = bus_velocity_ * ((5 * 1.0) / (18 * 1.0));
}

void TransportRouter::BuildGraph(const transport_catalogue::TransportCatalogue& catalogue) {
	graph_ = graph::DirectedWeightedGraph<double>(static_cast<size_t>(catalogue.GetAllStopsCount() * 2));
	for (const auto& [bus_name, bus] : catalogue.GetAllBuses()) {
		for (const auto& stop : bus->route) {
			if (stop_to_vertexId.count(stop->name) == 0) {
				stop_to_vertexId[stop->name] = stop_to_vertexId.size();
			}
			if (stop_to_vertexId.count(stop->name + "_mirror"s) == 0) {
				stop_to_vertexId[stop->name + "_mirror"s] = stop_to_vertexId.size();
				auto edgeId = graph_.value().AddEdge({ stop_to_vertexId.at(stop->name), stop_to_vertexId.at(stop->name + "_mirror"s), bus_wait_time_ * 1.0 });
				id_to_edge_info[edgeId] = { "Wait"s, stop->name, ""s, bus_wait_time_ * 1.0, 0 };
			}
		}
		for (auto it_begin = bus->route.begin(); it_begin != bus->route.end(); ++it_begin) {
			double time_forward = 0.0;
			double time_backward = 0.0;
			int stops_passed = 0;
			auto prev_stop = *it_begin;
			for (auto it_end = std::next(it_begin); it_end != bus->route.end(); ++it_end) {
				time_forward += ((catalogue.GetStopToStopDistance(prev_stop->name, (*it_end)->name) * 1.0) / mph) / 60;
				time_backward += ((catalogue.GetStopToStopDistance((*it_end)->name, prev_stop->name) * 1.0) / mph) / 60;
				auto edgeId = graph_.value().AddEdge({ stop_to_vertexId.at((*it_begin)->name + "_mirror"s), stop_to_vertexId.at((*it_end)->name), time_forward });
				id_to_edge_info[edgeId] = { "Bus"s, ""s, bus->name, time_forward, ++stops_passed };
				if (!bus->is_rounded) {
					edgeId = graph_.value().AddEdge({ stop_to_vertexId.at((*it_end)->name + "_mirror"s), stop_to_vertexId.at((*it_begin)->name), time_backward });
					id_to_edge_info[edgeId] = { "Bus"s, ""s, bus->name, time_backward, stops_passed };
				}
				prev_stop = *it_end;
			}
		}
	}
}

std::optional<std::vector<transport_router::EdgeInfo>> TransportRouter::BuildRoute(const transport_catalogue::TransportCatalogue& catalogue, const std::string& from, const std::string& to) {
	if (!graph_) {
		BuildGraph(catalogue);
		if (graph_->GetEdgeCount() > 0) {
			router_ = std::make_unique<graph::Router<double>>(graph::Router(graph_.value()));
		}
	}
	if (stop_to_vertexId.count(from) == 0 || stop_to_vertexId.count(to) == 0) {
		return {};
	}
	if (graph_->GetEdgeCount() == 0) {
		return {};
	}
	std::vector<EdgeInfo> result;
	if (from == to) {
		return result;
	}
	auto route = router_->BuildRoute(stop_to_vertexId.at(from), stop_to_vertexId.at(to));
	if (route.has_value()) {
		for (const auto& res : route.value().edges) {
			result.push_back(id_to_edge_info.at(res));
		}
		return result;
	}
	else {
		return {};
	}
}

int TransportRouter::GetBusWaitTime() const {
	return bus_wait_time_;
}
double TransportRouter::GetBusVelocity() const {
	return bus_velocity_;
}
