#pragma once

namespace utils
{
string env(string_view name);

bool parse_args(argparse::ArgumentParser& program,
                   int argc, const char* argv[]);

bool save_to_file(str_view path,
                  str_cref content);
}
