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

#include <jsonifier/Serialize_Impl.hpp>
#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/Minifier.hpp>
#include <jsonifier/Write.hpp>

namespace jsonifier {

	struct prettify_options {
		bool newLinesInArray{ true };
		uint64_t indentSize{ 3 };
		uint64_t maxDepth{ 100 };
		bool tabs{ false };
	};

}

namespace jsonifier_internal {

	enum class prettify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Exceeded_Max_Depth		   = 2,
		Incorrect_Structural_Index = 3,
	};

	template<typename derived_type> struct prettify_impl;

	template<typename derived_type> class prettifier {
	  public:
		JSONIFIER_INLINE prettifier& operator=(const prettifier& other) = delete;
		JSONIFIER_INLINE prettifier(const prettifier& other)			= delete;

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier ::concepts::string_t string_type>
		JSONIFIER_INLINE auto prettifyJson(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() * 6) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() * 6);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end(), in.size(), derivedRef.stringBuffer, derivedRef.errors };
			if (!iter) {
				iter.template createError<error_classes::Prettifying>(prettify_errors::No_Input);
				derivedRef.index = 0;
				return jsonifier::concepts::unwrap_t<string_type>{};
			}
			jsonifier::concepts::unwrap_t<string_type> newString{};
			derivedRef.index = impl<options>(iter, derivedRef.stringBuffer);
			if (derivedRef.index < std::numeric_limits<uint32_t>::max()) {
				newString.resize(derivedRef.index);
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, newString.data());
				derivedRef.index = 0;
				return newString;
			} else {
				derivedRef.index = 0;
				return jsonifier::concepts::unwrap_t<string_type>{};
			}
		}

		template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
		JSONIFIER_INLINE bool prettifyJson(string_type01&& in, string_type02&& out) noexcept {
			if (out.size() < in.size() * 6) [[unlikely]] {
				out.resize(in.size() * 6);
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.end(), in.size(), derivedRef.stringBuffer, derivedRef.errors };
			if (!iter) {
				iter.template createError<error_classes::Prettifying>(prettify_errors::No_Input);
				derivedRef.index = 0;
				return false;
			}
			derivedRef.index = impl<options>(iter, derivedRef.stringBuffer);
			if (derivedRef.index < std::numeric_limits<uint32_t>::max()) {
				out.resize(derivedRef.index);
				std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + derivedRef.index, out.data());
				derivedRef.index = 0;
				return true;
			} else {
				derivedRef.index = 0;
				return false;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE prettifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		template<jsonifier::prettify_options options, jsonifier::concepts::string_t string_type,
			jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type&& out) noexcept {
			return prettify_impl<derived_type>::template impl<options>(std::forward<iterator_type>(iter), std::forward<string_type>(out));
		}

		JSONIFIER_INLINE ~prettifier() noexcept = default;
	};

}// namespace jsonifier_internal
