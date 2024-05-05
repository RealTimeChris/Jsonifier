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

#include <jsonifier/SimdStructuralIterator.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/ISADetection.hpp>

namespace jsonifier_internal {

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipToEndOfValue(iterator&& iter) {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B:
					[[unlikely]] case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D:
					[[unlikely]] case 0x7D: {
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
			[[unlikely]] case 0x5B:
			[[unlikely]] case 0x7B: {
				++iter;
				skipToEnd();
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipToNextValue(iterator&& iter) {
		switch (*iter) {
			[[unlikely]] case 0x7B:
			[[unlikely]] case 0x5B: {
				skipToEndOfValue(iter);
				break;
			}
			[[unlikely]] case 0x00ll: { return; }
				[[likely]] default : {
					++iter;
				}
		}
	}

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static uint64_t countValueElements(iterator iter) {
		auto newValue = *iter;
		uint64_t currentDepth{ 1 };
		if (newValue == 0x5Du || newValue == 0x7Du) [[unlikely]] {
			return 0;
		}
		uint64_t currentCount{ 1 };
		while (iter && currentDepth > 0) {
			switch (*iter) {
				[[unlikely]] case 0x5B: {
					++currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case 0x5D: {
					--currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case 0x7B: {
					++currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case 0x7D: {
					--currentDepth;
					++iter;
					break;
				}
				[[unlikely]] case 0x2Cu: {
					if (currentDepth == 1) [[likely]] {
						++currentCount;
					}
					++iter;
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

	template<jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE void skipWs(iterator_type& iter) noexcept {
		while (whitespaceTable[static_cast<uint64_t>(*iter)]) {
			++iter;
		}
	}

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipKey(iterator&& iter, iterator&& newIter02, iterator&& end) {
		while (newIter02 < end) {
			if (*newIter02 == '"' && *(iter - 1) != '\\') {
				++newIter02;
				break;
			}
			++newIter02;
		}
		iter = newIter02 + 1;
		while (iter < end) {
			if (*iter == '"' && *(iter - 1) != '\\') {
				break;
			}
			++iter;
		}
	}

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipToEndOfValue(iterator&& iter, iterator&& end) {
		uint64_t currentDepth{ 1 };
		auto skipToEnd = [&]() {
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B:
					[[unlikely]] case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D:
					[[unlikely]] case 0x7D: {
						--currentDepth;
						++iter;
						break;
					}
					case 0x22u: {
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
			[[unlikely]] case 0x5B:
			[[unlikely]] case 0x7B: {
				++iter;
				skipToEnd();
				break;
			}
			case 0x22u: {
				skipString(iter, end);
				break;
			}
			case 0x74u: {
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
			case 0x30u:
			case 0x31u:
			case 0x32u:
			case 0x33u:
			case 0x34u:
			case 0x35u:
			case 0x36u:
			case 0x37u:
			case 0x38u:
			case 0x39u: {
				skipNumber(iter, end);
				break;
			}
				[[likely]] default : {
					++iter;
					break;
				}
		}
	}

	template<char startChar, char endChar, jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE uint64_t countValueElements(iterator iter, iterator end) {
		auto newValue = *iter;
		if (newValue == 0x5Du || newValue == 0x7Du) [[unlikely]] {
			return 0;
		}
		uint64_t currentCount{ 1 };
		while (iter < end) {
			skipWs(iter);
			switch (*iter) {
				[[unlikely]] case 0x2Cu: {
					++currentCount;
					++iter;
					break;
				}
				[[unlikely]] case 0x7B:
				[[unlikely]] case 0x5B: {
					skipToEndOfValue(iter, end);
					break;
				}
				[[unlikely]] case endChar: { return currentCount; }
				[[unlikely]] case 0x22u: {
					skipString(iter, end);
					break;
				}
				[[unlikely]] case 0x5Cu: {
					++iter;
					++iter;
					break;
				}
				[[unlikely]] case 0x74u: {
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
				[[unlikely]] case 0x3A: {
					++iter;
					break;
				}
				[[unlikely]] case 0x30u:
				[[unlikely]] case 0x31u:
				[[unlikely]] case 0x32u:
				[[unlikely]] case 0x33u:
				[[unlikely]] case 0x34u:
				[[unlikely]] case 0x35u:
				[[unlikely]] case 0x36u:
				[[unlikely]] case 0x37u:
				[[unlikely]] case 0x38u:
				[[unlikely]] case 0x39u:
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

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipString(iterator&& iter, iterator&& end) {
		++iter;
		auto newPtr	   = iter.operator->();
		auto newLength = static_cast<uint64_t>(end - iter);
		iter		   = skipStringImpl(newPtr, newLength);
	}

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipNumber(iterator&& iter, iterator&& end) noexcept {
		iter += *iter == '-';
		const auto sig_start_it = iter;
		auto frac_start_it		= end;
		auto fracStart			= [&]() {
			 frac_start_it = iter;
			 iter		   = std::find_if_not(iter, end, isNumberType);
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
			const auto exp_start_it = iter;
			iter					= std::find_if_not(iter, end, isNumberType);
			if (iter == exp_start_it) {
				return true;
			}
			return false;
		};
		if (*iter == 0x30u) {
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

	template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE void skipToNextValue(iterator&& iter, iterator&& end) {
		switch (*iter) {
			[[unlikely]] case 0x7B:
			[[unlikely]] case 0x5B: {
				skipToEndOfValue(iter, end);
				break;
			}
			[[unlikely]] case 0x22u: {
				skipString(iter, end);
				break;
			}
			[[unlikely]] case 0x5Cu: {
				++iter;
				++iter;
				break;
			}
			[[unlikely]] case 0x74u: {
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
			[[unlikely]] case 0x30u:
			[[unlikely]] case 0x31u:
			[[unlikely]] case 0x32u:
			[[unlikely]] case 0x33u:
			[[unlikely]] case 0x34u:
			[[unlikely]] case 0x35u:
			[[unlikely]] case 0x36u:
			[[unlikely]] case 0x37u:
			[[unlikely]] case 0x38u:
			[[unlikely]] case 0x39u:
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

}