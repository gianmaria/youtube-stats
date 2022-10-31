#pragma once

string env(string_view name);


bool save_to_file(str_view path,
                  str_cref content);

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

bool parse_args(argparse::ArgumentParser& program,
                   int argc, const char* argv[]);

void download_youtube_stats(str_view channel,
                            str_view output_file,
                            str_view key,
                            bool by_id = false);
