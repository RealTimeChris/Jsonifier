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

	enum class ascii_classes {
		errorVal	= -1,
		quote		= 0,
		colon		= 1,
		comma		= 2,
		lsqrb		= 3,
		rsqrb		= 4,
		false_val	= 5,
		null_val	= 6,
		true_val	= 7,
		lcurb		= 8,
		rcurb		= 9,
		num_val		= 10,
		class_count = 11
	};

	constexpr ascii_classes asciiClassesMap[]{ ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::quote, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::comma, ascii_classes::num_val, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::num_val,
		ascii_classes::num_val, ascii_classes::num_val, ascii_classes::num_val, ascii_classes::num_val, ascii_classes::num_val, ascii_classes::num_val, ascii_classes::num_val,
		ascii_classes::num_val, ascii_classes::num_val, ascii_classes::colon, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::lsqrb, ascii_classes::errorVal, ascii_classes::rsqrb,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::false_val, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::null_val, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::true_val, ascii_classes::errorVal,
		ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::errorVal, ascii_classes::lcurb, ascii_classes::errorVal,
		ascii_classes::rcurb, ascii_classes::errorVal, ascii_classes::errorVal };

	static constexpr jsonifier::string_view falseString{ "false" };
	static constexpr jsonifier::string_view trueString{ "true" };
	static constexpr jsonifier::string_view nullString{ "null" };

	template<typename iterator_type> void appendCharacter(auto character, iterator_type& outPtr) {
		*outPtr = static_cast<std::remove_pointer_t<jsonifier::concepts::unwrap_t<iterator_type>>>(character);
		++outPtr;
	}

	struct minify {
		template<jsonifier::concepts::string_t string_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type& out) noexcept;
	};

	template<typename derived_type> class minifier {
	  public:
		JSONIFIER_INLINE minifier& operator=(const minifier& other) = delete;
		JSONIFIER_INLINE minifier(const minifier& other)			= delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE auto minify(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() * 2) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() * 2);
			}
			derivedRef.index = 0; 
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.getStringView(), derivedRef.stringBuffer, derivedRef.errors };
			derivedRef.index = minify::impl(iter, derivedRef.stringBuffer);
			if constexpr (jsonifier::concepts::has_resize<string_type>) {
				jsonifier::concepts::unwrap_t<string_type> newString{};
				if (derivedRef.index < std::numeric_limits<uint64_t>::max()) [[likely]] {
					newString.resize(derivedRef.index);
					std::memcpy(newString.data(), derivedRef.stringBuffer.data(), derivedRef.index);
				}
				return newString;
			} else {
				jsonifier::string newString{};
				if (derivedRef.index < std::numeric_limits<uint64_t>::max()) [[likely]] {
					newString.resize(derivedRef.index);
					std::memcpy(newString.data(), derivedRef.stringBuffer.data(), derivedRef.index);
				}
				return newString;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE minifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~minifier() noexcept = default;
	};

}// namespace jsonifier_internal
