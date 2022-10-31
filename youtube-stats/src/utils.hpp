#pragma once

#include <string_view>
#include <string>

using std::string;
using std::string_view;

using str_cref = const std::string&;
using str_view = std::string_view;

#include <argparse/argparse.hpp>


namespace utils
{

string env(string_view name);

bool save_to_file(str_view path,
                  str_cref content);

}
