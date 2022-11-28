#pragma once

#include <memory>
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {
	struct EdgeInfo {
		std::string type;
		std::string stop_name;
		std::string bus;
		double time;
		int span_count;
	};

	class TransportRouter {
	public:
		TransportRouter() = default;
		void SetBusWaitTime(int bus_wait_time);
		void SetBusVelocity(double bus_velocity);
		int GetBusWaitTime() const;
		double GetBusVelocity() const;
		void BuildGraph(const transport_catalogue::TransportCatalogue& catalogue);
		std::optional<std::vector<transport_router::EdgeInfo>> BuildRoute(const transport_catalogue::TransportCatalogue& catalogue, const std::string& from, const std::string& to);
	private:
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0;
		double mph;
		std::unordered_map<int, EdgeInfo> id_to_edge_info;
		std::unordered_map<std::string, size_t> stop_to_vertexId;
		std::optional<graph::DirectedWeightedGraph<double>> graph_;
		std::unique_ptr<graph::Router<double>> router_;
	};

} //namespace transport_router  
