#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
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
	for (const std::string_view& bus_name : db_.GetAllBusesNames()){ //Проход по всем маршрутам и их отрисовка + заполнение массивов всех маршрутов и остановок для дальнейшей отрисовки
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
