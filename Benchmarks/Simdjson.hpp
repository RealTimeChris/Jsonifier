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
	auto result = jsonData.get(static_cast<bool&>(data));
	if (result) {
		throwError(result);
	}
	return;
}

simdjson::ondemand::array getArray(simdjson::ondemand::value jsonData) {
	simdjson::ondemand::array newerArray{};
	auto newArr = jsonData.get_array().get(newerArray);
	if (newArr) {
		throwError(newArr);
	}
	return newerArray;
}

simdjson::ondemand::object getObject(simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object newerObject{};
	auto newObj = jsonData.get_object().get(newerObject);
	if (newObj) {
		throwError(newObj);
	}
	return newerObject;
}

template<vector_t value_type> void getValue(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::array newArray{ getArray(jsonData) };
	const auto size = value.size();
	auto iter		= newArray.begin();
	typename value_type::value_type valueNew{};
	simdjson::simdjson_result<simdjson::ondemand::value> resultNew;
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		resultNew = iter.value().operator*().value();
		getValue(valueNew, resultNew.value());
		value[x] = jsonifier::internal::move(valueNew);
	}
	for (; iter != newArray.end(); ++iter) {
		resultNew = iter.value().operator*().value();
		getValue(valueNew, resultNew.value());
		value.emplace_back(jsonifier::internal::move(valueNew));
	}
}

template<> void getValue(jsonifier::skip&, simdjson::ondemand::value jsonData) {
	if (!jsonData.is_null().value()) {
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
		value[key] = jsonifier::internal::move(newValue);
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
	if (jsonData.type() == simdjson::ondemand::json_type::null) {
		return;
	}
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
	getValue(value.source_status_id, obj, "source_status_id");
	getValue(value.source_status_id_str, obj, "source_status_id_str");
}

template<> void getValue(url_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.url, obj, "url");
	getValue(value.expanded_url, obj, "expanded_url");
	getValue(value.display_url, obj, "display_url");
	getValue(value.indices, obj, "indices");
}

template<> void getValue(user_mention_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.screen_name, obj, "screen_name");
	getValue(value.name, obj, "name");
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.indices, obj, "indices");
}

template<> void getValue(status_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.hashtags, obj, "hashtags");
	getValue(value.symbols, obj, "symbols");
	getValue(value.urls, obj, "urls");
	getValue(value.user_mentions, obj, "user_mentions");
	getValue(value.media, obj, "media");
}

template<> void getValue(metadata_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.result_type, obj, "result_type");
	getValue(value.iso_language_code, obj, "iso_language_code");
}

template<> void getValue(description_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.urls, obj, "urls");
}

template<> void getValue(user_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.description, obj, "description");
	getValue(value.url, obj, "url");
}

template<> void getValue(twitter_user_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.id, obj, "id");
	getValue(value.id_str, obj, "id_str");
	getValue(value.name, obj, "name");
	getValue(value.screen_name, obj, "screen_name");
	getValue(value.location, obj, "location");
	getValue(value.description, obj, "description");
	getValue(value.url, obj, "url");
	getValue(value.entities, obj, "entities");
	getValue(value.protectedVal, obj, "protected");
	getValue(value.followers_count, obj, "followers_count");
	getValue(value.friends_count, obj, "friends_count");
	getValue(value.listed_count, obj, "listed_count");
	getValue(value.created_at, obj, "created_at");
	getValue(value.favourites_count, obj, "favourites_count");
	getValue(value.utc_offset, obj, "utc_offset");
	getValue(value.time_zone, obj, "time_zone");
	getValue(value.geo_enabled, obj, "geo_enabled");
	getValue(value.verified, obj, "verified");
	getValue(value.statuses_count, obj, "statuses_count");
	getValue(value.lang, obj, "lang");
	getValue(value.contributors_enabled, obj, "contributors_enabled");
	getValue(value.is_translator, obj, "is_translator");
	getValue(value.is_translation_enabled, obj, "is_translation_enabled");
	getValue(value.profile_background_color, obj, "profile_background_color");
	getValue(value.profile_background_image_url, obj, "profile_background_image_url");
	getValue(value.profile_background_image_url_https, obj, "profile_background_image_url_https");
	getValue(value.profile_background_tile, obj, "profile_background_tile");
	getValue(value.profile_image_url, obj, "profile_image_url");
	getValue(value.profile_image_url_https, obj, "profile_image_url_https");
	getValue(value.profile_banner_url, obj, "profile_banner_url");
	getValue(value.profile_link_color, obj, "profile_link_color");
	getValue(value.profile_sidebar_border_color, obj, "profile_sidebar_border_color");
	getValue(value.profile_sidebar_fill_color, obj, "profile_sidebar_fill_color");
	getValue(value.profile_text_color, obj, "profile_text_color");
	getValue(value.profile_use_background_image, obj, "profile_use_background_image");
	getValue(value.default_profile, obj, "default_profile");
	getValue(value.default_profile_image, obj, "default_profile_image");
	getValue(value.following, obj, "following");
	getValue(value.follow_request_sent, obj, "follow_request_sent");
	getValue(value.notifications, obj, "notifications");
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

template<> void getValue(user_data_partial& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.screen_name, obj, "screen_name");
}

template<> void getValue(status_data_partial& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.retweet_count, obj, "retweet_count");
	getValue(value.text, obj, "text");
	getValue(value.user, obj, "user");
}

template<> void getValue(twitter_partial_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.statuses, obj, "statuses");
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

template<> void getValue(emoji_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.id, obj, "id");
	getValue(value.name, obj, "name");
	getValue(value.roles, obj, "roles");
	getValue(value.require_colons, obj, "require_colons");
	getValue(value.managed, obj, "managed");
	getValue(value.animated, obj, "animated");
	getValue(value.available, obj, "available");
}

template<> void getValue(incidents_data_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.invites_disabled_until, obj, "invites_disabled_until");
	getValue(value.dms_disabled_until, obj, "dms_disabled_until");
}

template<> void getValue(tags_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.bot_id, obj, "bot_id");
	getValue(value.guild_connections, obj, "guild_connections");
}

template<> void getValue(role_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.id, obj, "id");
	getValue(value.name, obj, "name");
	getValue(value.description, obj, "description");
	getValue(value.permissions, obj, "permissions");
	getValue(value.position, obj, "position");
	getValue(value.color, obj, "color");
	getValue(value.hoist, obj, "hoist");
	getValue(value.managed, obj, "managed");
	getValue(value.mentionable, obj, "mentionable");
	getValue(value.icon, obj, "icon");
	getValue(value.unicode_emoji, obj, "unicode_emoji");
	getValue(value.flags, obj, "flags");
	getValue(value.tags, obj, "tags");
}

template<> void getValue(sticker_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.id, obj, "id");
	getValue(value.name, obj, "name");
	getValue(value.tags, obj, "tags");
	getValue(value.type, obj, "type");
	getValue(value.format_type, obj, "format_type");
	getValue(value.description, obj, "description");
	getValue(value.asset, obj, "asset");
	getValue(value.available, obj, "available");
	getValue(value.guild_id, obj, "guild_id");
}

template<> void getValue(discord_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(value.id, obj, "id");
	getValue(value.name, obj, "name");
	getValue(value.icon, obj, "icon");
	getValue(value.description, obj, "description");
	getValue(value.home_header, obj, "home_header");
	getValue(value.splash, obj, "splash");
	getValue(value.discovery_splash, obj, "discovery_splash");
	getValue(value.features, obj, "features");
	getValue(value.banner, obj, "banner");
	getValue(value.owner_id, obj, "owner_id");
	getValue(value.application_id, obj, "application_id");
	getValue(value.region, obj, "region");
	getValue(value.afk_channel_id, obj, "afk_channel_id");
	getValue(value.afk_timeout, obj, "afk_timeout");
	getValue(value.system_channel_id, obj, "system_channel_id");
	getValue(value.system_channel_flags, obj, "system_channel_flags");
	getValue(value.widget_enabled, obj, "widget_enabled");
	getValue(value.widget_channel_id, obj, "widget_channel_id");
	getValue(value.verification_level, obj, "verification_level");
	getValue(value.roles, obj, "roles");
	getValue(value.default_message_notifications, obj, "default_message_notifications");
	getValue(value.mfa_level, obj, "mfa_level");
	getValue(value.explicit_content_filter, obj, "explicit_content_filter");
	getValue(value.max_presences, obj, "max_presences");
	getValue(value.max_members, obj, "max_members");
	getValue(value.max_stage_video_channel_users, obj, "max_stage_video_channel_users");
	getValue(value.max_video_channel_users, obj, "max_video_channel_users");
	getValue(value.vanity_url_code, obj, "vanity_url_code");
	getValue(value.premium_tier, obj, "premium_tier");
	getValue(value.premium_subscription_count, obj, "premium_subscription_count");
	getValue(value.preferred_locale, obj, "preferred_locale");
	getValue(value.rules_channel_id, obj, "rules_channel_id");
	getValue(value.safety_alerts_channel_id, obj, "safety_alerts_channel_id");
	getValue(value.public_updates_channel_id, obj, "public_updates_channel_id");
	getValue(value.hub_type, obj, "hub_type");
	getValue(value.premium_progress_bar_enabled, obj, "premium_progress_bar_enabled");
	getValue(value.latest_onboarding_question_id, obj, "latest_onboarding_question_id");
	getValue(value.nsfw, obj, "nsfw");
	getValue(value.nsfw_level, obj, "nsfw_level");
	getValue(value.emojis, obj, "emojis");
	getValue(value.stickers, obj, "stickers");
	getValue(value.incidents_data, obj, "incidents_data");
	getValue(value.inventory_settings, obj, "inventory_settings");
	getValue(value.embed_enabled, obj, "embed_enabled");
	getValue(value.embed_channel_id, obj, "embed_channel_id");
	getValue(value.approximate_member_count, obj, "approximate_member_count");
	getValue(value.approximate_presence_count, obj, "approximate_presence_count");
}

template<> void getValue(abc_test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.testVals04, obj, "testVals04");
	getValue(returnValue.testVals03, obj, "testVals03");
	getValue(returnValue.testVals01, obj, "testVals01");
	getValue(returnValue.testVals05, obj, "testVals05");
	getValue(returnValue.testVals02, obj, "testVals02");
}

template<> void getValue(abc_test<abc_test_struct>& returnValue, simdjson::ondemand::value jsonData) {
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

template<> void getValue(partial_test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.testVals05, obj, "testVals05");
	getValue(returnValue.testVals01, obj, "testVals01");
}

template<> void getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.testVals02, obj, "testVals02");
	getValue(returnValue.testVals05, obj, "testVals05");
	getValue(returnValue.testVals01, obj, "testVals01");
	getValue(returnValue.testVals03, obj, "testVals03");
	getValue(returnValue.testVals04, obj, "testVals04");
}

template<> void getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
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

template<typename value_type> void getValue(partial_test<value_type>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	getValue(returnValue.m, obj, "m");
}

#endif