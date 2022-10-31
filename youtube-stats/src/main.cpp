#include "pch.h"

#include "youtube_stats.hpp"

int main(int argc, const char* argv[])
{
    argparse::ArgumentParser program;

    try
    {
        if (not parse_args(program, argc, argv))
        {
            return 1;
        }

        string channel{};
        bool by_id = false;

        if (auto name = program.present("--name"sv))
        {
            channel = name.value();
        }
        else if (auto id = program.present("--id"sv))
        {
            channel = id.value();
            by_id = true;
        }
        else
        {
            cout << "You need to provide either '--name' or '--id'"sv << endl;
            cout << program; // print help
            return 1;
        }

        const auto& output = program.get("--output"sv);
        const auto& key = program.get("--key"sv);

        download_youtube_stats(channel, output, key, by_id);

        return 0;
    }
    catch (const std::exception& e)
    {
        cout << e.what() << endl;
    }

    return 1;
}
