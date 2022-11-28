#include "json_reader.h"
#include <iomanip>
#include <iostream>


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
	handler.SetUnderlayerColorToRenderer(GetColorFromNode(node.AsMap().at("underlayer_color")));
	handler.SetUnderlayerWidthToRenderer(node.AsMap().at("underlayer_width").AsDouble());
	std::vector<svg::Color> color_palette;
	for (const auto& color_node : node.AsMap().at("color_palette").AsArray()) {
		color_palette.push_back(GetColorFromNode(color_node));
	}
	handler.SetColorPaletteToRenderer(color_palette);
}

std::pair<std::string, transport_catalogue::TransportCatalogue> JsonReader::ParseInput(std::istream& input) {
	json::Document queries(json::Load(input));
	std::string file_name = "";
	for (const auto& [key, value] : queries.GetRoot().AsMap()) {
		if (key == "base_requests") {
			ParseBaseRequests(value);
		}
		else if (key == "render_settings") {
			ParseRenderSettings(value);
		}
		else if (key == "routing_settings") {
			handler.SetBusVelocity(value.AsMap().at("bus_velocity").AsDouble());
			handler.SetBusWaitTime(value.AsMap().at("bus_wait_time").AsInt());
		}
		else if (key == "serialization_settings") {
			file_name = value.AsMap().at("file").AsString();
		}
	}
	return std::make_pair(file_name, catalogue_);
}

std::pair<std::string, std::stringstream> JsonReader::ParseStatInput(std::istream& input) {
	json::Document queries(json::Load(input));
	std::string file_name = "";
	std::stringstream ss;
	for (const auto& [key, value] : queries.GetRoot().AsMap()) {
		if (key == "stat_requests") {
			json::Print(queries, ss);
		}
		else if (key == "serialization_settings") {
			file_name = value.AsMap().at("file").AsString();
		}
	}
	return std::make_pair(file_name, std::move(ss));
}

std::string JsonReader::ParseFilename(std::istream& input) {
	json::Document queries(json::Load(input));
	for (const auto& [key, value] : queries.GetRoot().AsMap()) {
		if (key == "serialization_settings") {
			return value.AsMap().at("file").AsString();
		}
	}
	return "";
}

void JsonReader::SetCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
	catalogue_ = catalogue;
	handler.SetCatalogue(catalogue_);
}

void JsonReader::CalculateOutput(std::istream& input) {
	json::Document queries(json::Load(input));
	for (const auto& [key, value] : queries.GetRoot().AsMap()) {
		if (key == "stat_requests") {
			std::cout << Print(handler.ParseStatRequests(value)) << std::endl;
		}
	}
}

const renderer::MapRenderer::MapSettings JsonReader::GetMapSettings() const {
	return map_renderer_.GetSettings();
}

void JsonReader::SetRenderer(renderer::MapRenderer map_renderer) {
	map_renderer_ = map_renderer;
	handler.SetRenderer(map_renderer_);
}

svg::Color JsonReader::GetColorFromNode(const json::Node& node) {
	if (node.IsString()) {
		return svg::Color(node.AsString());
	}
	else if (node.IsArray()) {
		if (node.AsArray().size() == 4) {
			return svg::Color(svg::Rgba(node.AsArray().at(0).AsInt(), node.AsArray().at(1).AsInt(), node.AsArray().at(2).AsInt(), node.AsArray().at(3).AsDouble()));
		}
		else {
			return svg::Color(svg::Rgb(node.AsArray().at(0).AsInt(), node.AsArray().at(1).AsInt(), node.AsArray().at(2).AsInt()));
		}
	}
	return svg::Color();
}

std::pair<int, double> JsonReader::GetBusRouterInfo() const {
	return std::make_pair(handler.GetBusWaitTime(), handler.GetBusVelocity());
}

void JsonReader::SetBusRouterInfo(std::pair<int, double>& router_info) {
	handler.SetBusWaitTime(router_info.first);
	handler.SetBusVelocity(router_info.second);
}
