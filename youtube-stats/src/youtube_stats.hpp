#pragma once


string get_channel_id(str_view channel_name,
                      str_view key);

string get_channel_info(str_view channel,
                        str_view key,
                        bool by_id);

string get_playlist_items(str_view playlist_id,
                          str_view key,
                          str_view next_page_token);

string get_video_info(str_view video_id,
                      str_view key);


void download_youtube_stats(str_view channel,
                            str_view output_file,
                            str_view key,
                            bool by_id = false);
