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

#include <jsonifier/SimdStructuralIterator.hpp>
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

	template<typename derived_type> class minifier {
	  public:
		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			= delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE auto minifyJson(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() / 4) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() / 4);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end(), in.size(), derivedRef.stringBuffer, derivedRef.errors };
			if (!iter) {
				iter.template createError<error_classes::Minifying>(minify_errors::No_Input);
				derivedRef.index = 0;
				return jsonifier::concepts::unwrap_t<string_type>{};
			}
			jsonifier::concepts::unwrap_t<string_type> newString{};
			if (derivedRef.index = impl(iter, derivedRef.stringBuffer); derivedRef.index == minifyError) {
				derivedRef.index = 0;
				return newString;
			} else {
				newString.resize(derivedRef.index);
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, newString.data());
			}
			derivedRef.index = 0;
			return newString;
		}

		template<jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_INLINE bool minifyJson(string_type01&& in, string_type02&& out) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() / 4) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() / 4);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end(), in.size(), derivedRef.stringBuffer, derivedRef.errors };
			if (!iter) {
				iter.template createError<error_classes::Minifying>(minify_errors::No_Input);
				derivedRef.index = 0;
				return false;
			}
			if (derivedRef.index = impl(iter, derivedRef.stringBuffer); derivedRef.index == minifyError) {
				derivedRef.index = 0;
				return false;
			} else {
				out.resize(derivedRef.index);
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, out.data());
			}
			derivedRef.index = 0;
			return true;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		template<jsonifier::concepts::string_t string_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type&& out) noexcept {
			return minify_impl<derived_type>::impl(std::forward<iterator_type>(iter), std::forward<string_type>(out));
		}

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_interna;