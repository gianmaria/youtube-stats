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

}
