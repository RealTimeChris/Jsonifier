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

namespace jsonifier_internal {

	struct prettify {
		template<bool newLinesInArray, bool tabs, uint64_t indentSize, uint64_t maxDepth, jsonifier::concepts::string_t string_type,
			jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static uint64_t impl(iterator_type&& iter, string_type& out) noexcept;
	};

	template<typename derived_type> class prettifier {
	  public:
		JSONIFIER_INLINE prettifier& operator=(const prettifier& other) = delete;
		JSONIFIER_INLINE prettifier(const prettifier& other)			= delete;

		template<bool newLinesInArray = true, bool tabs = false, uint64_t indentSize = 4, uint64_t maxDepth = 100, jsonifier ::concepts::string_t string_type>
		JSONIFIER_INLINE auto prettify(string_type&& in) noexcept {
			if (derivedRef.stringBuffer.size() < in.size() * 10) [[unlikely]] {
				derivedRef.stringBuffer.resize(in.size() * 10);
			}
			derivedRef.errors.clear();
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.stringBuffer, derivedRef.errors };
			uint64_t index{ prettify::impl<newLinesInArray, tabs, indentSize, maxDepth>(iter, derivedRef.stringBuffer) };
			if constexpr (jsonifier::concepts::has_resize<string_type>) {
				jsonifier::concepts::unwrap_t<string_type> newString{};
				if (index < std::numeric_limits<uint64_t>::max()) [[likely]] {
					newString.resize(index);
					std::memcpy(newString.data(), derivedRef.stringBuffer.data(), index);
				}
				return newString;
			} else {
				jsonifier::string newString{};
				if (index < std::numeric_limits<uint64_t>::max()) [[likely]] {
					newString.resize(index);
					std::memcpy(newString.data(), derivedRef.stringBuffer.data(), index);
				}
				return newString;
			}
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE prettifier() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~prettifier() noexcept = default;
	};

}// namespace jsonifier_internal
