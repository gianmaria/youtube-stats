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

template<typename T, size_t N>
constexpr size_t array_count(T(&)[N]) noexcept
{
    return N;
}

struct Error
{
    string description;
};

template<typename T>
using ResultOrError = std::variant<T, Error>;

string env(string_view name);

bool save_to_file(str_view path,
                  str_cref content);

ResultOrError<argparse::ArgumentParser>
parse_args(int argc, const char* argv[]);

void query_channel_id_by_channel_name(str_view channel_name,
                                      str_view key);

void download_channel_stats(str_view channel_id,
                            str_view output_file,
                            str_view key);
}
