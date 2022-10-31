#include "pch.h"

#include "utils.hpp"

[[noreturn]] void run_tests()
{
    try
    {
        //utils::query_channel_id_by_channel_name("zini", utils::env("KEY"));
        //assert(utils::save_to_file("penzolone.txt", "???\n\r"));

        /*const char* argv[] = {
        "program_name",
        "--query", "pewdiepie"
        };
        int argc = utils::array_count(argv);

        argparse::ArgumentParser program;
        utils::parse_args(program, argc, argv);

        auto name = program.present("-q"sv);*/

    }
    catch (const std::exception& e)
    {
        cout << "[EXCEPTION]" << e.what() << endl;
    }

    std::exit(0);
}