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
#include "simdjson.h"
#include "common.hpp"

#if !defined(ASAN_ENABLED)

template<typename value_type>
concept bool_t =
	std::same_as<jsonifier::internal::remove_cvref_t<value_type>, bool> || std::same_as<jsonifier::internal::remove_cvref_t<value_type>, std::vector<bool>::reference> ||
	std::same_as<jsonifier::internal::remove_cvref_t<value_type>, std::vector<bool>::const_reference>;

template<typename value_type>
concept floating_type = std::floating_point<jsonifier::internal::remove_cvref_t<value_type>> && !bool_t<value_type>;

template<typename value_type>
concept unsigned_type = std::unsigned_integral<jsonifier::internal::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept signed_type = std::signed_integral<jsonifier::internal::remove_cvref_t<value_type>> && !floating_type<value_type> && !bool_t<value_type>;

template<typename value_type>
concept has_range = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	typename jsonifier::internal::remove_cvref_t<value_type>::value_type;
	{ value.begin() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::const_iterator>;
	{ value.end() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::const_iterator>;
} || requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	typename jsonifier::internal::remove_cvref_t<value_type>::value_type;
	{ value.begin() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::iterator>;
	{ value.end() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::iterator>;
};

template<typename value_type>
concept map_subscriptable = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value[typename jsonifier::internal::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<const typename jsonifier::internal::remove_cvref_t<value_type>::mapped_type&>;
} || requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value[typename jsonifier::internal::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::mapped_type&>;
};

template<typename value_type>
concept map_t = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	typename jsonifier::internal::remove_cvref_t<value_type>::mapped_type;
	typename jsonifier::internal::remove_cvref_t<value_type>::key_type;
} && has_range<value_type> && map_subscriptable<value_type> && !std::is_integral_v<jsonifier::internal::remove_cvref_t<value_type>>;

template<typename value_type>
concept vector_subscriptable = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value[typename jsonifier::internal::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::const_reference>;
} || requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value[typename jsonifier::internal::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::reference>;
};

template<typename value_type>
concept has_substr = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{
		value.substr(typename jsonifier::internal::remove_cvref_t<value_type>::size_type{}, typename jsonifier::internal::remove_cvref_t<value_type>::size_type{})
	} -> std::same_as<jsonifier::internal::remove_cvref_t<value_type>>;
};

template<typename value_type>
concept has_resize = requires(jsonifier::internal::remove_cvref_t<value_type> value) { value.resize(typename jsonifier::internal::remove_cvref_t<value_type>::size_type{}); };

template<typename value_type>
concept has_data = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value.data() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::const_pointer>;
} || requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value.data() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::pointer>;
};

template<typename value_type>
concept has_size = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value.size() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::size_type>;
};

template<typename value_type>
concept string_t = has_substr<value_type> && has_data<value_type> && has_size<value_type> && !std::same_as<jsonifier::internal::remove_cvref_t<value_type>, char> &&
	vector_subscriptable<value_type>;

template<typename value_type>
concept copyable = std::copyable<jsonifier::internal::remove_cvref_t<value_type>>;

template<typename value_type>
concept has_release = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value.release() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::pointer>;
};

template<typename value_type>
concept has_get = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	{ value.get() } -> std::same_as<typename jsonifier::internal::remove_cvref_t<value_type>::element_type*>;
};

template<typename value_type>
concept unique_ptr_t = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
	typename jsonifier::internal::remove_cvref_t<value_type>::element_type;
	typename jsonifier::internal::remove_cvref_t<value_type>::deleter_type;
} && has_release<value_type>;

template<typename value_type>
concept shared_ptr_t = has_get<value_type> && copyable<value_type>;

template<typename value_type>
concept vector_t = !map_t<value_type> && vector_subscriptable<value_type> && !std::is_pointer_v<jsonifier::internal::remove_cvref_t<value_type>> && !string_t<value_type>;

void throwError(auto error, std::source_location location = std::source_location::current()) {
	std::stringstream stream{};
	stream << "Error: " << error << std::endl;
	stream << "Thrown from: " << location.file_name() << ", At Line: " << location.line() << std::endl;
	std::cout << stream.str();
}

template<typename value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData);
template<typename value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData);

template<floating_type value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData) {
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

template<unsigned_type value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData) {
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

template<signed_type value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData) {
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

template<> void get_value(std::nullptr_t&, simdjson::ondemand::value value) {
	auto result = value.is_null();
	if (auto resultNew = result.error(); resultNew) {
		throwError(resultNew);
	}
}

template<string_t value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData) {
	std::string_view newValue;
	if (auto result = jsonData.get(newValue); result) {
		throwError(result);
	}
	data = static_cast<value_type>(newValue);
}

template<bool_t value_type> void get_value(value_type&& data, simdjson::ondemand::value jsonData) {
	bool value{};
	auto result = jsonData.get(value);
	data		= value;
	if (result) {
		throwError(result);
	}
	return;
}

template<bool_t value_type> void get_value(value_type& data, simdjson::ondemand::value jsonData) {
	bool value{};
	auto result = jsonData.get(value);
	data		= value;
	if (result) {
		throwError(result);
	}
	return;
}

template<bool_t value_type> void get_value(value_type&& data, simdjson::ondemand::document jsonData) {
	bool value{};
	auto result = jsonData.get(value);
	data		= value;
	if (result) {
		throwError(result);
	}
	return;
}

template<bool_t value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData) {
	bool value{};
	auto result = jsonData.get(value);
	data		= value;
	if (result) {
		throwError(result);
	}
	return;
}

template<floating_type value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData) {
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

template<unsigned_type value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData) {
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

template<signed_type value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData) {
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

template<string_t value_type> void get_value(value_type& data, simdjson::ondemand::document jsonData) {
	std::string_view newValue;
	if (auto result = jsonData.get(newValue); result) {
		throwError(result);
	}
	data = static_cast<value_type>(newValue);
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

template<vector_t value_type> void get_value(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::array newArray{ getArray(jsonData) };
	const auto size = value.size();
	auto iter		= newArray.begin();
	typename value_type::value_type valueNew{};
	simdjson::simdjson_result<simdjson::ondemand::value> resultNew;
	for (size_t x = 0; (x < size) && (iter != newArray.end()); ++x, ++iter) {
		resultNew = iter.value().operator*().value();
		get_value(valueNew, resultNew.value());
		value[x] = jsonifier::internal::move(valueNew);
	}
	for (; iter != newArray.end(); ++iter) {
		resultNew = iter.value().operator*().value();
		get_value(valueNew, resultNew.value());
		value.emplace_back(jsonifier::internal::move(valueNew));
	}
}

template<> void get_value(jsonifier::skip&, simdjson::ondemand::value jsonData) {
	if (!jsonData.is_null().value()) {
	}
}

template<typename value_type> void get_value(std::optional<value_type>& vec, simdjson::ondemand::value jsonData) {
	if (!jsonData.is_null().value()) {
		get_value(vec.emplace(), jsonData);
	}
}

template<map_t value_type> void get_value(value_type& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object object{ getObject(jsonData) };
	for (auto field: object) {
		typename jsonifier::internal::remove_cvref_t<decltype(value)>::key_type key;
		auto key_result = field.unescaped_key();
		if (auto result = key_result.error()) {
			throwError(result);
		}

		key = static_cast<typename jsonifier::internal::remove_cvref_t<decltype(value)>::key_type>(std::string(key_result.value()));

		simdjson::ondemand::value field_value = field.value();
		typename jsonifier::internal::remove_cvref_t<decltype(value)>::mapped_type newValue;
		get_value(newValue, field_value);
		value[key] = jsonifier::internal::move(newValue);
	}

	return;
}

template<unique_ptr_t value_type> void get_value(value_type& e, simdjson::ondemand::value jsonData) {
	if (!e) {
		e = std::make_unique<jsonifier::internal::remove_cvref_t<decltype(*e)>>();
	}
	return get_value(*e, jsonData);
}

template<shared_ptr_t value_type> void get_value(value_type& e, simdjson::ondemand::value jsonData) {
	if (jsonData.type() == simdjson::ondemand::json_type::null) {
		return;
	}
	if (!e) {
		e = std::make_shared<jsonifier::internal::remove_cvref_t<decltype(*e)>>();
	}
	return get_value(*e, jsonData);
}

template<> void get_value(std::string*& value, simdjson::ondemand::value jsonData) {
	if (!value) {
		value = new std::string{};
	}
	return get_value(*value, jsonData);
}

template<typename value_type> void get_value(value_type& value, simdjson::ondemand::object jsonData, const std::string_view& key) {
	simdjson::ondemand::value jsonValue;
	auto error = jsonData[key].get(jsonValue);
	if (error == simdjson::SUCCESS) {
		return get_value(value, jsonValue);
	} else if (error == simdjson::NO_SUCH_FIELD) {
		return;
	} else {
		throwError(error);
	}
}

template<> void get_value(search_metadata_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.completed_in, obj, "completed_in");
	get_value(value.max_id, obj, "max_id");
	get_value(value.max_id_str, obj, "max_id_str");
	get_value(value.next_results, obj, "next_results");
	get_value(value.query, obj, "query");
	get_value(value.refresh_url, obj, "refresh_url");
	get_value(value.count, obj, "count");
	get_value(value.since_id, obj, "since_id");
	get_value(value.since_id_str, obj, "since_id_str");
}

template<> void get_value(hashtag_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.indices, obj, "indices");
	get_value(value.text, obj, "text");
}

template<> void get_value(large_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.w, obj, "w");
	get_value(value.h, obj, "h");
	get_value(value.resize, obj, "resize");
}

template<> void get_value(sizes_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.medium, obj, "medium");
	get_value(value.small, obj, "small");
	get_value(value.thumb, obj, "thumb");
	get_value(value.large, obj, "large");
}

template<> void get_value(media_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.id_str, obj, "id_str");
	get_value(value.indices, obj, "indices");
	get_value(value.media_url, obj, "media_url");
	get_value(value.media_url_https, obj, "media_url_https");
	get_value(value.url, obj, "url");
	get_value(value.display_url, obj, "display_url");
	get_value(value.expanded_url, obj, "expanded_url");
	get_value(value.type, obj, "type");
	get_value(value.sizes, obj, "sizes");
	get_value(value.source_status_id, obj, "source_status_id");
	get_value(value.source_status_id_str, obj, "source_status_id_str");
}

template<> void get_value(url_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.url, obj, "url");
	get_value(value.expanded_url, obj, "expanded_url");
	get_value(value.display_url, obj, "display_url");
	get_value(value.indices, obj, "indices");
}

template<> void get_value(user_mention_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.screen_name, obj, "screen_name");
	get_value(value.name, obj, "name");
	get_value(value.id, obj, "id");
	get_value(value.id_str, obj, "id_str");
	get_value(value.indices, obj, "indices");
}

template<> void get_value(status_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.hashtags, obj, "hashtags");
	get_value(value.symbols, obj, "symbols");
	get_value(value.urls, obj, "urls");
	get_value(value.user_mentions, obj, "user_mentions");
	get_value(value.media, obj, "media");
}

template<> void get_value(metadata_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.result_type, obj, "result_type");
	get_value(value.iso_language_code, obj, "iso_language_code");
}

template<> void get_value(description_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.urls, obj, "urls");
}

template<> void get_value(user_entities& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.description, obj, "description");
	get_value(value.url, obj, "url");
}

template<> void get_value(twitter_user_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.id_str, obj, "id_str");
	get_value(value.name, obj, "name");
	get_value(value.screen_name, obj, "screen_name");
	get_value(value.location, obj, "location");
	get_value(value.description, obj, "description");
	get_value(value.url, obj, "url");
	get_value(value.entities, obj, "entities");
	get_value(value.protectedVal, obj, "protected");
	get_value(value.followers_count, obj, "followers_count");
	get_value(value.friends_count, obj, "friends_count");
	get_value(value.listed_count, obj, "listed_count");
	get_value(value.created_at, obj, "created_at");
	get_value(value.favourites_count, obj, "favourites_count");
	get_value(value.utc_offset, obj, "utc_offset");
	get_value(value.time_zone, obj, "time_zone");
	get_value(value.geo_enabled, obj, "geo_enabled");
	get_value(value.verified, obj, "verified");
	get_value(value.statuses_count, obj, "statuses_count");
	get_value(value.lang, obj, "lang");
	get_value(value.contributors_enabled, obj, "contributors_enabled");
	get_value(value.is_translator, obj, "is_translator");
	get_value(value.is_translation_enabled, obj, "is_translation_enabled");
	get_value(value.profile_background_color, obj, "profile_background_color");
	get_value(value.profile_background_image_url, obj, "profile_background_image_url");
	get_value(value.profile_background_image_url_https, obj, "profile_background_image_url_https");
	get_value(value.profile_background_tile, obj, "profile_background_tile");
	get_value(value.profile_image_url, obj, "profile_image_url");
	get_value(value.profile_image_url_https, obj, "profile_image_url_https");
	get_value(value.profile_banner_url, obj, "profile_banner_url");
	get_value(value.profile_link_color, obj, "profile_link_color");
	get_value(value.profile_sidebar_border_color, obj, "profile_sidebar_border_color");
	get_value(value.profile_sidebar_fill_color, obj, "profile_sidebar_fill_color");
	get_value(value.profile_text_color, obj, "profile_text_color");
	get_value(value.profile_use_background_image, obj, "profile_use_background_image");
	get_value(value.default_profile, obj, "default_profile");
	get_value(value.default_profile_image, obj, "default_profile_image");
	get_value(value.following, obj, "following");
	get_value(value.follow_request_sent, obj, "follow_request_sent");
	get_value(value.notifications, obj, "notifications");
}

template<> void get_value(status_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.metadata, obj, "metadata");
	get_value(value.created_at, obj, "created_at");
	get_value(value.id, obj, "id");
	get_value(value.id_str, obj, "id_str");
	get_value(value.text, obj, "text");
	get_value(value.source, obj, "source");
	get_value(value.truncated, obj, "truncated");
	get_value(value.in_reply_to_status_id, obj, "in_reply_to_status_id");
	get_value(value.in_reply_to_status_id_str, obj, "in_reply_to_status_id_str");
	get_value(value.in_reply_to_user_id, obj, "in_reply_to_user_id");
	get_value(value.in_reply_to_user_id_str, obj, "in_reply_to_user_id_str");
	get_value(value.in_reply_to_screen_name, obj, "in_reply_to_screen_name");
	get_value(value.user, obj, "user");
	get_value(value.geo, obj, "geo");
	get_value(value.coordinates, obj, "coordinates");
	get_value(value.place, obj, "place");
	get_value(value.contributors, obj, "contributors");
	get_value(value.retweet_count, obj, "retweet_count");
	get_value(value.favorite_count, obj, "favorite_count");
	get_value(value.entities, obj, "entities");
	get_value(value.favorited, obj, "favorited");
	get_value(value.retweeted, obj, "retweeted");
	get_value(value.lang, obj, "lang");
	get_value(value.retweeted_status, obj, "retweeted_status");
	get_value(value.possibly_sensitive, obj, "possibly_sensitive");
}

template<> void get_value(twitter_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.statuses, obj, "statuses");
	get_value(value.search_metadata, obj, "search_metadata");
}

template<> void get_value(user_data_partial& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.screen_name, obj, "screen_name");
}

template<> void get_value(status_data_partial& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.retweet_count, obj, "retweet_count");
	get_value(value.text, obj, "text");
	get_value(value.user, obj, "user");
}

template<> void get_value(twitter_partial_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.statuses, obj, "statuses");
}

template<> void get_value(audience_sub_category_names& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(p.the337100890, obj, "337100890");
}

template<> void get_value(names&, simdjson::ondemand::value) {
	// No data extraction needed
}

template<> void get_value(event& e, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(e.description, obj, "description");
	get_value(e.id, obj, "id");
	get_value(e.logo, obj, "logo");
	get_value(e.name, obj, "name");
	get_value(e.topicIds, obj, "subTopicIds");
	get_value(e.subjectCode, obj, "subjectCode");
	get_value(e.subtitle, obj, "subtitle");
	get_value(e.topicIds, obj, "topicIds");
}

template<> void get_value(price& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(p.amount, obj, "amount");
	get_value(p.audienceSubCategoryId, obj, "audienceSubCategoryId");
	get_value(p.seatCategoryId, obj, "seatCategoryId");
}

template<> void get_value(area& a, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(a.areaId, obj, "areaId");
	get_value(a.blockIds, obj, "blockIds");
}

template<> void get_value(seat_category& sc, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(sc.areas, obj, "areas");
	get_value(sc.seatCategoryId, obj, "seatCategoryId");
}

template<> void get_value(venue_names& vn, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(vn.PLEYEL_PLEYEL, obj, "PLEYEL_PLEYEL");
}

template<> void get_value(performance& p, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(p.eventId, obj, "eventId");
	get_value(p.eventId, obj, "id");
	get_value(p.logo, obj, "logo");
	get_value(p.name, obj, "name");
	get_value(p.prices, obj, "prices");
	get_value(p.seatCategories, obj, "seatCategories");
	get_value(p.seatMapImage, obj, "seatMapImage");
	get_value(p.start, obj, "start");
	get_value(p.venueCode, obj, "venueCode");
}

template<> void get_value(citm_catalog_message& msg, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(msg.areaNames, obj, "areaNames");
	get_value(msg.audienceSubCategoryNames, obj, "audienceSubCategoryNames");
	get_value(msg.blockNames, obj, "blockNames");
	get_value(msg.events, obj, "events");
	get_value(msg.performances, obj, "performances");
	get_value(msg.seatCategoryNames, obj, "seatCategoryNames");
	get_value(msg.subTopicNames, obj, "subTopicNames");
	get_value(msg.subjectNames, obj, "subjectNames");
	get_value(msg.topicNames, obj, "topicNames");
	get_value(msg.topicSubTopics, obj, "topicSubTopics");
	get_value(msg.venueNames, obj, "venueNames");
}

template<> void get_value(geometry_data& geometry, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(geometry.type, obj, "type");
	get_value(geometry.coordinates, obj, "coordinates");
}

template<> void get_value(properties_data& properties, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(properties.name, obj, "name");
}

template<> void get_value(feature& f, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(f.type, obj, "type");
	get_value(f.properties, obj, "properties");
	get_value(f.geometry, obj, "geometry");
}

template<> void get_value(canada_message& message, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(message.type, obj, "type");
	get_value(message.features, obj, "features");
}

template<> void get_value(int32_t*& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp{};
	if (auto result = jsonData.get(temp); result) {
		throwError(result);
	}
	out_value = new int32_t{ static_cast<int32_t>(temp) };
}

template<> void get_value(std::unique_ptr<int32_t>& out_value, simdjson::ondemand::value jsonData) {
	int64_t temp{};
	if (auto result = jsonData.get(temp); result) {
		throwError(result);
	}
	out_value = std::make_unique<int32_t>(static_cast<int32_t>(temp));
}

template<> void get_value(emoji_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.name, obj, "name");
	get_value(value.roles, obj, "roles");
	get_value(value.require_colons, obj, "require_colons");
	get_value(value.managed, obj, "managed");
	get_value(value.animated, obj, "animated");
	get_value(value.available, obj, "available");
}

template<> void get_value(incidents_data_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.invites_disabled_until, obj, "invites_disabled_until");
	get_value(value.dms_disabled_until, obj, "dms_disabled_until");
}

template<> void get_value(tags_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.bot_id, obj, "bot_id");
	get_value(value.guild_connections, obj, "guild_connections");
}

template<> void get_value(role_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.name, obj, "name");
	get_value(value.description, obj, "description");
	get_value(value.permissions, obj, "permissions");
	get_value(value.position, obj, "position");
	get_value(value.color, obj, "color");
	get_value(value.hoist, obj, "hoist");
	get_value(value.managed, obj, "managed");
	get_value(value.mentionable, obj, "mentionable");
	get_value(value.icon, obj, "icon");
	get_value(value.unicode_emoji, obj, "unicode_emoji");
	get_value(value.flags, obj, "flags");
	get_value(value.tags, obj, "tags");
}

template<> void get_value(sticker_data& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.name, obj, "name");
	get_value(value.tags, obj, "tags");
	get_value(value.type, obj, "type");
	get_value(value.format_type, obj, "format_type");
	get_value(value.description, obj, "description");
	get_value(value.asset, obj, "asset");
	get_value(value.available, obj, "available");
	get_value(value.guild_id, obj, "guild_id");
}

template<> void get_value(discord_message& value, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(value.id, obj, "id");
	get_value(value.name, obj, "name");
	get_value(value.icon, obj, "icon");
	get_value(value.description, obj, "description");
	get_value(value.home_header, obj, "home_header");
	get_value(value.splash, obj, "splash");
	get_value(value.discovery_splash, obj, "discovery_splash");
	get_value(value.features, obj, "features");
	get_value(value.banner, obj, "banner");
	get_value(value.owner_id, obj, "owner_id");
	get_value(value.application_id, obj, "application_id");
	get_value(value.region, obj, "region");
	get_value(value.afk_channel_id, obj, "afk_channel_id");
	get_value(value.afk_timeout, obj, "afk_timeout");
	get_value(value.system_channel_id, obj, "system_channel_id");
	get_value(value.system_channel_flags, obj, "system_channel_flags");
	get_value(value.widget_enabled, obj, "widget_enabled");
	get_value(value.widget_channel_id, obj, "widget_channel_id");
	get_value(value.verification_level, obj, "verification_level");
	get_value(value.roles, obj, "roles");
	get_value(value.default_message_notifications, obj, "default_message_notifications");
	get_value(value.mfa_level, obj, "mfa_level");
	get_value(value.explicit_content_filter, obj, "explicit_content_filter");
	get_value(value.max_presences, obj, "max_presences");
	get_value(value.max_members, obj, "max_members");
	get_value(value.max_stage_video_channel_users, obj, "max_stage_video_channel_users");
	get_value(value.max_video_channel_users, obj, "max_video_channel_users");
	get_value(value.vanity_url_code, obj, "vanity_url_code");
	get_value(value.premium_tier, obj, "premium_tier");
	get_value(value.premium_subscription_count, obj, "premium_subscription_count");
	get_value(value.preferred_locale, obj, "preferred_locale");
	get_value(value.rules_channel_id, obj, "rules_channel_id");
	get_value(value.safety_alerts_channel_id, obj, "safety_alerts_channel_id");
	get_value(value.public_updates_channel_id, obj, "public_updates_channel_id");
	get_value(value.hub_type, obj, "hub_type");
	get_value(value.premium_progress_bar_enabled, obj, "premium_progress_bar_enabled");
	get_value(value.latest_onboarding_question_id, obj, "latest_onboarding_question_id");
	get_value(value.nsfw, obj, "nsfw");
	get_value(value.nsfw_level, obj, "nsfw_level");
	get_value(value.emojis, obj, "emojis");
	get_value(value.stickers, obj, "stickers");
	get_value(value.incidents_data, obj, "incidents_data");
	get_value(value.inventory_settings, obj, "inventory_settings");
	get_value(value.embed_enabled, obj, "embed_enabled");
	get_value(value.embed_channel_id, obj, "embed_channel_id");
	get_value(value.approximate_member_count, obj, "approximate_member_count");
	get_value(value.approximate_presence_count, obj, "approximate_presence_count");
}

template<> void get_value(abc_test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.test_bool, obj, "testBool");
	get_value(returnValue.test_double, obj, "testDouble");
	get_value(returnValue.test_int, obj, "testInt");
	get_value(returnValue.test_uint, obj, "testUint");
	get_value(returnValue.test_string, obj, "testString");
}

template<> void get_value(abc_test<abc_test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.z, obj, "z");
	get_value(returnValue.y, obj, "y");
	get_value(returnValue.x, obj, "x");
	get_value(returnValue.w, obj, "w");
	get_value(returnValue.v, obj, "v");
	get_value(returnValue.u, obj, "u");
	get_value(returnValue.t, obj, "t");
	get_value(returnValue.s, obj, "s");
	get_value(returnValue.r, obj, "r");
	get_value(returnValue.q, obj, "q");
	get_value(returnValue.p, obj, "p");
	get_value(returnValue.o, obj, "o");
	get_value(returnValue.n, obj, "n");
	get_value(returnValue.m, obj, "m");
	get_value(returnValue.l, obj, "l");
	get_value(returnValue.k, obj, "k");
	get_value(returnValue.j, obj, "j");
	get_value(returnValue.i, obj, "i");
	get_value(returnValue.h, obj, "h");
	get_value(returnValue.g, obj, "g");
	get_value(returnValue.f, obj, "f");
	get_value(returnValue.e, obj, "e");
	get_value(returnValue.d, obj, "d");
	get_value(returnValue.c, obj, "c");
	get_value(returnValue.b, obj, "b");
	get_value(returnValue.a, obj, "a");
}

template<> void get_value(partial_test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.test_bool, obj, "testBool");
	get_value(returnValue.test_string, obj, "testString");
}

template<> void get_value(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.test_string, obj, "testString");
	get_value(returnValue.test_uint, obj, "testUint");
	get_value(returnValue.test_int, obj, "testInt");
	get_value(returnValue.test_double, obj, "testDouble");
	get_value(returnValue.test_bool, obj, "testBool");
}

template<> void get_value(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.a, obj, "a");
	get_value(returnValue.b, obj, "b");
	get_value(returnValue.c, obj, "c");
	get_value(returnValue.d, obj, "d");
	get_value(returnValue.e, obj, "e");
	get_value(returnValue.f, obj, "f");
	get_value(returnValue.g, obj, "g");
	get_value(returnValue.h, obj, "h");
	get_value(returnValue.i, obj, "i");
	get_value(returnValue.j, obj, "j");
	get_value(returnValue.k, obj, "k");
	get_value(returnValue.l, obj, "l");
	get_value(returnValue.m, obj, "m");
	get_value(returnValue.n, obj, "n");
	get_value(returnValue.o, obj, "o");
	get_value(returnValue.p, obj, "p");
	get_value(returnValue.q, obj, "q");
	get_value(returnValue.r, obj, "r");
	get_value(returnValue.s, obj, "s");
	get_value(returnValue.t, obj, "t");
	get_value(returnValue.u, obj, "u");
	get_value(returnValue.v, obj, "v");
	get_value(returnValue.w, obj, "w");
	get_value(returnValue.x, obj, "x");
	get_value(returnValue.y, obj, "y");
	get_value(returnValue.z, obj, "z");
}

template<typename value_type> void get_value(partial_test<value_type>& returnValue, simdjson::ondemand::value jsonData) {
	simdjson::ondemand::object obj{ getObject(jsonData) };
	get_value(returnValue.m, obj, "m");
}

#endif