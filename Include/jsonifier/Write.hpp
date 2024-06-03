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

	constexpr jsonifier::string_view falseString{ "false" };
	constexpr jsonifier::string_view trueString{ "true" };
	constexpr jsonifier::string_view nullString{ "null" };

	template<jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacter(const char c, buffer_type& buffer, uint64_t& index) noexcept {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() == 0 ? 128 : buffer.size() * 2);
		}
		buffer[index] = c;
		++index;
	}

	template<char c, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacter(buffer_type& buffer, uint64_t& index) noexcept {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() == 0 ? 128 : buffer.size() * 2);
		}
		buffer[index] = c;
		++index;
	}

	template<string_literal str, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacters(buffer_type& buffer, uint64_t& index) noexcept {
		static constexpr auto s = str.operator jsonifier::string_view();
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<char c, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacterUnchecked(buffer_type& buffer, uint64_t& index) noexcept {
		buffer[index] = c;
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacterUnchecked(const char c, buffer_type& buffer, uint64_t& index) noexcept {
		buffer[index] = c;
		++index;
	}

	template<string_literal str, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharactersUnchecked(buffer_type& buffer, uint64_t& index) noexcept {
		static constexpr auto s = str.operator jsonifier::string_view();
		static constexpr auto n = s.size();

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<char c, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacters(size_t n, buffer_type& buffer, uint64_t& index) noexcept {
		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		std::memset(buffer.data() + index, c, n);
		index += n;
	}

	template<char c, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharactersUnchecked(size_t n, buffer_type& buffer, uint64_t& index) noexcept {
		std::memset(buffer.data() + index, c, n);
		index += n;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacters(buffer_type& buffer, uint64_t& index) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeCharacters(buffer_type& buffer, const char* str, uint64_t size, uint64_t& index) noexcept {
		auto n = size;

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		std::memcpy(buffer.data() + index, str, n);
		index += n;
	}

	template<jsonifier::concepts::buffer_like buffer_type>
	JSONIFIER_INLINE void writeCharactersUnchecked(buffer_type& buffer, const char* str, uint64_t size, uint64_t& index) noexcept {
		std::memcpy(buffer.data() + index, str, size);
		index += size;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type>
	JSONIFIER_INLINE void writeCharactersUnchecked(buffer_type& buffer, uint64_t& index) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeNewLineUnchecked(buffer_type& buffer, uint64_t& index) {
		auto indent		 = options.indent;
		auto indentSize	 = options.optionsReal.indentSize;
		auto indentTotal = indent * indentSize;
		buffer[index]	 = '\n';
		++index;
		std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
		index += indentTotal;
	};

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeNewLine(buffer_type& buffer, uint64_t& index) {
		auto indent		 = options.indent;
		auto indentSize	 = options.optionsReal.indentSize;
		auto indentTotal = indent * indentSize;
		auto n			 = 3 + indentTotal;
		if (index + n >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		buffer[index] = '\n';
		++index;
		std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
		index += indentTotal;
	};

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeEntrySeparator(buffer_type&& buffer, uint64_t& index) noexcept {
		if constexpr (options.optionsReal.prettify) {
			if (auto k = index + options.indent + 256; k > buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			static constexpr char s[]{ ",\n" };
			static constexpr auto n = std::size(s) - 1;
			auto indent				= options.indent;
			auto indentSize			= options.optionsReal.indentSize;
			std::memcpy(buffer.data() + index, s, n);
			index += n;
			std::memset(buffer.data() + index, options.optionsReal.indentChar, indent * indentSize);
			index += indent * indentSize;
		} else {
			buffer[index] = ',';
			++index;
		}
	}

	template<uint64_t objectSize, auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeObjectEntry(buffer_type& buffer, uint64_t& index) {
		if constexpr (options.optionsReal.prettify && objectSize > 0) {
			++options.indent;
			auto indent		 = options.indent;
			auto indentSize	 = options.optionsReal.indentSize;
			auto indentTotal = indent * indentSize;
			auto n			 = 3 + indentTotal;
			if (index + n >= buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
			}
			buffer[index] = '{';
			++index;
			buffer[index] = '\n';
			++index;
			std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
			index += indentTotal;
		} else {
			auto n = 1;
			if (index + n >= buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
			}
			buffer[index] = '{';
			++index;
		}
	}

	template<uint64_t objectSize, auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeObjectExit(buffer_type& buffer, uint64_t& index) {
		if constexpr (options.optionsReal.prettify && objectSize > 0) {
			--options.indent;
			auto indent		 = options.indent;
			auto indentSize	 = options.optionsReal.indentSize;
			auto indentTotal = indent * indentSize;
			auto n			 = 3 + indentTotal;
			if (index + n >= buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
			}
			buffer[index] = '\n';
			++index;
			std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
			index += indentTotal;
		} else {
			auto n = index + 1;
			if (index + n >= buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
			}
		}
		buffer[index] = '}';
		++index;
	}

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeObjectEntry(buffer_type& buffer, uint64_t& index, uint64_t size) {
		if constexpr (options.optionsReal.prettify) {
			if (size > 0) {
				++options.indent;
				auto indent		 = options.indent;
				auto indentSize	 = options.optionsReal.indentSize;
				auto indentTotal = indent * indentSize;
				auto n			 = 3 + indentTotal;
				if (index + n >= buffer.size()) [[unlikely]] {
					buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
				}
				buffer[index] = '{';
				++index;
				buffer[index] = '\n';
				++index;
				std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
				index += indentTotal;
				return;
			}
		}
		auto n = 1;
		if (index + n >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		buffer[index] = '{';
		++index;
	}

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeObjectExit(buffer_type& buffer, uint64_t& index, uint64_t size) {
		if constexpr (options.optionsReal.prettify) {
			if (size > 0) {
				--options.indent;
				auto indent		 = options.indent;
				auto indentSize	 = options.optionsReal.indentSize;
				auto indentTotal = indent * indentSize;
				auto n			 = 3 + indentTotal;
				if (index + n >= buffer.size()) [[unlikely]] {
					buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
				}
				buffer[index] = '\n';
				++index;
				std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
				index += indentTotal;
				buffer[index] = '}';
				++index;
				return;
			}
		}
		auto n = index + 1;
		if (index + n >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		buffer[index] = '}';
		++index;
	}

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeArrayEntry(buffer_type& buffer, uint64_t& index, uint64_t size) {
		if constexpr (options.optionsReal.prettify) {
			if (size > 0) {
				++options.indent;
				auto indent		 = options.indent;
				auto indentSize	 = options.optionsReal.indentSize;
				auto indentTotal = indent * indentSize;
				auto n			 = 3 + indentTotal;
				if (index + n >= buffer.size()) [[unlikely]] {
					buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
				}
				buffer[index] = '[';
				++index;
				buffer[index] = '\n';
				++index;
				std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
				index += indentTotal;
				return;
			}
		}
		auto n = 1;
		if (index + n >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		buffer[index] = '[';
		++index;
	}

	template<auto& options, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeArrayExit(buffer_type& buffer, uint64_t& index, uint64_t size) {
		if constexpr (options.optionsReal.prettify) {
			if (size > 0) {
				--options.indent;
				auto indent		 = options.indent;
				auto indentSize	 = options.optionsReal.indentSize;
				auto indentTotal = indent * indentSize;
				auto n			 = 3 + indentTotal;
				if (index + n >= buffer.size()) [[unlikely]] {
					buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
				}
				buffer[index] = '\n';
				++index;
				std::memset(buffer.data() + index, options.optionsReal.indentChar, indentTotal);
				index += indentTotal;
			}
		}
		auto n = index + 1;
		if (index + n >= buffer.size()) [[unlikely]] {
			buffer.resize(buffer.size() * 2 > index + n ? buffer.size() * 2 : index + n);
		}
		buffer[index] = ']';
		++index;
	}

	template<string_literal Str> struct chars_impl {
		static constexpr jsonifier::string_view value{ Str.values, Str.size() };
	};

	template<string_literal Str> constexpr jsonifier::string_view chars = chars_impl<Str>::value;

	template<const jsonifier::string_view&... Strs> JSONIFIER_INLINE constexpr jsonifier::string_view join() {
		constexpr auto joined_arr = []() {
			constexpr size_t len = (Strs.size() + ... + 0);
			std::array<char, len + 1> arr{};
			auto append = [i = 0, &arr](auto& s) mutable {
				for (char c: s)
					arr[i++] = c;
			};
			(append(Strs), ...);
			arr[len] = 0;
			return arr;
		}();
		auto& static_arr = make_static<joined_arr>::value;
		return { static_arr.data(), static_arr.size() - 1 };
	}

	template<const jsonifier::string_view&... Strs> constexpr auto joinV = join<Strs...>();

}// namespace jsonifier_internal