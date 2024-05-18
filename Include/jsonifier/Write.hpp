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

	static constexpr jsonifier::string_view falseString{ "false" };
	static constexpr jsonifier::string_view trueString{ "true" };
	static constexpr jsonifier::string_view nullString{ "null" };

	template<auto character, typename iterator_type> JSONIFIER_INLINE void writeCharacter(iterator_type& out) {
		*out = static_cast<std::remove_pointer_t<jsonifier::concepts::unwrap_t<iterator_type>>>(character);
		++out;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& out, index_type&& index, const char_type c) noexcept {
		const auto k = index + 1;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}

		out[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<auto c, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& out, index_type&& index) noexcept {
		const auto k = index + 1;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}

		out[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacterUnchecked(buffer_type&& out, index_type&& index, const char_type c) noexcept {
		out[index] = static_cast<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, uint64_t size, typename char_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& out, index_type&& index, const char_type (&str)[size]) noexcept {
		static constexpr auto sizeNew = size - 1;
		const auto k				  = index + sizeNew;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}
		std::copy(str, str + sizeNew, out.data() + std::forward<index_type>(index));
		index += sizeNew;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& out, index_type&& index) noexcept {
		const auto sizeNew = str.size();
		const auto k	   = index + sizeNew;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}
		std::copy(str.data(), str.data() + sizeNew, out.data() + std::forward<index_type>(index));
		index += sizeNew;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& out, index_type&& index, char_type* chars, uint64_t length) noexcept {
		const auto sizeNew = length;
		const auto k	   = index + sizeNew;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}
		std::copy(chars, chars + sizeNew, out.data() + std::forward<index_type>(index));
		index += sizeNew;
	}

	template<auto character, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& out, index_type&& index, int64_t count) noexcept {
		const auto sizeNew = count;
		const auto k	   = index + sizeNew;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}
		std::memset(out.data() + index, character, static_cast<uint64_t>(count));
		index += sizeNew;
	}

	template<bool tabs, uint64_t indentSize, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeNewLine(buffer_type& out, int64_t& indent, index_type&& index) {
		uint64_t sizeNew{ 1 };
		if constexpr (tabs) {
			sizeNew += indent;
		} else {
			sizeNew += indent * indentSize;
		}
		const auto k = index + sizeNew;
		if (k >= out.size()) [[unlikely]] {
			out.resize(max(out.size() * 2, k));
		}
		writeCharacter<0x0Au>(out, index);
		if constexpr (tabs) {
			std::memset(out.data() + index, 0x09, indent);
			index += indent;
		} else {
			std::memset(out.data() + index, 0x20u, indent * indentSize);
			index += indent * indentSize;
		}
	}

	template<typename iterator_type01, typename iterator_type02> JSONIFIER_INLINE void writeCharacters(int64_t currentDistance, iterator_type01& out, iterator_type02 currentVal) {
		if (currentDistance > 0) [[likely]] {
			std::memcpy(out, currentVal, static_cast<uint64_t>(currentDistance));
			out += currentDistance;
		}
	}

	template<bool tabs, uint64_t indentSize, typename iterator_type> JSONIFIER_INLINE void writeNewLine(iterator_type& outPtr, int64_t& indent) {
		writeCharacter<0x0Au>(outPtr);
		if constexpr (tabs) {
			std::memset(outPtr, 0x09, indent);
			outPtr += indent;
		} else {
			std::memset(outPtr, 0x20u, indent * indentSize);
			outPtr += indent * indentSize;
		}
	}

	template<bool tabs, uint64_t indentSize, jsonifier::concepts::buffer_like buffer_type> JSONIFIER_INLINE void writeNewLine(buffer_type& out, uint64_t& index, int64_t& indent) {
		writeCharacter<0x0Au>(out, index);
		if constexpr (tabs) {
			std::memset(out.data() + index, 0x09u, indent);
			index += indent;
		} else {
			std::memset(out.data() + index, 0x20u, indent * indentSize);
			index += indent * indentSize;
		}
	}

	JSONIFIER_INLINE void dump(const char c, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		if (ix == b.size()) [[unlikely]] {
			b.resize(b.size() == 0 ? 128 : b.size() * 2);
		}

		b[ix] = c;
		++ix;
	}

	template<typename char_type> JSONIFIER_INLINE void dump(const char c, char_type*& b) noexcept {
		*b = c;
		++b;
	}

	template<uint8_t c> JSONIFIER_INLINE void dump(jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		if (ix == b.size()) [[unlikely]] {
			b.resize(b.size() == 0 ? 128 : b.size() * 2);
		}

		using V = std::decay_t<decltype(b[0])>;
		if constexpr (std::same_as<V, char>) {
			b[ix] = c;
		} else {
			b[ix] = static_cast<V>(c);
		}
		++ix;
	}

	template<uint8_t c> JSONIFIER_INLINE void dump(auto* b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dump(char_type*& b) noexcept {
		*b = c;
		++b;
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dump_unchecked(jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		using V = std::decay_t<decltype(b[0])>;
		if constexpr (std::same_as<V, char>) {
			b[ix] = c;
		} else {
			b[ix] = static_cast<V>(c);
		}
		++ix;
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dump_unchecked(auto* b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	JSONIFIER_INLINE void dump_unchecked(const char c, auto* b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	JSONIFIER_INLINE void dump_unchecked(const char c, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	JSONIFIER_INLINE void dump_unchecked(const jsonifier::string_view str, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		const auto n = str.size();
		std::memcpy(b.data() + ix, str.data(), n);
		ix += n;
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dumpn(size_t n, char_type*& b) noexcept {
		std::memset(b, c, n);
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dumpn_unchecked(size_t n, char_type*& b) noexcept {
		std::memset(b, c, n);
	}

	template<uint8_t c> JSONIFIER_INLINE void dumpn(size_t n, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memset(b.data() + ix, c, n);
		ix += n;
	}

	template<typename char_type, char_type c> JSONIFIER_INLINE void dumpn_unchecked(size_t n, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		std::memset(b.data() + ix, c, n);
		ix += n;
	}

	template<char IndentChar> JSONIFIER_INLINE void dump_newline_indent(size_t n, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		if (const auto k = ix + n + 256; k > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, k));
		}

		b[ix] = '\n';
		++ix;
		std::memset(b.data() + ix, IndentChar, n);
		ix += n;
	}

	template<const jsonifier::string_view& str> JSONIFIER_INLINE void dump(jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, s.data(), n);
		ix += n;
	}

	template<const jsonifier::string_view& str> JSONIFIER_INLINE void dump_unchecked(jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		std::memcpy(b.data() + ix, s.data(), n);
		ix += n;
	}

	template<const jsonifier::string_view& str> JSONIFIER_INLINE void dump(auto* b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		std::memcpy(b + ix, s.data(), n);
		ix += n;
	}

	template<const jsonifier::string_view& str> JSONIFIER_INLINE void dump_unchecked(auto* b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		std::memcpy(b + ix, s.data(), n);
		ix += n;
	}

	JSONIFIER_INLINE void dump_not_empty(const jsonifier::string_view str, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		const auto n = str.size();
		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, str.data(), n);
		ix += n;
	}

	JSONIFIER_INLINE void dump_not_empty(const jsonifier::string_view str, auto* b, auto& ix) noexcept {
		const auto n = str.size();

		std::memcpy(b + ix, str.data(), n);
		ix += n;
	}

	JSONIFIER_INLINE void dump_maybe_empty(const jsonifier::string_view str, jsonifier::concepts::buffer_like auto& b, auto& ix) noexcept {
		const auto n = str.size();
		if (n) {
			if (ix + n > b.size()) [[unlikely]] {
				b.resize((std::max)(b.size() * 2, ix + n));
			}

			std::memcpy(b.data() + ix, str.data(), n);
			ix += n;
		}
	}

	JSONIFIER_INLINE void dump_maybe_empty(const jsonifier::string_view str, auto* b, auto& ix) noexcept {
		const auto n = str.size();
		if (n) {
			std::memcpy(b + ix, str.data(), n);
			ix += n;
		}
	}

	template<typename char_type> JSONIFIER_INLINE void dump(const jsonifier::string_view str, char_type*& b) noexcept {
		for (auto& c: str) {
			*b = c;
			++b;
		}
	}

}// namespace jsonifier_internal
