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

#include <jsonifier-incl/serializing/serialize_impl.hpp>
#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/serializing/minifier.hpp>

namespace jsonifier {

	struct prettify_options {
		uint64_t indentSize{ 3 };
		char indentChar{ ' ' };
	};

}

namespace jsonifier::internal {

	template<typename derived_type> class prettifier {
	  public:
		inline prettifier& operator=(const prettifier& other) = delete;
		inline prettifier(const prettifier& other)			  = delete;

		template<prettify_options options = prettify_options{}, concepts::string_t string_type> inline auto prettifyJson(string_type&& in) noexcept {
			if JSONIFIER_UNLIKELY (derivedRef.stringBuffer.size() < in.size() * 5) {
				derivedRef.stringBuffer.resize(in.size() * 5);
			}
			static constexpr prettify_options optionsFinal{ options };
			const auto* dataPtr = in.data();
			derivedRef.errors.clear();
			rootIter = dataPtr;
			endIter	 = dataPtr + in.size();
			derivedRef.section.template reset<true>(dataPtr, in.size());
			uint32_t* iter{ derivedRef.section.begin() };
			auto* endStructural = derivedRef.section.end();
			if JSONIFIER_UNLIKELY (iter == endStructural) {
				getErrors().emplace_back(error::constructError<status_classes::Prettifying, prettify_status::No_Input>(0, in.size(), dataPtr));
				return jsonifier::internal::remove_cvref_t<string_type>{};
			}
			jsonifier::internal::remove_cvref_t<string_type> newString{};
			auto index = impl<optionsFinal>(iter, endStructural, dataPtr, derivedRef.stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint64_t>::max()) {
				newString.resize(index);
				std::memcpy(newString.data(), derivedRef.stringBuffer.data(), index);
				return newString;
			} else {
				return jsonifier::internal::remove_cvref_t<string_type>{};
			}
		}

		template<prettify_options options = prettify_options{}, concepts::string_t string_type01, concepts::string_t string_type02>
		inline bool prettifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if JSONIFIER_UNLIKELY (derivedRef.stringBuffer.size() < in.size() * 5) {
				derivedRef.stringBuffer.resize(in.size() * 5);
			}
			static constexpr prettify_options optionsFinal{ options };
			derivedRef.errors.clear();
			const auto* dataPtr = in.data();
			rootIter			= dataPtr;
			endIter				= dataPtr + in.size();
			derivedRef.section.template reset<true>(dataPtr, in.size());
			uint32_t* iter{ derivedRef.section.begin() };
			auto* endStructural = derivedRef.section.end();
			if JSONIFIER_UNLIKELY (iter == endStructural) {
				getErrors().emplace_back(error::constructError<status_classes::Prettifying, prettify_status::No_Input>(0, static_cast<int64_t>(in.size()), dataPtr));
				return false;
			}
			auto index = impl<optionsFinal>(iter, endStructural, dataPtr, derivedRef.stringBuffer);
			if JSONIFIER_LIKELY (index != std::numeric_limits<uint64_t>::max()) {
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
		std::vector<json_structural_type> state{};
		string_view_ptr rootIter{};
		string_view_ptr endIter{};

		inline prettifier() noexcept : derivedRef{ initializeSelfRef() } {
			state.resize(64);
		}

		inline derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		inline std::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		template<prettify_options options, concepts::string_t string_type, typename iterator>
		inline uint64_t impl(iterator& iter, iterator endStructural, string_view_ptr stringRoot, string_type&& out) noexcept {
			string_view_ptr newPtr{};
			uint64_t newSize{};
			int64_t indent{};
			int64_t depth{};
			uint64_t index{};
			while (iter < endStructural) {
				switch (static_cast<uint64_t>(jsonTypes[static_cast<uint8_t>(stringRoot[*iter])])) {
					case static_cast<uint64_t>(json_structural_type::string): {
						newPtr = stringRoot + *iter;
						++iter;
						newSize = static_cast<uint64_t>((stringRoot + *iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::comma): {
						static constexpr uint16_t commaNewLine{ packValues2(",\n") };
						std::memcpy(&out[index], &commaNewLine, 2);
						index += 2;
						++iter;
						std::memset(&out[index], options.indentChar, static_cast<uint64_t>(indent));
						index += static_cast<uint64_t>(indent);
						break;
					}
					case static_cast<uint64_t>(json_structural_type::number): {
						newPtr = stringRoot + *iter;
						++iter;
						newSize = static_cast<uint64_t>((stringRoot + *iter) - newPtr);
						std::memcpy(&out[index], newPtr, newSize);
						index += newSize;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::colon): {
						static constexpr char valuesNew[]{ ':', options.indentChar };
						static constexpr uint16_t colonIndentChar{ packValues2(valuesNew) };
						std::memcpy(&out[index], &colonIndentChar, 2);
						index += 2;
						++iter;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::array_start): {
						out[index] = '[';
						++index;
						++iter;
						indent += options.indentSize;
						if JSONIFIER_UNLIKELY (static_cast<uint64_t>(depth) >= state.size()) {
							state.resize(state.size() * 2);
						}
						state[static_cast<uint64_t>(depth)] = json_structural_type::array_start;
						++depth;
						if JSONIFIER_LIKELY (stringRoot[*iter] != ']') {
							out[index] = '\n';
							++index;
							std::memset(&out[index], options.indentChar, static_cast<uint64_t>(indent));
							index += static_cast<uint64_t>(indent);
						} else {
							indent -= options.indentSize;
							--depth;
							out[index] = ']';
							++index;
							if (indent < 0) {
								getErrors().emplace_back(
									error::constructError<status_classes::Prettifying, prettify_status::Incorrect_Structural_Index>(*iter, endIter - rootIter, rootIter));
								return std::numeric_limits<uint64_t>::max();
							}
							++iter;
						}
						break;
					}
					case static_cast<uint64_t>(json_structural_type::array_end): {
						indent -= options.indentSize;
						--depth;
						if (indent < 0) {
							getErrors().emplace_back(
								error::constructError<status_classes::Prettifying, prettify_status::Incorrect_Structural_Index>(*iter, endIter - rootIter, rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						out[index] = '\n';
						++index;
						std::memset(&out[index], options.indentChar, static_cast<uint64_t>(indent));
						index += static_cast<uint64_t>(indent);
						out[index] = ']';
						++index;
						++iter;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::null): {
						JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
						std::memcpy(&out[index], nullV, 4);
						index += 4;
						++iter;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::boolean): {
						if (stringRoot[*iter] == 'f') {
							JSONIFIER_ALIGN(8) static constexpr char falseV[]{ "false" };
							std::memcpy(&out[index], falseV, 5);
							index += 5;
							++iter;
						} else {
							JSONIFIER_ALIGN(4) static constexpr char trueV[]{ "true" };
							std::memcpy(&out[index], trueV, 4);
							index += 4;
							++iter;
						}
						break;
					}
					case static_cast<uint64_t>(json_structural_type::object_start): {
						out[index] = '{';
						++index;
						++iter;
						indent += options.indentSize;
						if JSONIFIER_UNLIKELY (static_cast<uint64_t>(depth) >= state.size()) {
							state.resize(state.size() * 2);
						}
						state[static_cast<uint64_t>(depth)] = json_structural_type::object_start;
						++depth;
						if (stringRoot[*iter] != '}') {
							out[index] = '\n';
							++index;
							std::memset(&out[index], options.indentChar, static_cast<uint64_t>(indent));
							index += static_cast<uint64_t>(indent);
						} else {
							--depth;
							out[index] = '}';
							++index;
							++iter;
						}
						break;
					}
					case static_cast<uint64_t>(json_structural_type::object_end): {
						indent -= options.indentSize;
						--depth;
						if (indent < 0) {
							getErrors().emplace_back(
								error::constructError<status_classes::Prettifying, prettify_status::Incorrect_Structural_Index>(*iter, endIter - rootIter, rootIter));
							return std::numeric_limits<uint64_t>::max();
						}
						out[index] = '\n';
						++index;
						std::memset(&out[index], options.indentChar, static_cast<uint64_t>(indent));
						index += static_cast<uint64_t>(indent);
						out[index] = '}';
						++index;
						++iter;
						break;
					}
					case static_cast<uint64_t>(json_structural_type::type_count):
						[[fallthrough]];
					case static_cast<uint64_t>(json_structural_type::error):
						[[fallthrough]];
					case static_cast<uint64_t>(json_structural_type::unset): {
						return index;
					}
					default: {
						getErrors().emplace_back(
							error::constructError<status_classes::Prettifying, prettify_status::Incorrect_Structural_Index>(*iter, endIter - rootIter, rootIter));
						return std::numeric_limits<uint64_t>::max();
					}
				}
			}
			return index;
		}

		inline ~prettifier() noexcept = default;
	};

}// namespace internal
