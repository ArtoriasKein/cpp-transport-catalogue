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

	void RequestHandler::RenderMap(svg::Document& doc) const {
		int color_index = 0;
		int color_capacity = renderer_.GetColorCapacity();
		std::vector<BusRender> all_buses;
		std::set<std::string_view> all_stops;
		std::vector<geo::Coordinates> all_stops_coordinates = db_.GetAllStopCoordinates();
		for (const std::string_view& bus_name : db_.GetAllBusesNames()) { //Проход по всем маршрутам и их отрисовка + заполнение массивов всех маршрутов и остановок для дальнейшей отрисовки
			BusRender new_bus;
			new_bus.name = bus_name;
			new_bus.color_index = color_index;
			std::vector<geo::Coordinates> current_stops_coordinates;
			auto stat = GetBusStat(bus_name);
			if (stat.has_value()) {
				if (stat.value().is_rounded) {
					new_bus.is_rounded = true;
					for (const auto& stop_name : db_.GetBusRoute(bus_name)) {
						new_bus.route.push_back(stop_name);
						all_stops.insert(stop_name);
						current_stops_coordinates.push_back(db_.GetStopCoordinates(stop_name));
					}
				}
				else {
					new_bus.is_rounded = false;
					auto bus_stops = db_.GetBusRoute(bus_name);
					for (const auto& stop_name : bus_stops) {
						new_bus.route.push_back(stop_name);
						all_stops.insert(stop_name);
						current_stops_coordinates.push_back(db_.GetStopCoordinates(stop_name));
					}
					std::reverse(bus_stops.begin(), bus_stops.end());
					bool is_first = true;
					for (const auto& stop_name : bus_stops) {
						if (is_first) {
							is_first = false;
							continue;
						}
						current_stops_coordinates.push_back(db_.GetStopCoordinates(stop_name));
					}
				}
			}

			doc.Add(renderer_.MakeRoutePolilyne(all_stops_coordinates, current_stops_coordinates, color_index));
			if (color_index + 1 == color_capacity) {
				color_index = 0;
			}
			else {
				++color_index;
			}
			all_buses.push_back(new_bus);
		}
		for (const BusRender& bus : all_buses) { // Отрисовка названий маршрутов
			if (bus.is_rounded) {
				doc.Add(renderer_.MakeBusUnderlayer(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.name));
				doc.Add(renderer_.MakeBusName(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
			}
			else {
				if (*(bus.route.begin()) == *(bus.route.end() - 1)) {
					doc.Add(renderer_.MakeBusUnderlayer(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.name));
					doc.Add(renderer_.MakeBusName(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
				}
				else {
					doc.Add(renderer_.MakeBusUnderlayer(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.name));
					doc.Add(renderer_.MakeBusName(all_stops_coordinates, db_.GetStopCoordinates(*bus.route.begin()), bus.color_index, bus.name));
					doc.Add(renderer_.MakeBusUnderlayer(all_stops_coordinates, db_.GetStopCoordinates(*(bus.route.end() - 1)), bus.name));
					doc.Add(renderer_.MakeBusName(all_stops_coordinates, db_.GetStopCoordinates(*(bus.route.end() - 1)), bus.color_index, bus.name));
				}
			}
		}
		for (const std::string_view& stop_name : all_stops) { // Отрисовка кружков остановок
			doc.Add(renderer_.MakeStop(all_stops_coordinates, db_.GetStopCoordinates(stop_name)));
		}
		for (const std::string_view& stop_name : all_stops) { // Отрисовка названий остановок
			doc.Add(renderer_.MakeStopUnderlayer(all_stops_coordinates, db_.GetStopCoordinates(stop_name), stop_name));
			doc.Add(renderer_.MakeStopName(all_stops_coordinates, db_.GetStopCoordinates(stop_name), stop_name));
		}
	}

	BusInput RequestHandler::ParseBusInput(const json::Node& node) {
		BusInput result;
		result.name = node.AsMap().at("name").AsString();
		result.is_rounded = node.AsMap().at("is_roundtrip").AsBool();
		for (const auto& stop_name : node.AsMap().at("stops").AsArray()) {
			result.route.push_back(stop_name.AsString());
		}
		return result;
	}
	StopInput RequestHandler::ParseStopInput(const json::Node& node) {
		StopInput result;
		result.name = node.AsMap().at("name").AsString();
		result.latitude = node.AsMap().at("latitude").AsDouble();
		result.longitude = node.AsMap().at("longitude").AsDouble();
		return result;
	}
	StopDistancesInput RequestHandler::ParseStopWithDistanceInput(const json::Node& node) {
		StopDistancesInput result;
		result.name = node.AsMap().at("name").AsString();
		for (const auto& [key, value] : node.AsMap().at("road_distances").AsMap()) {
			result.stop_to_distance.push_back({ key, value.AsInt() });
		}
		return result;
	}

	void RequestHandler::ParseBaseRequests(const json::Node& array) {
		std::vector<BusInput> bus_inputs;
		std::vector<StopDistancesInput> stop_distances_inputs;
		for (const json::Node& node : array.AsArray()) {
			if (node.AsMap().at("type").AsString() == "Bus") {
				bus_inputs.push_back(ParseBusInput(node));
			}
			if (node.AsMap().at("type").AsString() == "Stop") {
				if (node.AsMap().count("road_distances") != 0) {
					StopInput stop_input = ParseStopInput(node);
					db_.AddStop(stop_input.name, stop_input.latitude, stop_input.longitude);
					stop_distances_inputs.push_back(ParseStopWithDistanceInput(node));
				}
				else {
					StopInput stop_input = ParseStopInput(node);
					db_.AddStop(stop_input.name, stop_input.latitude, stop_input.longitude);

				}
			}
		}
		for (const auto& stop : stop_distances_inputs) {
			db_.AddStopDistances(stop.name, stop.stop_to_distance);
		}
		for (const auto& bus : bus_inputs) {
			db_.AddBus(bus.name, bus.route, bus.is_rounded);
		}
	}

	void RequestHandler::ParseRenderSettings(const json::Node& node) {
		renderer_.SetWidth(node.AsMap().at("width").AsDouble());
		renderer_.SetHeight(node.AsMap().at("height").AsDouble());
		renderer_.SetPadding(node.AsMap().at("padding").AsDouble());
		renderer_.SetLineWidth(node.AsMap().at("line_width").AsDouble());
		renderer_.SetStopRadius(node.AsMap().at("stop_radius").AsDouble());
		renderer_.SetBusLabelFontSize(node.AsMap().at("bus_label_font_size").AsInt());
		renderer_.SetBusLabelOffset(node.AsMap().at("bus_label_offset").AsArray());
		renderer_.SetStopLabelFontSize(node.AsMap().at("stop_label_font_size").AsInt());
		renderer_.SetStopLabelOffset(node.AsMap().at("stop_label_offset").AsArray());
		renderer_.SetUnderlayerColor(node.AsMap().at("underlayer_color"));
		renderer_.SetUnderlayerWidth(node.AsMap().at("underlayer_width").AsDouble());
		renderer_.SetColorPalette(node.AsMap().at("color_palette").AsArray());
	}

	json::Node RequestHandler::MakeJsonOutputBus(const json::Node& node) {
		auto request_result = GetBusStat(node.AsMap().at("name").AsString());
		if (request_result) {
			return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"stop_count", static_cast<int>(request_result.value().stops_count)}, {"unique_stop_count", static_cast<int>(request_result.value().unique_stops_count)}, {"route_length", request_result.value().distance}, {"curvature", request_result.value().curvature} });
		}
		return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"error_message", std::string("not found")} });
	}

	json::Node RequestHandler::MakeJsonOutputStop(const json::Node& node) {
		auto request_result = GetBusesByStop(node.AsMap().at("name").AsString());
		if (!request_result) {
			return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"error_message", std::string("not found")} });
		}
		json::Array buses;
		for (const auto& bus : request_result.value()) {
			buses.push_back(json::Node(std::string(bus)));
		}
		return json::Node(json::Dict{ {"request_id", node.AsMap().at("id").AsInt()}, {"buses", buses} });
	}

	json::Node RequestHandler::MakeJsonOutputMap(const json::Node& node, svg::Document& map) {
		RenderMap(map);
		std::ostringstream strm;
		map.Render(strm);
		return json::Node(json::Dict{ { "request_id", node.AsMap().at("id").AsInt() }, {"map", json::Node(strm.str())} });
	}

	json::Array RequestHandler::ParseStatRequests(const json::Node& array, svg::Document& map) {
		json::Array requests;
		for (const json::Node& node : array.AsArray()) {
			if (node.AsMap().at("type").AsString() == "Bus") {
				requests.push_back(MakeJsonOutputBus(node));
			}
			else if (node.AsMap().at("type").AsString() == "Stop") {
				requests.push_back(MakeJsonOutputStop(node));
			}
			else if (node.AsMap().at("type").AsString() == "Map") {
				requests.push_back(MakeJsonOutputMap(node, map));
			}
		}
		return requests;
	}

} //namespace request_handler
