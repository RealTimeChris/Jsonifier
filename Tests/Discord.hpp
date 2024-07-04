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

struct icon_emoji_data {
	std::optional<std::string> name;
	std::nullptr_t id;
};

struct permission_overwrite {
	std::string allow;
	int64_t type;
	std::string deny;
	std::string id;
};

struct channel_data {
	int64_t default_thread_rate_limit_per_user;
	int64_t default_auto_archive_duration;
	std::vector<permission_overwrite> permission_overwrites;
	int64_t rate_limit_per_user;
	int64_t video_quality_mode;
	int64_t total_message_sent;
	std::string last_pin_timestamp;
	std::optional<std::string> last_message_id;
	std::string application_id;
	int64_t message_count;
	int64_t member_count;
	std::vector<std::nullptr_t> applied_tags;
	std::string permissions;
	int64_t user_limit;
	icon_emoji_data icon_emoji;
	std::vector<std::nullptr_t> recipients;
	std::string parent_id;
	int64_t position;
	std::string guild_id;
	std::string owner_id;
	bool managed;
	int64_t bitrate;
	int64_t version;
	std::nullptr_t status;
	int64_t flags;
	std::nullptr_t topic;
	bool nsfw;
	int64_t type;
	std::string icon;
	std::string name;
	std::string id;
};

struct user_data {
	std::nullptr_t avatar_decoration_data;
	std::string discriminator;
	int64_t public_flags;
	int64_t premium_type;
	int64_t accent_color;
	std::optional<std::string> display_name;
	bool mfa_enabled;
	std::optional<std::string> global_name;
	std::string user_name;
	bool verified;
	bool system;
	std::nullptr_t locale;
	std::nullptr_t banner;
	std::optional<std::string> avatar;
	int64_t flags;
	std::string email;
	bool bot;
	std::string id;
};

struct member_data {
	std::nullptr_t communication_disabled_until;
	std::nullptr_t premium_since;
	std::string permissions;
	std::string joined_at;
	std::string guild_id;
	bool pending;
	std::nullptr_t avatar;
	int64_t flags;
	std::vector<std::string> roles;
	bool mute;
	bool deaf;
	user_data user;
	std::optional<std::string> nick;
};

struct tags_data {
	std::nullptr_t premium_subscriber;
	std::optional<std::string> bot_id;
};

struct role_data {
	std::nullptr_t unicode_emoji;
	bool mentionable;
	std::string permissions;
	int64_t position;
	bool managed;
	int64_t version;
	bool hoist;
	int64_t flags;
	int64_t color;
	tags_data tags;
	std::string name;
	std::nullptr_t icon;
	std::string id;
};

struct guild_data {
	std::nullptr_t latest_on_boarding_question_id;
	int64_t max_stage_video_channel_users;
	int64_t default_message_notifications;
	bool premium_progress_bar_enabled;
	int64_t approximate_presence_count;
	int64_t premium_subscription_count;
	std::string public_updates_channel_id;
	int64_t approximate_member_count;
	std::nullptr_t safety_alerts_channel_id;
	int64_t max_video_channel_users;
	int64_t explicit_content_filter;
	std::vector<std::nullptr_t> guild_scheduled_events;
	int64_t system_channel_flags;
	int64_t verification_level;
	std::nullptr_t inventory_settings;
	std::string widget_channel_id;
	std::string system_channel_id;
	std::string rules_channel_id;
	std::string preferred_locale;
	std::nullptr_t discovery_splash;
	std::nullptr_t vanity_url_code;
	bool widget_enabled;
	std::nullptr_t afk_channel_id;
	std::nullptr_t application_id;
	int64_t max_presences;
	int64_t premium_tier;
	int64_t member_count;
	std::vector<std::nullptr_t> voice_states;
	bool unavailable;
	int64_t afk_timeout;
	int64_t max_members;
	std::string permissions;
	std::nullptr_t description;
	int64_t nsfw_level;
	int64_t mfa_level;
	std::string joined_at;
	std::string discovery;
	std::string owner_id;
	std::nullptr_t hub_type;
	std::vector<std::string> stickers;
	std::vector<std::string> features;
	std::vector<channel_data> channels;
	std::vector<member_data> members;
	std::vector<std::nullptr_t> threads;
	std::string region;
	std::nullptr_t banner;
	std::nullptr_t splash;
	bool owner;
	bool large;
	int64_t flags;
	std::vector<role_data> roles;
	bool lazy;
	bool nsfw;
	std::string icon;
	std::string name;
	std::string id;
};

struct discord_message {
	int64_t op;
	int64_t s;
	guild_data d;
	std::string t;
};
