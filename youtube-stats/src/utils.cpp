#include "pch.h"

#include "utils.hpp"

namespace utils
{
string env(string_view name)
{
    auto split_line = [](str_cref line)
        -> std::pair<string, string>
    {
        const string delimiter = "=";
        auto pos = line.find(delimiter);

        if (pos == std::string::npos)
            return {};

        std::pair<string, string> res;

        res.first = line.substr(0, pos);
        res.second = line.substr(pos + 1);

        return res;
    };

    std::ifstream ifs(".env");

    string res{};

    if (ifs.is_open())
    {
        for (string line;
             std::getline(ifs, line);
             )
        {
            auto [key, value] = split_line(line);

            if (key == name)
            {
                res = value;
                break;
            }
        }
    }

    return res;
}

bool save_to_file(str_view path,
                  str_cref content)
{
    std::ofstream ofs(fs::path{path},
                      std::ofstream::trunc);

    if (ofs.is_open())
    {
        ofs << content;
        return true;
    }

    return false;
}

bool parse_args(argparse::ArgumentParser& program,
                int argc, const char* argv[])
{
    try
    {
        program = argparse::ArgumentParser("youtube-stat", "1.0.0");

        program.add_description("Youtube Stat\n"
                                "Download all data about uploaded "
                                "video for a specific channel name or id.");

        program.add_argument("--name"sv)
            .help("name of the youtube channel, "
                  "the one that you can find in the url, "
                  "e.g. PewDiePie, greymatter, veritasium, "
                  "MrBeast6000 etc...");

        program.add_argument("--id"sv)
            .help("id of the channel if name is not available");

        program.add_argument("-o"sv, "--output"sv)
            .required()
            .help("specify the output file");

        program.add_argument("--key"sv)
            .required()
            .help("your youtube data api key");

        program.parse_args(argc, argv);

        return true;
    }
    catch (const std::runtime_error& err)
    {
        cout << err.what();
        cout << program; // print help

        return false;
    }
}

}
