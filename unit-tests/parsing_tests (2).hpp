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
/// https://github.com/RealTimeChris/Json-Performance
#pragma once

#include "all_tests.hpp"

namespace tests {

	inline static jsonifier::string_base<char, 1024 * 1024> stringBuffer{};

	static constexpr jsonifier::parse_options opts{};

	template<jsonifier::internal::string_literal key> JSONIFIER_INLINE bool check_key(const char* iter) noexcept {
		return jsonifier::internal::string_literal_comparitor<decltype(key), key>::impl(iter);
	}

	JSONIFIER_INLINE const char* parse_string(const char* iter, const char* end, std::string& out) noexcept {
		++iter;
		const auto new_ptr = jsonifier::internal::string_parser<opts, const char*, char*>::impl(iter, stringBuffer.data(), static_cast<uint64_t>(end - iter));
		if JSONIFIER_LIKELY (new_ptr) {
			const auto new_size = static_cast<uint64_t>(new_ptr - stringBuffer.data());
			if (out.size() != new_size) {
				out.resize(new_size);
			}
			std::memcpy(out.data(), stringBuffer.data(), new_size);
			++iter;
		}
		return iter;
	}

	template<typename value_type> JSONIFIER_INLINE const char* parse_number(const char* iter, const char* end, value_type& out) noexcept {
		if constexpr (sizeof(value_type) == 8) {
			if constexpr (jsonifier::concepts::float_t<value_type>) {
				jsonifier::internal::parseFloat(out, iter, end);
			} else {
				jsonifier::internal::integer_parser<value_type>::parseInt(out, iter, end);
			}
		} else {
			if constexpr (jsonifier::concepts::float_t<value_type>) {
				double value{};
				jsonifier::internal::parseFloat(value, iter, end);
				out = static_cast<value_type>(value);
			} else if constexpr (jsonifier::concepts::signed_t<value_type>) {
				int64_t value{};
				jsonifier::internal::integer_parser<int64_t>::parseInt(value, iter, end);
				out = static_cast<value_type>(value);
			} else {
				uint64_t value{};
				jsonifier::internal::integer_parser<uint64_t>::parseInt(value, iter, end);
				out = static_cast<value_type>(value);
			}
		}
		return iter;
	}

	JSONIFIER_INLINE const char* skip_null(const char* iter) noexcept {
		return iter + 4;
	}
	template<typename value_type> JSONIFIER_INLINE const char* parse_vector(const char* iter, const char* end, std::vector<value_type>& out) noexcept;

	template<typename value_type> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, value_type& out) noexcept {
		if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
			return skip_null(iter);
		} else if constexpr (std::is_same_v<value_type, bool>) {
			out = (*iter == 't');
			return iter + (*iter == 't' ? 4 : 5);
		} else if constexpr (std::is_same_v<value_type, std::string>) {
			return parse_string(iter, end, out);
		} else if constexpr (jsonifier::concepts::num_t<value_type>) {
			return parse_number(iter, end, out);
		} else if constexpr (jsonifier::concepts::vector_t<value_type>) {
			return parse_vector(iter, end, out);
		} else {
			static_assert(std::is_same_v<value_type, void>, "parse_value: unhandled type � add a specialization!");
		}
	}

	template<typename value_type> JSONIFIER_INLINE const char* parse_vector(const char* iter JSONIFIER_LIFETIME_BOUND, const char* end JSONIFIER_LIFETIME_BOUND,
		std::vector<value_type>& out JSONIFIER_LIFETIME_BOUND) noexcept {
		++iter;// skip '['
		if (*iter == ']') {
			++iter;
			return iter;
		}
		const uint64_t size{ out.size() };
		uint64_t x{};
		for (; x < size; ++x) {
			if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
				iter = skip_null(iter);
			} else {
				iter = parse_value(iter, end, out[x]);
				if (!iter)
					return nullptr;
			}
			if (*iter == ',') {
				++iter;
			} else {
				++iter;
				out.resize(x + 1);
				return iter;
			}
		}
		while (true) {
			if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
				iter = skip_null(iter);
				out.emplace_back(nullptr);
			} else {
				out.emplace_back();
				iter = parse_value(iter, end, out.back());
				if (!iter)
					return nullptr;
			}
			if (*iter == ',') {
				++iter;
			} else {
				++iter;
				break;
			}
		}
		return iter;
	}

	JSONIFIER_INLINE const char* parse_area(const char* iter, const char* end, area& out) noexcept;
	JSONIFIER_INLINE const char* parse_price(const char* iter, const char* end, price& out) noexcept;
	JSONIFIER_INLINE const char* parse_seat_category(const char* iter, const char* end, seat_category& out) noexcept;
	JSONIFIER_INLINE const char* parse_performance(const char* iter, const char* end, performance& out) noexcept;

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, std::vector<int64_t>& out) noexcept {
		return parse_vector(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_area(const char* iter, const char* end, area& out) noexcept {
		if (!check_key<"{\"areaId\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_number(iter, end, out.areaId);
		if (!check_key<",\"blockIds\":">(iter))
			return nullptr;
		iter += 12;
		iter = parse_vector(iter, end, out.blockIds);
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_price(const char* iter, const char* end, price& out) noexcept {
		if (!check_key<"{\"amount\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_number(iter, end, out.amount);
		if (!check_key<",\"audienceSubCategoryId\":">(iter))
			return nullptr;
		iter += 25;
		iter = parse_number(iter, end, out.audienceSubCategoryId);
		if (!check_key<",\"seatCategoryId\":">(iter))
			return nullptr;
		iter += 18;
		iter = parse_number(iter, end, out.seatCategoryId);
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_seat_category(const char* iter, const char* end, seat_category& out) noexcept {
		if (!check_key<"{\"areas\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_vector(iter, end, out.areas);
		if (!iter)
			return nullptr;
		if (!check_key<",\"seatCategoryId\":">(iter))
			return nullptr;
		iter += 18;
		iter = parse_number(iter, end, out.seatCategoryId);
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_event(const char* iter, const char* end, event& out) noexcept {
		if (!check_key<"{\"description\":">(iter))
			return nullptr;
		iter += 15;
		iter = skip_null(iter);
		if (!check_key<",\"id\":">(iter))
			return nullptr;
		iter += 6;
		iter = parse_number(iter, end, out.id);
		if (!check_key<",\"logo\":">(iter))
			return nullptr;
		iter += 8;
		if (*iter == 'n') {
			out.logo = std::nullopt;
			iter	 = skip_null(iter);
		} else {
			out.logo.emplace();
			iter = parse_string(iter, end, *out.logo);
		}
		if (!check_key<",\"name\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.name);
		if (!check_key<",\"subTopicIds\":">(iter))
			return nullptr;
		iter += 15;
		iter = parse_vector(iter, end, out.subTopicIds);
		if (!check_key<",\"subjectCode\":">(iter))
			return nullptr;
		iter += 15;
		iter = skip_null(iter);
		if (!check_key<",\"subtitle\":">(iter))
			return nullptr;
		iter += 12;
		iter = skip_null(iter);
		if (!check_key<",\"topicIds\":">(iter))
			return nullptr;
		iter += 12;
		iter = parse_vector(iter, end, out.topicIds);
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_performance(const char* iter, const char* end, performance& out) noexcept {
		if (!check_key<"{\"eventId\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_number(iter, end, out.eventId);
		if (!check_key<",\"id\":">(iter))
			return nullptr;
		iter += 6;
		iter = parse_number(iter, end, out.id);
		if (!check_key<",\"logo\":">(iter))
			return nullptr;
		iter += 8;
		if (*iter == 'n') {
			out.logo = std::nullopt;
			iter	 = skip_null(iter);
		} else {
			out.logo.emplace();
			iter = parse_string(iter, end, *out.logo);
		}
		if (!check_key<",\"name\":">(iter))
			return nullptr;
		iter += 8;
		iter = skip_null(iter);
		if (!check_key<",\"prices\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_vector(iter, end, out.prices);
		if (!iter)
			return nullptr;
		if (!check_key<",\"seatCategories\":">(iter))
			return nullptr;
		iter += 18;
		iter = parse_vector(iter, end, out.seatCategories);
		if (!iter)
			return nullptr;
		if (!check_key<",\"seatMapImage\":">(iter))
			return nullptr;
		iter += 16;
		iter = skip_null(iter);
		if (!check_key<",\"start\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_number(iter, end, out.start);
		if (!check_key<",\"venueCode\":">(iter))
			return nullptr;
		iter += 13;
		iter = parse_string(iter, end, out.venueCode);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, area& out) noexcept {
		return parse_area(iter, end, out);
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, price& out) noexcept {
		return parse_price(iter, end, out);
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, seat_category& out) noexcept {
		return parse_seat_category(iter, end, out);
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, event& out) noexcept {
		return parse_event(iter, end, out);
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, performance& out) noexcept {
		return parse_performance(iter, end, out);
	}

	template<typename value_type> JSONIFIER_INLINE const char* parse_object(const char* iter, const char* end, std::unordered_map<std::string, value_type>& out) noexcept {
		if (*iter == '}') {
			++iter;
			return iter;
		}
		static thread_local std::string key{};
		while (true) {
			iter = parse_string(iter, end, key);
			++iter;// skip ':'
			auto [it, inserted] = out.try_emplace(key);
			if (inserted) {
				key = {};
			}
			iter = parse_value(iter, end, it->second);
			if (!iter)
				return nullptr;
			if (*iter == ',') {
				++iter;
			} else {
				++iter;
				break;
			}
		}
		return iter;
	}

	JSONIFIER_INLINE const char* parse_citm_catalog(const char* iter, const char* end, citm_catalog_message& out) noexcept {
		if (!check_key<"{\"areaNames\":{">(iter))
			return nullptr;
		iter += 14;
		iter = parse_object(iter, end, out.areaNames);
		if (!iter)
			return nullptr;
		if (!check_key<",\"audienceSubCategoryNames\":{\"337100890\":">(iter))
			return nullptr;
		iter += 41;
		iter = parse_string(iter, end, out.audienceSubCategoryNames.the337100890);
		++iter;// skip '}'
		if (!check_key<",\"blockNames\":{}">(iter))
			return nullptr;
		iter += 16;
		if (!check_key<",\"events\":{">(iter))
			return nullptr;
		iter += 11;
		iter = parse_object(iter, end, out.events);
		if (!iter)
			return nullptr;
		if (!check_key<",\"performances\":">(iter))
			return nullptr;
		iter += 16;
		iter = parse_vector(iter, end, out.performances);
		if (!iter)
			return nullptr;
		if (!check_key<",\"seatCategoryNames\":{">(iter))
			return nullptr;
		iter += 22;
		iter = parse_object(iter, end, out.seatCategoryNames);
		if (!iter)
			return nullptr;
		if (!check_key<",\"subTopicNames\":{">(iter))
			return nullptr;
		iter += 18;
		iter = parse_object(iter, end, out.subTopicNames);
		if (!iter)
			return nullptr;
		if (!check_key<",\"subjectNames\":{}">(iter))
			return nullptr;
		iter += 18;
		if (!check_key<",\"topicNames\":{">(iter))
			return nullptr;
		iter += 15;
		iter = parse_object(iter, end, out.topicNames);
		if (!iter)
			return nullptr;
		if (!check_key<",\"topicSubTopics\":{">(iter))
			return nullptr;
		iter += 19;
		iter = parse_object(iter, end, out.topicSubTopics);
		if (!iter)
			return nullptr;
		if (!check_key<",\"venueNames\":{\"PLEYEL_PLEYEL\":">(iter))
			return nullptr;
		iter += 31;
		iter = parse_string(iter, end, out.venueNames.PLEYEL_PLEYEL);
		iter += 2;// skip '}}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, citm_catalog_message&out) noexcept {
		return parse_citm_catalog(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_geometry(const char* iter, const char* end, geometry_data& out) noexcept {
		if (!check_key<"{\"type\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.type);
		if (!check_key<",\"coordinates\":">(iter))
			return nullptr;
		iter += 15;
		iter = parse_vector(iter, end, out.coordinates);
		if (!iter)
			return nullptr;
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_properties(const char* iter, const char* end, properties_data& out) noexcept {
		if (!check_key<"{\"name\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.name);
		++iter;// skip '}'
		return iter;
	}

	JSONIFIER_INLINE const char* parse_feature(const char* iter, const char* end, feature& out) noexcept {
		if (!check_key<"{\"type\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.type);
		if (!check_key<",\"properties\":{\"name\":">(iter))
			return nullptr;
		iter += 22;
		iter = parse_string(iter, end, out.properties.name);
		++iter;// skip '}'
		if (!check_key<",\"geometry\":{\"type\":">(iter))
			return nullptr;
		iter += 20;
		iter = parse_string(iter, end, out.geometry.type);
		if (!check_key<",\"coordinates\":">(iter))
			return nullptr;
		iter += 15;
		iter = parse_vector(iter, end, out.geometry.coordinates);
		if (!iter)
			return nullptr;
		iter += 2;// skip '}}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, feature& out) noexcept {
		return parse_feature(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_geojson(const char* iter, const char* end, canada_message& out) noexcept {
		if (!check_key<"{\"type\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.type);
		if (!check_key<",\"features\":">(iter))
			return nullptr;
		iter += 12;
		iter = parse_vector(iter, end, out.features);
		if (!iter)
			return nullptr;
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, canada_message& out) noexcept {
		return parse_geojson(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_batch(const char* iter, const char* end, batch& out) noexcept {
		if (!check_key<"{\"indexRange\":">(iter))
			return nullptr;
		iter += 14;
		iter = parse_vector(iter, end, out.indexRange);
		if (!iter)
			return nullptr;
		if (!check_key<",\"vertexRange\":">(iter))
			return nullptr;
		iter += 15;
		iter = parse_vector(iter, end, out.vertexRange);
		if (!iter)
			return nullptr;
		if (!check_key<",\"usedBones\":">(iter))
			return nullptr;
		iter += 13;
		iter = parse_vector(iter, end, out.usedBones);
		if (!iter)
			return nullptr;
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, batch& out) noexcept {
		return parse_batch(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_mesh(const char* iter, const char* end, mesh_message& out) noexcept {
		if (!check_key<"{\"batches\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_vector(iter, end, out.batches);
		if (!iter)
			return nullptr;
		if (!check_key<",\"morphTargets\":{}">(iter))
			return nullptr;
		iter += 18;
		if (!check_key<",\"positions\":">(iter))
			return nullptr;
		iter += 13;
		iter = parse_vector(iter, end, out.positions);
		if (!iter)
			return nullptr;
		if (!check_key<",\"tex0\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_vector(iter, end, out.tex0);
		if (!iter)
			return nullptr;
		if (!check_key<",\"colors\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_vector(iter, end, out.colors);
		if (!iter)
			return nullptr;
		if (!check_key<",\"influences\":">(iter))
			return nullptr;
		iter += 14;
		iter = parse_vector(iter, end, out.influences);
		if (!iter)
			return nullptr;
		if (!check_key<",\"normals\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_vector(iter, end, out.normals);
		if (!iter)
			return nullptr;
		if (!check_key<",\"indices\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_vector(iter, end, out.indices);
		if (!iter)
			return nullptr;
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, mesh_message& out) noexcept {
		return parse_mesh(iter, end, out);
	}

	

	JSONIFIER_INLINE const char* parse_friend_element(const char* iter, const char* end, friend_element& out) noexcept {
		if (!check_key<"{\"id\":">(iter))
			return nullptr;
		iter += 6;
		iter = parse_number(iter, end, out.id);
		if (!check_key<",\"name\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.name);
		if (!check_key<",\"phone\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_string(iter, end, out.phone);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, friend_element& out) noexcept {
		return parse_friend_element(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_result_data(const char* iter, const char* end, result_data& out) noexcept {
		if (!check_key<"{\"id\":">(iter))
			return nullptr;
		iter += 6;
		iter = parse_number(iter, end, out.id);
		if (!check_key<",\"avatar\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_string(iter, end, out.avatar);
		if (!check_key<",\"age\":">(iter))
			return nullptr;
		iter += 7;
		iter = parse_number(iter, end, out.age);
		if (!check_key<",\"admin\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_value(iter, end, out.admin);
		if (!check_key<",\"name\":">(iter))
			return nullptr;
		iter += 8;
		iter = parse_string(iter, end, out.name);
		if (!check_key<",\"company\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_string(iter, end, out.company);
		if (!check_key<",\"phone\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_string(iter, end, out.phone);
		if (!check_key<",\"email\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_string(iter, end, out.email);
		if (!check_key<",\"birthDate\":">(iter))
			return nullptr;
		iter += 13;
		iter = parse_string(iter, end, out.birthDate);
		if (!check_key<",\"friends\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_vector(iter, end, out.friends);
		if (!iter)
			return nullptr;
		if (!check_key<",\"field\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_string(iter, end, out.field);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, result_data& out) noexcept {
		return parse_result_data(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_random_message(const char* iter, const char* end, random_message& out) noexcept {
		if (!check_key<"{\"id\":">(iter))
			return nullptr;
		iter += 6;
		iter = parse_number(iter, end, out.id);
		if (!check_key<",\"jsonrpc\":">(iter))
			return nullptr;
		iter += 11;
		iter = parse_string(iter, end, out.jsonrpc);
		if (!check_key<",\"total\":">(iter))
			return nullptr;
		iter += 9;
		iter = parse_number(iter, end, out.total);
		if (!check_key<",\"result\":">(iter))
			return nullptr;
		iter += 10;
		iter = parse_vector(iter, end, out.result);
		if (!iter)
			return nullptr;
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value(const char* iter, const char* end, random_message& out) noexcept {
		return parse_random_message(iter, end, out);
	}

	

	JSONIFIER_INLINE const char* skip_ws(const char* iter) noexcept {
		while (jsonifier::internal::whitespaceTable[static_cast<uint8_t>(*iter)]) {
			++iter;
		}
		return iter;
	}

	JSONIFIER_INLINE const char* skip_key_sep(const char* iter) noexcept {
		iter = skip_ws(iter);
		++iter;// skip ':'
		return skip_ws(iter);
	}

	JSONIFIER_INLINE const char* skip_comma(const char* iter) noexcept {
		++iter;// skip ','
		return skip_ws(iter);
	}

	template<typename value_type> JSONIFIER_INLINE const char* parse_value_prettified(const char* iter, const char* end, value_type& out) noexcept;

	template<typename value_type> JSONIFIER_INLINE const char* parse_vector_prettified(const char* iter, const char* end, std::vector<value_type>& out) noexcept {
		++iter;// skip '['
		iter = skip_ws(iter);
		if (*iter == ']') {
			++iter;
			return iter;
		}
		const uint64_t size{ out.size() };
		uint64_t x{};
		for (; x < size; ++x) {
			if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
				iter = skip_null(iter);
			} else {
				iter = parse_value_prettified(iter, end, out[x]);
				if (!iter)
					return nullptr;
			}
			iter = skip_ws(iter);
			if (*iter == ',') {
				iter = skip_ws(++iter);
			} else {
				++iter;
				out.resize(x + 1);
				return iter;
			}
		}
		while (true) {
			if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
				iter = skip_null(iter);
				out.emplace_back(nullptr);
			} else {
				out.emplace_back();
				iter = parse_value_prettified(iter, end, out.back());
				if (!iter)
					return nullptr;
			}
			iter = skip_ws(iter);
			if (*iter == ',') {
				iter = skip_ws(++iter);
			} else {
				++iter;
				break;
			}
		}
		return iter;
	}

	JSONIFIER_INLINE const char* parse_friend_element_prettified(const char* iter, const char* end, friend_element& out) noexcept;
	JSONIFIER_INLINE const char* parse_result_data_prettified(const char* iter, const char* end, result_data& out) noexcept;

	template<typename value_type> JSONIFIER_INLINE const char* parse_value_prettified(const char* iter, const char* end, value_type& out) noexcept {
		if constexpr (std::is_same_v<value_type, std::nullptr_t>) {
			return skip_null(iter);
		} else if constexpr (std::is_same_v<value_type, bool>) {
			out = (*iter == 't');
			return iter + (*iter == 't' ? 4 : 5);
		} else if constexpr (std::is_same_v<value_type, std::string>) {
			return parse_string(iter, end, out);
		} else if constexpr (jsonifier::concepts::num_t<value_type>) {
			return parse_number(iter, end, out);
		} else if constexpr (jsonifier::concepts::vector_t<value_type>) {
			return parse_vector_prettified(iter, end, out);
		} else {
			static_assert(std::is_same_v<value_type, void>, "parse_value_prettified: unhandled type � add a specialization!");
		}
	}

	JSONIFIER_INLINE const char* parse_friend_element_prettified(const char* iter, const char* end, friend_element& out) noexcept {
		iter = skip_ws(++iter);// skip '{' + ws
		if (!check_key<"\"id\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 4);
		iter = parse_number(iter, end, out.id);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"name\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 6);
		iter = parse_string(iter, end, out.name);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"phone\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_string(iter, end, out.phone);
		iter = skip_ws(iter);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value_prettified(const char* iter, const char* end, friend_element& out) noexcept {
		return parse_friend_element_prettified(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_result_data_prettified(const char* iter, const char* end, result_data& out) noexcept {
		iter = skip_ws(++iter);// skip '{' + ws
		if (!check_key<"\"id\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 4);
		iter = parse_number(iter, end, out.id);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"avatar\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 8);
		iter = parse_string(iter, end, out.avatar);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"age\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 5);
		iter = parse_number(iter, end, out.age);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"admin\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_value_prettified(iter, end, out.admin);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"name\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 6);
		iter = parse_string(iter, end, out.name);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"company\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 9);
		iter = parse_string(iter, end, out.company);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"phone\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_string(iter, end, out.phone);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"email\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_string(iter, end, out.email);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"birthDate\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 11);
		iter = parse_string(iter, end, out.birthDate);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"friends\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 9);
		iter = parse_vector_prettified(iter, end, out.friends);
		if (!iter)
			return nullptr;
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"field\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_string(iter, end, out.field);
		iter = skip_ws(iter);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value_prettified(const char* iter, const char* end, result_data& out) noexcept {
		return parse_result_data_prettified(iter, end, out);
	}

	JSONIFIER_INLINE const char* parse_random_message_prettified(const char* iter, const char* end, random_message& out) noexcept {
		iter = skip_ws(++iter);// skip '{' + ws
		if (!check_key<"\"id\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 4);
		iter = parse_number(iter, end, out.id);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"jsonrpc\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 9);
		iter = parse_string(iter, end, out.jsonrpc);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"total\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 7);
		iter = parse_number(iter, end, out.total);
		iter = skip_comma(skip_ws(iter));
		if (!check_key<"\"result\"">(iter))
			return nullptr;
		iter = skip_key_sep(iter + 8);
		iter = parse_vector_prettified(iter, end, out.result);
		if (!iter)
			return nullptr;
		iter = skip_ws(iter);
		++iter;// skip '}'
		return iter;
	}

	template<> JSONIFIER_INLINE const char* parse_value_prettified(const char* iter, const char* end, random_message& out) noexcept {
		return parse_random_message_prettified(iter, end, out);
	}

	template<bnch_swt::string_literal test_name, typename value_type, bool prettified> static void run_benchmark(std::string& json_data) {
		value_type value_jsonifier{};
		value_type value_bespoke{};

		jsonifier::jsonifier_core<> parser{};
		parser.parseJson<jsonifier::parse_options{ .knownOrder = true, .minified = true }>(value_jsonifier, json_data);

		using benchmark_stage = bnch_swt::benchmark_stage<"bespoke-vs-jsonifier", bnch_swt::stage_config_data{ .max_iteration_count = 10000, .max_time_in_s = 10 }>;

		uint64_t bytes_bespoke{ [&] {
			value_type out{};
			if constexpr (prettified) {
				parse_value_prettified(json_data.data(), json_data.data() + json_data.size(), out);
			} else {
				parse_value(json_data.data(), json_data.data() + json_data.size(), out);
			}
			auto result = parser.serializeJson<jsonifier::serialize_options{}, value_type>(out);
			return result.size();
		}() };

		uint64_t bytes_jsonifier{ [&] {
			value_type out{};
			parser.parseJson<jsonifier::parse_options{ .knownOrder = true, .minified = !prettified }>(out, json_data);
			auto result = parser.serializeJson<jsonifier::serialize_options{}, value_type>(out);
			return result.size();
		}() };
		value_type out_01{};
		value_type out_02{};

		struct jsonifier_parse {
			static size_t impl(jsonifier::jsonifier_core<>& parser_new, std::string& json_data_in, uint64_t bytes) {
				value_type out;
				parser_new.parseJson<jsonifier::parse_options{ .knownOrder = true, .minified = !prettified }>(out, json_data_in);
				bnch_swt::do_not_optimize_away(out);
				return bytes;
			}
		};

		struct bespoke_parse {
			static size_t impl(std::string& json_data_in, uint64_t bytes) {
				value_type out;
				if constexpr (prettified) {
					parse_value_prettified(json_data_in.data(), json_data_in.data() + json_data_in.size(), out);
				} else {
					parse_value(json_data_in.data(), json_data_in.data() + json_data_in.size(), out);
				}
				bnch_swt::do_not_optimize_away(out);
				return bytes;
			}
		};

		benchmark_stage::template run_benchmark<"CITM", "jsonifier", jsonifier_parse>(parser, json_data, bytes_jsonifier);
		benchmark_stage::template run_benchmark<"CITM", "bespoke", bespoke_parse>(json_data, bytes_bespoke);
		std::string result_bespoke	 = static_cast<std::string>(parser.serializeJson<jsonifier::serialize_options{ .prettify = true }, value_type>(out_01));
		std::string result_jsonifier = static_cast<std::string>(parser.serializeJson<jsonifier::serialize_options{ .prettify = true }, value_type>(out_02));
		bnch_swt::file_handle::save_file(result_bespoke, "bespoke_out.json");
		bnch_swt::file_handle::save_file(result_jsonifier, "jsonifier_out.json");
		auto result = benchmark_stage::get_test_results("CITM");
		result.print();
	}

	enum class test_types {
		parse_serialize,
		minify,
		prettify,
		validate,
	};

	template<test_types test_type, rt_ut::string_literal testName, typename value_type, bool prettified, bool partial, bool knownOder> void parsing_tests(jsonifier::jsonifier_core<>& parser) {
		auto dataToParse = file_handle::get(basePath.operator std::string() + "/json/" + testName.operator std::string() + ".json");
		value_type value;
		rt_ut::unit_test<testName, true>::run([&]() {
			if constexpr (test_type == test_types::parse_serialize) {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = knownOder, .minified = !prettified }>(value, dataToParse);
				parser.template serializeJson<jsonifier::serialize_options{ .prettify = prettified }, value_type>(value);
			} else if constexpr (test_type == test_types::minify) {
				parser.minifyJson(value, dataToParse);
			} else if constexpr (test_type == test_types::prettify) {
				parser.prettifyJson(value, dataToParse);
			} else if constexpr (test_type == test_types::validate) {
				parser.validateJson(dataToParse);
			}
			if (auto& errors = parser.getErrors(); errors.size() > 0) {
				for (auto& error_value: errors) {
					std::cout << "Jsonifier Error: " << error_value << std::endl;
				}
				return false;
			} else {
				return true;
			}
		});
	};

	template<bool knownOrder> void parsing_tests_impl() {
		jsonifier::jsonifier_core<> parser{};
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Partial Test (Minified)", abc_partial_test<abc_partial_test_struct>, false, true, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Partial Test (Prettified)", abc_partial_test<abc_partial_test_struct>, true, true, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Minified)", abc_test<abc_test_struct>, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Prettified)", abc_test<abc_test_struct>, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Minified)", apache_builds_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Prettified)", apache_builds_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Minified)", discord_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Prettified)", discord_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Minified)", github_events_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Prettified)", github_events_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Minified)", google_maps_response_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Prettified)", google_maps_response_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Canada Test (Minified)", canada_message, false, false, knownOrder>(parser); 
		parsing_tests<test_types::parse_serialize, "Canada Test (Prettified)", canada_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Minified)", citm_catalog_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Prettified)", citm_catalog_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Minified)", instruments_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Prettified)", instruments_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Minified)", marine_ik, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Prettified)", marine_ik, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Minified)", mesh_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Prettified)", mesh_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Minified)", random_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Prettified)", random_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Minified)", twitter_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Prettified)", twitter_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Minified)", twitter_partial_message, false, true, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Prettified)", twitter_partial_message, true, true, knownOrder>(parser);
		parsing_tests<test_types::minify, "Minify Test", std::string, false, false, knownOrder>(parser);
		parsing_tests<test_types::prettify, "Prettify Test", std::string, false, false, knownOrder>(parser);
		parsing_tests<test_types::validate, "Validate Test", std::string, false, false, knownOrder>(parser);
	}

	void parsing_tests() {
		parsing_tests_impl<true>();
		parsing_tests_impl<false>();
	}

}
