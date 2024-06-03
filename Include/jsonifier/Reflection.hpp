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
/// NOTE: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/StringView.hpp>

namespace jsonifier {

	template<typename char_type> class string_view_base;

}

namespace jsonifier_internal {

	using string_view = jsonifier::string_view;

	template<uint64_t sizeVal> struct uint_string_literal {
		static constexpr uint64_t length{ sizeVal > 0 ? sizeVal - 1 : 0 };

		constexpr uint_string_literal() noexcept = default;

		constexpr uint_string_literal(const char (&str)[sizeVal]) {
			std::copy(str, str + sizeVal, values);
		}

		constexpr uint64_t size() const {
			return length;
		}

		constexpr operator jsonifier::string_view() const {
			return { values, length };
		}

		constexpr jsonifier::string_view sv() const {
			return { values, length };
		}

		char values[sizeVal]{};
	};

	/**
	 * @def JSONIFIER_PRETTY_FUNCTION
	 * @brief Define JSONIFIER_PRETTY_FUNCTION based on compiler type.
	 */

#if defined(JSONIFIER_CLANG) || defined(JSONIFIER_GNUCXX)
	#define JSONIFIER_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define JSONIFIER_PRETTY_FUNCTION __FUNCSIG__
#endif

	/**
	 * @brief External template variable declaration.
	 *
	 * Declare an external template variable to be defined elsewhere.
	 *
	 * @tparam value_type The type of the external template variable.
	 */
	template<typename value_type> extern const value_type external;

	/**
	 * @brief Struct to remove member pointers.
	 *
	 * Define a struct to remove member pointers from the given type.
	 *
	 * @tparam value_type The type from which to remove member pointers.
	 */
	template<typename value_type> struct remove_member_pointer {
		using type = value_type; /**< The type without member pointers. */
	};

	template<typename member_type, typename value_type> struct remove_member_pointer<value_type member_type::*> {
		using type = member_type; /**< The type without member pointers. */
	};

	template<auto valueNew> struct make_static {
		static constexpr auto value{ valueNew };
	};

	/**
	 * @def pretty_function_tail
	 * @brief Define pretty_function_tail based on compiler type.
	 */

#if defined(JSONIFIER_CLANG)
	constexpr auto pretty_function_tail = "]";
#else
	constexpr auto pretty_function_tail = ";";
#endif

	/**
	 * @brief Get the name of a member pointer in MSVC.
	 *
	 * Function to extract the name of a member pointer when using MSVC compiler.
	 *
	 * @tparam value_type The type of the member pointer.
	 * @tparam p The member pointer.
	 * @return The name of the member pointer.
	 */
	template<typename value_type, auto p> constexpr decltype(auto) getNameMSVC() noexcept {
		constexpr std::string_view str = JSONIFIER_PRETTY_FUNCTION;
		constexpr auto strNew		   = str.substr(str.find("->") + 2);
		constexpr auto strNewer		   = strNew.substr(0, strNew.find(">"));
		std::array<char, strNewer.size()> returnValues{};
		std::copy(strNewer.data(), strNewer.data() + strNewer.size(), returnValues.data());
		return returnValues;
	}

	template<typename value_type, auto p> constexpr decltype(auto) getNameMSVCFinal() noexcept {
		constexpr auto& stringNew = make_static<getNameMSVC<value_type, p>()>::value;
		return jsonifier::string_view{ stringNew.data(), stringNew.size() };
	}

	template<auto p> constexpr decltype(auto) getNameUint() noexcept {
		constexpr jsonifier::string_view str = JSONIFIER_PRETTY_FUNCTION;
		constexpr auto str2					 = str.substr(str.find("&") + 1);
		constexpr auto str3					 = str2.substr(0, str2.find(pretty_function_tail));
		constexpr auto str4					 = str3.substr(str3.rfind("::") + 2);
		std::array<char, str4.size()> returnValues{};
		std::copy(str4.data(), str4.data() + str4.size(), returnValues.data());
		return returnValues;
	}

	template<auto p> constexpr decltype(auto) getNameUintFinal() noexcept {
		constexpr auto& stringNew = make_static<getNameUint<p>()>::value;
		return jsonifier::string_view{ stringNew.data(), stringNew.size() };
	}

	/**
	 * @brief Get the name of a member pointer.
	 *
	 * Function to extract the name of a member pointer.
	 *
	 * @tparam p The member pointer.
	 * @return The name of the member pointer.
	 */
	template<auto p>
	//requires(std::is_member_object_pointer_v<decltype(p)>)
	constexpr decltype(auto) getName() noexcept {
#if defined(JSONIFIER_CLANG) || defined(JSONIFIER_GNUCXX)
		return getNameUintFinal<p>();
#else
		using value_type   = remove_member_pointer<jsonifier::concepts::unwrap_t<decltype(p)>>::type;
		constexpr auto ptr = p;
		return getNameMSVCFinal<value_type, &(external<value_type>.*ptr)>();
#endif
	}

	/**
	 * @brief Get the names of multiple member pointers.
	 *
	 * Function to extract the names of multiple member pointers.
	 *
	 * @tparam args Member pointers.
	 * @return An array of member pointer names.
	 */
	template<auto... args> constexpr decltype(auto) getNames() {
		return std::array<jsonifier::string_view, sizeof...(args)>{ getName<args>()... };
	}

	/**
	 * @brief Helper function to generate an interleaved tuple of member names and values.
	 *
	 * Helper function to generate an interleaved tuple of member names and values.
	 *
	 * @tparam tuple_types Types of tuple elements.
	 * @tparam Indices Indices of tuple elements.
	 * @param tuple The input tuple.
	 * @param views Array of member names.
	 * @return Interleaved tuple of member names and values.
	 */
	template<typename... tuple_types, size_t... Indices> constexpr decltype(auto) generateInterleavedTupleHelper(const std::tuple<tuple_types...>& tuple,
		const std::array<jsonifier::string_view, sizeof...(Indices)>& views, std::index_sequence<Indices...>) {
		return std::make_tuple(std::make_tuple(views[Indices], std::get<Indices>(tuple))...);
	}

	/**
	 * @brief Generate an interleaved tuple of member names and values.
	 *
	 * Function to generate an interleaved tuple of member names and values.
	 *
	 * @tparam tuple_types Types of tuple elements.
	 * @param tuple The input tuple.
	 * @param views Array of member names.
	 * @return Interleaved tuple of member names and values.
	 */
	template<typename... tuple_types>
	constexpr decltype(auto) generateInterleavedTuple(const std::tuple<tuple_types...>& tuple, const std::array<jsonifier::string_view, sizeof...(tuple_types)>& views) {
		return generateInterleavedTupleHelper(tuple, views, std::index_sequence_for<tuple_types...>{});
	}

}