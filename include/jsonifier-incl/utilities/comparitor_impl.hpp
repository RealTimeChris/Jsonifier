/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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
/// Feb 20, 2023
#pragma once

#include <jsonifier-incl/utilities/number_utils.hpp>
#include <jsonifier-incl/utilities/string_utils.hpp>
#include <jsonifier-incl/containers/tuple.hpp>
#include <jsonifier-incl/utilities/comparitor.hpp>

namespace jsonifier::internal {

	template<typename comparator_entity_type> struct json_entity_compare : public comparator_entity_type {
		constexpr json_entity_compare() = default;

		template<typename value_type> inline static bool processIndex(const value_type& lhs, const value_type& rhs) {
			return json_comparator::impl(lhs.*comparator_entity_type::memberPtr,
				rhs.*comparator_entity_type::memberPtr);
		}
	};

	template<typename... bases> struct compare_map : public bases... {
		template<typename comparator_entity_type, typename... arg_types> inline static bool iterateValuesImpl(arg_types&&... args) {
			return comparator_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> inline static constexpr bool iterateValues(arg_types&&... args) {
			return (iterateValuesImpl<bases>(internal::forward<arg_types>(args)...) && ...);
		}
	};

	template<typename value_type, typename index_sequence, typename... value_types> struct get_compare_base;

	template<typename value_type, uint64_t... index> struct get_compare_base<value_type, index_sequence<index...>> {
		using type = compare_map<json_entity_compare<remove_cvref_t<decltype(get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue))>>...>;
	};

	template<typename value_type> using compare_base_t = typename get_compare_base<value_type, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<concepts::jsonifier_object_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			static constexpr auto membercount{ core_tuple_size<value_type> };
			if constexpr (membercount > 0) {
				return compare_base_t<remove_cvref_t<value_type>>::iterateValues(lhs, rhs);
			} else {
				return true;
			}
		}
	};

	template<concepts::map_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (lhs.size() != rhs.size()) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			const auto end = lhs.end();
			for (auto iter = lhs.begin(); iter != end; ++iter) {
				const auto found = rhs.find(iter->first);
				if (found == rhs.end()) {
					std::stringstream stream{};
					stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
					stream << "On Line: " << std::source_location::current().line() << std::endl;
					throw std::runtime_error{ stream.str() };
				}
				if (!json_comparator::impl(iter->second, found->second)) {
					std::stringstream stream{};
					stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
					stream << "On Line: " << std::source_location::current().line() << std::endl;
					throw std::runtime_error{ stream.str() };
				}
			}
			return true;
		}
	};

	template<concepts::tuple_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			static constexpr auto size = std::tuple_size_v<remove_cvref_t<value_type>>;
			return compareElements<0, size>(lhs, rhs);
		}

		template<uint64_t currentIndex, uint64_t newSize, typename value_type_new> inline static bool compareElements(value_type_new&& lhs, value_type_new&& rhs) {
			if constexpr (currentIndex < newSize) {
				auto lhsSub = get<currentIndex>(lhs);
				auto rhsSub = get<currentIndex>(rhs);
				if (!json_comparator::impl(lhsSub, rhsSub)) {
					std::stringstream stream{};
					stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
					stream << "On Line: " << std::source_location::current().line() << std::endl;
					throw std::runtime_error{ stream.str() };
				}
				return compareElements<currentIndex + 1, newSize>(lhs, rhs);
			} else {
				return true;
			}
		}
	};

	template<concepts::vector_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			const auto newSize = lhs.size();
			if (newSize != rhs.size()) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			auto lhsIter = getBeginIterVec(lhs);
			auto rhsIter = getBeginIterVec(rhs);
			for (int64_t index{ 0 }; index != static_cast<int64_t>(newSize); ++index) {
				auto& lhv = lhsIter[index];
				auto& rhv = rhsIter[index];
				if (!json_comparator::impl(lhv, rhv)) {
					std::stringstream stream{};
					stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
					stream << "On Line: " << std::source_location::current().line() << std::endl;
					throw std::runtime_error{ stream.str() };
				}
			}
			return true;
		}
	};

	template<concepts::raw_array_t value_type> struct json_comparator_impl<value_type> {
		template<template<typename, uint64_t> typename value_type_new, typename value_type_internal, uint64_t size>
		inline static bool impl(const value_type_new<value_type_internal, size>& lhs, const value_type_new<value_type_internal, size>& rhs) {
			auto lhsIter = getBeginIterVec(lhs);
			auto rhsIter = getBeginIterVec(rhs);
			for (int64_t index{ 0 }; index != static_cast<int64_t>(size); ++index) {
				if (!json_comparator::impl(lhsIter[index], rhsIter[index])) {
					std::stringstream stream{};
					stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
					stream << "On Line: " << std::source_location::current().line() << std::endl;
					throw std::runtime_error{ stream.str() };
				}
			}
			return true;
		}
	};

	template<concepts::string_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return lhs == rhs;
		}
	};

	template<concepts::char_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return lhs == rhs;
		}
	};

	template<concepts::enum_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs);
		}
	};

	template<concepts::num_t value_type> struct json_comparator_epsilon {
		static constexpr value_type relativeEpsilon{ static_cast<value_type>(1e-9) };
		static constexpr value_type absoluteEpsilon{ static_cast<value_type>(1e-12) };
	};

	template<concepts::num_t value_type, typename = void> struct json_comparator_num;

	template<concepts::float_t value_type> struct json_comparator_num<value_type> {
		inline static bool impl(value_type lhs, value_type rhs) {
			const value_type diff	 = lhs > rhs ? lhs - rhs : rhs - lhs;
			const value_type lhsMag	 = lhs < value_type{} ? -lhs : lhs;
			const value_type rhsMag	 = rhs < value_type{} ? -rhs : rhs;
			const value_type largest = lhsMag > rhsMag ? lhsMag : rhsMag;
			const value_type relEps	 = json_comparator_epsilon<value_type>::relativeEpsilon;
			const value_type absEps	 = json_comparator_epsilon<value_type>::absoluteEpsilon;
			const value_type bound	 = relEps * largest > absEps ? relEps * largest : absEps;
			return diff <= bound;
		}
	};

	template<concepts::integer_t value_type> struct json_comparator_num<value_type> {
		inline static bool impl(value_type lhs, value_type rhs) {
			return lhs == rhs;
		}
	};

	template<concepts::num_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return json_comparator_num<remove_cvref_t<value_type_new>>::impl(lhs, rhs);
		}
	};

	template<concepts::bool_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return lhs == rhs;
		}
	};

	template<concepts::always_null_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&&, value_type_new&&) {
			return true;
		}
	};

	template<concepts::pointer_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			if (!lhs) {
				return true;
			}
			return comparator<remove_cvref_t<decltype(*lhs)>>::impl(*lhs, *rhs);
		}
	};

	template<concepts::unique_ptr_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			if (!lhs) {
				return true;
			}
			return comparator<remove_cvref_t<decltype(*lhs)>>::impl(*lhs, *rhs);
		}
	};

	template<concepts::shared_ptr_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			if (!lhs) {
				return true;
			}
			return comparator<remove_cvref_t<decltype(*lhs)>>::impl(*lhs, *rhs);
		}
	};

	template<concepts::optional_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			if (!lhs) {
				return true;
			}
			return json_comparator::impl(lhs.value(), rhs.value());
		}
	};

	template<concepts::variant_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			if (lhs.index() != rhs.index()) {
				std::stringstream stream{};
				stream << "Failed in File: " << std::source_location::current().file_name() << std::endl;
				stream << "On Line: " << std::source_location::current().line() << std::endl;
				throw std::runtime_error{ stream.str() };
			}
			static constexpr auto lambda = [](auto&& lhsNewer, auto&& rhsNewer) {
				return json_comparator::impl(lhsNewer, rhsNewer);
			};
			return visit<lambda>(lhs, rhs);
		}
	};

	template<concepts::raw_json_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) {
			return lhs.rawJson() == rhs.rawJson();
		}
	};

	template<concepts::skip_t value_type> struct json_comparator_impl<value_type> {
		template<typename value_type_new> inline static bool impl(value_type_new&&, value_type_new&&) {
			return true;
		}
	};
}
