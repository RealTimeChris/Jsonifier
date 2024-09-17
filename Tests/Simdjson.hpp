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
#include "simdjson.h"
#include "Common.hpp"

#if !defined(ASAN_ENABLED)

template<typename value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	value.get(valueNew);
}

template<jsonifier::concepts::bool_t value_type> void getValue(value_type&& valueNew, simdjson::ondemand::value value) {
	getValue<bool>(valueNew, value);
}

template<> void getValue(std::nullptr_t&, simdjson::ondemand::value) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	auto oldSize = valueNew.size();
	if (auto resultCode = value.get(result); !resultCode) {
		auto iter = result.begin();
		for (size_t x = 0; x < oldSize && iter != result.end(); ++x, ++iter) {
			getValue(valueNew[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(valueNew.emplace_back(), iter.operator*().value());
		}
	}
}

template<> void getValue<std::string>(std::string& valueNew, simdjson::ondemand::value value) {
	std::string_view result;
	if (auto resultCode = value.get(result); !resultCode) {
		valueNew = static_cast<jsonifier::string>(result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value valueNewer) {
	simdjson::ondemand::value result;
	if (auto resultCode = valueNewer.get(result); !resultCode) {
		getValue(valueNew.emplace(), result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		if (result.type() != simdjson::ondemand::json_type::null) {
			getValue(returnValue.emplace(), result);
		}
	}
}

template<typename value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		getValue(returnValue, result);
	}
}

template<> void getValue(std::nullptr_t&, simdjson::ondemand::value, const std::string&) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::array result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (size_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
			getValue(returnValues[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(returnValues.emplace_back(), iter.operator*().value());
		}
	}
	return;
}

template<jsonifier::concepts::map_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::object result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (size_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
			typename value_type::mapped_type returnValue{};
			getValue(returnValue, iter.operator*().value());
			returnValues[static_cast<typename value_type::key_type>(iter.operator*().key().raw())] = std::move(returnValue);
		}
		for (; iter != result.end(); ++iter) {
			typename value_type::mapped_type returnValue{};
			getValue(returnValue, iter.operator*().value());
			returnValues[static_cast<typename value_type::key_type>(iter.operator*().key().raw())] = std::move(returnValue);
		}
	}
	return;
}

template<> void getValue(names& returnValue, simdjson::ondemand::value jsonData) {
}

template<> void getValue(audience_sub_category_names& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.the337100890, jsonData, "337100890");
}

template<> void getValue(event& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.subTopicIds, jsonData, "subTopicIds");
	getValue(returnValue.logo, jsonData, "logo");
	getValue(returnValue.topicIds, jsonData, "topicIds");
	getValue(returnValue.subjectCode, jsonData, "subjectCode");
	getValue(returnValue.subtitle, jsonData, "subtitle");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(price& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.audienceSubCategoryId, jsonData, "audienceSubCategoryId");
	getValue(returnValue.seatCategoryId, jsonData, "seatCategoryId");
	getValue(returnValue.amount, jsonData, "amount");
}

template<> void getValue(area& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.blockIds, jsonData, "blockIds");
	getValue(returnValue.areaId, jsonData, "areaId");
}

template<> void getValue(seat_category& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.areas, jsonData, "areas");
	getValue(returnValue.seatCategoryId, jsonData, "seatCategoryId");
}

template<> void getValue(performance& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.seatCategories, jsonData, "seatCategories");
	getValue(returnValue.logo, jsonData, "logo");
	getValue(returnValue.seatMapImage, jsonData, "seatMapImage");
	getValue(returnValue.prices, jsonData, "prices");
	getValue(returnValue.venueCode, jsonData, "venueCode");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.eventId, jsonData, "eventId");
	getValue(returnValue.start, jsonData, "start");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(venue_names& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.PLEYEL_PLEYEL, jsonData, "PLEYEL_PLEYEL");
}

template<> void getValue(citm_catalog_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.audienceSubCategoryNames, jsonData, "audienceSubCategoryNames");
	getValue(returnValue.topicSubTopics, jsonData, "topicSubTopics");
	getValue(returnValue.seatCategoryNames, jsonData, "seatCategoryNames");
	getValue(returnValue.subTopicNames, jsonData, "subTopicNames");
	getValue(returnValue.areaNames, jsonData, "areaNames");
	getValue(returnValue.topicNames, jsonData, "topicNames");
	getValue(returnValue.performances, jsonData, "performances");
	getValue(returnValue.events, jsonData, "events");
	getValue(returnValue.venueNames, jsonData, "venueNames");
	getValue(returnValue.subjectNames, jsonData, "subjectNames");
	getValue(returnValue.blockNames, jsonData, "blockNames");
}

template<> void getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.testBools, jsonData, "testBools");
	getValue(returnValue.testInts, jsonData, "testInts");
	getValue(returnValue.testUints, jsonData, "testUints");
	getValue(returnValue.testStrings, jsonData, "testStrings");
	getValue(returnValue.testDoubles, jsonData, "testDoubles");
}

template<> void getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.a, jsonData, "a");
	getValue(returnValue.b, jsonData, "b");
	getValue(returnValue.c, jsonData, "c");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.e, jsonData, "e");
	getValue(returnValue.f, jsonData, "f");
	getValue(returnValue.g, jsonData, "g");
	getValue(returnValue.h, jsonData, "h");
	getValue(returnValue.i, jsonData, "i");
	getValue(returnValue.j, jsonData, "j");
	getValue(returnValue.k, jsonData, "k");
	getValue(returnValue.l, jsonData, "l");
	getValue(returnValue.m, jsonData, "m");
	getValue(returnValue.n, jsonData, "n");
	getValue(returnValue.o, jsonData, "o");
	getValue(returnValue.p, jsonData, "p");
	getValue(returnValue.q, jsonData, "q");
	getValue(returnValue.r, jsonData, "r");
	getValue(returnValue.s, jsonData, "s");
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.u, jsonData, "u");
	getValue(returnValue.v, jsonData, "v");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.x, jsonData, "x");
	getValue(returnValue.y, jsonData, "y");
	getValue(returnValue.z, jsonData, "z");
}

template<> void getValue(geometry_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.coordinates, jsonData, "coordinates");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(properties_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.name, jsonData, "name");
}

template<> void getValue(feature& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.properties, jsonData, "properties");
	getValue(returnValue.geometry, jsonData, "geometry");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(canada_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.features, jsonData, "features");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(search_metadata_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.since_id_str, jsonData, "since_id_str");
	getValue(returnValue.next_results, jsonData, "next_results");
	getValue(returnValue.refresh_url, jsonData, "refresh_url");
	getValue(returnValue.max_id_str, jsonData, "max_id_str");
	getValue(returnValue.completed_in, jsonData, "completed_in");
	getValue(returnValue.query, jsonData, "query");
	getValue(returnValue.since_id, jsonData, "since_id");
	getValue(returnValue.count, jsonData, "count");
	getValue(returnValue.max_id, jsonData, "max_id");
}

template<> void getValue(hashtag_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.text, jsonData, "text");
}

template<> void getValue(large_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.resize, jsonData, "resize");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.h, jsonData, "h");
}

template<> void getValue(sizes_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.medium, jsonData, "medium");
	getValue(returnValue.small, jsonData, "small");
	getValue(returnValue.thumb, jsonData, "thumb");
	getValue(returnValue.large, jsonData, "large");
}

template<> void getValue(media_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.source_status_id_str, jsonData, "source_status_id_str");
	getValue(returnValue.source_status_id, jsonData, "source_status_id");
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.media_url_https, jsonData, "media_url_https");
	getValue(returnValue.expanded_url, jsonData, "expanded_url");
	getValue(returnValue.display_url, jsonData, "display_url");
	getValue(returnValue.media_url, jsonData, "media_url");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.type, jsonData, "type");
	getValue(returnValue.sizes, jsonData, "sizes");
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(url_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.expanded_url, jsonData, "expanded_url");
	getValue(returnValue.display_url, jsonData, "display_url");
	getValue(returnValue.url, jsonData, "url");
}

template<> void getValue(user_mention& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.screen_name, jsonData, "screen_name");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(status_entities& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.media, jsonData, "media");
	getValue(returnValue.user_mentions, jsonData, "user_mentions");
	getValue(returnValue.symbols, jsonData, "symbols");
	getValue(returnValue.hashtags, jsonData, "hashtags");
	getValue(returnValue.urls, jsonData, "urls");
}

template<> void getValue(metadata_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.iso_language_code, jsonData, "iso_language_code");
	getValue(returnValue.result_type, jsonData, "result_type");
}

template<> void getValue(description_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.urls, jsonData, "urls");
}

template<> void getValue(user_entities& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.description, jsonData, "description");
}

template<> void getValue(twitter_user_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.profile_background_image_url_https, jsonData, "profile_background_image_url_https");
	getValue(returnValue.profile_banner_url, jsonData, "profile_banner_url");
	getValue(returnValue.profile_background_image_url, jsonData, "profile_background_image_url");
	getValue(returnValue.profile_sidebar_fill_color, jsonData, "profile_sidebar_fill_color");
	getValue(returnValue.time_zone, jsonData, "time_zone");
	getValue(returnValue.profile_background_color, jsonData, "profile_background_color");
	getValue(returnValue.profile_image_url_https, jsonData, "profile_image_url_https");
	getValue(returnValue.utc_offset, jsonData, "utc_offset");
	getValue(returnValue.profile_use_background_image, jsonData, "profile_use_background_image");
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.profile_text_color, jsonData, "profile_text_color");
	getValue(returnValue.profile_link_color, jsonData, "profile_link_color");
	getValue(returnValue.profile_image_url, jsonData, "profile_image_url");
	getValue(returnValue.profile_background_tile, jsonData, "profile_background_tile");
	getValue(returnValue.is_translation_enabled, jsonData, "is_translation_enabled");
	getValue(returnValue.default_profile_image, jsonData, "default_profile_image");
	getValue(returnValue.contributors_enabled, jsonData, "contributors_enabled");
	getValue(returnValue.follow_request_sent, jsonData, "follow_request_sent");
	getValue(returnValue.favourites_count, jsonData, "favourites_count");
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.screen_name, jsonData, "screen_name");
	getValue(returnValue.followers_count, jsonData, "followers_count");
	getValue(returnValue.statuses_count, jsonData, "statuses_count");
	getValue(returnValue.created_at, jsonData, "created_at");
	getValue(returnValue.entities, jsonData, "entities");
	getValue(returnValue.friends_count, jsonData, "friends_count");
	getValue(returnValue.default_profile, jsonData, "default_profile");
	getValue(returnValue.listed_count, jsonData, "listed_count");
	getValue(returnValue.location, jsonData, "location");
	getValue(returnValue.protectedVal, jsonData, "protected");
	getValue(returnValue.is_translator, jsonData, "is_translator");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.notifications, jsonData, "notifications");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.geo_enabled, jsonData, "geo_enabled");
	getValue(returnValue.lang, jsonData, "lang");
	getValue(returnValue.following, jsonData, "following");
	getValue(returnValue.verified, jsonData, "verified");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(std::shared_ptr<status_data>& returnValue, simdjson::ondemand::value jsonData) {
	returnValue = std::make_shared<status_data>();
	getValue(*returnValue, jsonData, "retweeted_status");
}

template<> void getValue(status_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.in_reply_to_status_id_str, jsonData, "in_reply_to_status_id_str");
	getValue(returnValue.in_reply_to_user_id_str, jsonData, "in_reply_to_user_id_str");
	getValue(returnValue.in_reply_to_screen_name, jsonData, "in_reply_to_screen_name");
	getValue(returnValue.in_reply_to_status_id, jsonData, "in_reply_to_status_id");
	getValue(returnValue.in_reply_to_user_id, jsonData, "in_reply_to_user_id");
	getValue(returnValue.possibly_sensitive, jsonData, "possibly_sensitive");
	getValue(returnValue.contributors, jsonData, "contributors");
	getValue(returnValue.coordinates, jsonData, "coordinates");
	getValue(returnValue.place, jsonData, "place");
	getValue(returnValue.geo, jsonData, "geo");
	getValue(returnValue.entities, jsonData, "entities");
	getValue(returnValue.favorite_count, jsonData, "favorite_count");
	getValue(returnValue.metadata, jsonData, "metadata");
	getValue(returnValue.created_at, jsonData, "created_at");
	getValue(returnValue.retweet_count, jsonData, "retweet_count");
	getValue(returnValue.source, jsonData, "source");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.user, jsonData, "user");
	getValue(returnValue.lang, jsonData, "lang");
	getValue(returnValue.text, jsonData, "text");
	getValue(returnValue.truncated, jsonData, "truncated");
	getValue(returnValue.favorited, jsonData, "favorited");
	getValue(returnValue.retweeted, jsonData, "retweeted");
	getValue(returnValue.retweeted_status, jsonData, "retweeted_status");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(twitter_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.search_metadata, jsonData, "search_metadata");
	getValue(returnValue.statuses, jsonData, "statuses");
}

template<> void getValue(icon_emoji_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.name, jsonData, "name");
	// Since nullptr_t is not typically deserialized, handling it as necessary.
}

template<> void getValue(permission_overwrite& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.allow, jsonData, "allow");
	getValue(returnValue.deny, jsonData, "deny");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(channel_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.permission_overwrites, jsonData, "permission_overwrites");
	getValue(returnValue.last_message_id, jsonData, "last_message_id");
	getValue(returnValue.default_thread_rate_limit_per_user, jsonData, "default_thread_rate_limit_per_user");
	getValue(returnValue.applied_tags, jsonData, "applied_tags");
	getValue(returnValue.recipients, jsonData, "recipients");
	getValue(returnValue.default_auto_archive_duration, jsonData, "default_auto_archive_duration");
	getValue(returnValue.status, jsonData, "status");
	getValue(returnValue.last_pin_timestamp, jsonData, "last_pin_timestamp");
	getValue(returnValue.topic, jsonData, "topic");
	getValue(returnValue.rate_limit_per_user, jsonData, "rate_limit_per_user");
	getValue(returnValue.icon_emoji, jsonData, "icon_emoji");
	getValue(returnValue.total_message_sent, jsonData, "total_message_sent");
	getValue(returnValue.video_quality_mode, jsonData, "video_quality_mode");
	getValue(returnValue.application_id, jsonData, "application_id");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.message_count, jsonData, "message_count");
	getValue(returnValue.parent_id, jsonData, "parent_id");
	getValue(returnValue.member_count, jsonData, "member_count");
	getValue(returnValue.owner_id, jsonData, "owner_id");
	getValue(returnValue.guild_id, jsonData, "guild_id");
	getValue(returnValue.user_limit, jsonData, "user_limit");
	getValue(returnValue.position, jsonData, "position");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.version, jsonData, "version");
	getValue(returnValue.bitrate, jsonData, "bitrate");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.type, jsonData, "type");
	getValue(returnValue.managed, jsonData, "managed");
	getValue(returnValue.nsfw, jsonData, "nsfw");
}

template<> void getValue(user_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.avatar_decoration_data, jsonData, "avatar_decoration_data");
	getValue(returnValue.display_name, jsonData, "display_name");
	getValue(returnValue.global_name, jsonData, "global_name");
	getValue(returnValue.avatar, jsonData, "avatar");
	getValue(returnValue.banner, jsonData, "banner");
	getValue(returnValue.locale, jsonData, "locale");
	getValue(returnValue.discriminator, jsonData, "discriminator");
	getValue(returnValue.user_name, jsonData, "user_name");
	getValue(returnValue.accent_color, jsonData, "accent_color");
	getValue(returnValue.premium_type, jsonData, "premium_type");
	getValue(returnValue.public_flags, jsonData, "public_flags");
	getValue(returnValue.email, jsonData, "email");
	getValue(returnValue.mfa_enabled, jsonData, "mfa_enabled");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.verified, jsonData, "verified");
	getValue(returnValue.system, jsonData, "system");
	getValue(returnValue.bot, jsonData, "bot");
}

template<> void getValue(member_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.communication_disabled_until, jsonData, "communication_disabled_until");
	getValue(returnValue.premium_since, jsonData, "premium_since");
	getValue(returnValue.nick, jsonData, "nick");
	getValue(returnValue.avatar, jsonData, "avatar");
	getValue(returnValue.roles, jsonData, "roles");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.joined_at, jsonData, "joined_at");
	getValue(returnValue.guild_id, jsonData, "guild_id");
	getValue(returnValue.user, jsonData, "user");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.pending, jsonData, "pending");
	getValue(returnValue.deaf, jsonData, "deaf");
	getValue(returnValue.mute, jsonData, "mute");
}

template<> void getValue(tags_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.premium_subscriber, jsonData, "premium_subscriber");
	getValue(returnValue.bot_id, jsonData, "bot_id");
}

template<> void getValue(role_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.unicode_emoji, jsonData, "unicode_emoji");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.position, jsonData, "position");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.mentionable, jsonData, "mentionable");
	getValue(returnValue.version, jsonData, "version");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.tags, jsonData, "tags");
	getValue(returnValue.color, jsonData, "color");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.managed, jsonData, "managed");
	getValue(returnValue.hoist, jsonData, "hoist");
}

template<> void getValue(guild_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.latest_on_boarding_question_id, jsonData, "latest_on_boarding_question_id");
	getValue(returnValue.guild_scheduled_events, jsonData, "guild_scheduled_events");
	getValue(returnValue.safety_alerts_channel_id, jsonData, "safety_alerts_channel_id");
	getValue(returnValue.inventory_settings, jsonData, "inventory_settings");
	getValue(returnValue.voice_states, jsonData, "voice_states");
	getValue(returnValue.discovery_splash, jsonData, "discovery_splash");
	getValue(returnValue.vanity_url_code, jsonData, "vanity_url_code");
	getValue(returnValue.application_id, jsonData, "application_id");
	getValue(returnValue.afk_channel_id, jsonData, "afk_channel_id");
	getValue(returnValue.default_message_notifications, jsonData, "default_message_notifications");
	getValue(returnValue.max_stage_video_channel_users, jsonData, "max_stage_video_channel_users");
	getValue(returnValue.public_updates_channel_id, jsonData, "public_updates_channel_id");
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.threads, jsonData, "threads");
	getValue(returnValue.channels, jsonData, "channels");
	getValue(returnValue.premium_subscription_count, jsonData, "premium_subscription_count");
	getValue(returnValue.approximate_presence_count, jsonData, "approximate_presence_count");
	getValue(returnValue.features, jsonData, "features");
	getValue(returnValue.stickers, jsonData, "stickers");
	getValue(returnValue.premium_progress_bar_enabled, jsonData, "premium_progress_bar_enabled");
	getValue(returnValue.members, jsonData, "members");
	getValue(returnValue.hub_type, jsonData, "hub_type");
	getValue(returnValue.approximate_member_count, jsonData, "approximate_member_count");
	getValue(returnValue.explicit_content_filter, jsonData, "explicit_content_filter");
	getValue(returnValue.max_video_channel_users, jsonData, "max_video_channel_users");
	getValue(returnValue.splash, jsonData, "splash");
	getValue(returnValue.banner, jsonData, "banner");
	getValue(returnValue.system_channel_id, jsonData, "system_channel_id");
	getValue(returnValue.widget_channel_id, jsonData, "widget_channel_id");
	getValue(returnValue.preferred_locale, jsonData, "preferred_locale");
	getValue(returnValue.system_channel_flags, jsonData, "system_channel_flags");
	getValue(returnValue.rules_channel_id, jsonData, "rules_channel_id");
	getValue(returnValue.roles, jsonData, "roles");
	getValue(returnValue.verification_level, jsonData, "verification_level");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.max_presences, jsonData, "max_presences");
	getValue(returnValue.discovery, jsonData, "discovery");
	getValue(returnValue.joined_at, jsonData, "joined_at");
	getValue(returnValue.member_count, jsonData, "member_count");
	getValue(returnValue.premium_tier, jsonData, "premium_tier");
	getValue(returnValue.owner_id, jsonData, "owner_id");
	getValue(returnValue.max_members, jsonData, "max_members");
	getValue(returnValue.afk_timeout, jsonData, "afk_timeout");
	getValue(returnValue.widget_enabled, jsonData, "widget_enabled");
	getValue(returnValue.region, jsonData, "region");
	getValue(returnValue.nsfw_level, jsonData, "nsfw_level");
	getValue(returnValue.mfa_level, jsonData, "mfa_level");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.unavailable, jsonData, "unavailable");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.large, jsonData, "large");
	getValue(returnValue.owner, jsonData, "owner");
	getValue(returnValue.nsfw, jsonData, "nsfw");
	getValue(returnValue.lazy, jsonData, "lazy");
}

template<> void getValue(discord_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.op, jsonData, "op");
	getValue(returnValue.s, jsonData, "s");
}

template<> void getValue(abc_test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.a, jsonData, "a");
	getValue(returnValue.b, jsonData, "b");
	getValue(returnValue.c, jsonData, "c");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.e, jsonData, "e");
	getValue(returnValue.f, jsonData, "f");
	getValue(returnValue.g, jsonData, "g");
	getValue(returnValue.h, jsonData, "h");
	getValue(returnValue.i, jsonData, "i");
	getValue(returnValue.j, jsonData, "j");
	getValue(returnValue.k, jsonData, "k");
	getValue(returnValue.l, jsonData, "l");
	getValue(returnValue.m, jsonData, "m");
	getValue(returnValue.n, jsonData, "n");
	getValue(returnValue.o, jsonData, "o");
	getValue(returnValue.p, jsonData, "p");
	getValue(returnValue.q, jsonData, "q");
	getValue(returnValue.r, jsonData, "r");
	getValue(returnValue.s, jsonData, "s");
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.u, jsonData, "u");
	getValue(returnValue.v, jsonData, "v");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.x, jsonData, "x");
	getValue(returnValue.y, jsonData, "y");
	getValue(returnValue.z, jsonData, "z");
}

#endif