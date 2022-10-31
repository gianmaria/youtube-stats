#include "pch.h"

#include "youtube_api.hpp"
#include "utils.hpp"

[[noreturn]] extern void run_tests();

int main(int argc, const char* argv[])
{
    argparse::ArgumentParser program;

    try
    {
        if (not utils::parse_args(program, argc, argv))
        {
            return 1;
        }

        string channel{};

        auto query = program.present("-q"sv);
        auto key = program.present("-k"sv);
        auto id = program.present("-id"sv);
        auto output = program.present("-o"sv);

        if (query.has_value() and
            key.has_value())
        {
            utils::query_channel_id_by_channel_name(*query, *key);
        }
        else if (id.has_value() and
                 key.has_value() and
                 output.has_value())
        {
            utils::download_channel_stats(*id, *output, *key);
        }   
        else
        {
            cout << program; // print help
            return 1;
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        cout << "[EXCEPTION]" << e.what() << endl;

        return 1;
    }
}
