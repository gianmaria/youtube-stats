#include "pch.h"

#include "youtube_stats.hpp"

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


string get_channel_id(str_view channel_name,
                      str_view key)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://youtube.googleapis.com/youtube/v3/search"};
    auto params = cpr::Parameters
    {
         {"key", string(key.begin(), key.end())},
         {"part", "snippet"},
         {"fields", "nextPageToken,items(snippet(channelId,title,channelTitle))"},
         {"type", "channel"},
         {"order", "relevance"},
         {"maxResults", "50"},
         {"q", string(channel_name.begin(), channel_name.end())},
    };

    auto accept_encoding = cpr::AcceptEncoding{{deflate, gzip, zlib}};

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("[ERROR] {} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string get_channel_info(str_view channel,
                        str_view key,
                        bool by_id)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/channels"};
    auto params = cpr::Parameters
    {
         {"key", string(key.begin(), key.end())},
         {"part", "snippet,contentDetails,statistics"},
         {"fields", "items(statistics(viewCount,subscriberCount,videoCount),snippet(title),id,contentDetails(relatedPlaylists(uploads)))"},
    };

    if (by_id)
    {
        params.Add({"id", string(channel.begin(), channel.end())});
    }
    else
    {
        params.Add({"forUsername", string(channel.begin(), channel.end())});
    }

    auto accept_encoding = cpr::AcceptEncoding{{deflate, gzip, zlib}};

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("[ERROR] {} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string get_playlist_items(str_view playlist_id,
                          str_view key,
                          str_view next_page_token)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/playlistItems"};
    auto params = cpr::Parameters
    {
         {"key", string(key.begin(), key.end())},
         {"part", "snippet"},
         {"fields", "nextPageToken,prevPageToken,pageInfo(totalResults),items(snippet(position,publishedAt,title,resourceId(kind,videoId)))"},
         {"playlistId", string(playlist_id.begin(), playlist_id.end())},
         {"maxResults", "50"}
    };
    auto accept_encoding = cpr::AcceptEncoding{{deflate, gzip, zlib}};

    if (next_page_token != "")
    {
        params.Add({"pageToken", string(next_page_token.begin(), next_page_token.end())});
    }

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("[ERROR] {} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string get_video_info(str_view video_id,
                      str_view key)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/videos"};
    auto params = cpr::Parameters
    {
         {"key", string(key.begin(), key.end())},
         {"part", "contentDetails,statistics,snippet"},
         {"fields", "items(id,snippet(title),contentDetails(duration),statistics(viewCount,likeCount,commentCount))"},
         {"id", string(video_id.begin(), video_id.end())}
    };
    auto accept_encoding = cpr::AcceptEncoding{{deflate, gzip, zlib}};

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("[ERROR] {} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
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

void download_youtube_stats(str_view channel,
                            str_view output_file,
                            str_view key,
                            bool by_id)
{
    njson channel_info;

    if (by_id) // use ChannelId directly
    {
        auto channel_info_resp = get_channel_info(channel, key, by_id);
        channel_info = njson::parse(channel_info_resp);
    }
    else // first obtain ChannelId from channel name
    {
        auto search_resp = get_channel_id(channel, key);
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

        auto channel_info_resp = get_channel_info(channel_id, key, true);
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
        auto playlist_items_resp = get_playlist_items(uploads_playlist_id, key, next_page_token);
        auto playlist_items = njson::parse(playlist_items_resp);

        for (const auto& playlist_item : playlist_items["items"sv])
        {
            auto& snippet = playlist_item["snippet"sv];

            njson obj;
            obj["videoId"sv] = snippet["resourceId"sv]["videoId"sv];
            obj["title"sv] = snippet["title"sv];
            obj["publishedAt"sv] = snippet["publishedAt"sv];

            auto video_info_txt = get_video_info(obj["videoId"sv].get<str_view>(), key);
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

    if (not save_to_file(output_file, out.dump(2)))
    {
        auto msg = std::format("[ERROR] Cannot save file '{}' to disk"sv,
                               output_file);
        throw std::runtime_error(msg);
    }

    cout << "[INFO] Done!"sv << endl;
}


