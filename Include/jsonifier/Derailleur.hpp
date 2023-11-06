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
#include <jsonifier/Base.hpp>

namespace jsonifier_internal {

	template<json_structural_type currentValue> JSONIFIER_INLINE bool containsValue(uint8_t value) {
		return static_cast<uint8_t>(currentValue) == value;
	}

	template<> JSONIFIER_INLINE bool containsValue<json_structural_type::Bool>(uint8_t value) {
		return value == 0x74u || value == 0x66u;
	}

	template<> JSONIFIER_INLINE bool containsValue<json_structural_type::Number>(uint8_t value) {
		return numberTable[value];
	}

	class derailleur {
	  public:
		using size_type = uint64_t;

		template<json_structural_type c, jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static bool checkForMatchClosed(iterator&& iter) {
			if (containsValue<c>(*iter)) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<uint8_t c, jsonifier::concepts::is_fwd_iterator iterator>
		JSONIFIER_INLINE static bool checkForMatchClosed(iterator&& iter, iterator&& end, std::source_location location = std::source_location::current()) {
			auto oldValue = *iter;
			if (containsValue<c>(*iter)) {
				++iter;
				return true;
			} else {
				auto oldIter = iter;
				skipValue(iter, end);
				return false;
			}
		}

		template<json_structural_type c, jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static bool checkForMatchOpen(iterator&& iter) {
			if (*iter == static_cast<uint8_t>(c)) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipKey(iterator&& iter, iterator&& end) {
			if constexpr (std::same_as<simd_structural_iterator<parser<jsonifier::jsonifier_core>, jsonifier::jsonifier_core>, iterator>) {
				++iter;
				return;
			} else {
				++iter;
				while (iter != end) {
					switch (*iter) {
						case 0x22u: {
							++iter;
							return;
						}
						default: {
							++iter;
							break;
						}
					}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToEndOfArray(iterator&& iter) {
			size_type currentDepth{ 1 };
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case 0x5B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x5D: {
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToEndOfObject(iterator&& iter) {
			size_type currentDepth{ 1 };
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x7D: {
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToNextValue(iterator&& iter) {
			while (iter != iter && *iter != 0x2Cu) {
				switch (*iter) {
					case 0x7B: {
						skipObject(std::forward<iterator>(iter));
						break;
					}
					case 0x5B: {
						skipArray(std::forward<iterator>(iter));
						break;
					}
					case 0x00ll: {
						return;
					}
					default: {
						++iter;
					}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToNextValue(iterator&& iter, iterator&& end) {
			while (iter != end && *iter != 0x2Cu) {
				switch (*iter) {
					case 0x7B: {
						skipObject(std::forward<iterator>(iter), std::forward<iterator>(end));
						break;
					}
					case 0x5B: {
						skipArray(std::forward<iterator>(iter), std::forward<iterator>(end));
						break;
					}
					case 0x00ll: {
						return;
					}
					default: {
						++iter;
					}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipValue(iterator&& iter) {
			switch (*iter) {
				case 0x7B: {
					skipObject(std::forward<iterator>(iter));
					break;
				}
				case 0x5B: {
					skipArray(std::forward<iterator>(iter));
					break;
				}
				case 0x00ll: {
					return;
				}
				default: {
					++iter;
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipValue(iterator&& iter, iterator&& end) {
			switch (*iter) {
				case 0x7B: {
					skipObject(std::forward<iterator>(iter), std::forward<iterator>(end));
					break;
				}
				case 0x5B: {
					skipArray(std::forward<iterator>(iter), std::forward<iterator>(end));
					break;
				}
				case 0x00ll: {
					return;
				}
				default: {
					++iter;
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static size_type countValueElements(iterator iter, iterator end) {
			size_type currentDepth{ 1 };
			size_type currentCount{ 0 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case 0x5B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x5D: {
						--currentDepth;
						++iter;
						break;
					}
					case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x7D: {
						--currentDepth;
						++iter;
						break;
					}
					case 0x2Cu: {
						if (currentDepth == 1) {
							++currentCount;
						}
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
			return currentCount;
		}

	  protected:
		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipObject(iterator&& iter) {
			++iter;
			size_type currentDepth{ 1 };
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x7D: {
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipObject(iterator&& iter, iterator&& end) {
			++iter;
			size_type currentDepth{ 1 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case 0x7B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x7D: {
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipArray(iterator&& iter) {
			++iter;
			size_type currentDepth{ 1 };
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case 0x5B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x5D: {
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipArray(iterator&& iter, iterator&& end) {
			++iter;
			size_type currentDepth{ 1 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case 0x5B: {
						++currentDepth;
						++iter;
						break;
					}
					case 0x5D: {
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
		}
	};

}