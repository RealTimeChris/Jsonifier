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

	THE SOFTWARE IS PROVIDED  WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
#pragma once

#include <BnchSwt/BenchmarkSuite.hpp>
#include <glaze/glaze.hpp>
#include "Common.hpp"

#if !defined(ASAN_ENABLED)

template<> struct glz::meta<audience_sub_category_names> {
	using value_type			= audience_sub_category_names;
	static constexpr auto value = object("337100890", &value_type::the337100890);
};

template<> struct glz::meta<names> {
	using value_type			= names;
	static constexpr auto value = object();
};

template<> struct glz::meta<event> {
	using value_type			= event;
	static constexpr auto value = object(&value_type::description, &value_type::subTopicIds, &value_type::logo, &value_type::topicIds, &value_type::subjectCode,
		&value_type::subtitle, &value_type::name, &value_type::id);
};

template<> struct glz::meta<price> {
	using value_type			= price;
	static constexpr auto value = object(&value_type::audienceSubCategoryId, &value_type::seatCategoryId, &value_type::amount);
};

template<> struct glz::meta<area> {
	using value_type			= area;
	static constexpr auto value = object(&value_type::blockIds, &value_type::areaId);
};

template<> struct glz::meta<seat_category> {
	using value_type			= seat_category;
	static constexpr auto value = object(&value_type::areas, &value_type::seatCategoryId);
};

template<> struct glz::meta<performance> {
	using value_type			= performance;
	static constexpr auto value = object(&value_type::seatCategories, &value_type::logo, &value_type::seatMapImage, &value_type::prices, &value_type::venueCode, &value_type::name,
		&value_type::eventId, &value_type::start, &value_type::id);
};

template<> struct glz::meta<venue_names> {
	using value_type			= venue_names;
	static constexpr auto value = object(&value_type::PLEYEL_PLEYEL);
};

template<> struct glz::meta<citm_catalog_message> {
	using value_type = citm_catalog_message;
	static constexpr auto value =
		object(&value_type::audienceSubCategoryNames, &value_type::topicSubTopics, &value_type::seatCategoryNames, &value_type::subTopicNames, &value_type::areaNames,
			&value_type::topicNames, &value_type::performances, &value_type::events, &value_type::venueNames, &value_type::subjectNames, &value_type::blockNames);
};

template<> struct glz::meta<search_metadata_data> {
	using value_type			= search_metadata_data;
	static constexpr auto value = object(&value_type::since_id_str, &value_type::next_results, &value_type::refresh_url, &value_type::max_id_str, &value_type::completed_in,
		&value_type::query, &value_type::since_id, &value_type::count, &value_type::max_id);
};

template<> struct glz::meta<hashtag_data> {
	using value_type			= hashtag_data;
	static constexpr auto value = object(&value_type::indices, &value_type::text);
};

template<> struct glz::meta<large_data> {
	using value_type			= large_data;
	static constexpr auto value = object(&value_type::resize, &value_type::w, &value_type::h);
};

template<> struct glz::meta<sizes_data> {
	using value_type			= sizes_data;
	static constexpr auto value = object(&value_type::medium, &value_type::small, &value_type::thumb, &value_type::large);
};

template<> struct glz::meta<media_data> {
	using value_type			= media_data;
	static constexpr auto value = object(&value_type::source_status_id_str, &value_type::source_status_id, &value_type::indices, &value_type::media_url_https,
		&value_type::expanded_url, &value_type::display_url, &value_type::media_url, &value_type::id_str, &value_type::type, &value_type::sizes, &value_type::url, &value_type::id);
};

template<> struct glz::meta<url_data> {
	using value_type			= url_data;
	static constexpr auto value = object(&value_type::indices, &value_type::expanded_url, &value_type::display_url, &value_type::url);
};

template<> struct glz::meta<user_mention> {
	using value_type			= user_mention;
	static constexpr auto value = object(&value_type::indices, &value_type::screen_name, &value_type::id_str, &value_type::name, &value_type::id);
};

template<> struct glz::meta<status_entities> {
	using value_type			= status_entities;
	static constexpr auto value = object(&value_type::media, &value_type::user_mentions, &value_type::symbols, &value_type::hashtags, &value_type::urls);
};

template<> struct glz::meta<metadata_data> {
	using value_type			= metadata_data;
	static constexpr auto value = object(&value_type::iso_language_code, &value_type::result_type);
};

template<> struct glz::meta<description_data> {
	using value_type			= description_data;
	static constexpr auto value = object(&value_type::urls);
};

template<> struct glz::meta<user_entities> {
	using value_type			= user_entities;
	static constexpr auto value = object(&value_type::url, &value_type::description);
};

template<> struct glz::meta<twitter_user_data> {
	using value_type			= twitter_user_data;
	static constexpr auto value = object(&value_type::profile_background_image_url_https, &value_type::profile_banner_url, &value_type::profile_background_image_url,
		&value_type::profile_sidebar_border_color, &value_type::profile_sidebar_fill_color, &value_type::time_zone, &value_type::profile_background_color,
		&value_type::profile_image_url_https, &value_type::utc_offset, &value_type::profile_use_background_image, &value_type::url, &value_type::profile_text_color,
		&value_type::profile_link_color, &value_type::profile_image_url, &value_type::profile_background_tile, &value_type::is_translation_enabled,
		&value_type::default_profile_image, &value_type::contributors_enabled, &value_type::follow_request_sent, &value_type::favourites_count, &value_type::description,
		&value_type::screen_name, &value_type::followers_count, &value_type::statuses_count, &value_type::created_at, &value_type::entities, &value_type::friends_count,
		&value_type::default_profile, &value_type::listed_count, &value_type::location, "protected", &value_type::protectedVal, &value_type::is_translator, &value_type::id_str,
		&value_type::notifications, &value_type::name, &value_type::geo_enabled, &value_type::lang, &value_type::following, &value_type::verified, &value_type::id);
};

template<> struct glz::meta<status_data> {
	using value_type			= status_data;
	static constexpr auto value = object(&value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id_str, &value_type::in_reply_to_screen_name,
		&value_type::in_reply_to_status_id, &value_type::in_reply_to_user_id, &value_type::possibly_sensitive, &value_type::contributors, &value_type::coordinates,
		&value_type::place, &value_type::geo, &value_type::entities, &value_type::favorite_count, &value_type::metadata, &value_type::created_at, &value_type::retweeted_status,
		&value_type::retweet_count, &value_type::source, &value_type::id_str, &value_type::user, &value_type::lang, &value_type::text, &value_type::truncated,
		&value_type::favorited, &value_type::retweeted, &value_type::id);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object(&value_type::search_metadata, &value_type::statuses);
};

template<> struct glz::meta<icon_emoji_data> {
	using value_type			= icon_emoji_data;
	static constexpr auto value = object(&value_type::name, &value_type::id);
};

template<> struct glz::meta<permission_overwrite> {
	using value_type			= permission_overwrite;
	static constexpr auto value = object(&value_type::allow, &value_type::deny, &value_type::id, &value_type::type);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object(&value_type::permission_overwrites, &value_type::last_message_id, &value_type::default_thread_rate_limit_per_user,
		&value_type::applied_tags, &value_type::recipients, &value_type::default_auto_archive_duration, &value_type::status, &value_type::last_pin_timestamp, &value_type::topic,
		&value_type::rate_limit_per_user, &value_type::icon_emoji, &value_type::total_message_sent, &value_type::video_quality_mode, &value_type::application_id,
		&value_type::permissions, &value_type::message_count, &value_type::parent_id, &value_type::member_count, &value_type::owner_id, &value_type::guild_id,
		&value_type::user_limit, &value_type::position, &value_type::name, &value_type::icon, &value_type::version, &value_type::bitrate, &value_type::id, &value_type::flags,
		&value_type::type, &value_type::managed, &value_type::nsfw);
};

template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object(&value_type::avatar_decoration_data, &value_type::display_name, &value_type::global_name, &value_type::avatar, &value_type::banner,
		&value_type::locale, &value_type::discriminator, &value_type::user_name, &value_type::accent_color, &value_type::premium_type, &value_type::public_flags,
		&value_type::email, &value_type::mfa_enabled, &value_type::id, &value_type::flags, &value_type::verified, &value_type::system, &value_type::bot);
};

template<> struct glz::meta<member_data> {
	using value_type			= member_data;
	static constexpr auto value = object(&value_type::communication_disabled_until, &value_type::premium_since, &value_type::nick, &value_type::avatar, &value_type::roles,
		&value_type::permissions, &value_type::joined_at, &value_type::guild_id, &value_type::user, &value_type::flags, &value_type::pending, &value_type::deaf, &value_type::mute);
};

template<> struct glz::meta<tags_data> {
	using value_type			= tags_data;
	static constexpr auto value = object(&value_type::premium_subscriber, &value_type::bot_id);
};

template<> struct glz::meta<role_data> {
	using value_type			= role_data;
	static constexpr auto value = object(&value_type::unicode_emoji, &value_type::icon, &value_type::permissions, &value_type::position, &value_type::name,
		&value_type::mentionable, &value_type::version, &value_type::id, &value_type::tags, &value_type::color, &value_type::flags, &value_type::managed, &value_type::hoist);
};

template<> struct glz::meta<guild_data> {
	using value_type			= guild_data;
	static constexpr auto value = object(&value_type::latest_on_boarding_question_id, &value_type::guild_scheduled_events, &value_type::safety_alerts_channel_id,
		&value_type::inventory_settings, &value_type::voice_states, &value_type::discovery_splash, &value_type::vanity_url_code, &value_type::application_id,
		&value_type::afk_channel_id, &value_type::default_message_notifications, &value_type::max_stage_video_channel_users, &value_type::public_updates_channel_id,
		&value_type::description, &value_type::threads, &value_type::channels, &value_type::premium_subscription_count, &value_type::approximate_presence_count,
		&value_type::features, &value_type::stickers, &value_type::premium_progress_bar_enabled, &value_type::members, &value_type::hub_type, &value_type::approximate_member_count,
		&value_type::explicit_content_filter, &value_type::max_video_channel_users, &value_type::splash, &value_type::banner, &value_type::system_channel_id,
		&value_type::widget_channel_id, &value_type::preferred_locale, &value_type::system_channel_flags, &value_type::rules_channel_id, &value_type::roles,
		&value_type::verification_level, &value_type::permissions, &value_type::max_presences, &value_type::discovery, &value_type::joined_at, &value_type::member_count,
		&value_type::premium_tier, &value_type::owner_id, &value_type::max_members, &value_type::afk_timeout, &value_type::widget_enabled, &value_type::region,
		&value_type::nsfw_level, &value_type::mfa_level, &value_type::name, &value_type::icon, &value_type::unavailable, &value_type::id, &value_type::flags, &value_type::large,
		&value_type::owner, &value_type::nsfw, &value_type::lazy);
};

template<> struct glz::meta<discord_message> {
	using value_type			= discord_message;
	static constexpr auto value = object(&value_type::t, &value_type::d, &value_type::op, &value_type::s);
};

template<> struct glz::meta<geometry_data> {
	using value_type			= geometry_data;
	static constexpr auto value = object(&value_type::coordinates, &value_type::type);
};

template<> struct glz::meta<properties_data> {
	using value_type			= properties_data;
	static constexpr auto value = object(&value_type::name);
};

template<> struct glz::meta<feature> {
	using value_type			= feature;
	static constexpr auto value = object(&value_type::properties, &value_type::geometry, &value_type::type);
};

template<> struct glz::meta<canada_message> {
	using value_type			= canada_message;
	static constexpr auto value = object(&value_type::features, &value_type::type);
};

template<> struct glz::meta<test_struct> {
	using value_type			= test_struct;
	static constexpr auto value = object(&value_type::testStrings, &value_type::testUints, &value_type::testDoubles, &value_type::testInts, &value_type::testBools);
};

template<> struct glz::meta<test<test_struct>> {
	using value_type			= test<test_struct>;
	static constexpr auto value = object(&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z);
};

template<> struct glz::meta<partial_test<test_struct>> {
	using value_type				   = partial_test<test_struct>;
	static constexpr auto value		   = object(&value_type::m);
	static constexpr auto partial_read = true;
};

template<> struct glz::meta<test_generator<test_struct>> {
	using value_type			= test_generator<test_struct>;
	static constexpr auto value = object(&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z);
};

template<> struct glz::meta<abc_test<test_struct>> {
	using value_type			= abc_test<test_struct>;
	static constexpr auto value = object(&value_type::z, &value_type::y, &value_type::x, &value_type::w, &value_type::v, &value_type::u, &value_type::t, &value_type::s,
		&value_type::r, &value_type::q, &value_type::p, &value_type::o, &value_type::n, &value_type::m, &value_type::l, &value_type::k, &value_type::j, &value_type::i,
		&value_type::h, &value_type::g, &value_type::f, &value_type::e, &value_type::d, &value_type::c, &value_type::b, &value_type::a);
};

#endif