#pragma once
#include <iostream>
#include <string>
#include "json.h"
#include "request_handler.h"

std::string Print(const json::Node& node);
void ParseInput(std::istream& input);
