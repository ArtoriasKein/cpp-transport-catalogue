#include "json_builder.h"
#include "request_handler.h"
#include <sstream>

namespace request_handler {

	RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
		: db_(db),
		renderer_(renderer) {
	}

	std::optional<domain::Statistics> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		return db_.GetBusInfo(bus_name);
	}

	const std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return db_.BusesOnStop(stop_name);
	}

	const std::vector<geo::Coordinates> RequestHandler::GetAllStopCoordinates() const {
		return db_.GetAllStopCoordinates();
	}
	const std::set<std::string_view> RequestHandler::GetAllBusesNames() const {
		return db_.GetAllBusesNames();
	}
	const std::vector<std::string_view> RequestHandler::GetBusRoute(const std::string_view bus_name) const {
		return db_.GetBusRoute(bus_name);
	}
	const geo::Coordinates RequestHandler::GetStopCoordinates(const std::string_view stop_name) const {
		return db_.GetStopCoordinates(stop_name);
	}

	void RequestHandler::RenderMap(svg::Document& doc) {
		renderer_.RenderMap(*this, doc);
	}

	void RequestHandler::AddStopToCatalogue(std::string stop_name, double latitude, double longitude) {
		db_.AddStop(stop_name, latitude, longitude);
	}

	void RequestHandler::AddStopDistancesToCatalogue(std::string& stop_name, std::vector<std::pair<std::string, int>> stop_to_distance) {
		db_.AddStopDistances(stop_name, stop_to_distance);
	}

	void RequestHandler::AddBusToCatalogue(std::string& bus_name, std::vector<std::string>& route, bool is_rounded) {
		db_.AddBus(bus_name, route, is_rounded);
	}

	void RequestHandler::SetWidthToRenderer(double width) {
		renderer_.SetWidth(width);
	}
	void RequestHandler::SetHeightToRenderer(double height) {
		renderer_.SetHeight(height);
	}
	void RequestHandler::SetPaddingToRenderer(double padding) {
		renderer_.SetPadding(padding);
	}
	void RequestHandler::SetLineWidthToRenderer(double line_width) {
		renderer_.SetLineWidth(line_width);
	}
	void RequestHandler::SetStopRadiusToRenderer(double stop_radius) {
		renderer_.SetStopRadius(stop_radius);
	}
	void RequestHandler::SetBusLabelFontSizeToRenderer(int bus_label_font_size) {
		renderer_.SetBusLabelFontSize(bus_label_font_size);
	}
	void RequestHandler::SetBusLabelOffsetToRenderer(const json::Array& as_array) {
		renderer_.SetBusLabelOffset(as_array);
	}
	void RequestHandler::SetStopLabelFontSizeToRenderer(int stop_label_font_size) {
		renderer_.SetStopLabelFontSize(stop_label_font_size);
	}
	void RequestHandler::SetStopLabelOffsetToRenderer(const json::Array& as_array) {
		renderer_.SetStopLabelOffset(as_array);
	}
	void RequestHandler::SetUnderlayerColorToRenderer(const json::Node& node) {
		renderer_.SetUnderlayerColor(node);
	}
	void RequestHandler::SetUnderlayerWidthToRenderer(double underlayer_width) {
		renderer_.SetUnderlayerWidth(underlayer_width);
	}
	void RequestHandler::SetColorPaletteToRenderer(const json::Array& as_array) {
		renderer_.SetColorPalette(as_array);
	}

	json::Node RequestHandler::MakeJsonOutputBus(const json::Node& node) {
		auto request_result = GetBusStat(node.AsMap().at("name").AsString());
		if (request_result) {
			return json::Builder{}.StartDict()
				.Key("request_id").Value(node.AsMap().at("id").AsInt())
				.Key("stop_count").Value(static_cast<int>(request_result.value().stops_count))
				.Key("unique_stop_count").Value(static_cast<int>(request_result.value().unique_stops_count))
				.Key("route_length").Value(request_result.value().distance)
				.Key("curvature").Value(request_result.value().curvature)
				.EndDict().Build();
		}
		return json::Builder{}.StartDict()
			.Key("request_id").Value(node.AsMap().at("id").AsInt())
			.Key("error_message").Value(std::string("not found"))
			.EndDict().Build();
	}

	json::Node RequestHandler::MakeJsonOutputStop(const json::Node& node) {
		auto request_result = GetBusesByStop(node.AsMap().at("name").AsString());
		if (!request_result) {
			return json::Builder{}.StartDict()
				.Key("request_id").Value(node.AsMap().at("id").AsInt())
				.Key("error_message").Value(std::string("not found"))
				.EndDict().Build();
		}
		json::Array buses;
		for (const auto& bus : request_result.value()) {
			buses.push_back(json::Node(std::string(bus)));
		}
		return json::Builder{}.StartDict()
			.Key("request_id").Value(node.AsMap().at("id").AsInt())
			.Key("buses").Value(buses)
			.EndDict().Build();
	}

	json::Node RequestHandler::MakeJsonOutputMap(const json::Node& node, svg::Document& map) {
		RenderMap(map);
		std::ostringstream strm;
		map.Render(strm);
		return json::Builder{}.StartDict()
			.Key("request_id").Value(node.AsMap().at("id").AsInt())
			.Key("map").Value(json::Node(strm.str()).AsString())
			.EndDict().Build();
	}

	json::Array RequestHandler::ParseStatRequests(const json::Node& array) {
		json::Array requests;
		for (const json::Node& node : array.AsArray()) {
			if (node.AsMap().at("type").AsString() == "Bus") {
				requests.push_back(MakeJsonOutputBus(node));
			}
			else if (node.AsMap().at("type").AsString() == "Stop") {
				requests.push_back(MakeJsonOutputStop(node));
			}
			else if (node.AsMap().at("type").AsString() == "Map") {
				svg::Document map;
				requests.push_back(MakeJsonOutputMap(node, map));
			}
		}
		return requests;
	}

} //namespace request
