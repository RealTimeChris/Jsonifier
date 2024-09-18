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

template<typename value_type> inline bool getValue(value_type& data, simdjson::ondemand::value json_value);

template<> inline bool getValue(double& data, simdjson::ondemand::value json_value) {
	return !json_value.get(data);
}

template<> inline bool getValue(bool& data, simdjson::ondemand::value json_value) {
	return !json_value.get(data);
}

template<> inline bool getValue(std::string& data, simdjson::ondemand::value json_value) {
	std::string_view val;
	if (json_value.get(val))
		return false;
	data = val;
	return true;
}

template<jsonifier::concepts::bool_t value_type> inline bool getValue(value_type&& data, simdjson::ondemand::value json_value) {
	bool val;
	if (json_value.get(val))
		return false;
	data = val;
	return true;
}

template<typename value_type> inline bool getValue(std::vector<value_type>& value, simdjson::ondemand::value json_value) {
	const auto size = value.size();
	auto newArray	= json_value.get_array();
	auto iter		= newArray.begin();
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		if (!getValue(value[x], iter.value().operator*().value())) {
			return false;
		}
	}
	for (; iter != newArray.end(); ++iter) {
		if (!getValue(value.emplace_back(), iter.value().operator*().value())) {
			return false;
		}
	}
	return true;
}

template<typename value_type> inline bool getValue(std::optional<value_type>& vec, simdjson::ondemand::value json_value) {
	if (!getValue(vec.emplace(), json_value)) {
		return false;
	}
	return true;
}

template<> inline bool getValue(std::vector<bool>& value, simdjson::ondemand::value json_value) {
	const auto size = value.size();
	auto newArray	= json_value.get_array();
	auto iter		= newArray.begin();
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		bool newValue{};
		if (!getValue(newValue, iter.value().operator*().value())) {
			return false;
		}
		value[x] = newValue;
	}
	for (; iter != newArray.end(); ++iter) {
		if (!getValue(value.emplace_back(), iter.value().operator*().value())) {
			return false;
		}
	}
	return true;
}

template<> inline bool getValue(std::vector<std::nullptr_t>&, simdjson::ondemand::value json_value) {
	auto newArray = json_value.get_array();
	auto iter	  = newArray.begin();
	for (; iter != newArray.end(); ++iter) {
		if (!iter.value().operator*().is_null()) {
			return false;
		}
	}
	return true;
}

template<> inline bool getValue(std::string*& value, simdjson::ondemand::value json_value) {
	if (!value) {
		value = new std::string{};
	}
	return getValue(*value, json_value);
}

template<jsonifier::concepts::map_t map_type> inline bool getValue(map_type& map, simdjson::ondemand::value json_value) {
	simdjson::ondemand::object object;
	auto error = json_value.get_object().get(object);
	if (error) {
		return false;
	}
	for (auto field: object) {
		typename map_type::key_type key;
		auto key_result = field.unescaped_key();
		if (key_result.error()) {
			return false;
		}

		key = static_cast<typename map_type::key_type>(std::string(key_result.value()));

		simdjson::ondemand::value field_value = field.value();
		typename map_type::mapped_type newValue;
		if (!getValue(newValue, field_value)) {
			return false;
		}
		map[key] = newValue;
	}

	return true;
}

template<> inline bool getValue(int8_t& data, simdjson::ondemand::value json_value) {
	int64_t newValue{};
	auto result = json_value.get_int64().get(newValue);
	data		= static_cast<int8_t>(newValue);
	return !result;
}

template<> inline bool getValue(int16_t& data, simdjson::ondemand::value json_value) {
	int64_t newValue{};
	auto result = json_value.get_int64().get(newValue);
	data		= static_cast<int16_t>(newValue);
	return !result;
}

template<> inline bool getValue(int32_t& data, simdjson::ondemand::value json_value) {
	int64_t newValue{};
	auto result = json_value.get_int64().get(newValue);
	data		= static_cast<int32_t>(newValue);
	return !result;
}

template<> inline bool getValue(int64_t& data, simdjson::ondemand::value json_value) {
	auto result = json_value.get_int64().get(data);
	return !result;
}

template<> inline bool getValue(uint8_t& data, simdjson::ondemand::value json_value) {
	uint64_t newValue{};
	auto result = json_value.get_uint64().get(newValue);
	data		= static_cast<uint8_t>(newValue);
	return !result;
}

template<> inline bool getValue(uint16_t& data, simdjson::ondemand::value json_value) {
	uint64_t newValue{};
	auto result = json_value.get_uint64().get(newValue);
	data		= static_cast<uint16_t>(newValue);
	return !result;
}

template<> inline bool getValue(uint32_t& data, simdjson::ondemand::value json_value) {
	uint64_t newValue{};
	auto result = json_value.get_uint64().get(newValue);
	data		= static_cast<uint32_t>(newValue);
	return !result;
}

template<> inline bool getValue(uint64_t& data, simdjson::ondemand::value json_value) {
	auto result = json_value.get_uint64().get(data);
	return !result;
}

template<> inline bool getValue(std::nullptr_t&, simdjson::ondemand::value value) {
	return value.is_null();
}

inline std::string to_string(std::string_view view) {
	return std::string(view.data(), view.size());
}

template<> inline bool getValue(hashtag_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	getValue(value.text, obj["text"]);
	return getValue(value.indices, obj["indices"]);
}

template<> inline bool getValue(large_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.w, obj["w"]) && getValue(value.h, obj["h"]) && getValue(value.resize, obj["resize"]);
}

template<> inline bool getValue(sizes_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.medium, obj["medium"]) && getValue(value.small, obj["small"]) && getValue(value.thumb, obj["thumb"]) && getValue(value.large, obj["large"]);
}

template<> inline bool getValue(media_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.id, obj["id"]))
		return false;
	if (!getValue(value.id_str, obj["id_str"]))
		return false;
	if (!getValue(value.indices, obj["indices"]))
		return false;
	if (!getValue(value.media_url, obj["media_url"]))
		return false;
	if (!getValue(value.media_url_https, obj["media_url_https"]))
		return false;
	if (!getValue(value.url, obj["url"]))
		return false;
	if (!getValue(value.display_url, obj["display_url"]))
		return false;
	if (!getValue(value.expanded_url, obj["expanded_url"]))
		return false;
	if (!getValue(value.type, obj["type"]))
		return false;
	if (!getValue(value.sizes, obj["sizes"]))
		return false;
	if (!getValue(value.source_status_id, obj["source_status_id"]))
		return false;
	if (!getValue(value.source_status_id_str, obj["source_status_id_str"]))
		return false;
	return true;
}

template<> inline bool getValue(url_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.url, obj["url"]))
		return false;
	if (!getValue(value.expanded_url, obj["expanded_url"]))
		return false;
	if (!getValue(value.display_url, obj["display_url"]))
		return false;
	if (!getValue(value.indices, obj["indices"]))
		return false;

	return true;
}

template<> inline bool getValue(user_mention& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.screen_name, obj["screen_name"]))
		return false;
	if (!getValue(value.name, obj["name"]))
		return false;
	if (!getValue(value.id, obj["id"]))
		return false;
	if (!getValue(value.id_str, obj["id_str"]))
		return false;

	return getValue(value.indices, obj["indices"]);
}

template<> inline bool getValue(status_entities& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.hashtags, obj["hashtags"]) && getValue(value.symbols, obj["symbols"]) && getValue(value.urls, obj["urls"]) &&
		getValue(value.user_mentions, obj["user_mentions"]) && getValue(value.media, obj["media"]);
}

template<> inline bool getValue(metadata_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.result_type, obj["result_type"]))
		return false;
	if (!getValue(value.iso_language_code, obj["iso_language_code"]))
		return false;

	return true;
}

template<> inline bool getValue(search_metadata_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.completed_in, obj["completed_in"]))
		return false;
	if (!getValue(value.max_id, obj["max_id"]))
		return false;
	if (!getValue(value.max_id_str, obj["max_id_str"]))
		return false;
	if (!getValue(value.next_results, obj["next_results"]))
		return false;
	if (!getValue(value.query, obj["query"]))
		return false;
	if (!getValue(value.refresh_url, obj["refresh_url"]))
		return false;
	if (!getValue(value.count, obj["count"]))
		return false;
	if (!getValue(value.since_id, obj["since_id"]))
		return false;
	if (!getValue(value.since_id_str, obj["since_id_str"]))
		return false;

	return true;
}

template<> inline bool getValue(description_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.urls, obj["urls"]);
}

template<> inline bool getValue(user_entities& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.description, obj["description"]) && getValue(value.url, obj["url"]);
}

template<> inline bool getValue(twitter_user_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	if (!getValue(value.id, obj["id"]) || !getValue(value.id_str, obj["id_str"]) || !getValue(value.name, obj["name"]) || !getValue(value.screen_name, obj["screen_name"]) ||
		!getValue(value.location, obj["location"]) || !getValue(value.description, obj["description"]) || !getValue(value.url, obj["url"]) ||
		!getValue(value.protectedVal, obj["protected"]) || !getValue(value.followers_count, obj["followers_count"]) || !getValue(value.friends_count, obj["friends_count"]) ||
		!getValue(value.listed_count, obj["listed_count"]) || !getValue(value.created_at, obj["created_at"]) || !getValue(value.favourites_count, obj["favourites_count"]) ||
		!getValue(value.utc_offset, obj["utc_offset"]) || !getValue(value.time_zone, obj["time_zone"]) || !getValue(value.geo_enabled, obj["geo_enabled"]) ||
		!getValue(value.verified, obj["verified"]) || !getValue(value.statuses_count, obj["statuses_count"]) || !getValue(value.lang, obj["lang"]) ||
		!getValue(value.contributors_enabled, obj["contributors_enabled"]) || !getValue(value.is_translator, obj["is_translator"]) ||
		!getValue(value.is_translation_enabled, obj["is_translation_enabled"])) {
		return false;
	}

	return true;
}

template<typename value_type> inline bool getValue(std::unique_ptr<value_type>& e, simdjson::ondemand::value json_value) {
	if (!e) {
		e = std::make_unique<value_type>();
	}
	return getValue(*e, json_value);
}

template<typename value_type> inline bool getValue(std::shared_ptr<value_type>& e, simdjson::ondemand::value json_value) {
	if (!e) {
		e = std::make_shared<value_type>();
	}
	return getValue(*e, json_value);
}

template<> bool getValue(status_data& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;

	if (!getValue(value.metadata, obj["metadata"]) || !getValue(value.created_at, obj["created_at"]) || !getValue(value.id, obj["id"]) || !getValue(value.id_str, obj["id_str"]) ||
		!getValue(value.text, obj["text"]) || !getValue(value.source, obj["source"]) || !getValue(value.truncated, obj["truncated"]) ||
		!getValue(value.in_reply_to_status_id, obj["in_reply_to_status_id"]) || !getValue(value.in_reply_to_status_id_str, obj["in_reply_to_status_id_str"]) ||
		!getValue(value.in_reply_to_user_id, obj["in_reply_to_user_id"]) || !getValue(value.in_reply_to_user_id_str, obj["in_reply_to_user_id_str"]) ||
		!getValue(value.in_reply_to_screen_name, obj["in_reply_to_screen_name"]) || !getValue(value.user, obj["user"]) || !getValue(value.geo, obj["geo"]) ||
		!getValue(value.coordinates, obj["coordinates"]) || !getValue(value.place, obj["place"]) || !getValue(value.contributors, obj["contributors"]) ||
		!getValue(value.retweet_count, obj["retweet_count"]) || !getValue(value.favorite_count, obj["favorite_count"]) || !getValue(value.entities, obj["entities"]) ||
		!getValue(value.favorited, obj["favorited"]) || !getValue(value.retweeted, obj["retweeted"]) || !getValue(value.lang, obj["lang"]) ||
		!getValue(value.retweeted_status, obj["retweeted_status"]) || !getValue(value.possibly_sensitive, obj["possibly_sensitive"]))
		return false;

	return true;
}

template<> inline bool getValue(twitter_message& value, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(value.statuses, obj["statuses"]) && getValue(value.search_metadata, obj["search_metadata"]);
}

template<> inline bool getValue(audience_sub_category_names& p, simdjson::ondemand::value json_value) {
	auto obj = json_value.get_object();
	if (obj.error())
		return false;
	return getValue(p.the337100890, obj["337100890"]);
}

template<> bool getValue(price& p, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(p.amount, obj["amount"]) && getValue(p.audienceSubCategoryId, obj["audienceSubCategoryId"]) && getValue(p.seatCategoryId, obj["seatCategoryId"]);
}

template<> bool getValue(area& a, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(a.areaId, obj["areaId"]) && getValue(a.blockIds, obj["blockIds"]);
}

template<> bool getValue(seat_category& sc, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(sc.areas, obj["areas"]) && getValue(sc.seatCategoryId, obj["seatCategoryId"]);
}

template<> bool getValue(venue_names& vn, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(vn.PLEYEL_PLEYEL, obj["PLEYEL_PLEYEL"]);
}

template<> bool getValue(names&, simdjson::ondemand::value) {
	return true;
}

template<> bool getValue(event& e, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(e.description, obj["description"]) && getValue(e.id, obj["id"]) && getValue(e.logo, obj["logo"]) && getValue(e.name, obj["name"]) &&
		getValue(e.topicIds, obj["subvalue_typeopicIds"]) && getValue(e.subjectCode, obj["subjectCode"]) && getValue(e.subtitle, obj["subtitle"]) &&
		getValue(e.topicIds, obj["topicIds"]);
}

template<> bool getValue(performance& p, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(p.eventId, obj["eventId"]) && getValue(p.id, obj["id"]) && getValue(p.logo, obj["logo"]) && getValue(p.name, obj["name"]) &&
		getValue(p.prices, obj["prices"]) && getValue(p.seatCategories, obj["seatCategories"]) && getValue(p.venueCode, obj["venueId"]);
}

template<> bool getValue(citm_catalog_message& msg, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	if (!getValue(msg.areaNames, obj["areaNames"]))
		return false;
	if (!getValue(msg.audienceSubCategoryNames, obj["audienceSubCategoryNames"]))
		return false;
	if (!getValue(msg.blockNames, obj["blockNames"]))
		return false;
	if (!getValue(msg.events, obj["events"]))
		return false;
	if (!getValue(msg.performances, obj["performances"]))
		return false;
	if (!getValue(msg.seatCategoryNames, obj["seatCategoryNames"]))
		return false;
	if (!getValue(msg.subTopicNames, obj["subTopicNames"]))
		return false;
	if (!getValue(msg.subjectNames, obj["subjectNames"]))
		return false;
	if (!getValue(msg.topicNames, obj["topicNames"]))
		return false;
	if (!getValue(msg.topicSubTopics, obj["topicSubTopics"]))
		return false;
	if (!getValue(msg.venueNames, obj["venueNames"]))
		return false;

	return true;
}

template<> bool getValue(geometry_data& geometry, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(geometry.type, obj["type"]) && getValue(geometry.coordinates, obj["coordinates"]);
}

template<> bool getValue(properties_data& properties, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(properties.name, obj["name"]);
}

template<> bool getValue(feature& f, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(f.type, obj["type"]) && getValue(f.properties, obj["properties"]) && getValue(f.geometry, obj["geometry"]);
}

template<> bool getValue(canada_message& message, simdjson::ondemand::value json_element) {
	auto obj = json_element.get_object();
	if (obj.error() != simdjson::SUCCESS)
		return false;

	return getValue(message.type, obj["type"]) && getValue(message.features, obj["features"]);
}

template<> bool getValue(int*& out_value, simdjson::ondemand::value json_element) {
	int64_t temp{};
	if (json_element.get(temp) != simdjson::SUCCESS) {
		return false;
	}
	out_value = new int(static_cast<int>(temp));
	return true;
}

template<> bool getValue<std::map<std::string, std::string>>(std::map<std::string, std::string>& out_value, simdjson::ondemand::value json_value) {
	simdjson::ondemand::object object;
	if (json_value.get_object().get(object)) {
		std::cerr << "Error parsing object\n";
		return false;
	}

	for (auto field: object) {
		std::string_view key;
		std::string_view value;

		if (field.unescaped_key().get(key) || field.value().get(value)) {
			std::cerr << "Error retrieving key-value pair\n";
			return false;
		}

		out_value.emplace(std::move(key), std::move(value));
	}
	return true;
}

template<> bool getValue<std::unique_ptr<int>>(std::unique_ptr<int>& out_value, simdjson::ondemand::value json_value) {
	int64_t temp;
	if (json_value.get(temp)) {
		std::cerr << "Error parsing integer value\n";
		return false;
	}
	out_value = std::make_unique<int>(temp);
	return true;
}

template<> bool getValue(icon_emoji_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.name, jsonData["name"]);
	// Since nullptr_t is not typically deserialized, handling it as necessary.
	return true;
}

template<> bool getValue(permission_overwrite& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.allow, jsonData["allow"]);
	getValue(returnValue.deny, jsonData["deny"]);
	getValue(returnValue.id, jsonData["id"]);
	getValue(returnValue.type, jsonData["type"]);
	return true;
}

template<> bool getValue(channel_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.permission_overwrites, jsonData["permission_overwrites"]);
	getValue(returnValue.last_message_id, jsonData["last_message_id"]);
	getValue(returnValue.default_thread_rate_limit_per_user, jsonData["default_thread_rate_limit_per_user"]);
	getValue(returnValue.applied_tags, jsonData["applied_tags"]);
	getValue(returnValue.recipients, jsonData["recipients"]);
	getValue(returnValue.default_auto_archive_duration, jsonData["default_auto_archive_duration"]);
	getValue(returnValue.status, jsonData["status"]);
	getValue(returnValue.last_pin_timestamp, jsonData["last_pin_timestamp"]);
	getValue(returnValue.topic, jsonData["topic"]);
	getValue(returnValue.rate_limit_per_user, jsonData["rate_limit_per_user"]);
	getValue(returnValue.icon_emoji, jsonData["icon_emoji"]);
	getValue(returnValue.total_message_sent, jsonData["total_message_sent"]);
	getValue(returnValue.video_quality_mode, jsonData["video_quality_mode"]);
	getValue(returnValue.application_id, jsonData["application_id"]);
	getValue(returnValue.permissions, jsonData["permissions"]);
	getValue(returnValue.message_count, jsonData["message_count"]);
	getValue(returnValue.parent_id, jsonData["parent_id"]);
	getValue(returnValue.member_count, jsonData["member_count"]);
	getValue(returnValue.owner_id, jsonData["owner_id"]);
	getValue(returnValue.guild_id, jsonData["guild_id"]);
	getValue(returnValue.user_limit, jsonData["user_limit"]);
	getValue(returnValue.position, jsonData["position"]);
	getValue(returnValue.name, jsonData["name"]);
	getValue(returnValue.icon, jsonData["icon"]);
	getValue(returnValue.version, jsonData["version"]);
	getValue(returnValue.bitrate, jsonData["bitrate"]);
	getValue(returnValue.id, jsonData["id"]);
	getValue(returnValue.flags, jsonData["flags"]);
	getValue(returnValue.type, jsonData["type"]);
	getValue(returnValue.managed, jsonData["managed"]);
	getValue(returnValue.nsfw, jsonData["nsfw"]);
	return true;
}

template<> bool getValue(user_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.avatar_decoration_data, jsonData["avatar_decoration_data"]);
	getValue(returnValue.display_name, jsonData["display_name"]);
	getValue(returnValue.global_name, jsonData["global_name"]);
	getValue(returnValue.avatar, jsonData["avatar"]);
	getValue(returnValue.banner, jsonData["banner"]);
	getValue(returnValue.locale, jsonData["locale"]);
	getValue(returnValue.discriminator, jsonData["discriminator"]);
	getValue(returnValue.user_name, jsonData["user_name"]);
	getValue(returnValue.accent_color, jsonData["accent_color"]);
	getValue(returnValue.premium_type, jsonData["premium_type"]);
	getValue(returnValue.public_flags, jsonData["public_flags"]);
	getValue(returnValue.email, jsonData["email"]);
	getValue(returnValue.mfa_enabled, jsonData["mfa_enabled"]);
	getValue(returnValue.id, jsonData["id"]);
	getValue(returnValue.flags, jsonData["flags"]);
	getValue(returnValue.verified, jsonData["verified"]);
	getValue(returnValue.system, jsonData["system"]);
	getValue(returnValue.bot, jsonData["bot"]);
	return true;
}

template<> bool getValue(member_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.communication_disabled_until, jsonData["communication_disabled_until"]);
	getValue(returnValue.premium_since, jsonData["premium_since"]);
	getValue(returnValue.nick, jsonData["nick"]);
	getValue(returnValue.avatar, jsonData["avatar"]);
	getValue(returnValue.roles, jsonData["roles"]);
	getValue(returnValue.permissions, jsonData["permissions"]);
	getValue(returnValue.joined_at, jsonData["joined_at"]);
	getValue(returnValue.guild_id, jsonData["guild_id"]);
	getValue(returnValue.user, jsonData["user"]);
	getValue(returnValue.flags, jsonData["flags"]);
	getValue(returnValue.pending, jsonData["pending"]);
	getValue(returnValue.deaf, jsonData["deaf"]);
	getValue(returnValue.mute, jsonData["mute"]);
	return true;
}

template<> bool getValue(tags_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.premium_subscriber, jsonData["premium_subscriber"]);
	getValue(returnValue.bot_id, jsonData["bot_id"]);
	return true;
}

template<> bool getValue(role_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.unicode_emoji, jsonData["unicode_emoji"]);
	getValue(returnValue.icon, jsonData["icon"]);
	getValue(returnValue.permissions, jsonData["permissions"]);
	getValue(returnValue.position, jsonData["position"]);
	getValue(returnValue.name, jsonData["name"]);
	getValue(returnValue.mentionable, jsonData["mentionable"]);
	getValue(returnValue.version, jsonData["version"]);
	getValue(returnValue.id, jsonData["id"]);
	getValue(returnValue.tags, jsonData["tags"]);
	getValue(returnValue.color, jsonData["color"]);
	getValue(returnValue.flags, jsonData["flags"]);
	getValue(returnValue.managed, jsonData["managed"]);
	getValue(returnValue.hoist, jsonData["hoist"]);
	return true;
}

template<> bool getValue(guild_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.latest_on_boarding_question_id, jsonData["latest_on_boarding_question_id"]);
	getValue(returnValue.guild_scheduled_events, jsonData["guild_scheduled_events"]);
	getValue(returnValue.safety_alerts_channel_id, jsonData["safety_alerts_channel_id"]);
	getValue(returnValue.inventory_settings, jsonData["inventory_settings"]);
	getValue(returnValue.voice_states, jsonData["voice_states"]);
	getValue(returnValue.discovery_splash, jsonData["discovery_splash"]);
	getValue(returnValue.vanity_url_code, jsonData["vanity_url_code"]);
	getValue(returnValue.application_id, jsonData["application_id"]);
	getValue(returnValue.afk_channel_id, jsonData["afk_channel_id"]);
	getValue(returnValue.default_message_notifications, jsonData["default_message_notifications"]);
	getValue(returnValue.max_stage_video_channel_users, jsonData["max_stage_video_channel_users"]);
	getValue(returnValue.public_updates_channel_id, jsonData["public_updates_channel_id"]);
	getValue(returnValue.description, jsonData["description"]);
	getValue(returnValue.threads, jsonData["threads"]);
	getValue(returnValue.channels, jsonData["channels"]);
	getValue(returnValue.premium_subscription_count, jsonData["premium_subscription_count"]);
	getValue(returnValue.approximate_presence_count, jsonData["approximate_presence_count"]);
	getValue(returnValue.features, jsonData["features"]);
	getValue(returnValue.stickers, jsonData["stickers"]);
	getValue(returnValue.premium_progress_bar_enabled, jsonData["premium_progress_bar_enabled"]);
	getValue(returnValue.members, jsonData["members"]);
	getValue(returnValue.hub_type, jsonData["hub_type"]);
	getValue(returnValue.approximate_member_count, jsonData["approximate_member_count"]);
	getValue(returnValue.explicit_content_filter, jsonData["explicit_content_filter"]);
	getValue(returnValue.max_video_channel_users, jsonData["max_video_channel_users"]);
	getValue(returnValue.splash, jsonData["splash"]);
	getValue(returnValue.banner, jsonData["banner"]);
	getValue(returnValue.system_channel_id, jsonData["system_channel_id"]);
	getValue(returnValue.widget_channel_id, jsonData["widget_channel_id"]);
	getValue(returnValue.preferred_locale, jsonData["preferred_locale"]);
	getValue(returnValue.system_channel_flags, jsonData["system_channel_flags"]);
	getValue(returnValue.rules_channel_id, jsonData["rules_channel_id"]);
	getValue(returnValue.roles, jsonData["roles"]);
	getValue(returnValue.verification_level, jsonData["verification_level"]);
	getValue(returnValue.permissions, jsonData["permissions"]);
	getValue(returnValue.max_presences, jsonData["max_presences"]);
	getValue(returnValue.discovery, jsonData["discovery"]);
	getValue(returnValue.joined_at, jsonData["joined_at"]);
	getValue(returnValue.member_count, jsonData["member_count"]);
	getValue(returnValue.premium_tier, jsonData["premium_tier"]);
	getValue(returnValue.owner_id, jsonData["owner_id"]);
	getValue(returnValue.max_members, jsonData["max_members"]);
	getValue(returnValue.afk_timeout, jsonData["afk_timeout"]);
	getValue(returnValue.widget_enabled, jsonData["widget_enabled"]);
	getValue(returnValue.region, jsonData["region"]);
	getValue(returnValue.nsfw_level, jsonData["nsfw_level"]);
	getValue(returnValue.mfa_level, jsonData["mfa_level"]);
	getValue(returnValue.name, jsonData["name"]);
	getValue(returnValue.icon, jsonData["icon"]);
	getValue(returnValue.unavailable, jsonData["unavailable"]);
	getValue(returnValue.id, jsonData["id"]);
	getValue(returnValue.flags, jsonData["flags"]);
	getValue(returnValue.large, jsonData["large"]);
	getValue(returnValue.owner, jsonData["owner"]);
	getValue(returnValue.nsfw, jsonData["nsfw"]);
	getValue(returnValue.lazy, jsonData["lazy"]);
	return true;
}

template<> bool getValue(discord_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.t, jsonData["t"]);
	getValue(returnValue.d, jsonData["d"]);
	getValue(returnValue.op, jsonData["op"]);
	getValue(returnValue.s, jsonData["s"]);
	return true;
}

template<> bool getValue(abc_test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.z, jsonData["z"]);
	getValue(returnValue.y, jsonData["y"]);
	getValue(returnValue.x, jsonData["x"]);
	getValue(returnValue.w, jsonData["w"]);
	getValue(returnValue.v, jsonData["v"]);
	getValue(returnValue.u, jsonData["u"]);
	getValue(returnValue.t, jsonData["t"]);
	getValue(returnValue.s, jsonData["s"]);
	getValue(returnValue.r, jsonData["r"]);
	getValue(returnValue.q, jsonData["q"]);
	getValue(returnValue.p, jsonData["p"]);
	getValue(returnValue.o, jsonData["o"]);
	getValue(returnValue.n, jsonData["n"]);
	getValue(returnValue.m, jsonData["m"]);
	getValue(returnValue.l, jsonData["l"]);
	getValue(returnValue.k, jsonData["k"]);
	getValue(returnValue.j, jsonData["j"]);
	getValue(returnValue.i, jsonData["i"]);
	getValue(returnValue.h, jsonData["h"]);
	getValue(returnValue.g, jsonData["g"]);
	getValue(returnValue.f, jsonData["f"]);
	getValue(returnValue.e, jsonData["e"]);
	getValue(returnValue.d, jsonData["d"]);
	getValue(returnValue.c, jsonData["c"]);
	getValue(returnValue.b, jsonData["b"]);
	getValue(returnValue.a, jsonData["a"]);
	return true;
}

template<> bool getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.testBools, jsonData["testBools"]);
	getValue(returnValue.testInts, jsonData["testInts"]);
	getValue(returnValue.testUints, jsonData["testUints"]);
	getValue(returnValue.testStrings, jsonData["testStrings"]);
	getValue(returnValue.testDoubles, jsonData["testDoubles"]);
	return true;
}

template<> bool getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.a, jsonData["a"]);
	getValue(returnValue.b, jsonData["b"]);
	getValue(returnValue.c, jsonData["c"]);
	getValue(returnValue.d, jsonData["d"]);
	getValue(returnValue.e, jsonData["e"]);
	getValue(returnValue.f, jsonData["f"]);
	getValue(returnValue.g, jsonData["g"]);
	getValue(returnValue.h, jsonData["h"]);
	getValue(returnValue.i, jsonData["i"]);
	getValue(returnValue.j, jsonData["j"]);
	getValue(returnValue.k, jsonData["k"]);
	getValue(returnValue.l, jsonData["l"]);
	getValue(returnValue.m, jsonData["m"]);
	getValue(returnValue.n, jsonData["n"]);
	getValue(returnValue.o, jsonData["o"]);
	getValue(returnValue.p, jsonData["p"]);
	getValue(returnValue.q, jsonData["q"]);
	getValue(returnValue.r, jsonData["r"]);
	getValue(returnValue.s, jsonData["s"]);
	getValue(returnValue.t, jsonData["t"]);
	getValue(returnValue.u, jsonData["u"]);
	getValue(returnValue.v, jsonData["v"]);
	getValue(returnValue.w, jsonData["w"]);
	getValue(returnValue.x, jsonData["x"]);
	getValue(returnValue.y, jsonData["y"]);
	getValue(returnValue.z, jsonData["z"]);
	return true;
}

#endif