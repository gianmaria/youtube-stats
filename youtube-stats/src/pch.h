#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <string>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>

using std::cout;
using std::endl;
using std::string;
using std::string_view;
using std::optional;
using namespace std::literals;
using namespace std::chrono_literals;

namespace fs = std::filesystem;

using str_cref = const std::string&;
using str_view = std::string_view;

#include <argparse/argparse.hpp>

#include <cpr/cpr.h>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
using njson = nlohmann::json;
