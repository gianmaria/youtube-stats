#pragma once

class YoutubeStats
{
public:
    explicit YoutubeStats(string api_key);
    
    string get_channel_id(str_view channel_name);

    string get_channel_info(str_view channel,
                            bool by_id);

    string get_playlist_items(str_view playlist_id,
                              str_view next_page_token);

    string get_video_info(str_view video_id);

private:

    string api_key;
};

