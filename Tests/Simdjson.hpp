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
concept bool_t = std::same_as<std::remove_cvref_t<value_type>, bool> || std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::reference> ||
	std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::const_reference>;

template<typename value_type>
concept floating_type = std::floating_point<std::remove_cvref_t<value_type>> && !bool_t<value_type>;

template<typename value_type>
concept unsigned_type = std::unsigned_integral<std::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept signed_type = std::signed_integral<std::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept has_range = requires(std::remove_cvref_t<value_type> value) {
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
} && has_range<value_type> && map_subscriptable<value_type> && !std::is_integral_v<std::remove_cvref_t<value_type>>;

template<typename value_type>
concept vector_subscriptable = requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::const_reference>;
} || requires(std::remove_cvref_t<value_type> value) {
	{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::reference>;
};

template<typename value_type>
concept has_substr = requires(std::remove_cvref_t<value_type> value) {
	{ value.substr(typename std::remove_cvref_t<value_type>::size_type{}, typename std::remove_cvref_t<value_type>::size_type{}) } -> std::same_as<std::remove_cvref_t<value_type>>;
};

template<typename value_type>
concept has_resize = requires(std::remove_cvref_t<value_type> value) { value.resize(typename std::remove_cvref_t<value_type>::size_type{}); };

template<typename value_type>
concept has_data = requires(std::remove_cvref_t<value_type> value) {
	{ value.data() } -> std::same_as<typename std::remove_cvref_t<value_type>::const_pointer>;
} || requires(std::remove_cvref_t<value_type> value) {
	{ value.data() } -> std::same_as<typename std::remove_cvref_t<value_type>::pointer>;
};

template<typename value_type>
concept has_size = requires(std::remove_cvref_t<value_type> value) {
	{ value.size() } -> std::same_as<typename std::remove_cvref_t<value_type>::size_type>;
};

template<typename value_type>
concept string_t =
	has_substr<value_type> && has_data<value_type> && has_size<value_type> && !std::same_as<std::remove_cvref_t<value_type>, char> && vector_subscriptable<value_type>;

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
concept vector_t = !map_t<value_type> && vector_subscriptable<value_type> && !std::is_pointer_v<std::remove_cvref_t<value_type>> && !string_t<value_type>;

void throwError(auto error, std::source_location location = std::source_location::current()) {
	std::stringstream stream{};
	stream << "Error: " << error << std::endl;
	stream << "Thrown from: " << location.file_name() << ", At Line: " << location.line() << std::endl;
	std::cout << stream.str();
}

template<typename value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData);

template<floating_type value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		if (auto result = jsonData.get_double().get(data); result) {
			throwError(result);
		}
		return;
	} else {
		double newValue{};
		if (auto result = jsonData.get_double().get(newValue); result) {
			throwError(result);
		}
		data = static_cast<value_type>(newValue);
		return;
	}
}

template<unsigned_type value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		if (auto result = jsonData.get_uint64().get(data); result) {
			throwError(result);
		}
		return;
	} else {
		uint64_t newValue{};
		if (auto result = jsonData.get_uint64().get(newValue); result) {
			throwError(result);
		}
		data = static_cast<value_type>(newValue);
		return;
	}
}

template<signed_type value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData) {
	if constexpr (sizeof(value_type) == 8) {
		if (auto result = jsonData.get_int64().get(data); result) {
			throwError(result);
		}
		return;
	} else {
		int64_t newValue{};
		if (auto result = jsonData.get_int64().get(newValue); result) {
			throwError(result);
		}
		data = static_cast<value_type>(newValue);
		return;
	}
}

template<> void getValue(std::nullptr_t&, simdjson::ondemand::value value) {
	auto result = value.is_null();
	if (auto resultNew = result.error(); resultNew) {
		throwError(resultNew);
	}
}

template<string_t value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData) {
	std::string_view newValue;
	if (auto result = jsonData.get(newValue); result) {
		throwError(result);
	}
	data = static_cast<value_type>(newValue);
}

template<bool_t value_type> void getValue(value_type& data, simdjson::ondemand::value jsonData) {
	bool newValue;
	auto result = jsonData.get(newValue);
	if (result) {
		throwError(result);
	}
	data = static_cast<value_type>(newValue);
	return;
}

simdjson::ondemand::array getArray(simdjson::ondemand::value jsonData) {
	auto newArr = jsonData.get_array();
	if (auto result = newArr.error()) {
		throwError(result);
	}
	return newArr.value();
}

simdjson::ondemand::object getObject(simdjson::ondemand::value jsonData) {
	auto newObj = jsonData.get_object();
	if (auto result = newObj.error()) {
		throwError(result);
	}
	return newObj.value();
}

template<vector_t value_type> void getValue(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::array newArray{ getArray(jsonData) };
	const auto size = value.size();
	auto iter		= newArray.begin();
	typename value_type::value_type valueNew;
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		getValue(valueNew, iter.value().operator*().value());
		value[x] = std::move(valueNew);
	}
	for (; iter != newArray.end(); ++iter) {
		getValue(valueNew, iter.value().operator*().value());
		value.emplace_back(std::move(valueNew));
	}
}

template<typename value_type> void getValue(std::optional<value_type>& vec, simdjson::ondemand::value jsonData) {
	if (!jsonData.is_null().value()) {
		getValue(vec.emplace(), jsonData);
	}
}

template<map_t value_type> void getValue(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object object{ getObject(jsonData) };
	for (auto field: object) {
		typename std::remove_cvref_t<decltype(value)>::key_type key;
		auto key_result = field.unescaped_key();
		if (auto result = key_result.error()) {
			throwError(result);
		}

		key = static_cast<typename std::remove_cvref_t<decltype(value)>::key_type>(std::string(key_result.value()));

		simdjson::ondemand::value field_value = field.value();
		typename std::remove_cvref_t<decltype(value)>::mapped_type newValue;
		getValue(newValue, field_value);
		value[key] = std::move(newValue);
	}

	return;
}

template<unique_ptr_t value_type> void getValue(value_type& e, simdjson::ondemand::value jsonData) {
	if (!e) {
		e = std::make_unique<std::remove_cvref_t<decltype(*e)>>();
	}
	return getValue(*e, jsonData);
}

template<shared_ptr_t value_type> void getValue(value_type& e, simdjson::ondemand::value jsonData) {
	if (!e) {
		e = std::make_shared<std::remove_cvref_t<decltype(*e)>>();
	}
	return getValue(*e, jsonData);
}

template<> void getValue(std::string*& value, simdjson::ondemand::value jsonData) {
	if (!value) {
		value = new std::string{};
	}
	return getValue(*value, jsonData);
}

template<typename value_type> void getValue(value_type& value, simdjson::ondemand::object jsonData, const std::string_view& key) {
	simdjson::ondemand::value jsonValue;
	auto error = jsonData[key].get(jsonValue);
	if (error == simdjson::SUCCESS) {
		return getValue(value, jsonValue);
	} else if (error == simdjson::NO_SUCH_FIELD) {
		return;
	} else {
		throwError(error);
	}
}

template<> void getValue(hashtag_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.indices, obj, "indices");
	getValue(value.text, obj, "text");
}

template<> void getValue(large_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.w, obj, "w");
	getValue(value.h, obj, "h");
	getValue(value.resize, obj, "resize");
}

template<> void getValue(sizes_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.medium, obj, "medium");
	getValue(value.small, obj, "small");
	getValue(value.thumb, obj, "thumb");
	getValue(value.large, obj, "large");
}

template<> void getValue(media_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.source_status_id, obj, "source_status_id");
	getValue(value.source_status_id_str, obj, "source_status_id_str");
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.indices, obj, "indices");
	getValue(value.media_url, obj, "media_url");
	getValue(value.media_url_https, obj, "media_url_https");
	getValue(value.url, obj, "url");
	getValue(value.display_url, obj, "display_url");
	getValue(value.expanded_url, obj, "expanded_url");
	getValue(value.type, obj, "type");
	getValue(value.sizes, obj, "sizes");
}

template<> void getValue(url_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.url, obj, "url");
	getValue(value.expanded_url, obj, "expanded_url");
	getValue(value.display_url, obj, "display_url");
	getValue(value.indices, obj, "indices");
}

template<> void getValue(user_mention& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.screen_name, obj, "screen_name");
	getValue(value.name, obj, "name");
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.indices, obj, "indices");
}

template<> void getValue(status_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.media, obj, "media");
	getValue(value.hashtags, obj, "hashtags");
	getValue(value.symbols, obj, "symbols");
	getValue(value.urls, obj, "urls");
	getValue(value.user_mentions, obj, "user_mentions");
}

template<> void getValue(metadata_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.result_type, obj, "result_type");
	getValue(value.iso_language_code, obj, "iso_language_code");
}

template<> void getValue(search_metadata_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.completed_in, obj, "completed_in");
	getValue(value.max_id, obj, "max_id");
	getValue(value.max_id_str, obj, "max_id_str");
	getValue(value.next_results, obj, "next_results");
	getValue(value.query, obj, "query");
	getValue(value.refresh_url, obj, "refresh_url");
	getValue(value.count, obj, "count");
	getValue(value.since_id, obj, "since_id");
	getValue(value.since_id_str, obj, "since_id_str");
}

template<> void getValue(description_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.urls, obj, "urls");
}

template<> void getValue(user_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.url, obj, "url");
	getValue(value.description, obj, "description");
}

template<> void getValue(twitter_user_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.url, obj, "url");
	getValue(value.utc_offset, obj, "utc_offset");
	getValue(value.time_zone, obj, "time_zone");
	getValue(value.profile_banner_url, obj, "profile_banner_url");
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.name, obj, "name");
	getValue(value.screen_name, obj, "screen_name");
	getValue(value.location, obj, "location");
	getValue(value.description, obj, "description");
	getValue(value.protectedVal, obj, "protected");
	getValue(value.followers_count, obj, "followers_count");
	getValue(value.friends_count, obj, "friends_count");
	getValue(value.listed_count, obj, "listed_count");
	getValue(value.created_at, obj, "created_at");
	getValue(value.favourites_count, obj, "favourites_count");
	getValue(value.geo_enabled, obj, "geo_enabled");
	getValue(value.verified, obj, "verified");
	getValue(value.statuses_count, obj, "statuses_count");
	getValue(value.lang, obj, "lang");
	getValue(value.contributors_enabled, obj, "contributors_enabled");
	getValue(value.is_translator, obj, "is_translator");
	getValue(value.is_translation_enabled, obj, "is_translation_enabled");
}

template<> void getValue(status_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.metadata, obj, "metadata");
	getValue(value.created_at, obj, "created_at");
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.text, obj, "text");
	getValue(value.source, obj, "source");
	getValue(value.truncated, obj, "truncated");
	getValue(value.in_reply_to_status_id, obj, "in_reply_to_status_id");
	getValue(value.in_reply_to_status_id_str, obj, "in_reply_to_status_id_str");
	getValue(value.in_reply_to_user_id, obj, "in_reply_to_user_id");
	getValue(value.in_reply_to_user_id_str, obj, "in_reply_to_user_id_str");
	getValue(value.in_reply_to_screen_name, obj, "in_reply_to_screen_name");
	getValue(value.user, obj, "user");
	getValue(value.geo, obj, "geo");
	getValue(value.coordinates, obj, "coordinates");
	getValue(value.place, obj, "place");
	getValue(value.contributors, obj, "contributors");
	getValue(value.retweet_count, obj, "retweet_count");
	getValue(value.favorite_count, obj, "favorite_count");
	getValue(value.entities, obj, "entities");
	getValue(value.favorited, obj, "favorited");
	getValue(value.retweeted, obj, "retweeted");
	getValue(value.lang, obj, "lang");
	getValue(value.retweeted_status, obj, "retweeted_status");
	getValue(value.possibly_sensitive, obj, "possibly_sensitive");
}

template<> void getValue(twitter_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.statuses, obj, "statuses");
	getValue(value.search_metadata, obj, "search_metadata");
}

template<> void getValue(audience_sub_category_names& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(p.the337100890, obj, "337100890");
}

template<> void getValue(names&, simdjson::ondemand::value) {
	// No data extraction needed
}

template<> void getValue(event& e, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(e.description, obj, "description");
	getValue(e.id, obj, "id");
	getValue(e.logo, obj, "logo");
	getValue(e.name, obj, "name");
	getValue(e.topicIds, obj, "subTopicIds");
	getValue(e.subjectCode, obj, "subjectCode");
	getValue(e.subtitle, obj, "subtitle");
	getValue(e.topicIds, obj, "topicIds");
}

template<> void getValue(price& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(p.amount, obj, "amount");
	getValue(p.audienceSubCategoryId, obj, "audienceSubCategoryId");
	getValue(p.seatCategoryId, obj, "seatCategoryId");
}

template<> void getValue(area& a, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(a.areaId, obj, "areaId");
	getValue(a.blockIds, obj, "blockIds");
}

template<> void getValue(seat_category& sc, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(sc.areas, obj, "areas");
	getValue(sc.seatCategoryId, obj, "seatCategoryId");
}

template<> void getValue(venue_names& vn, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(vn.PLEYEL_PLEYEL, obj, "PLEYEL_PLEYEL");
}

template<> void getValue(performance& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(p.eventId, obj, "eventId");
	getValue(p.eventId, obj, "id");
	getValue(p.logo, obj, "logo");
	getValue(p.name, obj, "name");
	getValue(p.prices, obj, "prices");
	getValue(p.seatCategories, obj, "seatCategories");
	getValue(p.seatMapImage, obj, "seatMapImage");
	getValue(p.start, obj, "start");
	getValue(p.venueCode, obj, "venueCode");
}

template<> void getValue(citm_catalog_message& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.areaNames, obj, "areaNames");
	getValue(msg.audienceSubCategoryNames, obj, "audienceSubCategoryNames");
	getValue(msg.blockNames, obj, "blockNames");
	getValue(msg.events, obj, "events");
	getValue(msg.performances, obj, "performances");
	getValue(msg.seatCategoryNames, obj, "seatCategoryNames");
	getValue(msg.subTopicNames, obj, "subTopicNames");
	getValue(msg.subjectNames, obj, "subjectNames");
	getValue(msg.topicNames, obj, "topicNames");
	getValue(msg.topicSubTopics, obj, "topicSubTopics");
	getValue(msg.venueNames, obj, "venueNames");
}

template<> void getValue(geometry_data& geometry, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(geometry.type, obj, "type");
	getValue(geometry.coordinates, obj, "coordinates");
}

template<> void getValue(properties_data& properties, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(properties.name, obj, "name");
}

template<> void getValue(feature& f, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(f.type, obj, "type");
	getValue(f.properties, obj, "properties");
	getValue(f.geometry, obj, "geometry");
}

template<> void getValue(canada_message& message, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(message.type, obj, "type");
	getValue(message.features, obj, "features");
}

template<> void getValue(int32_t*& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp{};
	if (auto result = jsonData.get(temp); result) {
		throwError(result);
	}
	out_value = new int32_t{ static_cast<int32_t>(temp) };
}

template<> void getValue(std::unique_ptr<int32_t>& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp{};
	if (auto result = jsonData.get(temp); result) {
		throwError(result);
	}
	out_value = std::make_unique<int32_t>(static_cast<int32_t>(temp));
}

template<> void getValue(icon_emoji_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.name, obj, "name");
	getValue(msg.id, obj, "id");
}

template<> void getValue(permission_overwrite& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.allow, obj, "allow");
	getValue(msg.type, obj, "type");
	getValue(msg.deny, obj, "deny");
	getValue(msg.id, obj, "id");
}

template<> void getValue(channel_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.default_thread_rate_limit_per_user, obj, "default_thread_rate_limit_per_user");
	getValue(msg.default_auto_archive_duration, obj, "default_auto_archive_duration");
	getValue(msg.permission_overwrites, obj, "permission_overwrites");
	getValue(msg.rate_limit_per_user, obj, "rate_limit_per_user");
	getValue(msg.video_quality_mode, obj, "video_quality_mode");
	getValue(msg.total_message_sent, obj, "total_message_sent");
	getValue(msg.last_pin_timestamp, obj, "last_pin_timestamp");
	getValue(msg.last_message_id, obj, "last_message_id");
	getValue(msg.application_id, obj, "application_id");
	getValue(msg.message_count, obj, "message_count");
	getValue(msg.member_count, obj, "member_count");
	getValue(msg.applied_tags, obj, "applied_tags");
	getValue(msg.permissions, obj, "permissions");
	getValue(msg.user_limit, obj, "user_limit");
	getValue(msg.icon_emoji, obj, "icon_emoji");
	getValue(msg.recipients, obj, "recipients");
	getValue(msg.parent_id, obj, "parent_id");
	getValue(msg.position, obj, "position");
	getValue(msg.guild_id, obj, "guild_id");
	getValue(msg.owner_id, obj, "owner_id");
	getValue(msg.managed, obj, "managed");
	getValue(msg.bitrate, obj, "bitrate");
	getValue(msg.version, obj, "version");
	getValue(msg.status, obj, "status");
	getValue(msg.flags, obj, "flags");
	getValue(msg.topic, obj, "topic");
	getValue(msg.nsfw, obj, "nsfw");
	getValue(msg.type, obj, "type");
	getValue(msg.icon, obj, "icon");
	getValue(msg.name, obj, "name");
	getValue(msg.id, obj, "id");
}

template<> void getValue(user_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.avatar_decoration_data, obj, "avatar_decoration_data");
	getValue(msg.discriminator, obj, "discriminator");
	getValue(msg.public_flags, obj, "public_flags");
	getValue(msg.premium_type, obj, "premium_type");
	getValue(msg.accent_color, obj, "accent_color");
	getValue(msg.display_name, obj, "display_name");
	getValue(msg.mfa_enabled, obj, "mfa_enabled");
	getValue(msg.global_name, obj, "global_name");
	getValue(msg.user_name, obj, "user_name");
	getValue(msg.verified, obj, "verified");
	getValue(msg.system, obj, "system");
	getValue(msg.locale, obj, "locale");
	getValue(msg.banner, obj, "banner");
	getValue(msg.avatar, obj, "avatar");
	getValue(msg.flags, obj, "flags");
	getValue(msg.email, obj, "email");
	getValue(msg.bot, obj, "bot");
	getValue(msg.id, obj, "id");
}

template<> void getValue(member_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.communication_disabled_until, obj, "communication_disabled_until");
	getValue(msg.premium_since, obj, "premium_since");
	getValue(msg.permissions, obj, "permissions");
	getValue(msg.joined_at, obj, "joined_at");
	getValue(msg.guild_id, obj, "guild_id");
	getValue(msg.pending, obj, "pending");
	getValue(msg.avatar, obj, "avatar");
	getValue(msg.flags, obj, "flags");
	getValue(msg.roles, obj, "roles");
	getValue(msg.mute, obj, "mute");
	getValue(msg.deaf, obj, "deaf");
	getValue(msg.user, obj, "user");
	getValue(msg.nick, obj, "nick");
}

template<> void getValue(tags_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.premium_subscriber, obj, "premium_subscriber");
	getValue(msg.bot_id, obj, "bot_id");
}

template<> void getValue(role_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.unicode_emoji, obj, "unicode_emoji");
	getValue(msg.mentionable, obj, "mentionable");
	getValue(msg.permissions, obj, "permissions");
	getValue(msg.position, obj, "position");
	getValue(msg.managed, obj, "managed");
	getValue(msg.version, obj, "version");
	getValue(msg.hoist, obj, "hoist");
	getValue(msg.flags, obj, "flags");
	getValue(msg.color, obj, "color");
	getValue(msg.tags, obj, "tags");
	getValue(msg.name, obj, "name");
	getValue(msg.icon, obj, "icon");
	getValue(msg.id, obj, "id");
}

template<> void getValue(guild_data& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(msg.latest_on_boarding_question_id, obj, "latest_on_boarding_question_id");
	getValue(msg.max_stage_video_channel_users, obj, "max_stage_video_channel_users");
	getValue(msg.default_message_notifications, obj, "default_message_notifications");
	getValue(msg.premium_progress_bar_enabled, obj, "premium_progress_bar_enabled");
	getValue(msg.approximate_presence_count, obj, "approximate_presence_count");
	getValue(msg.premium_subscription_count, obj, "premium_subscription_count");
	getValue(msg.public_updates_channel_id, obj, "public_updates_channel_id");
	getValue(msg.approximate_member_count, obj, "approximate_member_count");
	getValue(msg.safety_alerts_channel_id, obj, "safety_alerts_channel_id");
	getValue(msg.max_video_channel_users, obj, "max_video_channel_users");
	getValue(msg.explicit_content_filter, obj, "explicit_content_filter");
	getValue(msg.guild_scheduled_events, obj, "guild_scheduled_events");
	getValue(msg.system_channel_flags, obj, "system_channel_flags");
	getValue(msg.verification_level, obj, "verification_level");
	getValue(msg.inventory_settings, obj, "inventory_settings");
	getValue(msg.widget_channel_id, obj, "widget_channel_id");
	getValue(msg.system_channel_id, obj, "system_channel_id");
	getValue(msg.rules_channel_id, obj, "rules_channel_id");
	getValue(msg.preferred_locale, obj, "preferred_locale");
	getValue(msg.discovery_splash, obj, "discovery_splash");
	getValue(msg.vanity_url_code, obj, "vanity_url_code");
	getValue(msg.widget_enabled, obj, "widget_enabled");
	getValue(msg.afk_channel_id, obj, "afk_channel_id");
	getValue(msg.application_id, obj, "application_id");
	getValue(msg.max_presences, obj, "max_presences");
	getValue(msg.premium_tier, obj, "premium_tier");
	getValue(msg.member_count, obj, "member_count");
	getValue(msg.voice_states, obj, "voice_states");
	getValue(msg.unavailable, obj, "unavailable");
	getValue(msg.afk_timeout, obj, "afk_timeout");
	getValue(msg.max_members, obj, "max_members");
	getValue(msg.permissions, obj, "permissions");
	getValue(msg.description, obj, "description");
	getValue(msg.nsfw_level, obj, "nsfw_level");
	getValue(msg.mfa_level, obj, "mfa_level");
	getValue(msg.joined_at, obj, "joined_at");
	getValue(msg.discovery, obj, "discovery");
	getValue(msg.owner_id, obj, "owner_id");
	getValue(msg.hub_type, obj, "hub_type");
	getValue(msg.stickers, obj, "stickers");
	getValue(msg.features, obj, "features");
	getValue(msg.channels, obj, "channels");
	getValue(msg.members, obj, "members");
	getValue(msg.threads, obj, "threads");
	getValue(msg.region, obj, "region");
	getValue(msg.banner, obj, "banner");
	getValue(msg.splash, obj, "splash");
	getValue(msg.owner, obj, "owner");
	getValue(msg.large, obj, "large");
	getValue(msg.flags, obj, "flags");
	getValue(msg.roles, obj, "roles");
	getValue(msg.lazy, obj, "lazy");
	getValue(msg.nsfw, obj, "nsfw");
	getValue(msg.icon, obj, "icon");
	getValue(msg.name, obj, "name");
	getValue(msg.id, obj, "id");
}

void getValue(discord_message& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.t, obj, "t");
	getValue(returnValue.d, obj, "d");
	getValue(returnValue.op, obj, "op");
	getValue(returnValue.s, obj, "s");
}

void getValue(abc_test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
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
}

void getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.testBools, obj, "testBools");
	getValue(returnValue.testInts, obj, "testInts");
	getValue(returnValue.testUints, obj, "testUints");
	getValue(returnValue.testStrings, obj, "testStrings");
	getValue(returnValue.testDoubles, obj, "testDoubles");
}

void getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
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
}

#endif