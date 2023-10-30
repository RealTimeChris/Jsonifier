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

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Base02.hpp>

namespace jsonifier_internal {

	template<json_structural_type currentValue> jsonifier_inline static bool containsValue(uint8_t value) {
		return static_cast<uint8_t>(currentValue) == value;
	}

	template<> jsonifier_inline bool containsValue<json_structural_type::Bool>(uint8_t value) {
		return value == 0x74u || value == 0x66u;
	}

	template<> jsonifier_inline bool containsValue<json_structural_type::Number>(uint8_t value) {
		return validNumberValues[value];
	}

	class derailleur {
	  public:
		using size_type = uint64_t;

		template<json_structural_type c> jsonifier_inline static bool checkForMatchClosed(structural_iterator& iter) {
			if (containsValue<c>(*iter)) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<uint8_t c, std::forward_iterator iterator>
		jsonifier_inline static bool checkForMatchClosed(iterator& iter, iterator& end, std::source_location location = std::source_location::current()) {
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

		template<json_structural_type c> jsonifier_inline static bool checkForMatchOpen(structural_iterator& iter) {
			if (*iter == static_cast<uint8_t>(c)) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<std::forward_iterator iterator> jsonifier_inline static void skipKey(iterator& iter, iterator& end) {
			if jsonifier_constexpr (std::same_as<structural_iterator, iterator>) {
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

		jsonifier_inline static void skipToEndOfArray(structural_iterator& iter) {
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

		jsonifier_inline static void skipToEndOfObject(structural_iterator& iter) {
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

		jsonifier_inline static void skipToNextValue(structural_iterator& iter) {
			while (iter != iter && *iter != 0x2Cu) {
				switch (*iter) {
					case 0x7B: {
						skipObject(iter);
						break;
					}
					case 0x5B: {
						skipArray(iter);
						break;
					}
					case 0x00: {
						return;
					}
					default: {
						++iter;
					}
				}
			}
		}

		template<std::forward_iterator iterator> jsonifier_inline static void skipToNextValue(iterator& iter, iterator end) {
			while (iter != end && *iter != 0x2Cu) {
				switch (*iter) {
					case 0x7B: {
						skipObject(iter, end);
						break;
					}
					case 0x5B: {
						skipArray(iter, end);
						break;
					}
					case 0x00: {
						return;
					}
					default: {
						++iter;
					}
				}
			}
		}

		jsonifier_inline static void skipValue(structural_iterator& iter) {
			switch (*iter) {
				case 0x7B: {
					skipObject(iter);
					break;
				}
				case 0x5B: {
					skipArray(iter);
					break;
				}
				case 0x00: {
					return;
				}
				default: {
					++iter;
				}
			}
		}

		template<std::forward_iterator iterator> jsonifier_inline static void skipValue(iterator& iter, iterator end) {
			switch (*iter) {
				case 0x7B: {
					skipObject(iter, end);
					break;
				}
				case 0x5B: {
					skipArray(iter, end);
					break;
				}
				case 0x00: {
					return;
				}
				default: {
					++iter;
				}
			}
		}

		template<std::forward_iterator iterator> jsonifier_inline static size_type countValueElements(iterator iter, iterator end) {
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
		jsonifier_inline static void skipObject(structural_iterator& iter) {
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

		template<std::forward_iterator iterator> jsonifier_inline static void skipObject(iterator& iter, iterator end) {
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

		jsonifier_inline static void skipArray(structural_iterator& iter) {
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

		template<std::forward_iterator iterator> jsonifier_inline static void skipArray(iterator& iter, iterator end) {
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