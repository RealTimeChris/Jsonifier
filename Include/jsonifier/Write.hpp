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
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>

namespace jsonifier_internal {

	constexpr std::array<json_structural_type, 256> asciiClassesMap{ [] {
		std::array<json_structural_type, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = json_structural_type::Unset;
		}
		returnValues['"'] = json_structural_type::String;
		returnValues['t'] = json_structural_type::Bool;
		returnValues['f'] = json_structural_type::Bool;
		returnValues['n'] = json_structural_type::Null;
		returnValues['0'] = json_structural_type::Number;
		returnValues['1'] = json_structural_type::Number;
		returnValues['2'] = json_structural_type::Number;
		returnValues['3'] = json_structural_type::Number;
		returnValues['4'] = json_structural_type::Number;
		returnValues['5'] = json_structural_type::Number;
		returnValues['6'] = json_structural_type::Number;
		returnValues['7'] = json_structural_type::Number;
		returnValues['8'] = json_structural_type::Number;
		returnValues['9'] = json_structural_type::Number;
		returnValues['-'] = json_structural_type::Number;
		returnValues['['] = json_structural_type::Array_Start;
		returnValues[']'] = json_structural_type::Array_End;
		returnValues['{'] = json_structural_type::Object_Start;
		returnValues['}'] = json_structural_type::Object_End;
		returnValues[':'] = json_structural_type::Colon;
		returnValues[','] = json_structural_type::Comma;
		return returnValues;
	}() };

	template<const auto... optionsNew> struct writer {
		static constexpr auto options{ [] {
			if constexpr (sizeof...(optionsNew) > 0) {
				std::tuple optionsNewer{ optionsNew... };
				return std::get<0>(optionsNewer);
			} else {
				return bool{};
			}
		}() };

		template<typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacter(const char c, buffer_type& buffer, index_type& index) noexcept {
			buffer[index] = c;
			++index;
		}

		template<char c, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacter(buffer_type& buffer, index_type& index) noexcept {
			buffer[index] = c;
			++index;
		}

		template<string_literal str, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, index_type& index) noexcept {
			static constexpr auto s = str.view();
			static constexpr auto n = s.size();
			std::copy_n(s.data(), n, buffer.data() + index);
			index += n;
		}

		template<typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(const jsonifier::string_view str, buffer_type& buffer, index_type& index) noexcept {
			const auto n = str.size();
			std::copy_n(str.data(), n, buffer.data() + index);
			index += n;
		}

		template<char c, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(uint64_t n, buffer_type& buffer, index_type& index) noexcept {
			std::fill_n(buffer.data() + index, n, c);
			index += n;
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeNewLine(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			auto indentTotal			= serializePair.indent * options.indentSize;
			buffer[serializePair.index] = '\n';
			++serializePair.index;
			std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
			serializePair.index += indentTotal;
		}

		template<const jsonifier::string_view& str, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, index_type& index) noexcept {
			static constexpr auto s = str;
			static constexpr auto n = s.size();
			std::copy_n(str.data(), n, buffer.data() + index);
			index += n;
		}

		template<typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, const char* string, uint64_t size, index_type& index) noexcept {
			const auto n = size;
			std::copy_n(string, n, buffer.data() + index);
			index += n;
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeEntrySeparator(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify) {
				static constexpr char s[]{ ",\n" };
				static constexpr auto n = std::size(s) - 1;
				auto indentTotal		= serializePair.indent * options.indentSize;
				std::copy_n(s, n, buffer.data() + serializePair.index);
				serializePair.index += n;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += serializePair.indent * options.indentSize;
			} else {
				buffer[serializePair.index] = ',';
				++serializePair.index;
			}
		}

		template<uint64_t objectSize, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify && objectSize > 0) {
				++serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '{';
				++serializePair.index;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			} else {
				buffer[serializePair.index] = '{';
				++serializePair.index;
			}
		}

		template<uint64_t objectSize, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify && objectSize > 0) {
				--serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			}
			buffer[serializePair.index] = '}';
			++serializePair.index;
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify) {
				++serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify) {
				--serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeArrayEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify) {
				++serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeArrayExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			if constexpr (options.prettify) {
				--serializePair.indent;
				auto indentTotal			= serializePair.indent * options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.indentChar);
				serializePair.index += indentTotal;
			}
		}
	};

}// namespace jsonifier_internal