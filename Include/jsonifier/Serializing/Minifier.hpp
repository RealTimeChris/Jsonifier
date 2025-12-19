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

#include <jsonifier/Utilities/Simd.hpp>

namespace jsonifier::internal {

	inline constexpr array<json_structural_type, 256> jsonTypes = []() constexpr {
		array<json_structural_type, 256> returnValues{};
		using enum json_structural_type;
		returnValues['"'] = string;
		returnValues[','] = comma;
		returnValues['0'] = number;
		returnValues['1'] = number;
		returnValues['2'] = number;
		returnValues['3'] = number;
		returnValues['4'] = number;
		returnValues['5'] = number;
		returnValues['6'] = number;
		returnValues['7'] = number;
		returnValues['8'] = number;
		returnValues['9'] = number;
		returnValues['-'] = number;
		returnValues[':'] = colon;
		returnValues['['] = array_start;
		returnValues[']'] = array_end;
		returnValues['n'] = null;
		returnValues['t'] = boolean;
		returnValues['f'] = boolean;
		returnValues['{'] = object_start;
		returnValues['}'] = object_end;
		return returnValues;
	}();

	template<typename derived_type> class minifier {
	  public:
		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			= delete;

		template<concepts::string_t string_type> JSONIFIER_INLINE auto minifyJson(string_type&& in) noexcept {
			if JSONIFIER_UNLIKELY (derivedRef.stringBuffer.size() < in.size()) {
				derivedRef.stringBuffer.resize(in.size());
			}
			derivedRef.errors.clear();
			rootIter = in.data();
			endIter	 = rootIter + in.size();
			derivedRef.section.template reset<false>(rootIter, in.size());
			string_view_ptr* iter{ derivedRef.section.begin() };
			jsonifier::internal::remove_cvref_t<string_type> newString{};
			if (!*iter) {
				getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::No_Input>(*iter - rootIter, in.end() - in.begin(), rootIter));
				return newString;
			}
			auto index = impl(iter, derivedRef.stringBuffer);
			if (index != std::numeric_limits<uint32_t>::max()) {
				newString.resize(index);
				std::memcpy(newString.data(), derivedRef.stringBuffer.data(), index);
				return newString;
			} else {
				return jsonifier::internal::remove_cvref_t<string_type>{};
			}
		}

		template<concepts::string_t string_type01, concepts::string_t string_type02> JSONIFIER_INLINE bool minifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if JSONIFIER_UNLIKELY (derivedRef.stringBuffer.size() < in.size()) {
				derivedRef.stringBuffer.resize(in.size());
			}
			derivedRef.errors.clear();
			rootIter = in.data();
			endIter	 = rootIter + in.size();
			derivedRef.section.template reset<false>(rootIter, in.size());
			string_view_ptr* iter{ derivedRef.section.begin() };
			if (!*iter) {
				getErrors().emplace_back(error::constructError<error_classes::Minifying, minify_errors::No_Input>(*iter - rootIter, in.end() - in.begin(), rootIter));
				return false;
			}
			auto index = impl(iter, derivedRef.stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint32_t>::max()) {
				if JSONIFIER_LIKELY (buffer.size() != index) {
					buffer.resize(index);
				}
				std::memcpy(buffer.data(), derivedRef.stringBuffer.data(), index);
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

		JSONIFIER_INLINE void skipWs(int64_t& currentDistance, string_view_ptr previousPtr) noexcept {
			while (whitespaceTable[static_cast<uint8_t>(previousPtr[--currentDistance])]) {
			}
		};

		template<typename iterator_type> JSONIFIER_INLINE void backTrackWs(int64_t& currentDistance, string_view_ptr& previousPtr, iterator_type iter) noexcept {
			currentDistance = *iter - previousPtr;
			skipWs(currentDistance, previousPtr);
			++currentDistance;
		}

		template<concepts::string_t string_type, typename iterator> JSONIFIER_INLINE uint64_t impl(iterator& iter, string_type&& out) noexcept {
			auto previousPtr = *iter;
			int64_t currentDistance{};
			uint64_t index{};
			++iter;

			while (*iter) {
				switch (jsonTypes[static_cast<uint8_t>(*previousPtr)]) {
					case json_structural_type::string: {
						backTrackWs(currentDistance, previousPtr, iter);
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
					case json_structural_type::comma: {
						out[index] = ',';
						++index;
						break;
					}
					case json_structural_type::number: {
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
					case json_structural_type::colon: {
						out[index] = ':';
						++index;
						break;
					}
					case json_structural_type::array_start: {
						out[index] = '[';
						++index;
						break;
					}
					case json_structural_type::array_end: {
						out[index] = ']';
						++index;
						break;
					}
					case json_structural_type::null: {
						JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
						std::memcpy(&out[index], nullV, 4);
						index += 4;
						break;
					}
					case json_structural_type::boolean: {
						if (*previousPtr == 'f') {
							JSONIFIER_ALIGN(8) static constexpr char falseV[]{ "false" };
							std::memcpy(&out[index], falseV, 5);
							index += 5;
						} else {
							JSONIFIER_ALIGN(4) static constexpr char trueV[]{ "true" };
							std::memcpy(&out[index], trueV, 4);
							index += 4;
						}
						break;
					}
					case json_structural_type::object_start: {
						out[index] = '{';
						++index;
						break;
					}
					case json_structural_type::object_end: {
						out[index] = '}';
						++index;
						break;
					}
					case json_structural_type::unset: {
						return index;
					}
					case json_structural_type::type_count:
						[[fallthrough]];
					case json_structural_type::error:
						[[fallthrough]];
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

		JSONIFIER_INLINE std::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace internal