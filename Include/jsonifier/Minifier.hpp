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

#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	enum class minify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Invalid_String_Length	   = 2,
		Invalid_Number_Value	   = 3,
		Incorrect_Structural_Index = 4,
	};

	template<typename derived_type> class minifier {
	  public:
		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			  = delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE auto minifyJson(string_type&& in) noexcept {
			if JSONIFIER_UNLIKELY (stringBuffer.size() < in.size()) {
				stringBuffer.resize(in.size());
			}
			derivedRef.errors.clear();
			const auto* dataPtr = in.data();
			rootIter			= dataPtr;
			endIter				= dataPtr + in.size();
			section.reset<false>(dataPtr, in.size());
			string_view_ptr* iter{ section.begin() };
			if (!*iter) {
				getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::No_Input>(*iter - dataPtr, in.end() - in.begin(), dataPtr));
				return std::remove_cvref_t<string_type>{};
			}
			std::remove_cvref_t<string_type> newString{};
			auto index = impl(iter, stringBuffer);
			if (index != std::numeric_limits<uint32_t>::max()) {
				newString.resize(index);
				std::memcpy(newString.data(), stringBuffer.data(), index);
				return newString;
			} else {
				return std::remove_cvref_t<string_type>{};
			}
		}

		template<jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_INLINE bool minifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if JSONIFIER_UNLIKELY (stringBuffer.size() < in.size()) {
				stringBuffer.resize(in.size());
			}
			derivedRef.errors.clear();
			const auto* dataPtr = in.data();
			rootIter			= dataPtr;
			endIter				= dataPtr + in.size();
			section.reset<false>(dataPtr, in.size());
			string_view_ptr* iter{ section.begin() };
			if (!*iter) {
				getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::No_Input>(*iter - dataPtr, in.end() - in.begin(), dataPtr));
				return false;
			}
			auto index = impl(iter, stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint32_t>::max()) {
				if JSONIFIER_LIKELY (buffer.size() != index) {
					buffer.resize(index);
				}
				std::memcpy(buffer.data(), stringBuffer.data(), index);
				return true;
			} else {
				return false;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		string_view_ptr rootIter{};
		string_view_ptr endIter{};

		JSONIFIER_INLINE size_t getSize() const {
			return endIter - rootIter;
		}

		template<jsonifier::concepts::string_t string_type, typename iterator> JSONIFIER_INLINE uint64_t impl(iterator& iter, string_type&& out) noexcept {
			auto previousPtr = *iter;
			int64_t currentDistance{};
			uint64_t index{};
			++iter;

			while (*iter) {
				switch (static_cast<uint8_t>(*previousPtr)) {
					case '"': {
						currentDistance = *iter - previousPtr;
						while (whitespaceTable[static_cast<uint8_t>(previousPtr[--currentDistance])]) {
						}
						++currentDistance;
						if JSONIFIER_LIKELY (currentDistance > 0) {
							std::memcpy(&out[index], previousPtr, static_cast<uint64_t>(currentDistance));
							index += static_cast<uint64_t>(currentDistance);
						} else {
							this->getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::Invalid_String_Length>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - this->rootIter), static_cast<int64_t>(this->endIter - this->rootIter), this->rootIter));
							return std::numeric_limits<uint32_t>::max();
						}
						break;
					}
					case ',': {
						out[index] = ',';
						++index;
						break;
					}
					case '0':
						[[fallthrough]];
					case '1':
						[[fallthrough]];
					case '2':
						[[fallthrough]];
					case '3':
						[[fallthrough]];
					case '4':
						[[fallthrough]];
					case '5':
						[[fallthrough]];
					case '6':
						[[fallthrough]];
					case '7':
						[[fallthrough]];
					case '8':
						[[fallthrough]];
					case '9':
						[[fallthrough]];
					case '-': {
						currentDistance = 0;
						while (!whitespaceTable[static_cast<uint8_t>(previousPtr[++currentDistance])] && ((previousPtr + currentDistance) < (*iter))) {
						}
						if JSONIFIER_LIKELY (currentDistance > 0) {
							std::memcpy(&out[index], previousPtr, static_cast<uint64_t>(currentDistance));
							index += static_cast<uint64_t>(currentDistance);
						} else {
							this->getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::Invalid_Number_Value>(
								static_cast<int64_t>(getUnderlyingPtr(iter) - this->rootIter), static_cast<int64_t>(this->endIter - this->rootIter), this->rootIter));
							return std::numeric_limits<uint32_t>::max();
						}
						break;
					}
					case ':': {
						out[index] = ':';
						++index;
						break;
					}
					case '[': {
						out[index] = '[';
						++index;
						break;
					}
					case ']': {
						out[index] = ']';
						++index;
						break;
					}
					case 'n': {
						std::memcpy(&out[index], "null", 4);
						index += 4;
						break;
					}
					case 'f': {
						std::memcpy(&out[index], "false", 5);
						index += 5;
						break;
					}
					case 't': {
						std::memcpy(&out[index], "true", 4);
						index += 4;
						break;
					}
					case '{': {
						out[index] = '{';
						++index;
						break;
					}
					case '}': {
						out[index] = '}';
						++index;
						break;
					}
					case '\0': {
						return index;
					}
					default: {
						this->getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::Incorrect_Structural_Index>(
							static_cast<int64_t>(getUnderlyingPtr(iter) - this->rootIter), static_cast<int64_t>(this->endIter - this->rootIter), this->rootIter));
						return std::numeric_limits<uint32_t>::max();
					}
				}
				previousPtr = (*iter);
				++iter;
			}
			if (previousPtr) {
				out[index] = *previousPtr;
				++index;
			}
			return index;
		}

		JSONIFIER_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_internal
