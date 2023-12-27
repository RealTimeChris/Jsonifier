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

	template<json_structural_type currentValue> JSONIFIER_INLINE bool containsValue(uint8_t value) {
		return static_cast<uint8_t>(currentValue) == value;
	}

	template<> JSONIFIER_INLINE bool containsValue<json_structural_type::Jsonifier_Bool>(uint8_t value) {
		return value == 0x74u || value == 0x66u;
	}

	template<> JSONIFIER_INLINE bool containsValue<json_structural_type::Jsonifier_Number>(uint8_t value) {
		return numberTable[value];
	}

	class derailleur {
	  public:

		template<jsonifier::concepts::is_fwd_iterator iterator_type> inline static void skipWs(iterator_type& iter) noexcept {
			while (true) {
				switch (*iter) {
					case 0x09u:
					case 0x0Au:
					case 0x0Du:
					case 0x20u:
						++iter;
						break;
					default: {
						return;
					}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipKey(iterator&& iter, iterator&& newIter02, iterator&& end) {
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

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToEndOfArray(iterator&& iter) {
			uint64_t currentDepth{ 1 };
			while (iter && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToEndOfObject(iterator&& iter) {
			uint64_t currentDepth{ 1 };
			while (iter && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x7B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
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
					[[unlikely]] case 0x5B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D : {
						--currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7D : {
						--currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x2Cu : {
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
			++iter;
			return currentCount;
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static uint64_t countValueElements(iterator iter, iterator end) {
			uint64_t currentDepth{ 1 };
			++iter;
			if (*iter == 0x5Du || *iter == 0x7Du) [[unlikely]] {
				return 0;
			}
			uint64_t currentCount{ 1 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D : {
						--currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7D : {
						--currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x2Cu : {
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

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipObject(iterator&& iter) {
			++iter;
			uint64_t currentDepth{ 1 };
			while (iter && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x7B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipObject(iterator&& iter, iterator&& end) {
			++iter;
			uint64_t currentDepth{ 1 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x7B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x7D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipArray(iterator&& iter) {
			++iter;
			uint64_t currentDepth{ 1 };
			while (iter && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipArray(iterator&& iter, iterator&& end) {
			++iter;
			uint64_t currentDepth{ 1 };
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					[[unlikely]] case 0x5B : {
						++currentDepth;
						++iter;
						break;
					}
					[[unlikely]] case 0x5D : {
						--currentDepth;
						++iter;
						break;
					}
						[[likely]] default : {
							++iter;
							break;
						}
				}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> static void collectCharacter(uint8_t character, iterator& iter, iterator end) {
			while (iter < end) {
				if (*iter == character) {
					++iter;
					break;
				}
				++iter;
			}
		};

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipString(iterator&& iter, iterator&& end) {
			++iter;
			while (iter != end) {
				switch (*iter) {
					[[unlikely]] case 0x22u : {
						if (*(iter - 1) != '\\') {
							++iter;
							return;
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
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToNextValue(iterator&& iter) {
			switch (*iter) {
				[[unlikely]] case 0x7B : {
					skipObject(iter);
					break;
				}
				[[unlikely]] case 0x5B : {
					skipArray(iter);
					break;
				}
				[[unlikely]] case 0x00ll : { return; }
					[[likely]] default : {
						++iter;
					}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> JSONIFIER_INLINE static void skipToNextValue(iterator&& iter, iterator&& end) {
			switch (*iter) {
				[[unlikely]] case 0x7B : {
					skipObject(iter, end);
					break;
				}
				[[unlikely]] case 0x5B : {
					skipArray(iter, end);
					break;
				}
				[[unlikely]] case 0x22 : {
					skipString(iter, end);
					break;
				}
				[[unlikely]] case 0x00ll : { return; }
					[[likely]] default : {
						uint64_t currentDepth{ 1 };
						while (iter != end && *iter != 0x2Cu && currentDepth > 0) {
							switch (*iter) {
								[[unlikely]] case 0x5B : {
									++currentDepth;
									++iter;
									break;
								}
								[[unlikely]] case 0x5D : {
									--currentDepth;
									++iter;
									break;
								}
									[[likely]] default : {
										++iter;
										break;
									}
							}
						};
					}
			}
		}

		template<jsonifier::concepts::is_fwd_iterator iterator> static auto collectKey(iterator& iter, iterator end) {
			auto newIter02 = iter;
			skipKey(iter, newIter02, end);
			jsonifier::string newString{};
			auto newSize = iter - newIter02;
			newString.resize(static_cast<uint64_t>(newSize));
			std::memcpy(newString.data(), newIter02.operator->(), static_cast<uint64_t>(newSize));
			iter = newIter02;
			return newString;
		};

		template<jsonifier::concepts::is_fwd_iterator iterator> static auto collectValue(iterator& iter, iterator end) {
			auto newIter02 = iter;
			skipToNextValue(newIter02, end);
			jsonifier::string newString{};
			auto newSize = newIter02 - iter;
			newString.resize(static_cast<uint64_t>(newSize));
			std::memcpy(newString.data(), iter.operator->(), static_cast<uint64_t>(newSize));
			iter = newIter02;
			return newString;
		};
	};
	
}