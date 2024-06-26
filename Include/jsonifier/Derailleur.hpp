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

#include <jsonifier/JsonStructuralIterator.hpp>
#include <jsonifier/Config.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	template<simd_structural_iterator_t iterator_type> JSONIFIER_INLINE void skipNumber(iterator_type& iter, iterator_type&) noexcept {
		++iter;
	}

	template<typename iterator_type> JSONIFIER_INLINE iterator_type skipWs(iterator_type iter) noexcept {
		while (whitespaceTable[static_cast<uint8_t>(*iter)]) {
			++iter;
		}
		return iter;
	}

	template<simd_structural_iterator_t iterator_type> JSONIFIER_INLINE void skipToEndOfValue(iterator_type& iter, iterator_type& end) {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case '[':
					[[unlikely]] case '{': {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case ']':
					[[unlikely]] case '}': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		};
		switch (*iter) {
			[[unlikely]] case '[':
			[[unlikely]] case '{': {
				++iter;
				skipToEnd();
				break;
			}
			case '"': {
				++iter;
				break;
			}
			case ':': {
				++iter;
				skipToEndOfValue(iter, end);
				break;
			}
			case 't': {
				++iter;
				break;
			}
			case 'f': {
				++iter;
				break;
			}
			case 'n': {
				++iter;
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				skipNumber(iter, end);
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<simd_structural_iterator_t iterator_type> JSONIFIER_INLINE void skipToNextValue(iterator_type& iter, iterator_type& end) {
		switch (*iter) {
			[[unlikely]] case '{':
			[[unlikely]] case '[': {
				skipToEndOfValue(iter, end);
				break;
			}
			[[unlikely]] case '"': {
				++iter;
				break;
			}
			[[unlikely]] case ':': {
				++iter;
				skipToNextValue(iter, end);
				break;
			}
			[[unlikely]] case ',': {
				++iter;
				skipToNextValue(iter, end);
				break;
			}
			[[unlikely]] case '\\': {
				++iter;
				break;
			}
			[[unlikely]] case 't': {
				++iter;
				break;
			}
			[[unlikely]] case 'f': {
				++iter;
				break;
			}
			[[unlikely]] case 'n': {
				++iter;
				break;
			}
			[[unlikely]] case '0':
			[[unlikely]] case '1':
			[[unlikely]] case '2':
			[[unlikely]] case '3':
			[[unlikely]] case '4':
			[[unlikely]] case '5':
			[[unlikely]] case '6':
			[[unlikely]] case '7':
			[[unlikely]] case '8':
			[[unlikely]] case '9':
			[[unlikely]] case '-': {
				++iter;
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<typename iterator_type> JSONIFIER_INLINE void skipNumber(iterator_type& iter, iterator_type& end) noexcept {
		iter += *iter == '-';
		auto sig_start_it  = iter;
		auto frac_start_it = end;
		auto fracStart	   = [&]() {
			frac_start_it = iter;
			iter		  = std::find_if_not(iter, end, isNumberType);
			if (iter == frac_start_it) {
				return true;
			}
			if ((*iter | ('E' ^ 'e')) != 'e') {
				return true;
			}
			++iter;
			return false;
		};

		auto expStart = [&]() {
			iter += *iter == '+' || *iter == '-';
			auto exp_start_it = iter;
			iter			  = std::find_if_not(iter, end, isNumberType);
			if (iter == exp_start_it) {
				return true;
			}
			return false;
		};
		if (*iter == '0') {
			++iter;
			if (*iter != '.') {
				return;
			}
			++iter;
			if (fracStart()) {
				return;
			}
		}
		iter = std::find_if_not(iter, end, isNumberType);
		if (iter == sig_start_it) {
			return;
		}
		if ((*iter | ('E' ^ 'e')) == 'e') {
			++iter;
			if (expStart()) {
				return;
			}
		}
		if (*iter != '.')
			return;
		++iter;
	}

	template<typename iterator_type> JSONIFIER_INLINE void skipString(iterator_type& iter, iterator_type& end) {
		++iter;
		auto newLength = static_cast<uint64_t>(end - iter);
		skipStringImpl(iter, newLength);
	}

	template<typename iterator_type> JSONIFIER_INLINE void skipToEndOfValue(iterator_type& iter, iterator_type& end) {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case '[':
					[[unlikely]] case '{': {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case ']':
					[[unlikely]] case '}': {
						--currentDepth;
						++iter;
						break;
					}
					case '"': {
						skipString(iter, end);
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		};
		switch (*iter) {
			[[unlikely]] case '[':
			[[unlikely]] case '{': {
				++iter;
				skipToEnd();
				break;
			}
			case '"': {
				skipString(iter, end);
				break;
			}
			case ':': {
				++iter;
				skipToEndOfValue(iter, end);
				break;
			}
			case 't': {
				iter += 4;
				break;
			}
			case 'f': {
				iter += 5;
				break;
			}
			case 'n': {
				iter += 4;
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				skipNumber(iter, end);
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<typename iterator_type> JSONIFIER_INLINE void skipToNextValue(iterator_type& iter, iterator_type& end) {
		switch (*iter) {
			[[unlikely]] case '{':
			[[unlikely]] case '[': {
				skipToEndOfValue(iter, end);
				break;
			}
			[[unlikely]] case '"': {
				skipString(iter, end);
				break;
			}
			[[unlikely]] case ':': {
				++iter;
				skipToNextValue(iter, end);
				break;
			}
			[[unlikely]] case ',': {
				++iter;
				skipToNextValue(iter, end);
				break;
			}
			[[unlikely]] case '\\': {
				++iter;
				++iter;
				break;
			}
			[[unlikely]] case 't': {
				iter += 4;
				break;
			}
			[[unlikely]] case 'f': {
				iter += 5;
				break;
			}
			[[unlikely]] case 'n': {
				iter += 4;
				break;
			}
			[[unlikely]] case '0':
			[[unlikely]] case '1':
			[[unlikely]] case '2':
			[[unlikely]] case '3':
			[[unlikely]] case '4':
			[[unlikely]] case '5':
			[[unlikely]] case '6':
			[[unlikely]] case '7':
			[[unlikely]] case '8':
			[[unlikely]] case '9':
			[[unlikely]] case '-': {
				skipNumber(iter, end);
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<char startChar, char endChar, typename iterator_type> JSONIFIER_INLINE uint64_t countValueElements(iterator_type iter, iterator_type end) {
		auto newValue = *iter;
		if (newValue == ']' || newValue == '}') [[unlikely]] {
			return 0;
		}
		uint64_t currentCount{ 1 };
		while (iter != end) {
			switch (*iter) {
				[[unlikely]] case ',': {
					++currentCount;
					++iter;
					break;
				}
				[[unlikely]] case '{':
				[[unlikely]] case '[': {
					skipToEndOfValue(iter, end);
					break;
				}
				[[unlikely]] case endChar: { return currentCount; }
				[[likely]] case '"': {
					skipString(iter, end);
					break;
				}
				[[unlikely]] case '\\': {
					++iter;
					++iter;
					break;
				}
				[[unlikely]] case 't': {
					iter += 4;
					break;
				}
				[[unlikely]] case 'f': {
					iter += 5;
					break;
				}
				[[unlikely]] case 'n': {
					iter += 4;
					break;
				}
				[[unlikely]] case ':': {
					++iter;
					break;
				}
				[[unlikely]] case '0':
				[[unlikely]] case '1':
				[[unlikely]] case '2':
				[[unlikely]] case '3':
				[[unlikely]] case '4':
				[[unlikely]] case '5':
				[[unlikely]] case '6':
				[[unlikely]] case '7':
				[[unlikely]] case '8':
				[[unlikely]] case '9':
				[[unlikely]] case '-': {
					skipNumber(iter, end);
					break;
				}
					[[likely]] default : {
						++iter;
						break;
					}
			}
		}
		return currentCount;
	}

	struct key_stats_t {
		uint32_t minLength{ (std::numeric_limits<uint32_t>::max)() };
		uint32_t lengthRange{};
		uint32_t maxLength{};
	};

	template<key_stats_t stats, typename iterator_type> [[nodiscard]] JSONIFIER_INLINE jsonifier::string_view parseKeyCx(iterator_type&& iter) noexcept {
		static constexpr auto lengthRange = stats.lengthRange;

		auto start = iter;

		iter += stats.minLength;

		 if constexpr (lengthRange == 0) {
			return { start, stats.minLength };
		} else if constexpr (lengthRange == 1) {
			if (*iter != '"') {
				++iter;
			}
			return { start, size_t(iter - start) };
		} else if constexpr (lengthRange < 4) {
			for (const auto e = iter + stats.lengthRange + 1; iter < e; ++iter) {
				if (*iter == '"') {
					break;
				}
			}
			return { start, size_t(iter - start) };
		} else if constexpr (lengthRange == 7) {
			uint64_t chunk;
			std::memcpy(&chunk, iter, 8);
			const uint64_t testChunk = hasValue<'"', uint64_t>(chunk);
			if (testChunk) [[likely]] {
				iter += (simd_internal::tzcnt(testChunk) >> 3);
			}
			return { start, size_t(iter - start) };
		} else if constexpr (lengthRange > 15) {
			uint64_t chunk;
			std::memcpy(&chunk, iter, 8);
			uint64_t testChunk = hasValue<'"', uint64_t>(chunk);
			if (testChunk) {
				goto finish;
			}

			iter += 8;
			std::memcpy(&chunk, iter, 8);
			testChunk = hasValue<'"', uint64_t>(chunk);
			if (testChunk) {
				goto finish;
			}

			iter += 8;
			static constexpr auto rest = lengthRange + 1 - 16;
			chunk					   = 0;
			std::memcpy(&chunk, iter, rest);
			testChunk = hasValue<'"', uint64_t>(chunk);
			if (!testChunk) {
				testChunk = 1;
			}

		finish:
			iter += (simd_internal::tzcnt(testChunk) >> 3);
			return { start, size_t(iter - start) };
		} else if constexpr (lengthRange > 7) {
			uint64_t chunk;
			std::memcpy(&chunk, iter, 8);
			uint64_t testChunk = hasValue<'"', uint64_t>(chunk);
			if (testChunk) {
				iter += (simd_internal::tzcnt(testChunk) >> 3);
			} else {
				iter += 8;
				static constexpr auto rest = lengthRange + 1 - 8;
				chunk					   = 0;
				std::memcpy(&chunk, iter, rest);
				testChunk = hasValue<'"', uint64_t>(chunk);
				if (testChunk) {
					iter += (simd_internal::tzcnt(testChunk) >> 3);
				}
			}
			return { start, size_t(iter - start) };
		} else {
			uint64_t chunk{};
			std::memcpy(&chunk, iter, lengthRange + 1);
			const uint64_t testChunk = hasValue<'"', uint64_t>(chunk);
			if (testChunk) [[likely]] {
				iter += (simd_internal::tzcnt(testChunk) >> 3);
			}
			return { start, size_t(iter - start) };
		}
	}	

	template<typename value_type, size_t I> constexpr jsonifier::string_view getKey() noexcept {
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = jsonifier::concepts::unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return getName<first>();
		} else {
			return static_cast<jsonifier::string_view>(first);
		}
	}

	template<typename value_type, uint64_t index, uint64_t maxIndex> JSONIFIER_INLINE constexpr auto keyStatsHelper(key_stats_t stats) {
		if constexpr (index < maxIndex) {
			constexpr jsonifier::string_view key{ getKey<value_type, index>() };
			const auto n{ key.size() };
			if (n < stats.minLength) {
				stats.minLength = n;
			}
			if (n > stats.maxLength) {
				stats.maxLength = n;
			}
			return keyStatsHelper<value_type, index + 1, maxIndex>(stats);
		} else {
			if constexpr (maxIndex > 0) {
				stats.lengthRange = stats.maxLength - stats.minLength;
			}
			return stats;
		}
	}

	template<typename value_type> JSONIFIER_INLINE constexpr auto keyStats() {
		constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };

		return keyStatsHelper<value_type, 0, N>(key_stats_t{});
	}

	template<const auto& options, typename value_type, typename iterator_type>
	JSONIFIER_INLINE jsonifier::string_view parseKey(iterator_type& iter, iterator_type& end, jsonifier::vector<error>& errors) {
		if (*iter == '"') [[unlikely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - iter, options.rootIter));
			return {};
		}
		constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };

		static constexpr auto stats{ keyStats<value_type>() };
		if constexpr (N == 1) {
			static constexpr jsonifier::string_view key{ getKey<value_type, 0>() };
			iter += key.size() + 1;
			return key;
		} else if constexpr (N > 0) {
			if constexpr (stats.lengthRange < 24) {
				if ((iter + stats.maxLength) < end) [[likely]] {
					jsonifier::string_view newKey{ parseKeyCx<stats>(iter) };
					++iter;
					return newKey;
				}
			}
			auto start = iter;
			memchar<'"'>(iter, static_cast<uint64_t>(end - iter));
			jsonifier::string_view newKey{ start, size_t(iter - start) };
			++iter;
			return newKey;
		} else {
			auto start = iter;
			memchar<'"'>(iter, static_cast<uint64_t>(end - iter));
			jsonifier::string_view newKey{ start, size_t(iter - start) };
			++iter;
			return newKey;
		}
	}

	template<const auto& options, typename value_type, simd_structural_iterator_t iterator_type>
	JSONIFIER_INLINE jsonifier::string_view parseKey(iterator_type& iter, iterator_type& end, jsonifier::vector<error>& errors) {
		auto start{ iter.operator->() };

		if (*iter == '"') [[unlikely]] {
			++iter;
		} else {
			static constexpr auto sourceLocation{ std::source_location::current() };
			errors.emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(iter - options.rootIter, end - iter, options.rootIter));
			return {};
		}

		return jsonifier::string_view{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
	}

}