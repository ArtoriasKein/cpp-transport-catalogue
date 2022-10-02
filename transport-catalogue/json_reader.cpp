#include "json_reader.h"
#include <iomanip>
#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader()
	: catalogue_(transport_catalogue::TransportCatalogue()),
	map_renderer_(renderer::MapRenderer()),
	handler(catalogue_, map_renderer_)
{
}

std::string JsonReader::Print(const json::Node& node) {
	std::ostringstream out;
	json::Print(json::Document{ node }, out);
	return out.str();
}

JsonReader::BusInput JsonReader::ParseBusInput(const json::Node& node) {
	BusInput result;
	result.name = node.AsMap().at("name").AsString();
	result.is_rounded = node.AsMap().at("is_roundtrip").AsBool();
	for (const auto& stop_name : node.AsMap().at("stops").AsArray()) {
		result.route.push_back(stop_name.AsString());
	}
	return result;
}
JsonReader::StopInput JsonReader::ParseStopInput(const json::Node& node) {
	StopInput result;
	result.name = node.AsMap().at("name").AsString();
	result.latitude = node.AsMap().at("latitude").AsDouble();
	result.longitude = node.AsMap().at("longitude").AsDouble();
	return result;
}
JsonReader::StopDistancesInput JsonReader::ParseStopWithDistanceInput(const json::Node& node) {
	StopDistancesInput result;
	result.name = node.AsMap().at("name").AsString();
	for (const auto& [key, value] : node.AsMap().at("road_distances").AsMap()) {
		result.stop_to_distance.push_back({ key, value.AsInt() });
	}
	return result;
}

void JsonReader::ParseBaseRequests(const json::Node& array) {
	std::vector<BusInput> bus_inputs;
	std::vector<StopDistancesInput> stop_distances_inputs;
	for (const json::Node& node : array.AsArray()) {
		if (node.AsMap().at("type").AsString() == "Bus") {
			bus_inputs.push_back(ParseBusInput(node));
		}
		if (node.AsMap().at("type").AsString() == "Stop") {
			if (node.AsMap().count("road_distances") != 0) {
				StopInput stop_input = ParseStopInput(node);
				handler.AddStopToCatalogue(stop_input.name, stop_input.latitude, stop_input.longitude);
				stop_distances_inputs.push_back(ParseStopWithDistanceInput(node));
			}
			else {
				StopInput stop_input = ParseStopInput(node);
				handler.AddStopToCatalogue(stop_input.name, stop_input.latitude, stop_input.longitude);

			}
		}
	}
	for (auto& stop : stop_distances_inputs) {
		handler.AddStopDistancesToCatalogue(stop.name, stop.stop_to_distance);
	}
	for (auto& bus : bus_inputs) {
		handler.AddBusToCatalogue(bus.name, bus.route, bus.is_rounded);
	}
}

void JsonReader::ParseRenderSettings(const json::Node& node) {
	handler.SetWidthToRenderer(node.AsMap().at("width").AsDouble());
	handler.SetHeightToRenderer(node.AsMap().at("height").AsDouble());
	handler.SetPaddingToRenderer(node.AsMap().at("padding").AsDouble());
	handler.SetLineWidthToRenderer(node.AsMap().at("line_width").AsDouble());
	handler.SetStopRadiusToRenderer(node.AsMap().at("stop_radius").AsDouble());
	handler.SetBusLabelFontSizeToRenderer(node.AsMap().at("bus_label_font_size").AsInt());
	handler.SetBusLabelOffsetToRenderer(node.AsMap().at("bus_label_offset").AsArray());
	handler.SetStopLabelFontSizeToRenderer(node.AsMap().at("stop_label_font_size").AsInt());
	handler.SetStopLabelOffsetToRenderer(node.AsMap().at("stop_label_offset").AsArray());
	handler.SetUnderlayerColorToRenderer(node.AsMap().at("underlayer_color"));
	handler.SetUnderlayerWidthToRenderer(node.AsMap().at("underlayer_width").AsDouble());
	handler.SetColorPaletteToRenderer(node.AsMap().at("color_palette").AsArray());
}

void JsonReader::ParseInput(std::istream& input) {
	json::Document queries(json::Load(input));
	for (const auto& [key, value] : queries.GetRoot().AsMap()) {
		if (key == "base_requests") {
			ParseBaseRequests(value);
		}
		else if (key == "render_settings") {
			ParseRenderSettings(value);
		}
		else if (key == "stat_requests") {
			std::cout << Print(handler.ParseStatRequests(value)) << std::endl;
		}
	}
}
