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
#include <jsonifier/Write.hpp>

namespace jsonifier {

	struct prettify_options {
		bool newLinesInArray{ true };
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

	template<jsonifier::prettify_options, typename derived_type> struct prettify_impl;

	template<typename derived_type> class prettifier {
	  public:
		template<jsonifier::prettify_options, typename derived_type_new> friend struct prettify_impl;

		JSONIFIER_ALWAYS_INLINE prettifier& operator=(const prettifier& other) = delete;
		JSONIFIER_ALWAYS_INLINE prettifier(const prettifier& other)			   = delete;

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier ::concepts::string_t string_type>
		JSONIFIER_ALWAYS_INLINE auto prettifyJson(string_type&& in) noexcept {
			if JSONIFIER_UNLIKELY ((stringBuffer.size() < in.size() * 5)) {
				stringBuffer.resize(in.size() * 5);
			}
			static constexpr jsonifier::prettify_options optionsFinal{ options };
			prettifyPair.index	= 0;
			prettifyPair.indent = 0;
			derivedRef.errors.clear();
			prettifyPair.rootIter = in.data();
			prettifyPair.endIter  = in.data() + in.size();
			section.reset<true>(in.data(), in.size());
			const char** iter{ section.begin() };
			if JSONIFIER_UNLIKELY ((!*iter)) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::No_Input>(getUnderlyingPtr(iter) - in.data(),
					in.end() - in.begin(), in.data()));
				return std::remove_cvref_t<string_type>{};
			}
			std::remove_cvref_t<string_type> newString{};
			prettify_impl<optionsFinal, derived_type>::impl(iter, stringBuffer, prettifyPair, *this);
			if JSONIFIER_LIKELY ((prettifyPair.index != std::numeric_limits<uint32_t>::max())) {
				newString.resize(prettifyPair.index);
				std::memcpy(newString.data(), stringBuffer.data(), prettifyPair.index);
				return newString;
			} else {
				return std::remove_cvref_t<string_type>{};
			}
		}

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_ALWAYS_INLINE bool prettifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if JSONIFIER_UNLIKELY ((stringBuffer.size() < in.size() * 5)) {
				stringBuffer.resize(in.size() * 5);
			}
			static constexpr jsonifier::prettify_options optionsFinal{ options };
			prettifyPair.index	= 0;
			prettifyPair.indent = 0;
			derivedRef.errors.clear();
			prettifyPair.rootIter = in.data();
			prettifyPair.endIter  = in.data() + in.size();
			section.reset<true>(in.data(), in.size());
			const char** iter{ section.begin() };
			if JSONIFIER_UNLIKELY ((!*iter)) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Prettifying, prettify_errors::No_Input>(getUnderlyingPtr(iter) - in.data(),
					in.end() - in.begin(), in.data()));
				return false;
			}
			prettify_impl<optionsFinal, derived_type>::impl(iter, stringBuffer, prettifyPair, *this);
			if JSONIFIER_LIKELY ((prettifyPair.index != std::numeric_limits<uint32_t>::max())) {
				if JSONIFIER_LIKELY ((buffer.size() != prettifyPair.index)) {
					buffer.resize(prettifyPair.index);
				}
				std::memcpy(buffer.data(), stringBuffer.data(), prettifyPair.index);
				return true;
			} else {
				return false;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		struct prettify_pair {
			jsonifier::vector<json_structural_type> state{};
			const char* rootIter{};
			const char* endIter{};
			size_t indent{};
			size_t index{};
		} prettifyPair;

		JSONIFIER_ALWAYS_INLINE prettifier() noexcept : derivedRef{ initializeSelfRef() } {
			prettifyPair.state.resize(64);
		};

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		JSONIFIER_ALWAYS_INLINE ~prettifier() noexcept = default;
	};

}// namespace jsonifier_internal
