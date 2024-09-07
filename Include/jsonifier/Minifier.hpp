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
/// Feb 3, 2023
#pragma once

#include <jsonifier/Write.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	enum class minify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Invalid_String_Length	   = 2,
		Invalid_Number_Value	   = 3,
		Incorrect_Structural_Index = 4,
	};

	template<typename derived_type> struct minify_impl;

	template<typename derived_type> class minifier {
	  public:
		template<typename derived_type_new> friend struct minify_impl;

		JSONIFIER_ALWAYS_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_ALWAYS_INLINE minifier(const minifier& other)			   = delete;

		template<jsonifier ::concepts::string_t string_type> JSONIFIER_ALWAYS_INLINE auto minifyJson(string_type&& in) noexcept {
			if (stringBuffer.size() < in.size()) [[unlikely]] {
				stringBuffer.resize(in.size());
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			rootIter = in.data();
			endIter	 = in.data() + in.size();
			section.reset(in.data(), in.size());
			const char** iter{ section.begin() };
			if (!*iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(
					error::constructError<sourceLocation, error_classes::Minifying, minify_errors::No_Input>(*iter - in.data(), in.end() - in.begin(), in.data()));
				return unwrap_t<string_type>{};
			}
			unwrap_t<string_type> newString{};
			minify_impl<derived_type>::impl(iter, stringBuffer, derivedRef.index, *this);
			if (derivedRef.index != std::numeric_limits<uint32_t>::max()) {
				newString.resize(derivedRef.index);
				std::copy(stringBuffer.data(), stringBuffer.data() + derivedRef.index, newString.data());
				return newString;
			} else {
				return unwrap_t<string_type>{};
			}
		}

		template<jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_ALWAYS_INLINE bool minifyJson(string_type01&& in, string_type02&& buffer) noexcept {
			if (stringBuffer.size() < in.size()) [[unlikely]] {
				stringBuffer.resize(in.size());
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			rootIter = in.data();
			endIter	 = in.data() + in.size();
			section.reset(in.data(), in.size());
			const char** iter{ section.begin() };
			if (!*iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(
					error::constructError<sourceLocation, error_classes::Minifying, minify_errors::No_Input>(*iter - in.data(), in.end() - in.begin(), in.data()));
				return false;
			}
			minify_impl<derived_type>::impl(iter, stringBuffer, derivedRef.index, *this);
			if (derivedRef.index != std::numeric_limits<uint32_t>::max()) [[likely]] {
				if (!compare(stringBuffer.data(), buffer.data(), derivedRef.index)) [[likely]] {
					if (buffer.size() != derivedRef.index) [[likely]] {
						buffer.resize(derivedRef.index);
					}
					std::copy(stringBuffer.data(), stringBuffer.data() + derivedRef.index, buffer.data());
				}
				return true;
			} else {
				return false;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		const char* rootIter{};
		const char* endIter{};

		JSONIFIER_ALWAYS_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		JSONIFIER_ALWAYS_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_internal
