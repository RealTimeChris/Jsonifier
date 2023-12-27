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

namespace jsonifier {

	template<typename char_type> class string_view_base;

}

namespace jsonifier_internal {

	using string_view = jsonifier::string_view_base<char>;

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

	/**
	 * @brief Get the name of a member pointer in MSVC.
	 *
	 * Function to extract the name of a member pointer when using MSVC compiler.
	 *
	 * @tparam value_type The type of the member pointer.
	 * @tparam p The member pointer.
	 * @return The name of the member pointer.
	 */
	template<typename value_type, const auto p> constexpr decltype(auto) getNameMSVC() noexcept {
		jsonifier::string_view str = JSONIFIER_PRETTY_FUNCTION; /**< Pretty function signature. */

		size_t i = str.find("->");
		str		 = str.substr(i + 2);
		i		 = str.find(">");
		str		 = str.substr(0, i);
		return str;
	}

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
	 * @brief Get the name of a member pointer.
	 *
	 * Function to extract the name of a member pointer.
	 *
	 * @tparam p The member pointer.
	 * @return The name of the member pointer.
	 */
	template<const auto p>
	//requires(std::is_member_object_pointer_v<decltype(p)>)
	constexpr decltype(auto) getName() noexcept {
#if defined(JSONIFIER_CLANG) || defined(JSONIFIER_GNUCXX)
		jsonifier::string_view str = JSONIFIER_PRETTY_FUNCTION;
		str						   = str.substr(str.find("&") + 1);
		str						   = str.substr(0, str.find(pretty_function_tail));
		return str.substr(str.rfind("::") + 2);
#else
		using value_type   = remove_member_pointer<std::decay_t<decltype(p)>>::type;
		constexpr auto ptr = p;
		return getNameMSVC<value_type, &(external<value_type>.*ptr)>();
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
	template<typename... tuple_types, size_t... Indices> constexpr decltype(auto) generate_interleaved_tuple_helper(const std::tuple<tuple_types...>& tuple,
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
	constexpr decltype(auto) generate_interleaved_tuple(const std::tuple<tuple_types...>& tuple, const std::array<jsonifier::string_view, sizeof...(tuple_types)>& views) {
		return generate_interleaved_tuple_helper(tuple, views, std::index_sequence_for<tuple_types...>{});
	}

}

namespace jsonifier_internal {

	template<typename tuple_t> struct group_builder {
	  public:
		static constexpr auto impl(tuple_t&& object) {
			return makeGroupsImpl<starts, sizes_data>(std::forward<tuple_t>(object), std::make_index_sequence<nGroups>{});
		}

	  protected:
		template<size_t... indices, typename array_type> static constexpr auto shrinkIndexArray(const array_type& valuesNew01, std::index_sequence<indices...>) {
			std::array<size_t, sizeof...(indices)> res{};
			((res[indices] = valuesNew01[indices]), ...);
			return res;
		}

		static constexpr auto filter() {
			constexpr auto n = std::tuple_size_v<tuple_t>;
			std::array<size_t, n> indices{};
			size_t x		= 0;
			auto filterImpl = [&](auto index, auto&& filterImpl) {
				using value_type = jsonifier::concepts::unwrap_t<std::tuple_element_t<index, tuple_t>>;
				if constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
					indices[x++] = index - 1;
				}
				if constexpr (index < n - 1) {
					filterImpl(std::integral_constant<size_t, index + 1>{}, filterImpl);
				}
			};
			filterImpl(std::integral_constant<size_t, 0>{}, filterImpl);
			return std::make_pair(indices, x);
		}

		template<size_t... indicesNew, typename array_type01, typename array_type02>
		static constexpr void groupSizesImpl(array_type01& diffs, array_type02& indices, std::index_sequence<indicesNew...>) {
			((diffs[indicesNew] = indices[indicesNew + 1] - indices[indicesNew]), ...);
		}

		template<size_t nGroups> static constexpr auto groupSizes(const std::array<size_t, nGroups>& indices, size_t nTotal) {
			std::array<size_t, nGroups> diffs;
			groupSizesImpl(diffs, indices, std::make_index_sequence<nGroups - 1>{});
			diffs[nGroups - 1] = nTotal - indices[nGroups - 1];
			return diffs;
		}

		template<size_t value, typename value_type> static constexpr auto getElemImpl(const value_type& t) {
			if constexpr (value == 0 || std::convertible_to<value_type, jsonifier::string_view>) {
				return jsonifier::string_view(std::get<value>(t));
			} else {
				return std::get<value>(t);
			}
		}

		template<size_t start, typename value_type> static constexpr auto getElem(const value_type& t) {
			return getElemImpl<start>(t);
		}

		template<size_t start, size_t... indices> static constexpr auto makeGroup(const tuple_t& t, std::index_sequence<indices...>) {
			return std::make_tuple(getElem<start + indices>(t)...);
		}

		template<auto& GroupStartArr, auto& GroupSizeArr, size_t... GroupNumber> static constexpr auto makeGroupsImpl(tuple_t&& object, std::index_sequence<GroupNumber...>) {
			auto newTuple{ std::forward<tuple_t>(object) };
			return std::make_tuple(
				makeGroup<GroupStartArr[jsonifier::concepts::tag<GroupNumber>()]>(newTuple, std::make_index_sequence<GroupSizeArr[jsonifier::concepts::tag<GroupNumber>()]>{})...);
		}

		static constexpr auto makeGroupsImpl() {
			constexpr auto nNew			 = std::tuple_size_v<tuple_t>;
			constexpr auto filtered		 = filter();
			constexpr auto startsNew	 = shrinkIndexArray(filtered.first, std::make_index_sequence<filtered.second>{});
			constexpr auto sizes_dataNew = groupSizes(startsNew, nNew);
			return std::make_tuple(startsNew, sizes_dataNew);
		}

		static constexpr auto h			 = makeGroupsImpl();
		static constexpr auto starts	 = get<0>(h);
		static constexpr auto sizes_data = get<1>(h);
		static constexpr auto nGroups	 = starts.size();
	};

	template<typename value_type> constexpr decltype(auto) convSv(value_type&& value) noexcept {
		if constexpr (std::is_convertible_v<value_type, jsonifier::string_view>) {
			return jsonifier::string_view{ value };
		} else {
			return std::forward<value_type>(value);
		}
	}

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE decltype(auto) getMember(value_type01&& value, value_type02& member_ptr) {
		if constexpr (std::is_member_object_pointer_v<value_type02>) {
			return value.*member_ptr;
		} else if constexpr (std::is_pointer_v<value_type02>) {
			return *member_ptr;
		} else {
			return member_ptr;
		}
	}

}

namespace jsonifier {

	template<typename... value_types> constexpr decltype(auto) createValue(value_types&&... args) {
		if constexpr (sizeof...(value_types) > 0 && sizeof...(value_types) % 2 == 0) {
			auto newTuple	 = std::make_tuple(jsonifier_internal::convSv(std::forward<value_types>(args))...);
			using tuple_type = jsonifier::concepts::unwrap_t<decltype(newTuple)>;
			return value{ jsonifier_internal::group_builder<tuple_type>::impl(std::move(newTuple)) };
		} else if constexpr (sizeof...(value_types) == 1) {
			return scalar_value{ std::make_tuple(std::forward<value_types>(args)...) };
		} else {
			return value{ jsonifier::concepts::empty{} };
		}
	}

	/*
	* Function to create a reflected value from member pointers
	*/
	template<const auto... values> constexpr decltype(auto) createValue() {
		// Check if there are member pointers specified
		if constexpr (sizeof...(values) > 0) {
			// Create a tuple with the specified member pointers
			constexpr auto newTuple = std::make_tuple(values...);

			// Get the names of the specified member pointers
			constexpr auto memberNames = jsonifier_internal::getNames<values...>();

			// Extract the unwrapped tuple type
			using tuple_type = jsonifier::concepts::unwrap_t<decltype(newTuple)>;

			// Generate an interleaved tuple of member names and values
			return jsonifier::value{ jsonifier_internal::generate_interleaved_tuple(newTuple, memberNames) };
		} else {
			// If no member pointers are specified, create a reflected value with an empty placeholder
			return jsonifier::value{ jsonifier::concepts::empty{} };
		}
	}

}