/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Sep 17, 2024
#pragma once

#include <optional>
#include <string>
#include <vector>

struct search_metadata_data {
	double completed_in;
	int64_t max_id;
	std::string max_id_str;
	std::string next_results;
	std::string query;
	std::string refresh_url;
	int64_t count;
	int64_t since_id;
	std::string since_id_str;
};

struct hashtag_data {
	std::string text;
	std::vector<int64_t> indices;
};

struct large_data {
	int64_t w;
	int64_t h;
	std::string resize;
};

struct sizes_data {
	large_data medium;
	large_data small;
	large_data thumb;
	large_data large;
};

struct media_data {
	int64_t id;
	std::string id_str;
	std::vector<int64_t> indices;
	std::string media_url;
	std::string media_url_https;
	std::string url;
	std::string display_url;
	std::string expanded_url;
	std::string type;
	sizes_data sizes;
	std::optional<int64_t> source_status_id;
	std::optional<std::string> source_status_id_str;
};

struct url_data {
	std::string url;
	std::string expanded_url;
	std::string display_url;
	std::vector<int64_t> indices;
};

struct user_mention {
	std::string screen_name;
	std::string name;
	int64_t id;
	std::string id_str;
	std::vector<int64_t> indices;
};

struct status_entities {
	std::vector<hashtag_data> hashtags;
	std::vector<std::nullptr_t> symbols;
	std::vector<url_data> urls;
	std::vector<user_mention> user_mentions;
	std::optional<std::vector<media_data>> media;
};

struct metadata_data {
	std::string result_type;
	std::string iso_language_code;
};

struct description_data {
	std::vector<url_data> urls;
};

struct user_entities {
	description_data description;
	std::optional<description_data> url;
};

struct twitter_user_data {
	int64_t id;
	std::string id_str;
	std::string name;
	std::string screen_name;
	std::string location;
	std::string description;
	std::optional<std::string> url;
	user_entities entities;
	bool protectedVal;
	int64_t followers_count;
	int64_t friends_count;
	int64_t listed_count;
	std::string created_at;
	int64_t favourites_count;
	std::optional<int64_t> utc_offset;
	std::optional<std::string> time_zone;
	bool geo_enabled;
	bool verified;
	int64_t statuses_count;
	std::string lang;
	bool contributors_enabled;
	bool is_translator;
	bool is_translation_enabled;
	std::string profile_background_color;
	std::string profile_background_image_url;
	std::string profile_background_image_url_https;
	bool profile_background_tile;
	std::string profile_image_url;
	std::string profile_image_url_https;
	std::optional<std::string> profile_banner_url;
	std::string profile_link_color;
	std::string profile_sidebar_border_color;
	std::string profile_sidebar_fill_color;
	std::string profile_text_color;
	bool profile_use_background_image;
	bool default_profile;
	bool default_profile_image;
	bool following;
	bool follow_request_sent;
	bool notifications;
};

struct status_data {
	metadata_data metadata;
	std::string created_at;
	int64_t id;
	std::string id_str;
	std::string text;
	std::string source;
	bool truncated;
	std::optional<int64_t> in_reply_to_status_id;
	std::optional<std::string> in_reply_to_status_id_str;
	std::optional<int64_t> in_reply_to_user_id;
	std::optional<std::string> in_reply_to_user_id_str;
	std::optional<std::string> in_reply_to_screen_name;
	twitter_user_data user;
	std::nullptr_t geo;
	std::nullptr_t coordinates;
	std::nullptr_t place;
	std::nullptr_t contributors;
	int64_t retweet_count;
	int64_t favorite_count;
	status_entities entities;
	bool favorited;
	bool retweeted;
	std::string lang;
	std::shared_ptr<status_data> retweeted_status;
	std::optional<bool> possibly_sensitive;
};

struct twitter_message {
	std::vector<status_data> statuses;
	search_metadata_data search_metadata;
};
