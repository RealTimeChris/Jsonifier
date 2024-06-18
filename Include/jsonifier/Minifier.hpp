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

#include <jsonifier/JsonStructuralIterator.hpp>
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

	constexpr uint32_t minifyError{ std::numeric_limits<uint32_t>::max() };

	template<typename derived_type> struct minify_impl;

	template<typename derived_type> class minifier;

	template<typename derived_type> struct minify_options_internal {
		mutable minifier<derived_type>* minifierPtr{};
	};

	template<typename derived_type> class minifier {
	  public:
		template<typename derived_type_new> friend struct minify_impl;

		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			= delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE auto minifyJson(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() / 4) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() / 4);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.reset(in.data(), in.size());
			static constexpr minify_options_internal<derived_type> options{};
			options.minifierPtr = this;
			json_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end() };
			if (!iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::No_Input>(static_cast<int64_t>(iter - iter.getRootPtr()),
					static_cast<int64_t>(iter.getEndPtr() - iter.getRootPtr()), iter.getRootPtr()));
				return jsonifier::concepts::unwrap_t<string_type>{};
			}
			jsonifier::concepts::unwrap_t<string_type> newString{};
			minify_impl<derived_type>::template impl<options>(iter, derivedRef.stringBuffer, derivedRef.index);
			if (derivedRef.index == minifyError) {
				return newString;
			} else {
				newString.resize(derivedRef.index);
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, newString.data());
			}
			return newString;
		}

		template<jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_INLINE bool minifyJson(string_type01&& in, string_type02&& out) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() / 4) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() / 4);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.reset(in.data(), in.size());
			static constexpr minify_options_internal<derived_type> options{};
			options.minifierPtr = this;
			json_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end() };
			if (!iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Minifying, minify_errors::No_Input>(static_cast<int64_t>(iter - iter.getRootPtr()),
					static_cast<int64_t>(iter.getEndPtr() - iter.getRootPtr()), iter.getRootPtr()));
				return false;
			}
			minify_impl<derived_type>::template impl<options>(iter, derivedRef.stringBuffer, derivedRef.index);
			if (derivedRef.index == minifyError) {
				return false;
			} else {
				if (out.size() != derivedRef.index) {
					out.resize(derivedRef.index);
				}
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, out.data());
			}
			return true;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE jsonifier::vector<error>& getErrors() {
			return derivedRef.errors;
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_interna;