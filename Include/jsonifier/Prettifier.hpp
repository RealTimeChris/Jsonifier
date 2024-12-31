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

#include <jsonifier/Serialize_Impl.hpp>
#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/Minifier.hpp>

namespace jsonifier {

	struct prettify_options {
		uint64_t indentSize{ 3 };
		char indentChar{ ' ' };
	};

}

namespace jsonifier_internal {

	enum class prettify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Exceeded_Max_Depth		   = 2,
		Incorrect_Structural_Index = 3,
	};

	template<typename derived_type> class prettifier {
	  public:
		prettifier& operator=(const prettifier& other) = delete;
		prettifier(const prettifier& other)			   = delete;

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type>
		JSONIFIER_INLINE auto prettifyJson(string_type&& in) noexcept {
			if JSONIFIER_UNLIKELY (stringBuffer.size() < in.size() * 5) {
				stringBuffer.resize(in.size() * 5);
			}
			static constexpr jsonifier::prettify_options optionsFinal{ options };
			const auto* dataPtr = in.data();
			derivedRef.errors.clear();
			rootIter = dataPtr;
			endIter	 = dataPtr + in.size();
			section.reset<true>(dataPtr, in.size());
			string_view_ptr* iter{ section.begin() };
			if JSONIFIER_UNLIKELY (!*iter) {
				getErrors().emplace_back(
					error::constructError<error_classes::Prettifying, prettify_errors::No_Input>(getUnderlyingPtr(iter) - dataPtr, in.end() - in.begin(), dataPtr));
				return std::remove_cvref_t<string_type>{};
			}
			std::remove_cvref_t<string_type> newString{};
			auto index = impl<optionsFinal>(iter, stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint64_t>::max()) {
				newString.resize(index);
				std::memcpy(newString.data(), stringBuffer.data(), index);
				return newString;
			} else {
				return std::remove_cvref_t<string_type>{};
			}
		}

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_INLINE bool prettifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if JSONIFIER_UNLIKELY (stringBuffer.size() < in.size() * 5) {
				stringBuffer.resize(in.size() * 5);
			}
			static constexpr jsonifier::prettify_options optionsFinal{ options };
			derivedRef.errors.clear();
			const auto* dataPtr = in.data();
			rootIter			= dataPtr;
			endIter				= dataPtr + in.size();
			section.reset<true>(dataPtr, in.size());
			string_view_ptr* iter{ section.begin() };
			if JSONIFIER_UNLIKELY (!*iter) {
				getErrors().emplace_back(
					error::constructError<error_classes::Prettifying, prettify_errors::No_Input>(getUnderlyingPtr(iter) - dataPtr, in.end() - in.begin(), dataPtr));
				return false;
			}
			auto index = impl<optionsFinal>(iter, stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint64_t>::max()) {
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
		array<json_structural_type, 128> state{};
		string_view_ptr rootIter{};
		string_view_ptr endIter{};

		prettifier() noexcept : derivedRef{ initializeSelfRef() } {};

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		template<char indentChar, size_t indentSize> static constexpr auto createIndentationBuffer() {
			constexpr size_t bufferSize = 128 * indentSize + 1;
			std::array<char, bufferSize> buffer{};
			for (size_t i = 0; i < bufferSize; ++i) {
				buffer[i] = indentChar;
			}
			return buffer;
		}

		template<size_t indentSize> static constexpr auto createIndentationViews() {
			std::array<uint64_t, 128> returnValues{};
			for (size_t x = 0; x < 128; ++x) {
				returnValues[x] = x * indentSize;
			}
			return returnValues;
		}

		template<jsonifier::prettify_options options, jsonifier::concepts::string_t string_type, typename iterator>
		JSONIFIER_INLINE uint64_t impl(iterator& iter, string_type&& out) noexcept {
			static constexpr auto buffer			= createIndentationBuffer<options.indentChar, options.indentSize>();
			static constexpr auto* bufferPtr		= buffer.data();
			static constexpr auto indentViewsBuffer = createIndentationViews<options.indentSize>();
			static constexpr auto* indentViews		= indentViewsBuffer.data();
			string_view_ptr newPtr{};
			uint64_t newSize{};
			int64_t indent{};
			uint64_t index{};
			while (*iter) {
				switch (static_cast<uint8_t>(**iter)) {
					case '"': {
						newPtr = *iter;
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case ',': {
						out[index] = ',';
						++index;
						++iter;
						out[index] = '\n';
						++index;
						std::memcpy(out.data() + index, bufferPtr, indentViews[indent]);
						index += indentViews[indent];
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
						newPtr = (*iter);
						++iter;
						newSize = static_cast<uint64_t>((*iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case ':': {
						out[index] = ':';
						++index;
						out[index] = options.indentChar;
						++index;
						++iter;
						break;
					}
					case '[': {
						out[index] = '[';
						++index;
						++iter;
						++indent;
						state[static_cast<uint64_t>(indent)] = json_structural_type::Array_Start;
						if JSONIFIER_UNLIKELY (**iter != ']') {
							out[index] = '\n';
							++index;
							std::memcpy(out.data() + index, bufferPtr, indentViews[indent]);
							index += indentViews[indent];
						}
						break;
					}
					case ']': {
						--indent;
						if (indent < 0) {
							getErrors().emplace_back(error::constructError<error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - rootIter, endIter - rootIter, rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*iter[-1] != '[') {
							out[index] = '\n';
							++index;
							std::memcpy(out.data() + index, bufferPtr, indentViews[indent]);
							index += indentViews[indent];
						}
						out[index] = ']';
						++index;
						++iter;
						break;
					}
					case 'n': {
						std::memcpy(&out[index], nullV, 4);
						index += 4;
						++iter;
						break;
					}
					case 't': {
						std::memcpy(&out[index], trueV, 4);
						index += 4;
						++iter;
						break;
					}
					case 'f': {
						std::memcpy(&out[index], falseV, 5);
						index += 5;
						++iter;
						break;
					}
					case '{': {
						out[index] = '{';
						++index;
						++iter;
						++indent;
						state[static_cast<uint64_t>(indent)] = json_structural_type::Object_Start;
						if (**iter != '}') {
							out[index] = '\n';
							++index;
							std::memcpy(out.data() + index, bufferPtr, indentViews[indent]);
							index += indentViews[indent];
						}
						break;
					}
					case '}': {
						--indent;
						if (indent < 0) {
							getErrors().emplace_back(error::constructError<error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(
								getUnderlyingPtr(iter) - rootIter, endIter - rootIter, rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						if (*iter[-1] != '{') {
							out[index] = '\n';
							++index;
							std::memcpy(out.data() + index, bufferPtr, indentViews[indent]);
							index += indentViews[indent];
						}
						out[index] = '}';
						++index;
						++iter;
						break;
					}
					case '\0': {
						return index;
					}
					default: {
						getErrors().emplace_back(error::constructError<error_classes::Prettifying, prettify_errors::Incorrect_Structural_Index>(getUnderlyingPtr(iter) - rootIter,
							endIter - rootIter, rootIter));
						return std::numeric_limits<uint64_t>::max();
					}
				}
			}
			return index;
		}

		~prettifier() noexcept = default;
	};

}// namespace jsonifier_internal
