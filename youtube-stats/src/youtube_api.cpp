#include "pch.h"

#include "youtube_api.hpp"
#include "utils.hpp"

YoutubeAPI::YoutubeAPI(string api_key) :
    api_key(std::move(api_key))
{
}

string YoutubeAPI::get_channel_id(str_view channel_name)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://youtube.googleapis.com/youtube/v3/search"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
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

string YoutubeAPI::get_channel_info(str_view channel,
                                      bool by_id)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/channels"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
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

string YoutubeAPI::get_playlist_items(str_view playlist_id,
                                        str_view next_page_token)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/playlistItems"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
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

string YoutubeAPI::get_video_info(str_view video_id)
{
    using cpr::AcceptEncodingMethods::deflate;
    using cpr::AcceptEncodingMethods::gzip;
    using cpr::AcceptEncodingMethods::zlib;

    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/videos"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
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


