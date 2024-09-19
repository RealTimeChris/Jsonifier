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

template<typename value_type>
concept bool_t = std::is_same_v<std::remove_cvref_t<value_type>, bool> || std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::reference> ||
	std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::const_reference>;

template<typename value_type>
concept floating_type = std::floating_point<std::remove_cvref_t<value_type>> && !bool_t<value_type>;

template<typename value_type>
concept unsigned_type = std::unsigned_integral<std::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept signed_type = std::signed_integral<std::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept range = requires(std::remove_cvref_t<value_type> value) {
	typename std::remove_cvref_t<value_type>::value_type;
	{ value.begin() } -> std::same_as<typename std::remove_cvref_t<value_type>::const_iterator>;
	{ value.end() } -> std::same_as<typename std::remove_cvref_t<value_type>::const_iterator>;
} || requires(std::remove_cvref_t<value_type> value) {
	typename std::remove_cvref_t<value_type>::value_type;
	{ value.begin() } -> std::same_as<typename std::remove_cvref_t<value_type>::iterator>;
	{ value.end() } -> std::same_as<typename std::remove_cvref_t<value_type>::iterator>;
};

template<typename value_type>
concept map_subscriptable = requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<const typename std::remove_cvref_t<value_type>::mapped_type&>;
} || requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::mapped_type&>;
};

template<typename value_type>
concept map_t = requires(std::remove_cvref_t<value_type> value) {
	typename std::remove_cvref_t<value_type>::mapped_type;
	typename std::remove_cvref_t<value_type>::key_type;
} && range<value_type> && map_subscriptable<value_type> && !std::is_integral_v<std::remove_cvref_t<value_type>>;

template<typename value_type>
concept vector_subscriptable = requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::const_reference>;
} || requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::reference>;
};

template<typename value_type>
concept copyable = std::copyable<std::remove_cvref_t<value_type>>;

template<typename value_type>
concept has_release = requires(std::remove_cvref_t<value_type> value) {
	{ value.release() } -> std::same_as<typename std::remove_cvref_t<value_type>::pointer>;
};

template<typename value_type>
concept has_get = requires(std::remove_cvref_t<value_type> value) {
	{ value.get() } -> std::same_as<typename std::remove_cvref_t<value_type>::element_type*>;
};

template<typename value_type>
concept unique_ptr_t = requires(std::remove_cvref_t<value_type> value) {
	typename std::remove_cvref_t<value_type>::element_type;
	typename std::remove_cvref_t<value_type>::deleter_type;
} && has_release<value_type>;

template<typename value_type>
concept shared_ptr_t = has_get<value_type> && copyable<value_type>;

template<typename value_type>
concept vector_t = !map_t<value_type> && vector_subscriptable<value_type> && !std::is_pointer_v<std::remove_cvref_t<value_type>>;

void throwError(auto error, std::source_location location = std::source_location::current()) {
	std::stringstream stream{};
	stream << "Error: " << error << std::endl;
	stream << "Thrown from: " << location.file_name() << ", At Line: " << location.line() << std::endl;
	std::cout << stream.str();
	throw std::runtime_error{ stream.str() };
}

template<floating_type value_type> bool getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		auto result = jsonData.get_double().get(data);
		if (result) {
			return false;
		}
		return !result;
	} else {
		double newValue{};
		auto result = jsonData.get_double().get(newValue); 
		if (result) {
			return false;
		}
		data = static_cast<value_type>(newValue);
		return !result;
	}
}

template<unsigned_type value_type>
bool getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		auto result = jsonData.get_uint64().get(data);
		if (result) {
			return false;
		}
		return !result;
	} else {
		uint64_t newValue{};
		auto result = jsonData.get_uint64().get(newValue);
		if (result) {
			return false;
		}
		data = static_cast<value_type>(newValue);
		return !result;
	}
}

template<signed_type value_type> bool getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		auto result = jsonData.get_int64().get(data);
		if (result) {
			return false;
		}
		return !result;
	} else {
		int64_t newValue{};
		auto result = jsonData.get_int64().get(newValue);
		if (result) {
			return false;
		}
		data = static_cast<value_type>(newValue);
		return !result;
	}
}

bool getValue(std::nullptr_t& , simdjson::ondemand::value value) {
	auto result = value.is_null();
	if (result.error()) {
		return false;
	}
	if (result.value()) {
		return true;
	}
	return false;
}

bool getValue(std::string& data, simdjson::ondemand::value jsonData) {
	std::string_view newValue;
	auto result = jsonData.get(newValue);
	if (result) {
		return false;
	}
	data = static_cast<std::string>(newValue);
	return !result;
}

template<bool_t value_type> bool getValue(value_type& data, simdjson::ondemand::value jsonData) {
	bool newValue;
	auto result = jsonData.get(newValue);
	if (result) {
		return false;
	}
	data = static_cast<value_type>(newValue);
	return !result;
}

template<vector_t value_type> bool getValue(value_type& value, simdjson::ondemand::value jsonData) {
	auto newArr = jsonData.get_array();
	if (auto result = newArr.error()) {
		return false;
	}
	auto newArray	= newArr.value();
	const auto size = value.size();
	auto iter		= newArray.begin();
	typename value_type::value_type valueNew;
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		if (!getValue(valueNew, iter.value().operator*().value())) {
			return false;
		}
		value[x] = std::move(valueNew);
	}
	for (; iter != newArray.end(); ++iter) {
		if (!getValue(valueNew, iter.value().operator*().value())) {
			return false;
		}
		value.emplace_back(std::move(valueNew));
	}
	return true;
}

template<typename value_type> bool getValue(std::optional<value_type>& vec, simdjson::ondemand::value jsonData) {
	if (!jsonData.is_null().value()) {
		if (!getValue(vec.emplace(), jsonData)) {
			return false;
		}
	}
	return true;
}

template<map_t value_type> bool getValue(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object object;
	auto error = jsonData.get_object().get(object);
	if (error) {
		throwError(error);
	}
	for (auto field: object) {
		typename std::remove_cvref_t<decltype(value)>::key_type key;
		auto key_result = field.unescaped_key();
		if (auto result = key_result.error()) {
			return false;
		}

		key = static_cast<typename std::remove_cvref_t<decltype(value)>::key_type>(std::string(key_result.value()));

		simdjson::ondemand::value field_value = field.value();
		typename std::remove_cvref_t<decltype(value)>::mapped_type newValue;
		if (!getValue(newValue, field_value)) {
			return false;
		}
		value[key] = std::move(newValue);
	}

	return true;
}

template<typename value_type> bool getValue(value_type& value, simdjson::ondemand::object jsonData, const std::string_view& key) {
	simdjson::ondemand::value jsonValue;
	auto error = jsonData[key].get(jsonValue);
	if (error == simdjson::SUCCESS) {
		return getValue(value, jsonValue);
	}else if (error == simdjson::NO_SUCH_FIELD){
		return true;
	} else {
		return false;
	}
}

bool getValue(std::string*& value, simdjson::ondemand::value jsonData) {
	if (!value) {
		value = new std::string{};
	}
	return getValue(*value, jsonData);
}

bool getValue(hashtag_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(value.indices, obj, "indices") && getValue(value.text, obj, "text");
}

bool getValue(large_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(value.w, obj, "w") && getValue(value.h, obj, "h") && getValue(value.resize, obj, "resize");
}

bool getValue(sizes_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(value.medium, obj, "medium") && getValue(value.small, obj, "small") && getValue(value.thumb, obj, "thumb") && getValue(value.large, obj, "large");
}

bool getValue(media_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.source_status_id, obj, "source_status_id"))
		return false;
	if (!getValue(value.source_status_id_str, obj, "source_status_id_str"))
		return false;
	if (!getValue(value.id, obj, "id"))
		return false;
	if (!getValue(value.id_str, obj, "id_str"))
		return false;
	if (!getValue(value.indices, obj, "indices"))
		return false;
	if (!getValue(value.media_url, obj, "media_url"))
		return false;
	if (!getValue(value.media_url_https, obj, "media_url_https"))
		return false;
	if (!getValue(value.url, obj, "url"))
		return false;
	if (!getValue(value.display_url, obj, "display_url"))
		return false;
	if (!getValue(value.expanded_url, obj, "expanded_url"))
		return false;
	if (!getValue(value.type, obj, "type"))
		return false;
	if (!getValue(value.sizes, obj, "sizes"))
		return false;
	return true;
}

bool getValue(url_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.url, obj, "url"))
		return false;
	if (!getValue(value.expanded_url, obj, "expanded_url"))
		return false;
	if (!getValue(value.display_url, obj, "display_url"))
		return false;
	if (!getValue(value.indices, obj, "indices"))
		return false;

	return true;
}

bool getValue(user_mention& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.screen_name, obj, "screen_name"))
		return false;
	if (!getValue(value.name, obj, "name"))
		return false;
	if (!getValue(value.id, obj, "id"))
		return false;
	if (!getValue(value.id_str, obj, "id_str"))
		return false;

	return getValue(value.indices, obj, "indices");
}

bool getValue(status_entities& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(value.media, obj, "media");
	return getValue(value.hashtags, obj,"hashtags") && getValue(value.symbols, obj,"symbols") && getValue(value.urls, obj, "urls") &&
		getValue(value.user_mentions, obj, "user_mentions");
}

bool getValue(metadata_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.result_type, obj, "result_type"))
		return false;
	if (!getValue(value.iso_language_code, obj, "iso_language_code"))
		return false;

	return true;
}

bool getValue(search_metadata_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.completed_in, obj, "completed_in"))
		return false;
	if (!getValue(value.max_id, obj, "max_id"))
		return false;
	if (!getValue(value.max_id_str, obj, "max_id_str"))
		return false;
	if (!getValue(value.next_results, obj, "next_results"))
		return false;
	if (!getValue(value.query, obj, "query"))
		return false;
	if (!getValue(value.refresh_url, obj, "refresh_url"))
		return false;
	if (!getValue(value.count, obj, "count"))
		return false;
	if (!getValue(value.since_id, obj, "since_id"))
		return false;
	if (!getValue(value.since_id_str, obj, "since_id_str"))
		return false;

	return true;
}

bool getValue(description_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(value.urls, obj, "urls");
}

bool getValue(user_entities& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	if (!getValue(value.url, obj, "url")) {
		return false;
	}
	return getValue(value.description, obj, "description");
}

bool getValue(twitter_user_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	if (!getValue(value.url, obj, "url")) {
		return false;
	}
	if (!getValue(value.utc_offset, obj, "utc_offset")) {
		return false;
	}
	if (!getValue(value.time_zone, obj, "time_zone")) {
		return false;
	}
	if (!getValue(value.profile_banner_url, obj, "profile_banner_url")) {
		return false;
	}
	if (!getValue(value.id, obj, "id") || !getValue(value.id_str, obj, "id_str") || !getValue(value.name, obj, "name") || !getValue(value.screen_name, obj, "screen_name") ||
		!getValue(value.location, obj, "location") || !getValue(value.description, obj, "description") || !getValue(value.protectedVal, obj, "protected") ||
		!getValue(value.followers_count, obj, "followers_count") || !getValue(value.friends_count, obj, "friends_count") || !getValue(value.listed_count, obj, "listed_count") ||
		!getValue(value.created_at, obj, "created_at") || !getValue(value.favourites_count, obj, "favourites_count") || !getValue(value.geo_enabled, obj, "geo_enabled") ||
		!getValue(value.verified, obj, "verified") || !getValue(value.statuses_count, obj, "statuses_count") || !getValue(value.lang, obj, "lang") ||
		!getValue(value.contributors_enabled, obj, "contributors_enabled") || !getValue(value.is_translator, obj, "is_translator") ||
		!getValue(value.is_translation_enabled, obj, "is_translation_enabled")) {
		return false;
	}

	return true;
}

bool getValue(unique_ptr_t auto& e, simdjson::ondemand::value jsonData) {
	if (!e) {
		e = std::make_unique<std::remove_cvref_t<decltype(*e)>>();
	}
	return getValue(*e, jsonData);
}

bool getValue(shared_ptr_t auto& e, simdjson::ondemand::value jsonData) {
	if (!e) {
		e = std::make_shared<std::remove_cvref_t<decltype(*e)>>();
	}
	return getValue(*e, jsonData);
}

bool getValue(status_data& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(value.metadata, obj, "metadata")) {
		return false;
	}
	if (!getValue(value.created_at, obj, "created_at")) {
		return false;
	}
	if (!getValue(value.id, obj, "id")) {
		return false;
	}
	if (!getValue(value.id_str, obj, "id_str")) {
		return false;
	}
	if (!getValue(value.text, obj, "text")) {
		return false;		
	}
	if (!getValue(value.source, obj, "source")) {
		return false;		
	}
	if (!getValue(value.truncated, obj, "truncated")) {
		return false;
	}
	if (!getValue(value.in_reply_to_status_id, obj, "in_reply_to_status_id")) {
		return false;
	}
	if (!getValue(value.in_reply_to_status_id_str, obj, "in_reply_to_status_id_str")) {
		return false;
	}
	if (!getValue(value.in_reply_to_user_id, obj, "in_reply_to_user_id")) {
		return false;
	}
	if (!getValue(value.in_reply_to_user_id_str, obj, "in_reply_to_user_id_str")) {
		return false;
	}
	if (!getValue(value.in_reply_to_screen_name, obj, "in_reply_to_screen_name")) {
		return false;
	}
	if (!getValue(value.user, obj, "user")) {
		return false;
	}
	if (!getValue(value.geo, obj, "geo")) {
		return false;
	}
	if (!getValue(value.coordinates, obj, "coordinates")) {
		return false;
	}
	if (!getValue(value.place, obj, "place")) {
		return false;
	}
	if (!getValue(value.contributors, obj, "contributors")) {
		return false;
	}
	if (!getValue(value.retweet_count, obj, "retweet_count")) {
		return false;
	}
	if (!getValue(value.favorite_count, obj, "favorite_count")) {
		return false;
	}
	if (!getValue(value.entities, obj, "entities")) {
		return false;
	}
	if (!getValue(value.favorited, obj, "favorited")) {
		return false;
	}
	if (!getValue(value.retweeted, obj, "retweeted")) {
		return false;
	}
	if (!getValue(value.lang, obj, "lang")) {
		return false;
	}
	if (!getValue(value.retweeted_status, obj, "retweeted_status")) {
		return false;
	}
	if (!getValue(value.possibly_sensitive, obj, "possibly_sensitive")) {
		return false;
	}
	return true;
}

bool getValue(twitter_message& value, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(value.statuses, obj, "statuses") && getValue(value.search_metadata, obj, "search_metadata");
}

bool getValue(audience_sub_category_names& p, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	return getValue(p.the337100890, obj, "337100890");
}

bool getValue(names&, simdjson::ondemand::value) {
	return true;
}

bool getValue(event& e, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(e.description, obj, "description") && getValue(e.id, obj, "id") && getValue(e.logo, obj, "logo") && getValue(e.name, obj, "name") &&
		getValue(e.topicIds, obj, "subTopicIds") && getValue(e.subjectCode, obj, "subjectCode") && getValue(e.subtitle, obj, "subtitle") &&
		getValue(e.topicIds, obj, "topicIds");
}

bool getValue(price& p, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(p.amount, obj, "amount") && getValue(p.audienceSubCategoryId, obj, "audienceSubCategoryId") && getValue(p.seatCategoryId, obj, "seatCategoryId");
}

bool getValue(area& a, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(a.areaId, obj, "areaId") && getValue(a.blockIds, obj, "blockIds");
}

bool getValue(seat_category& sc, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(sc.areas, obj, "areas") && getValue(sc.seatCategoryId, obj, "seatCategoryId");
}

bool getValue(venue_names& vn, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(vn.PLEYEL_PLEYEL, obj, "PLEYEL_PLEYEL");
}

bool getValue(performance& p, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(p.eventId, obj, "eventId") && getValue(p.eventId, obj, "id") && getValue(p.logo, obj, "logo") && getValue(p.name, obj, "name") &&
		getValue(p.prices, obj, "prices") && getValue(p.seatCategories, obj, "seatCategories") && getValue(p.seatMapImage, obj, "seatMapImage") &&
		getValue(p.start, obj, "start") && getValue(p.venueCode, obj, "venueCode");
}

bool getValue(citm_catalog_message& msg, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	if (!getValue(msg.areaNames, obj, "areaNames"))
		return false;
	if (!getValue(msg.audienceSubCategoryNames, obj, "audienceSubCategoryNames"))
		return false;
	if (!getValue(msg.blockNames, obj, "blockNames"))
		return false;
	if (!getValue(msg.events, obj, "events"))
		return false;
	if (!getValue(msg.performances, obj, "performances"))
		return false;
	if (!getValue(msg.seatCategoryNames, obj, "seatCategoryNames"))
		return false;
	if (!getValue(msg.subTopicNames, obj, "subTopicNames"))
		return false;
	if (!getValue(msg.subjectNames, obj, "subjectNames"))
		return false;
	if (!getValue(msg.topicNames, obj, "topicNames"))
		return false;
	if (!getValue(msg.topicSubTopics, obj, "topicSubTopics"))
		return false;
	if (!getValue(msg.venueNames, obj, "venueNames"))
		return false;

	return true;
}

bool getValue(geometry_data& geometry, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();

	return getValue(geometry.type, obj, "type") && getValue(geometry.coordinates, obj, "coordinates");
}

bool getValue(properties_data& properties, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
if (newObj.error()) {
	return false;
}
auto obj = newObj.value();

	return getValue(properties.name, obj, "name");
}

bool getValue(feature& f, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
if (newObj.error()) {
	return false;
}
auto obj = newObj.value();

	return getValue(f.type, obj, "type") && getValue(f.properties, obj, "properties") && getValue(f.geometry, obj, "geometry");
}

bool getValue(canada_message& message, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
if (newObj.error()) {
	return false;
}
auto obj = newObj.value();

	return getValue(message.type, obj, "type") && getValue(message.features, obj, "features");
}

bool getValue(int*& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp{};
	if (jsonData.get(temp)) {
		return false;
	}
	out_value = new int(static_cast<int>(temp));
	return true;
}

bool getValue(std::unique_ptr<int>& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp;
	if (jsonData.get(temp)) {
		std::cerr << "Error parsing integer value\n";
		return false;
	}
	out_value = std::make_unique<int>(temp);
	return true;
}

bool getValue(icon_emoji_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.name, obj, "name");
	getValue(returnValue.id, obj, "name");
	return true;
}

bool getValue(permission_overwrite& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.allow, obj, "allow");
	getValue(returnValue.deny, obj, "deny");
	getValue(returnValue.id, obj, "id");
	getValue(returnValue.type, obj, "type");
	return true;
}

bool getValue(channel_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.permission_overwrites, obj, "permission_overwrites");
	getValue(returnValue.last_message_id, obj, "last_message_id");
	getValue(returnValue.default_thread_rate_limit_per_user, obj, "default_thread_rate_limit_per_user");
	getValue(returnValue.applied_tags, obj, "applied_tags");
	getValue(returnValue.recipients, obj, "recipients");
	getValue(returnValue.default_auto_archive_duration, obj, "default_auto_archive_duration");
	getValue(returnValue.status, obj, "status");
	getValue(returnValue.last_pin_timestamp, obj, "last_pin_timestamp");
	getValue(returnValue.topic, obj, "topic");
	getValue(returnValue.rate_limit_per_user, obj, "rate_limit_per_user");
	getValue(returnValue.icon_emoji, obj, "icon_emoji");
	getValue(returnValue.total_message_sent, obj, "total_message_sent");
	getValue(returnValue.video_quality_mode, obj, "video_quality_mode");
	getValue(returnValue.application_id, obj, "application_id");
	getValue(returnValue.permissions, obj, "permissions");
	getValue(returnValue.message_count, obj, "message_count");
	getValue(returnValue.parent_id, obj, "parent_id");
	getValue(returnValue.member_count, obj, "member_count");
	getValue(returnValue.owner_id, obj, "owner_id");
	getValue(returnValue.guild_id, obj, "guild_id");
	getValue(returnValue.user_limit, obj, "user_limit");
	getValue(returnValue.position, obj, "position");
	getValue(returnValue.name, obj, "name");
	getValue(returnValue.icon, obj, "icon");
	getValue(returnValue.version, obj, "version");
	getValue(returnValue.bitrate, obj, "bitrate");
	getValue(returnValue.id, obj, "id");
	getValue(returnValue.flags, obj, "flags");
	getValue(returnValue.type, obj, "type");
	getValue(returnValue.managed, obj, "managed");
	getValue(returnValue.nsfw, obj, "nsfw");
	return true;
}

bool getValue(user_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.avatar_decoration_data, obj, "avatar_decoration_data");
	getValue(returnValue.display_name, obj, "display_name");
	getValue(returnValue.global_name, obj, "global_name");
	getValue(returnValue.avatar, obj, "avatar");
	getValue(returnValue.banner, obj, "banner");
	getValue(returnValue.locale, obj, "locale");
	getValue(returnValue.discriminator, obj, "discriminator");
	getValue(returnValue.user_name, obj, "user_name");
	getValue(returnValue.accent_color, obj, "accent_color");
	getValue(returnValue.premium_type, obj, "premium_type");
	getValue(returnValue.public_flags, obj, "public_flags");
	getValue(returnValue.email, obj, "email");
	getValue(returnValue.mfa_enabled, obj, "mfa_enabled");
	getValue(returnValue.id, obj, "id");
	getValue(returnValue.flags, obj, "flags");
	getValue(returnValue.verified, obj, "verified");
	getValue(returnValue.system, obj, "system");
	getValue(returnValue.bot, obj, "bot");
	return true;
}

bool getValue(member_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.communication_disabled_until, obj, "communication_disabled_until");
	getValue(returnValue.premium_since, obj, "premium_since");
	getValue(returnValue.nick, obj, "nick");
	getValue(returnValue.avatar, obj, "avatar");
	getValue(returnValue.roles, obj, "roles");
	getValue(returnValue.permissions, obj, "permissions");
	getValue(returnValue.joined_at, obj, "joined_at");
	getValue(returnValue.guild_id, obj, "guild_id");
	getValue(returnValue.user, obj, "user");
	getValue(returnValue.flags, obj, "flags");
	getValue(returnValue.pending, obj, "pending");
	getValue(returnValue.deaf, obj, "deaf");
	getValue(returnValue.mute, obj, "mute");
	return true;
}

bool getValue(tags_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.premium_subscriber, obj, "premium_subscriber");
	getValue(returnValue.bot_id, obj, "bot_id");
	return true;
}

bool getValue(role_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.unicode_emoji, obj, "unicode_emoji");
	getValue(returnValue.icon, obj, "icon");
	getValue(returnValue.permissions, obj, "permissions");
	getValue(returnValue.position, obj, "position");
	getValue(returnValue.name, obj, "name");
	getValue(returnValue.mentionable, obj, "mentionable");
	getValue(returnValue.version, obj, "version");
	getValue(returnValue.id, obj, "id");
	getValue(returnValue.tags, obj, "tags");
	getValue(returnValue.color, obj, "color");
	getValue(returnValue.flags, obj, "flags");
	getValue(returnValue.managed, obj, "managed");
	getValue(returnValue.hoist, obj, "hoist");
	return true;
}

bool getValue(guild_data& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.latest_on_boarding_question_id, obj, "latest_on_boarding_question_id");
	getValue(returnValue.guild_scheduled_events, obj, "guild_scheduled_events");
	getValue(returnValue.safety_alerts_channel_id, obj, "safety_alerts_channel_id");
	getValue(returnValue.inventory_settings, obj, "inventory_settings");
	getValue(returnValue.voice_states, obj, "voice_states");
	getValue(returnValue.discovery_splash, obj, "discovery_splash");
	getValue(returnValue.vanity_url_code, obj, "vanity_url_code");
	getValue(returnValue.application_id, obj, "application_id");
	getValue(returnValue.afk_channel_id, obj, "afk_channel_id");
	getValue(returnValue.default_message_notifications, obj, "default_message_notifications");
	getValue(returnValue.max_stage_video_channel_users, obj, "max_stage_video_channel_users");
	getValue(returnValue.public_updates_channel_id, obj, "public_updates_channel_id");
	getValue(returnValue.description, obj, "description");
	getValue(returnValue.threads, obj, "threads");
	getValue(returnValue.channels, obj, "channels");
	getValue(returnValue.premium_subscription_count, obj, "premium_subscription_count");
	getValue(returnValue.approximate_presence_count, obj, "approximate_presence_count");
	getValue(returnValue.features, obj, "features");
	getValue(returnValue.stickers, obj, "stickers");
	getValue(returnValue.premium_progress_bar_enabled, obj, "premium_progress_bar_enabled");
	getValue(returnValue.members, obj, "members");
	getValue(returnValue.hub_type, obj, "hub_type");
	getValue(returnValue.approximate_member_count, obj, "approximate_member_count");
	getValue(returnValue.explicit_content_filter, obj, "explicit_content_filter");
	getValue(returnValue.max_video_channel_users, obj, "max_video_channel_users");
	getValue(returnValue.splash, obj, "splash");
	getValue(returnValue.banner, obj, "banner");
	getValue(returnValue.system_channel_id, obj, "system_channel_id");
	getValue(returnValue.widget_channel_id, obj, "widget_channel_id");
	getValue(returnValue.preferred_locale, obj, "preferred_locale");
	getValue(returnValue.system_channel_flags, obj, "system_channel_flags");
	getValue(returnValue.rules_channel_id, obj, "rules_channel_id");
	getValue(returnValue.roles, obj, "roles");
	getValue(returnValue.verification_level, obj, "verification_level");
	getValue(returnValue.permissions, obj, "permissions");
	getValue(returnValue.max_presences, obj, "max_presences");
	getValue(returnValue.discovery, obj, "discovery");
	getValue(returnValue.joined_at, obj, "joined_at");
	getValue(returnValue.member_count, obj, "member_count");
	getValue(returnValue.premium_tier, obj, "premium_tier");
	getValue(returnValue.owner_id, obj, "owner_id");
	getValue(returnValue.max_members, obj, "max_members");
	getValue(returnValue.afk_timeout, obj, "afk_timeout");
	getValue(returnValue.widget_enabled, obj, "widget_enabled");
	getValue(returnValue.region, obj, "region");
	getValue(returnValue.nsfw_level, obj, "nsfw_level");
	getValue(returnValue.mfa_level, obj, "mfa_level");
	getValue(returnValue.name, obj, "name");
	getValue(returnValue.icon, obj, "icon");
	getValue(returnValue.unavailable, obj, "unavailable");
	getValue(returnValue.id, obj, "id");
	getValue(returnValue.flags, obj, "flags");
	getValue(returnValue.large, obj, "large");
	getValue(returnValue.owner, obj, "owner");
	getValue(returnValue.nsfw, obj, "nsfw");
	getValue(returnValue.lazy, obj, "lazy");
	return true;
}

bool getValue(discord_message& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.t, obj, "t");
	getValue(returnValue.d, obj, "d");
	getValue(returnValue.op, obj, "op");
	getValue(returnValue.s, obj, "s");
	return true;
}

bool getValue(abc_test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.z, obj, "z");
	getValue(returnValue.y, obj, "y");
	getValue(returnValue.x, obj, "x");
	getValue(returnValue.w, obj, "w");
	getValue(returnValue.v, obj, "v");
	getValue(returnValue.u, obj, "u");
	getValue(returnValue.t, obj, "t");
	getValue(returnValue.s, obj, "s");
	getValue(returnValue.r, obj, "r");
	getValue(returnValue.q, obj, "q");
	getValue(returnValue.p, obj, "p");
	getValue(returnValue.o, obj, "o");
	getValue(returnValue.n, obj, "n");
	getValue(returnValue.m, obj, "m");
	getValue(returnValue.l, obj, "l");
	getValue(returnValue.k, obj, "k");
	getValue(returnValue.j, obj, "j");
	getValue(returnValue.i, obj, "i");
	getValue(returnValue.h, obj, "h");
	getValue(returnValue.g, obj, "g");
	getValue(returnValue.f, obj, "f");
	getValue(returnValue.e, obj, "e");
	getValue(returnValue.d, obj, "d");
	getValue(returnValue.c, obj, "c");
	getValue(returnValue.b, obj, "b");
	getValue(returnValue.a, obj, "a");
	return true;
}

bool getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.testBools, obj, "testBools");
	getValue(returnValue.testInts, obj, "testInts");
	getValue(returnValue.testUints, obj, "testUints");
	getValue(returnValue.testStrings, obj, "testStrings");
	getValue(returnValue.testDoubles, obj, "testDoubles");
	return true;
}

bool getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (newObj.error()) {
		return false;
	}
	auto obj = newObj.value();
	getValue(returnValue.a, obj, "a");
	getValue(returnValue.b, obj, "b");
	getValue(returnValue.c, obj, "c");
	getValue(returnValue.d, obj, "d");
	getValue(returnValue.e, obj, "e");
	getValue(returnValue.f, obj, "f");
	getValue(returnValue.g, obj, "g");
	getValue(returnValue.h, obj, "h");
	getValue(returnValue.i, obj, "i");
	getValue(returnValue.j, obj, "j");
	getValue(returnValue.k, obj, "k");
	getValue(returnValue.l, obj, "l");
	getValue(returnValue.m, obj, "m");
	getValue(returnValue.n, obj, "n");
	getValue(returnValue.o, obj, "o");
	getValue(returnValue.p, obj, "p");
	getValue(returnValue.q, obj, "q");
	getValue(returnValue.r, obj, "r");
	getValue(returnValue.s, obj, "s");
	getValue(returnValue.t, obj, "t");
	getValue(returnValue.u, obj, "u");
	getValue(returnValue.v, obj, "v");
	getValue(returnValue.w, obj, "w");
	getValue(returnValue.x, obj, "x");
	getValue(returnValue.y, obj, "y");
	getValue(returnValue.z, obj, "z");
	return true;
}

#endif