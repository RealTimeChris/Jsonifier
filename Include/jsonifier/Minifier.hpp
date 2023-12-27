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
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	constexpr json_structural_type asciiClassesMap[]{ json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::String, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Comma, json_structural_type::Number, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Number,
		json_structural_type::Number, json_structural_type::Number, json_structural_type::Number, json_structural_type::Number, json_structural_type::Number,
		json_structural_type::Number, json_structural_type::Number, json_structural_type::Number, json_structural_type::Number, json_structural_type::Colon,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Array_Start, json_structural_type::Unset, json_structural_type::Array_End,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Bool, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Null,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Bool,
		json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset, json_structural_type::Unset,
		json_structural_type::Unset, json_structural_type::Object_Start, json_structural_type::Unset, json_structural_type::Object_End, json_structural_type::Unset,
		json_structural_type::Unset };

	static constexpr jsonifier::string_view falseString{ "false" };
	static constexpr jsonifier::string_view trueString{ "true" };
	static constexpr jsonifier::string_view nullString{ "null" };

	template<auto character, typename iterator_type> void appendCharacter(iterator_type& outPtr) {
		*outPtr = static_cast<std::remove_pointer_t<jsonifier::concepts::unwrap_t<iterator_type>>>(character);
		++outPtr;
	}

	template<typename derived_type> struct minify_impl;

	template<typename derived_type> class minifier {
	  public:
		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			= delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE auto minify(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() * 2) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() * 2);
			}
			bool newBuffer{ false };
			if (derivedRef.index != 0) {
				newBuffer = true;
			}
			derivedRef.index = 0;
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.getStringView(), derivedRef.stringBuffer, derivedRef.errors };
			derivedRef.index = impl(iter, derivedRef.stringBuffer);
			if (newBuffer) {
				auto oldIndex	 = derivedRef.index;
				derivedRef.index = 0;
				if constexpr (jsonifier::concepts::has_resize<string_type>) {
					jsonifier::concepts::unwrap_t<string_type> newString{};
					if (oldIndex < std::numeric_limits<uint32_t>::max()) [[likely]] {
						newString.resize(oldIndex);
						std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + oldIndex, newString.data());
					}
					return newString;
				} else {
					jsonifier::concepts::unwrap_t<string_type> newString{};
					if (oldIndex < std::numeric_limits<uint32_t>::max()) [[likely]] {
						newString.resize(oldIndex);
						std::copy(derivedRef.stringBuffer.data(), derivedRef.stringBuffer.data() + oldIndex, newString.data());
					}
					return newString;
				}
			} else {
				if (derivedRef.index < std::numeric_limits<uint32_t>::max()) {
					auto oldIndex	 = derivedRef.index;
					derivedRef.index = 0;
					return jsonifier::concepts::unwrap_t<string_type>{ derivedRef.stringBuffer.data(), oldIndex };
				} else {
					return jsonifier::concepts::unwrap_t<string_type>{};
				}
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		template<jsonifier::concepts::string_t string_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type& out) noexcept {
			return minify_impl<derived_type>::impl(iter, out);
		}

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_internal