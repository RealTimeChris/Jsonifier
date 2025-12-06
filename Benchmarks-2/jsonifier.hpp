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
#pragma once

#include <bnch_swt/index.hpp>
#include <jsonifier/Index.hpp>
#include "common.hpp"

template<> struct jsonifier::core<audience_sub_category_names> {
	using value_type				 = audience_sub_category_names;
	static constexpr auto parseValue = createValue<makeJsonEntity<&value_type::the337100890, "337100890">()>();
};

template<> struct jsonifier::core<names> {
	using value_type				 = names;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<event> {
	using value_type				 = event;
	static constexpr auto parseValue = createValue<&value_type::description, &value_type::id, &value_type::logo, &value_type::name, &value_type::subTopicIds,
		&value_type::subjectCode, &value_type::subtitle, &value_type::topicIds>();
};

template<> struct jsonifier::core<price> {
	using value_type				 = price;
	static constexpr auto parseValue = createValue<&value_type::amount, &value_type::audienceSubCategoryId, &value_type::seatCategoryId>();
};

template<> struct jsonifier::core<area> {
	using value_type				 = area;
	static constexpr auto parseValue = createValue<&value_type::areaId, &value_type::blockIds>();
};

template<> struct jsonifier::core<seat_category> {
	using value_type				 = seat_category;
	static constexpr auto parseValue = createValue<&value_type::areas, &value_type::seatCategoryId>();
};

template<> struct jsonifier::core<performance> {
	using value_type				 = performance;
	static constexpr auto parseValue = createValue<&value_type::eventId, &value_type::id, &value_type::logo, &value_type::name, &value_type::prices, &value_type::seatCategories,
		&value_type::seatMapImage, &value_type::start, &value_type::venueCode>();
};

template<> struct jsonifier::core<venue_names> {
	using value_type				 = venue_names;
	static constexpr auto parseValue = createValue<&value_type::PLEYEL_PLEYEL>();
};

template<> struct jsonifier::core<citm_catalog_message> {
	using value_type = citm_catalog_message;
	static constexpr auto parseValue =
		createValue<&value_type::areaNames, &value_type::audienceSubCategoryNames, &value_type::blockNames, &value_type::events, &value_type::performances,
			&value_type::seatCategoryNames, &value_type::subTopicNames, &value_type::subjectNames, &value_type::topicNames, &value_type::topicSubTopics, &value_type::venueNames>();
};

template<> struct jsonifier::core<search_metadata_data> {
	using value_type				 = search_metadata_data;
	static constexpr auto parseValue = createValue<&value_type::completed_in, &value_type::max_id, &value_type::max_id_str, &value_type::next_results, &value_type::query,
		&value_type::refresh_url, &value_type::count, &value_type::since_id, &value_type::since_id_str>();
};

template<> struct jsonifier::core<hashtag_data> {
	using value_type				 = hashtag_data;
	static constexpr auto parseValue = createValue<&value_type::text, &value_type::indices>();
};

template<> struct jsonifier::core<large_data> {
	using value_type				 = large_data;
	static constexpr auto parseValue = createValue<&value_type::w, &value_type::h, &value_type::resize>();
};

template<> struct jsonifier::core<sizes_data> {
	using value_type				 = sizes_data;
	static constexpr auto parseValue = createValue<&value_type::medium, &value_type::small, &value_type::thumb, &value_type::large>();
};

template<> struct jsonifier::core<media_data> {
	using value_type = media_data;
	static constexpr auto parseValue =
		createValue<&value_type::id, &value_type::id_str, &value_type::indices, &value_type::media_url, &value_type::media_url_https, &value_type::url, &value_type::display_url,
			&value_type::expanded_url, &value_type::type, &value_type::sizes, &value_type::source_status_id, &value_type::source_status_id_str>();
};

template<> struct jsonifier::core<url_data> {
	using value_type				 = url_data;
	static constexpr auto parseValue = createValue<&value_type::url, &value_type::expanded_url, &value_type::display_url, &value_type::indices>();
};

template<> struct jsonifier::core<user_mention_data> {
	using value_type				 = user_mention_data;
	static constexpr auto parseValue = createValue<&value_type::screen_name, &value_type::name, &value_type::id, &value_type::id_str, &value_type::indices>();
};

template<> struct jsonifier::core<status_entities> {
	using value_type				 = status_entities;
	static constexpr auto parseValue = createValue<&value_type::hashtags, &value_type::symbols, &value_type::urls, &value_type::user_mentions, &value_type::media>();
};

template<> struct jsonifier::core<metadata_data> {
	using value_type				 = metadata_data;
	static constexpr auto parseValue = createValue<&value_type::result_type, &value_type::iso_language_code>();
};

template<> struct jsonifier::core<description_data> {
	using value_type				 = description_data;
	static constexpr auto parseValue = createValue<&value_type::urls>();
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue<&value_type::description, &value_type::url>();
};

template<> struct jsonifier::core<twitter_user_data> {
	using value_type = twitter_user_data;
	static constexpr auto parseValue =
		createValue<&value_type::id, &value_type::id_str, &value_type::name, &value_type::screen_name, &value_type::location, &value_type::description, &value_type::url,
			&value_type::entities, makeJsonEntity<&value_type::protectedVal, "protected">(), &value_type::followers_count, &value_type::friends_count, &value_type::listed_count,
			&value_type::created_at, &value_type::favourites_count, &value_type::utc_offset, &value_type::time_zone, &value_type::geo_enabled, &value_type::verified,
			&value_type::statuses_count, &value_type::lang, &value_type::contributors_enabled, &value_type::is_translator, &value_type::is_translation_enabled,
			&value_type::profile_background_color, &value_type::profile_background_image_url, &value_type::profile_background_image_url_https, &value_type::profile_background_tile,
			&value_type::profile_image_url, &value_type::profile_image_url_https, &value_type::profile_banner_url, &value_type::profile_link_color,
			&value_type::profile_sidebar_border_color, &value_type::profile_sidebar_fill_color, &value_type::profile_text_color, &value_type::profile_use_background_image,
			&value_type::default_profile, &value_type::default_profile_image, &value_type::following, &value_type::follow_request_sent, &value_type::notifications>();
};

template<> struct jsonifier::core<status_data> {
	using value_type				 = status_data;
	static constexpr auto parseValue = createValue<&value_type::metadata, &value_type::created_at, &value_type::id, &value_type::id_str, &value_type::text, &value_type::source,
		&value_type::truncated, &value_type::in_reply_to_status_id, &value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id, &value_type::in_reply_to_user_id_str,
		&value_type::in_reply_to_screen_name, &value_type::user, &value_type::geo, &value_type::coordinates, &value_type::place, &value_type::contributors,
		&value_type::retweet_count, &value_type::favorite_count, &value_type::entities, &value_type::favorited, &value_type::retweeted, &value_type::lang,
		&value_type::retweeted_status, &value_type::possibly_sensitive>();
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue<&value_type::statuses, &value_type::search_metadata>();
};

template<> struct jsonifier::core<user_data_partial> {
	using value_type				 = user_data_partial;
	static constexpr auto parseValue = createValue<&value_type::screen_name>();
};

template<> struct jsonifier::core<status_data_partial> {
	using value_type				 = status_data_partial;
	static constexpr auto parseValue = createValue<&value_type::retweet_count, &value_type::text, &value_type::user>();
};

template<> struct jsonifier::core<twitter_partial_message> {
	using value_type				 = twitter_partial_message;
	static constexpr auto parseValue = createValue<&value_type::statuses>();
};

template<> struct jsonifier::core<emoji_data> {
	using value_type = emoji_data;
	static constexpr auto parseValue =
		createValue<&value_type::id, &value_type::name, &value_type::roles, &value_type::require_colons, &value_type::managed, &value_type::animated, &value_type::available>();
};

template<> struct jsonifier::core<incidents_data_data> {
	using value_type				 = incidents_data_data;
	static constexpr auto parseValue = createValue<&value_type::invites_disabled_until, &value_type::dms_disabled_until>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<&value_type::bot_id, &value_type::guild_connections>();
};

template<> struct jsonifier::core<role_data> {
	using value_type = role_data;
	static constexpr auto parseValue =
		createValue<&value_type::id, &value_type::name, &value_type::description, &value_type::permissions, &value_type::position, &value_type::color, &value_type::hoist,
			&value_type::managed, &value_type::mentionable, &value_type::icon, &value_type::unicode_emoji, &value_type::flags, &value_type::tags>();
};

template<> struct jsonifier::core<sticker_data> {
	using value_type				 = sticker_data;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::name, &value_type::tags, &value_type::type, &value_type::format_type, &value_type::description,
		&value_type::asset, &value_type::available, &value_type::guild_id>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type = discord_message;
	static constexpr auto parseValue =
		createValue<&value_type::id, &value_type::name, &value_type::icon, &value_type::description, &value_type::home_header, &value_type::splash, &value_type::discovery_splash,
			&value_type::features, &value_type::banner, &value_type::owner_id, &value_type::application_id, &value_type::region, &value_type::afk_channel_id,
			&value_type::afk_timeout, &value_type::system_channel_id, &value_type::system_channel_flags, &value_type::widget_enabled, &value_type::widget_channel_id,
			&value_type::verification_level, &value_type::roles, &value_type::default_message_notifications, &value_type::mfa_level, &value_type::explicit_content_filter,
			&value_type::max_presences, &value_type::max_members, &value_type::max_stage_video_channel_users, &value_type::max_video_channel_users, &value_type::vanity_url_code,
			&value_type::premium_tier, &value_type::premium_subscription_count, &value_type::preferred_locale, &value_type::rules_channel_id, &value_type::safety_alerts_channel_id,
			&value_type::public_updates_channel_id, &value_type::hub_type, &value_type::premium_progress_bar_enabled, &value_type::latest_onboarding_question_id, &value_type::nsfw,
			&value_type::nsfw_level, &value_type::emojis, &value_type::stickers, &value_type::incidents_data, &value_type::inventory_settings, &value_type::embed_enabled,
			&value_type::embed_channel_id, &value_type::approximate_member_count, &value_type::approximate_presence_count>();
};

template<> struct jsonifier::core<geometry_data> {
	using value_type				 = geometry_data;
	static constexpr auto parseValue = createValue<&value_type::type, &value_type::coordinates>();
};

template<> struct jsonifier::core<properties_data> {
	using value_type				 = properties_data;
	static constexpr auto parseValue = createValue<&value_type::name>();
};

template<> struct jsonifier::core<feature> {
	using value_type				 = feature;
	static constexpr auto parseValue = createValue<&value_type::type, &value_type::properties, &value_type::geometry>();
};

template<> struct jsonifier::core<canada_message> {
	using value_type				 = canada_message;
	static constexpr auto parseValue = createValue<&value_type::type, &value_type::features>();
};
template<> struct jsonifier::core<test_element_final> {
	using value_type				 = test_element_final;
	static constexpr auto parseValue = createValue<&value_type::library_name, &value_type::result_type, &value_type::result_speed, &value_type::color>();
};
template<> struct jsonifier::core<test_elements_final> {
	using value_type				 = test_elements_final;
	static constexpr auto parseValue = createValue<&value_type::results, &value_type::test_name>();
};
template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<&value_type::test_string, &value_type::test_uint, &value_type::test_int, &value_type::test_double, &value_type::test_bool>();
};
template<> struct jsonifier::core<partial_test_struct> {
	using value_type				 = partial_test_struct;
	static constexpr auto parseValue = createValue<&value_type::test_bool, &value_type::test_string>();
};
template<> struct jsonifier::core<abc_test_struct> {
	using value_type				 = abc_test_struct;
	static constexpr auto parseValue = createValue<&value_type::test_bool, &value_type::test_double, &value_type::test_int, &value_type::test_uint, &value_type::test_string>();
};
template<typename value_type_new> struct jsonifier::core<test<value_type_new>> {
	using value_type				 = test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};
template<typename value_type_new> struct jsonifier::core<partial_test<value_type_new>> {
	using value_type				 = partial_test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::m>();
};
template<typename value_type_new> struct jsonifier::core<abc_test<value_type_new>> {
	using value_type				 = abc_test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::z, &value_type::y, &value_type::x, &value_type::w, &value_type::v, &value_type::u, &value_type::t, &value_type::s,
		&value_type::r, &value_type::q, &value_type::p, &value_type::o, &value_type::n, &value_type::m, &value_type::l, &value_type::k, &value_type::j, &value_type::i,
		&value_type::h, &value_type::g, &value_type::f, &value_type::e, &value_type::d, &value_type::c, &value_type::b, &value_type::a>();
};
template<result_type type> struct jsonifier::core<result<type>> {
	using value_type				 = result<type>;
	static constexpr auto parseValue = createValue<&value_type::byte_length, &value_type::json_speed, &value_type::json_time, &value_type::color>();
};
template<> struct jsonifier::core<results_data> {
	using value_type				 = results_data;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::read_result, &value_type::write_result, &value_type::test, &value_type::url>();
};
template<> struct jsonifier::core<test_results> {
	using value_type				 = test_results;
	static constexpr auto parseValue = createValue<&value_type::results, &value_type::test_name>();
};