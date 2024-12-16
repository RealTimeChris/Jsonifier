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

#include <BnchSwt/BenchmarkSuite.hpp>
#include <jsonifier/Index.hpp>
#include "Common.hpp"

template<> struct jsonifier::core<audience_sub_category_names> {
	using value_type				 = audience_sub_category_names;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::the337100890, "337100890">()>();
};

template<> struct jsonifier::core<names> {
	using value_type				 = names;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<event> {
	using value_type				 = event;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::description>(), make_json_entity<&value_type::id>(), make_json_entity<&value_type::logo>(),
		make_json_entity<&value_type::name>(), make_json_entity<&value_type::subTopicIds>(), make_json_entity<&value_type::subjectCode>(),
		make_json_entity<&value_type::subtitle>(), make_json_entity<&value_type::topicIds>()>();
};

template<> struct jsonifier::core<price> {
	using value_type				 = price;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::amount>(), &value_type::audienceSubCategoryId, make_json_entity<&value_type::seatCategoryId>()>();
};

template<> struct jsonifier::core<area> {
	using value_type				 = area;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::areaId>(), make_json_entity<&value_type::blockIds>()>();
};

template<> struct jsonifier::core<seat_category> {
	using value_type				 = seat_category;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::areas>(), make_json_entity<&value_type::seatCategoryId>()>();
};

template<> struct jsonifier::core<performance> {
	using value_type				 = performance;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::eventId>(), make_json_entity<&value_type::id>(), make_json_entity<&value_type::logo>(),
		make_json_entity<&value_type::name>(), make_json_entity<&value_type::prices>(), make_json_entity<&value_type::seatCategories>(),
		make_json_entity<&value_type::seatMapImage>(), make_json_entity<&value_type::start>(), make_json_entity<&value_type::venueCode>()>();
};

template<> struct jsonifier::core<venue_names> {
	using value_type				 = venue_names;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::PLEYEL_PLEYEL>()>();
};

template<> struct jsonifier::core<citm_catalog_message> {
	using value_type				 = citm_catalog_message;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::areaNames>(), make_json_entity<&value_type::audienceSubCategoryNames>(),
		make_json_entity<&value_type::blockNames>(), make_json_entity<&value_type::events>(), make_json_entity<&value_type::performances>(),
		make_json_entity<&value_type::seatCategoryNames>(), make_json_entity<&value_type::subTopicNames>(), make_json_entity<&value_type::subjectNames>(),
		make_json_entity<&value_type::topicNames>(), make_json_entity<&value_type::topicSubTopics>(), make_json_entity<&value_type::venueNames>()>();
};

template<> struct jsonifier::core<partial_price> {
	using parseValue_type			 = partial_price;
	static constexpr auto parseValue = createValue<make_json_entity<&parseValue_type::seatCategoryId>()>();
};

template<> struct jsonifier::core<partial_performance> {
	using parseValue_type			 = partial_performance;
	static constexpr auto parseValue = createValue<make_json_entity<&parseValue_type::prices>(), make_json_entity<&parseValue_type::venueCode>()>();
};

template<> struct jsonifier::core<citm_catalog_partial_message> {
	using parseValue_type			 = citm_catalog_partial_message;
	static constexpr auto parseValue = createValue<make_json_entity<&parseValue_type::performances>(), make_json_entity<&parseValue_type::venueNames>()>();
};

template<> struct jsonifier::core<search_metadata_data> {
	using value_type = search_metadata_data;
	static constexpr auto parseValue =
		createValue<make_json_entity<&value_type::completed_in>(), make_json_entity<&value_type::max_id>(), make_json_entity<&value_type::max_id_str>(),
			make_json_entity<&value_type::next_results>(), make_json_entity<&value_type::query>(), make_json_entity<&value_type::refresh_url>(),
			make_json_entity<&value_type::count>(), make_json_entity<&value_type::since_id>(), make_json_entity<&value_type::since_id_str>()>();
};

template<> struct jsonifier::core<hashtag_data> {
	using value_type				 = hashtag_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::text>(), make_json_entity<&value_type::indices>()>();
};


template<> struct jsonifier::core<large_data> {
	using value_type				 = large_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::w>(), make_json_entity<&value_type::h>(), make_json_entity<&value_type::resize>()>();
};

template<> struct jsonifier::core<sizes_data> {
	using value_type				 = sizes_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::medium>(), make_json_entity<&value_type::small>(), make_json_entity<&value_type::thumb>(),
		make_json_entity<&value_type::large>()>();
};

template<> struct jsonifier::core<media_data> {
	using value_type				 = media_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::id>(), make_json_entity<&value_type::id_str>(), make_json_entity<&value_type::indices>(),
		make_json_entity<&value_type::media_url>(), make_json_entity<&value_type::media_url_https>(), make_json_entity<&value_type::url>(),
		make_json_entity<&value_type::display_url>(), make_json_entity<&value_type::expanded_url>(), make_json_entity<&value_type::type>(), make_json_entity<&value_type::sizes>(),
		make_json_entity<&value_type::source_status_id>(), make_json_entity<&value_type::source_status_id_str>()>();
};

template<> struct jsonifier::core<url_data> {
	using value_type				 = url_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::url>(), make_json_entity<&value_type::expanded_url>(),
		make_json_entity<&value_type::display_url>(), make_json_entity<&value_type::indices>()>();
};

template<> struct jsonifier::core<user_mention_data> {
	using value_type				 = user_mention_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::screen_name>(), make_json_entity<&value_type::name>(), make_json_entity<&value_type::id>(),
		make_json_entity<&value_type::id_str>(), make_json_entity<&value_type::indices>()>();
};

template<> struct jsonifier::core<status_entities> {
	using value_type				 = status_entities;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::hashtags>(), make_json_entity<&value_type::symbols>(), make_json_entity<&value_type::urls>(),
		make_json_entity<&value_type::user_mentions>(), make_json_entity<&value_type::media>()>();
};

template<> struct jsonifier::core<metadata_data> {
	using value_type				 = metadata_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::result_type>(), make_json_entity<&value_type::iso_language_code>()>();
};

template<> struct jsonifier::core<description_data> {
	using value_type				 = description_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::urls>()>();
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::description>(), make_json_entity<&value_type::url>()>();
};

template<> struct jsonifier::core<twitter_user_data> {
	using value_type				 = twitter_user_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::id>(), make_json_entity<&value_type::id_str>(), make_json_entity<&value_type::name>(),
		make_json_entity<&value_type::screen_name>(), make_json_entity<&value_type::location>(), make_json_entity<&value_type::description>(), make_json_entity<&value_type::url>(),
		make_json_entity<&value_type::entities>(), make_json_entity<&value_type::protectedVal, "protected">(), make_json_entity<&value_type::followers_count>(),
		make_json_entity<&value_type::friends_count>(), make_json_entity<&value_type::listed_count>(), make_json_entity<&value_type::created_at>(),
		make_json_entity<&value_type::favourites_count>(), make_json_entity<&value_type::utc_offset>(), make_json_entity<&value_type::time_zone>(),
		make_json_entity<&value_type::geo_enabled>(), make_json_entity<&value_type::verified>(), make_json_entity<&value_type::statuses_count>(),
		make_json_entity<&value_type::lang>(), make_json_entity<&value_type::contributors_enabled>(), make_json_entity<&value_type::is_translator>(),
		make_json_entity<&value_type::is_translation_enabled>(), make_json_entity<&value_type::profile_background_color>(),
		make_json_entity<&value_type::profile_background_image_url>(), make_json_entity<&value_type::profile_background_image_url_https>(),
		make_json_entity<&value_type::profile_background_tile>(), make_json_entity<&value_type::profile_image_url>(), make_json_entity<&value_type::profile_image_url_https>(),
		make_json_entity<&value_type::profile_banner_url>(), make_json_entity<&value_type::profile_link_color>(), make_json_entity<&value_type::profile_sidebar_border_color>(),
		make_json_entity<&value_type::profile_sidebar_fill_color>(), make_json_entity<&value_type::profile_text_color>(),
		make_json_entity<&value_type::profile_use_background_image>(), make_json_entity<&value_type::default_profile>(), make_json_entity<&value_type::default_profile_image>(),
		make_json_entity<&value_type::following>(), make_json_entity<&value_type::follow_request_sent>(), make_json_entity<&value_type::notifications>()>();
};

template<> struct jsonifier::core<status_data> {
	using value_type				 = status_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::metadata>(), make_json_entity<&value_type::created_at>(), make_json_entity<&value_type::id>(),
		make_json_entity<&value_type::id_str>(), make_json_entity<&value_type::text>(), make_json_entity<&value_type::source>(), make_json_entity<&value_type::truncated>(),
		make_json_entity<&value_type::in_reply_to_status_id>(), make_json_entity<&value_type::in_reply_to_status_id_str>(), make_json_entity<&value_type::in_reply_to_user_id>(),
		make_json_entity<&value_type::in_reply_to_user_id_str>(), make_json_entity<&value_type::in_reply_to_screen_name>(), make_json_entity<&value_type::user>(),
		make_json_entity<&value_type::geo>(), make_json_entity<&value_type::coordinates>(), make_json_entity<&value_type::place>(), make_json_entity<&value_type::contributors>(),
		make_json_entity<&value_type::retweet_count>(), make_json_entity<&value_type::favorite_count>(), make_json_entity<&value_type::entities>(),
		make_json_entity<&value_type::favorited>(), make_json_entity<&value_type::retweeted>(), make_json_entity<&value_type::lang>(),
		make_json_entity<&value_type::retweeted_status>(), make_json_entity<&value_type::possibly_sensitive>()>();
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::statuses>(), make_json_entity<&value_type::search_metadata>()>();
};

template<> struct jsonifier::core<twitter_user_partial_data> {
	using value_type				 = twitter_user_partial_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::screen_name>()>();
};

template<> struct jsonifier::core<status_partial_data> {
	using value_type				 = status_partial_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::text>(), make_json_entity<&value_type::user>(), make_json_entity<&value_type::retweet_count>()>();
};

template<> struct jsonifier::core<twitter_partial_message> {
	using value_type				 = twitter_partial_message;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::statuses>()>();
};

template<> struct jsonifier::core<icon_emoji_data> {
	using value_type				 = icon_emoji_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::name>(), make_json_entity<&value_type::id>()>();
};

template<> struct jsonifier::core<permission_overwrite> {
	using value_type = permission_overwrite;
	static constexpr auto parseValue =
		createValue<make_json_entity<&value_type::allow>(), make_json_entity<&value_type::type>(), make_json_entity<&value_type::deny>(), make_json_entity<&value_type::id>()>();
};

template<> struct jsonifier::core<channel_data> {
	using value_type = channel_data;
	static constexpr auto parseValue =
		createValue<make_json_entity<&value_type::default_thread_rate_limit_per_user>(), make_json_entity<&value_type::default_auto_archive_duration>(),
			make_json_entity<&value_type::permission_overwrites>(), make_json_entity<&value_type::rate_limit_per_user>(), make_json_entity<&value_type::video_quality_mode>(),
			make_json_entity<&value_type::total_message_sent>(), make_json_entity<&value_type::last_pin_timestamp>(), make_json_entity<&value_type::last_message_id>(),
			make_json_entity<&value_type::application_id>(), make_json_entity<&value_type::message_count>(), make_json_entity<&value_type::member_count>(),
			make_json_entity<&value_type::applied_tags>(), make_json_entity<&value_type::permissions>(), make_json_entity<&value_type::user_limit>(),
			make_json_entity<&value_type::icon_emoji>(), make_json_entity<&value_type::recipients>(), make_json_entity<&value_type::parent_id>(),
			make_json_entity<&value_type::position>(), make_json_entity<&value_type::guild_id>(), make_json_entity<&value_type::owner_id>(),
			make_json_entity<&value_type::managed>(), make_json_entity<&value_type::bitrate>(), make_json_entity<&value_type::version>(), make_json_entity<&value_type::status>(),
			make_json_entity<&value_type::flags>(), make_json_entity<&value_type::topic>(), make_json_entity<&value_type::nsfw>(), make_json_entity<&value_type::type>(),
			make_json_entity<&value_type::icon>(), make_json_entity<&value_type::name>(), make_json_entity<&value_type::id>()>();
};


template<> struct jsonifier::core<user_data> {
	using value_type				 = user_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::avatar_decoration_data>(), make_json_entity<&value_type::discriminator>(),
		make_json_entity<&value_type::public_flags>(), make_json_entity<&value_type::premium_type>(), make_json_entity<&value_type::accent_color>(),
		make_json_entity<&value_type::display_name>(), make_json_entity<&value_type::mfa_enabled>(), make_json_entity<&value_type::global_name>(),
		make_json_entity<&value_type::user_name>(), make_json_entity<&value_type::verified>(), make_json_entity<&value_type::system>(), make_json_entity<&value_type::locale>(),
		make_json_entity<&value_type::banner>(), make_json_entity<&value_type::avatar>(), make_json_entity<&value_type::flags>(), make_json_entity<&value_type::email>(),
		make_json_entity<&value_type::bot>(), make_json_entity<&value_type::id>()>();
};

template<> struct jsonifier::core<member_data> {
	using value_type				 = member_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::communication_disabled_until>(), make_json_entity<&value_type::premium_since>(),
		make_json_entity<&value_type::permissions>(), make_json_entity<&value_type::joined_at>(), make_json_entity<&value_type::guild_id>(),
		make_json_entity<&value_type::pending>(), make_json_entity<&value_type::avatar>(), make_json_entity<&value_type::flags>(), make_json_entity<&value_type::roles>(),
		make_json_entity<&value_type::mute>(), make_json_entity<&value_type::deaf>(), make_json_entity<&value_type::user>(), make_json_entity<&value_type::nick>()>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::premium_subscriber>(), make_json_entity<&value_type::bot_id>()>();
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::unicode_emoji>(), make_json_entity<&value_type::mentionable>(),
		make_json_entity<&value_type::permissions>(), make_json_entity<&value_type::position>(), make_json_entity<&value_type::managed>(), make_json_entity<&value_type::version>(),
		make_json_entity<&value_type::hoist>(), make_json_entity<&value_type::flags>(), make_json_entity<&value_type::color>(), make_json_entity<&value_type::tags>(),
		make_json_entity<&value_type::name>(), make_json_entity<&value_type::icon>(), make_json_entity<&value_type::id>()>();
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::latest_on_boarding_question_id>(), make_json_entity<&value_type::max_stage_video_channel_users>(),
		make_json_entity<&value_type::default_message_notifications>(), make_json_entity<&value_type::premium_progress_bar_enabled>(),
		make_json_entity<&value_type::approximate_presence_count>(), make_json_entity<&value_type::premium_subscription_count>(),
		make_json_entity<&value_type::public_updates_channel_id>(), make_json_entity<&value_type::approximate_member_count>(),
		make_json_entity<&value_type::safety_alerts_channel_id>(), make_json_entity<&value_type::max_video_channel_users>(),
		make_json_entity<&value_type::explicit_content_filter>(), make_json_entity<&value_type::guild_scheduled_events>(), make_json_entity<&value_type::system_channel_flags>(),
		make_json_entity<&value_type::verification_level>(), make_json_entity<&value_type::inventory_settings>(), make_json_entity<&value_type::widget_channel_id>(),
		make_json_entity<&value_type::system_channel_id>(), make_json_entity<&value_type::rules_channel_id>(), make_json_entity<&value_type::preferred_locale>(),
		make_json_entity<&value_type::discovery_splash>(), make_json_entity<&value_type::vanity_url_code>(), make_json_entity<&value_type::widget_enabled>(),
		make_json_entity<&value_type::afk_channel_id>(), make_json_entity<&value_type::application_id>(), make_json_entity<&value_type::max_presences>(),
		make_json_entity<&value_type::premium_tier>(), make_json_entity<&value_type::member_count>(), make_json_entity<&value_type::voice_states>(),
		make_json_entity<&value_type::unavailable>(), make_json_entity<&value_type::afk_timeout>(), make_json_entity<&value_type::max_members>(),
		make_json_entity<&value_type::permissions>(), make_json_entity<&value_type::description>(), make_json_entity<&value_type::nsfw_level>(),
		make_json_entity<&value_type::mfa_level>(), make_json_entity<&value_type::joined_at>(), make_json_entity<&value_type::discovery>(),
		make_json_entity<&value_type::owner_id>(), make_json_entity<&value_type::hub_type>(), make_json_entity<&value_type::stickers>(), make_json_entity<&value_type::features>(),
		make_json_entity<&value_type::channels>(), make_json_entity<&value_type::members>(), make_json_entity<&value_type::threads>(), make_json_entity<&value_type::region>(),
		make_json_entity<&value_type::banner>(), make_json_entity<&value_type::splash>(), make_json_entity<&value_type::owner>(), make_json_entity<&value_type::large>(),
		make_json_entity<&value_type::flags>(), make_json_entity<&value_type::roles>(), make_json_entity<&value_type::lazy>(), make_json_entity<&value_type::nsfw>(),
		make_json_entity<&value_type::icon>(), make_json_entity<&value_type::name>(), make_json_entity<&value_type::id>()>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type = discord_message;
	static constexpr auto parseValue =
		createValue<make_json_entity<&value_type::op>(), make_json_entity<&value_type::s>(), make_json_entity<&value_type::d>(), make_json_entity<&value_type::t>()>();
};

template<> struct jsonifier::core<geometry_data> {
	using value_type				 = geometry_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::type>(), make_json_entity<&value_type::coordinates>()>();
};

template<> struct jsonifier::core<properties_data> {
	using value_type				 = properties_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::name>()>();
};

template<> struct jsonifier::core<feature> {
	using value_type				 = feature;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::type>(), make_json_entity<&value_type::properties>(), make_json_entity<&value_type::geometry>()>();
};

template<> struct jsonifier::core<canada_message> {
	using value_type				 = canada_message;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::type>(), make_json_entity<&value_type::features>()>();
};

template<> struct jsonifier::core<test_element_final> {
	using value_type				 = test_element_final;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::libraryName>(), make_json_entity<&value_type::resultType>(),
		make_json_entity<&value_type::resultSpeed>(), make_json_entity<&value_type::color>()>();
};

template<> struct jsonifier::core<test_elements_final> {
	using value_type				 = test_elements_final;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::results>(), make_json_entity<&value_type::testName>()>();
};

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::testVals02>(), make_json_entity<&value_type::testVals05>(),
		make_json_entity<&value_type::testVals01>(), make_json_entity<&value_type::testVals03>(), make_json_entity<&value_type::testVals04>()>();
};

template<> struct jsonifier::core<abc_test_struct> {
	using value_type				 = abc_test_struct;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::testVals04>(), make_json_entity<&value_type::testVals03>(),
		make_json_entity<&value_type::testVals01>(), make_json_entity<&value_type::testVals05>(), make_json_entity<&value_type::testVals02>()>();
};

template<typename value_type_new> struct jsonifier::core<test<value_type_new>> {
	using value_type				 = test<value_type_new>;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::a>(), make_json_entity<&value_type::b>(), make_json_entity<&value_type::c>(),
		make_json_entity<&value_type::d>(), make_json_entity<&value_type::e>(), make_json_entity<&value_type::f>(), make_json_entity<&value_type::g>(),
		make_json_entity<&value_type::h>(), make_json_entity<&value_type::i>(), make_json_entity<&value_type::j>(), make_json_entity<&value_type::k>(),
		make_json_entity<&value_type::l>(), make_json_entity<&value_type::m>(), make_json_entity<&value_type::n>(), make_json_entity<&value_type::o>(),
		make_json_entity<&value_type::p>(), make_json_entity<&value_type::q>(), make_json_entity<&value_type::r>(), make_json_entity<&value_type::s>(),
		make_json_entity<&value_type::t>(), make_json_entity<&value_type::u>(), make_json_entity<&value_type::v>(), make_json_entity<&value_type::w>(),
		make_json_entity<&value_type::x>(), make_json_entity<&value_type::y>(), make_json_entity<&value_type::z>()>();
};

template<typename value_type_new> struct jsonifier::core<test_generator<value_type_new>> {
	using value_type				 = test_generator<value_type_new>;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::a>(), make_json_entity<&value_type::b>(), make_json_entity<&value_type::c>(),
		make_json_entity<&value_type::d>(), make_json_entity<&value_type::e>(), make_json_entity<&value_type::f>(), make_json_entity<&value_type::g>(),
		make_json_entity<&value_type::h>(), make_json_entity<&value_type::i>(), make_json_entity<&value_type::j>(), make_json_entity<&value_type::k>(),
		make_json_entity<&value_type::l>(), make_json_entity<&value_type::m>(), make_json_entity<&value_type::n>(), make_json_entity<&value_type::o>(),
		make_json_entity<&value_type::p>(), make_json_entity<&value_type::q>(), make_json_entity<&value_type::r>(), make_json_entity<&value_type::s>(),
		make_json_entity<&value_type::t>(), make_json_entity<&value_type::u>(), make_json_entity<&value_type::v>(), make_json_entity<&value_type::w>(),
		make_json_entity<&value_type::x>(), make_json_entity<&value_type::y>(), make_json_entity<&value_type::z>()>();
};

template<typename value_type_new> struct jsonifier::core<abc_test<value_type_new>> {
	using value_type				 = abc_test<value_type_new>;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::z>(), make_json_entity<&value_type::y>(), make_json_entity<&value_type::x>(),
		make_json_entity<&value_type::w>(), make_json_entity<&value_type::v>(), make_json_entity<&value_type::u>(), make_json_entity<&value_type::t>(),
		make_json_entity<&value_type::s>(), make_json_entity<&value_type::r>(), make_json_entity<&value_type::q>(), make_json_entity<&value_type::p>(),
		make_json_entity<&value_type::o>(), make_json_entity<&value_type::n>(), make_json_entity<&value_type::m>(), make_json_entity<&value_type::l>(),
		make_json_entity<&value_type::k>(), make_json_entity<&value_type::j>(), make_json_entity<&value_type::i>(), make_json_entity<&value_type::h>(),
		make_json_entity<&value_type::g>(), make_json_entity<&value_type::f>(), make_json_entity<&value_type::e>(), make_json_entity<&value_type::d>(),
		make_json_entity<&value_type::c>(), make_json_entity<&value_type::b>(), make_json_entity<&value_type::a>()>();
};

template<result_type type> struct jsonifier::core<result<type>> {
	using value_type				 = result<type>;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::byteLength>(), make_json_entity<&value_type::jsonSpeed>(),
		make_json_entity<&value_type::jsonTime>(), make_json_entity<&value_type::color>()>();
};

template<> struct jsonifier::core<results_data> {
	using value_type				 = results_data;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::name>(), make_json_entity<&value_type::readResult>(), make_json_entity<&value_type::writeResult>(),
		make_json_entity<&value_type::test>(), make_json_entity<&value_type::url>()>();
};

template<> struct jsonifier::core<test_results> {
	using value_type				 = test_results;
	static constexpr auto parseValue = createValue<make_json_entity<&value_type::results>(), make_json_entity<&value_type::testName>()>();
};
