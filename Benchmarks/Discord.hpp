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

struct emoji_data {
	std::string id{};
	std::string name{};
	std::vector<std::nullptr_t> roles{};
	bool require_colons{};
	bool managed{};
	bool animated{};
	bool available{};
};

struct incidents_data_data {
	std::string invites_disabled_until{};
	std::string dms_disabled_until{};
};

struct tags_data {
	std::optional<std::string> bot_id{};
	std::nullptr_t guild_connections{};
};

struct role_data {
	std::string id{};
	std::string name{};
	std::nullptr_t description{};
	std::string permissions{};
	int64_t position{};
	int64_t color{};
	bool hoist{};
	bool managed{};
	bool mentionable{};
	std::nullptr_t icon{};
	std::nullptr_t unicode_emoji{};
	int64_t flags{};
	std::optional<tags_data> tags{};
};

struct sticker_data {
	std::string id{};
	std::string name{};
	std::string tags{};
	int64_t type{};
	int64_t format_type{};
	std::string description{};
	std::string asset{};
	bool available{};
	std::string guild_id{};
};

struct discord_message {
	std::string id{};
	std::string name{};
	std::string icon{};
	std::string description{};
	std::nullptr_t home_header{};
	std::nullptr_t splash{};
	std::string discovery_splash{};
	std::vector<std::string> features{};
	std::nullptr_t banner{};
	std::string owner_id{};
	std::nullptr_t application_id{};
	std::string region{};
	std::nullptr_t afk_channel_id{};
	int64_t afk_timeout{};
	std::string system_channel_id{};
	int64_t system_channel_flags{};
	bool widget_enabled{};
	std::string widget_channel_id{};
	int64_t verification_level{};
	std::vector<role_data> roles{};
	int64_t default_message_notifications{};
	int64_t mfa_level{};
	int64_t explicit_content_filter{};
	std::nullptr_t max_presences{};
	int64_t max_members{};
	int64_t max_stage_video_channel_users{};
	int64_t max_video_channel_users{};
	std::nullptr_t vanity_url_code{};
	int64_t premium_tier{};
	int64_t premium_subscription_count{};
	std::string preferred_locale{};
	std::string rules_channel_id{};
	std::string safety_alerts_channel_id{};
	std::string public_updates_channel_id{};
	std::nullptr_t hub_type{};
	bool premium_progress_bar_enabled{};
	std::string latest_onboarding_question_id{};
	bool nsfw{};
	int64_t nsfw_level{};
	std::vector<emoji_data> emojis{};
	std::vector<sticker_data> stickers{};
	incidents_data_data incidents_data{};
	std::nullptr_t inventory_settings{};
	bool embed_enabled{};
	std::string embed_channel_id{};
	int64_t approximate_member_count{};
	int64_t approximate_presence_count{};
};
