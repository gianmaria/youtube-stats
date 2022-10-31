#pragma once

#include <string>
#include <string_view>

class YoutubeAPI
{
public:

    explicit YoutubeAPI(string api_key);
    explicit YoutubeAPI(str_view api_key);

    string get_channel_id(str_view channel_name);

    string get_channel_info(str_view channel,
                            bool by_id);

    string get_playlist_items(str_view playlist_id,
                              str_view next_page_token);

    string get_video_info(str_view video_id);

private:

    string api_key;
};

