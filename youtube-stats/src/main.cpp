#include "pch.h"

#include "youtube_api.hpp"
#include "utils.hpp"

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

void download_youtube_stats(str_view channel,
                            str_view output_file,
                            str_view key,
                            bool by_id)
{

    YoutubeAPI yt{string(key.begin(), key.end())};

    njson channel_info;

    if (by_id) // use ChannelId directly
    {
        auto channel_info_resp = yt.get_channel_info(channel, by_id);
        channel_info = njson::parse(channel_info_resp);
    }
    else // first obtain ChannelId from channel name
    {
        auto search_resp = yt.get_channel_id(channel);
        auto search = njson::parse(search_resp);

        if (search.empty())
        {
            throw std::runtime_error("[ERROR] Returned json is empty, channel name not found");
        }
        else if (search["items"sv].size() != 1)
        {
            string buffer;
            buffer.reserve(1024);

            std::format_to(std::back_inserter(buffer),
                           "[WARN] Found more than one channelId for channel name '{}', "
                           "pick one from the list (note: the list is limited to 50 results):\n"sv, channel);

            unsigned counter = 1;
            for (const auto& item : search["items"sv])
            {
                const auto& snippet = item["snippet"sv];
                std::format_to(std::back_inserter(buffer),
                               "   {:03}. {} ({}) id: {} - https://www.youtube.com/channel/{}\n"sv,
                               counter++,
                               snippet["title"sv].get<str_view>(),
                               snippet["channelTitle"sv].get<str_view>(),
                               snippet["channelId"sv].get<str_view>(),
                               snippet["channelId"sv].get<str_view>()
                );
            }

            throw std::runtime_error(buffer);
        }

        auto channel_id = search["items"sv][0]["snippet"sv]["channelId"sv].get<str_view>();

        auto channel_info_resp = yt.get_channel_info(channel_id, true);
        channel_info = njson::parse(channel_info_resp);
    }

    //save_to_file(output_file, channel_info_resp);

    if (channel_info.empty())
    {
        throw std::runtime_error("[ERROR] Returned json is empty, check channel name or channel id");
    }

    auto& ci_item = channel_info["items"sv][0];

    njson out;
    out["id"sv] = ci_item["id"sv];
    out["title"sv] = ci_item["snippet"sv]["title"sv];
    out["viewCount"sv] = ci_item["statistics"sv]["viewCount"sv];
    out["subscriberCount"sv] = ci_item["statistics"sv]["subscriberCount"sv];
    out["videoCount"sv] = ci_item["statistics"sv]["videoCount"sv];
    out["items"sv] = njson::array();

    cout << std::format("[INFO] Downloading info for channel '{}'"sv, out["title"sv].get<str_view>()) << endl;
    cout << std::format("[INFO] Found {} videos"sv, out["videoCount"sv].get<str_view>()) << endl;
    //cout << out.dump(4) << endl;

    auto& uploads_playlist_id =
        ci_item["contentDetails"sv]["relatedPlaylists"sv]["uploads"sv].get_ref<str_cref>();

    string next_page_token{};
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

            cout << std::format("[INFO] [{}] {}"sv, snippet["position"sv].get<unsigned>() + 1,
                                obj["title"sv].get<str_view>()) << endl;

            out["items"sv].push_back(std::move(obj));

            //cout << ".";
            std::this_thread::sleep_for(10ms);
        }

        //cout << endl;

        if (not playlist_items.contains("nextPageToken"sv))
            break;

        next_page_token = playlist_items["nextPageToken"sv].get<string>();

        std::this_thread::sleep_for(10ms);
    }

    if (not utils::save_to_file(output_file, out.dump(2)))
    {
        auto msg = std::format("[ERROR] Cannot save file '{}' to disk"sv,
                               output_file);
        throw std::runtime_error(msg);
    }

    cout << "[INFO] Done!"sv << endl;
}


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
