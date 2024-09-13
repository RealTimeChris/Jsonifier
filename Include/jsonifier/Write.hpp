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

	template<const auto&... optionsNew> struct writer {
		static constexpr auto options{ [] {
			if constexpr (sizeof...(optionsNew) > 0) {
				std::tuple optionsNewer{ optionsNew... };
				return std::get<0>(optionsNewer);
			} else {
				return bool{};
			}
		}() };

		template<bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacter(const char c, buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (checked) {
				if (index == bufferSize) [[unlikely]] {
					buffer.resize(bufferSize == 0 ? 128 : bufferSize * 2);
				}
			}
			buffer[index] = c;
			++index;
		}

		template<char c, bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacter(buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (checked) {
				if (index == bufferSize) [[unlikely]] {
					buffer.resize(bufferSize == 0 ? 128 : bufferSize * 2);
				}
			}
			buffer[index] = c;
			++index;
		}

		template<string_literal str, bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize	= buffer.size();
			static constexpr auto s = str.view();
			static constexpr auto n = s.size();

			if constexpr (checked) {
				if (index + n > bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > index + n ? bufferSize * 2 : index + n);
				}
			}
			std::memcpy(buffer.data() + index, s.data(), n);
			index += n;
		}

		template<bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(const jsonifier::string_view str, buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize = buffer.size();
			const auto n		  = str.size();
			if constexpr (checked) {
				if (index + n > bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > index + n ? bufferSize * 2 : index + n);
				}
			}
			std::memcpy(buffer.data() + index, str.data(), n);
			index += n;
		}

		template<char c, bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(uint64_t n, buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (checked) {
				if (index + n > bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > index + n ? bufferSize * 2 : index + n);
				}
			}
			std::memset(buffer.data() + index, c, n);
			index += n;
		}

		template<char c, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacter(uint64_t n, buffer_type& buffer, index_type& index) noexcept {
			std::memset(buffer.data() + index, c, n);
			index += n;
		}

		template<bool checked = true, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeNewLine(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize	  = buffer.size();
			auto indent				  = serializePair.indent;
			constexpr auto indentSize = options.optionsReal.indentSize;
			auto indentTotal		  = indent * indentSize;
			auto n					  = 3 + indentTotal;
			if constexpr (checked) {
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
			}
			buffer[serializePair.index] = '\n';
			++serializePair.index;
			std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
			serializePair.index += indentTotal;
		}

		template<const jsonifier::string_view& str, bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, index_type& index) noexcept {
			const auto bufferSize	= buffer.size();
			static constexpr auto s = str;
			static constexpr auto n = s.size();

			if constexpr (checked) {
				if (index + n > bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > index + n ? bufferSize * 2 : index + n);
				}
			}
			std::memcpy(buffer.data() + index, s.data(), n);
			index += n;
		}

		template<bool checked = true, typename buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_ALWAYS_INLINE static void writeCharacters(buffer_type& buffer, const char* string, uint64_t size, index_type& index) noexcept {
			const auto bufferSize = buffer.size();
			const auto n		  = size;
			if constexpr (checked) {
				if (index + n > bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > index + n ? bufferSize * 2 : index + n);
				}
			}
			std::memcpy(buffer.data() + index, string, n);
			index += n;
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeEntrySeparator(buffer_type&& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify) {
				auto k = serializePair.index + serializePair.indent + 256;
				if (k > bufferSize) [[unlikely]] {
					buffer.resize(max(bufferSize * 2, k));
				}
				static constexpr char s[]{ ",\n" };
				static constexpr auto n	  = std::size(s) - 1;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				std::copy_n(s, n, buffer.data() + serializePair.index);
				serializePair.index += n;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indent * indentSize;
			} else {
				buffer[serializePair.index] = ',';
				++serializePair.index;
			}
		}

		template<uint64_t objectSize, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify && objectSize > 0) {
				++serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '{';
				++serializePair.index;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
				buffer[serializePair.index] = '{';
				++serializePair.index;
			}
		}

		template<uint64_t objectSize, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify && objectSize > 0) {
				--serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
			}
			buffer[serializePair.index] = '}';
			++serializePair.index;
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify) {
				++serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '{';
				++serializePair.index;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
				buffer[serializePair.index] = '{';
				++serializePair.index;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeObjectExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify) {
				--serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal + 1;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
				buffer[serializePair.index] = '}';
				++serializePair.index;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
				buffer[serializePair.index] = '}';
				++serializePair.index;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeArrayEntry(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify) {
				++serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '[';
				++serializePair.index;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
				buffer[serializePair.index] = '[';
				++serializePair.index;
			}
		}

		template<jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void writeArrayExit(buffer_type& buffer, serialize_pair_t& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			if constexpr (options.optionsReal.prettify) {
				--serializePair.indent;
				auto indent				  = serializePair.indent;
				constexpr auto indentSize = options.optionsReal.indentSize;
				auto indentTotal		  = indent * indentSize;
				auto n					  = 3 + indentTotal + 1;
				if (serializePair.index + n >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + n ? bufferSize * 2 : serializePair.index + n);
				}
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::fill_n(buffer.data() + serializePair.index, indentTotal, options.optionsReal.indentChar);
				serializePair.index += indentTotal;
				buffer[serializePair.index] = ']';
				++serializePair.index;
			} else {
				if (serializePair.index + 1 >= bufferSize) [[unlikely]] {
					buffer.resize(bufferSize * 2 > serializePair.index + 1 ? bufferSize * 2 : serializePair.index + 1);
				}
				buffer[serializePair.index] = ']';
				++serializePair.index;
			}
		}
	};

}// namespace jsonifier_internal