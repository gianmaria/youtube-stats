#include "pch.h"

#include "youtube_api.hpp"
#include "utils.hpp"

cpr::AcceptEncoding YoutubeAPI::accept_encoding = cpr::AcceptEncoding{{
        cpr::AcceptEncodingMethods::deflate,
        cpr::AcceptEncodingMethods::gzip,
        cpr::AcceptEncodingMethods::zlib}};

YoutubeAPI::YoutubeAPI(string api_key) :
    api_key(std::move(api_key))
{
}

YoutubeAPI::YoutubeAPI(str_view api_key) :
    api_key(api_key.begin(), api_key.end())
{
}

string YoutubeAPI::get_channel_id(str_view channel_name)
{
    auto base_url = cpr::Url{"https://youtube.googleapis.com/youtube/v3/search"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
         {"part", "snippet"},
         {"fields", "pageInfo(totalResults),nextPageToken,items(snippet(channelId,title,channelTitle))"},
         {"type", "channel"},
         {"order", "relevance"},
         {"safeSearch", "none"},
         {"maxResults", "50"},
         {"q", string(channel_name.begin(), channel_name.end())},
    };

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("{} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string YoutubeAPI::get_channel_info(str_view channel_id)
{
    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/channels"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
         {"part", "snippet,contentDetails,statistics"},
         {"fields", "items(statistics(viewCount,subscriberCount,videoCount),"
            "snippet(title),id,contentDetails(relatedPlaylists(uploads)))"},
         {"id", string(channel_id.begin(), channel_id.end())},
    };

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("{} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string YoutubeAPI::get_playlist_items(str_view playlist_id,
                                      str_view next_page_token)
{
    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/playlistItems"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
         {"part", "snippet"},
         {"fields", "nextPageToken,prevPageToken,pageInfo(totalResults),items(snippet(position,publishedAt,title,resourceId(kind,videoId)))"},
         {"playlistId", string(playlist_id.begin(), playlist_id.end())},
         {"maxResults", "50"}
    };

    if (next_page_token != "")
    {
        params.Add({"pageToken", string(next_page_token.begin(), next_page_token.end())});
    }

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("{} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}

string YoutubeAPI::get_video_info(str_view video_id)
{
    auto base_url = cpr::Url{"https://www.googleapis.com/youtube/v3/videos"};
    auto params = cpr::Parameters
    {
         {"key", api_key},
         {"part", "contentDetails,statistics,snippet"},
         {"fields", "items(id,snippet(title),contentDetails(duration),statistics(viewCount,likeCount,commentCount))"},
         {"id", string(video_id.begin(), video_id.end())}
    };

    cpr::Response resp = cpr::Get(base_url, params, accept_encoding);

    if (resp.status_code != 200)
    {
        auto msg = std::format("{} - {}\n{}"sv, resp.status_code, resp.reason, resp.text);
        throw std::runtime_error(msg);
    }

    return resp.text;
}


