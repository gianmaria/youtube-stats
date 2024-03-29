#include "pch.h"

#include "utils.hpp"
#include "youtube_api.hpp"

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

        bool success =
            ofs.good() and
            not (ofs.fail() or ofs.bad());

        return success;
    }

    return false;
}

ResultOrError<argparse::ArgumentParser>
parse_args(int argc, const char* argv[])
{
    argparse::ArgumentParser program;

    try
    {
        program = argparse::ArgumentParser("youtube-stat", "1.0.0");
        
        program.add_description("Youtube Stat\n"
                                "Download all data about uploaded "
                                "video for a specific channel id.");
        program.add_epilog(
            "Examples:\n"
            "youtube-stat -k API_KEY -q vsauce"
            "                                   # get the channel id from channel name\n"
            "youtube-stat -k API_KEY -id UC6nSFpj9HTCZ5t-N3Rm3-HA -o vsauce.json "
            "# save all the info about the uploaded video of vsauce\n"
        );

        program.add_argument("-q"sv, "--query"sv)
            .help("find channel id by channel name");

        program.add_argument("-id"sv)
            .help("id of the channel");

        program.add_argument("-k"sv, "--key"sv)
            .help("your youtube data api key (https://developers.google.com/youtube/v3/getting-started)");

        program.add_argument("-o"sv, "--output"sv)
            .help("specify the output file");

        program.parse_args(argc, argv);

        return program;
    }
    catch (const std::runtime_error& err)
    {
        return Error{string(err.what()) + program.help().str()};
    }
}

void query_channel_id_by_channel_name(str_view channel_name,
                                      str_view key)
{
    YoutubeAPI yt{key};

    auto search_resp = yt.get_channel_id(channel_name);
    auto search = njson::parse(search_resp);

    if (search.empty())
    {
        throw std::runtime_error("Returned json is empty, channel name not found");
    }
    else
    {
        auto total_res = search["pageInfo"]["totalResults"].get<size_t>();

        cout << std::format(R"(Found {:L} results for channel "{}" (showing only first 50 results))"sv,
                            total_res, channel_name) << endl;

        unsigned counter = 1;
        for (const auto& item : search["items"sv])
        {
            const auto& snippet = item["snippet"sv];
            size_t max_str_len = 50;
            cout << std::format(R"(   [{:02}] id: {} - {: <{}} - https://www.youtube.com/channel/{})"sv,
                                counter++,
                                snippet["channelId"sv].get<str_view>(),
                                snippet["channelTitle"sv].get<str_view>().substr(0, max_str_len), max_str_len,
                                snippet["channelId"sv].get<str_view>()
            ) << endl;
        }
    }
}

void download_channel_stats(str_view channel_id,
                            str_view output_file,
                            str_view key)
{

    YoutubeAPI yt{key};

    auto channel_info_resp = yt.get_channel_info(channel_id);
    njson channel_info = njson::parse(channel_info_resp);

    //save_to_file(output_file, channel_info_resp);

    if (channel_info.empty())
    {
        throw std::runtime_error("[ERROR] Returned json is empty, check channel id");
    }

    auto& ci_item = channel_info["items"sv][0];

    njson out;
    out["id"sv] = ci_item["id"sv];
    out["title"sv] = ci_item["snippet"sv]["title"sv];
    out["viewCount"sv] = ci_item["statistics"sv]["viewCount"sv];
    out["subscriberCount"sv] = ci_item["statistics"sv]["subscriberCount"sv];
    out["videoCount"sv] = ci_item["statistics"sv]["videoCount"sv];
    out["items"sv] = njson::array();

    cout << std::format("Downloading info for channel \"{}\""sv, out["title"sv].get<str_view>()) << endl;
    cout << std::format("Found {} videos"sv, out["videoCount"sv].get<str_view>()) << endl;
    //cout << out.dump(4) << endl;

    auto& uploads_playlist_id =
        ci_item["contentDetails"sv]["relatedPlaylists"sv]["uploads"sv].get_ref<str_cref>();

    string next_page_token;
    while (true)
    {
        auto playlist_items_resp = yt.get_playlist_items(uploads_playlist_id, next_page_token);
        auto playlist_items = njson::parse(playlist_items_resp);

        for (const auto& playlist_item : playlist_items["items"sv])
        {
            auto& snippet = playlist_item["snippet"sv];

            njson obj;
            obj["videoId"sv] = snippet["resourceId"sv]["videoId"sv];
            obj["title"sv] = snippet["title"sv];
            obj["publishedAt"sv] = snippet["publishedAt"sv];

            auto video_info_txt = yt.get_video_info(obj["videoId"sv].get<str_view>());
            auto video_info_json = njson::parse(video_info_txt);

            auto& video_info_item = video_info_json["items"sv][0];
            obj["duration"sv] = video_info_item["contentDetails"sv]["duration"sv];
            obj["viewCount"sv] = video_info_item["statistics"sv]["viewCount"sv];
            obj["likeCount"sv] = video_info_item["statistics"sv]["likeCount"sv];
            obj["commentCount"sv] = video_info_item["statistics"sv]["commentCount"sv];

            cout << std::format("    [{}] {}"sv, snippet["position"sv].get<unsigned>() + 1,
                                obj["title"sv].get<str_view>()) << endl;

            out["items"sv].push_back(std::move(obj));

            //cout << ".";
            std::this_thread::sleep_for(10ms);
        }

        //cout << endl;

        if (not playlist_items.contains("nextPageToken"sv))
            break; // we are done no more results

        next_page_token = playlist_items["nextPageToken"sv].get<string>();

        std::this_thread::sleep_for(10ms);
    }

    if (not utils::save_to_file(output_file, out.dump(2)))
    {
        auto msg = std::format("Cannot save file '{}' to disk"sv,
                               output_file);
        throw std::runtime_error(msg);
    }

    cout << "Done!"sv << endl;
}



}
