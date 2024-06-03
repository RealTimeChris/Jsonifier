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
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Prettifier.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Write.hpp>

namespace jsonifier {

	struct serialize_options {
		bool newLinesInArray{ true };
		uint64_t indentSize{ 3 };
		char indentChar{ ' ' };
		bool prettify{ false };
	};
}

namespace jsonifier_internal {

	enum class serialize_errors { Success = 0 };

	struct serialize_options_internal {
		jsonifier::serialize_options optionsReal{};
		mutable uint64_t indent{};
	};

	template<const serialize_options_internal& options, typename derived_type, typename value_type> struct serialize_impl;

	template<typename derived_type> class serializer {
	  public:
		template<const serialize_options_internal& options, typename derived_type_new, typename value_type> friend struct serialize_impl;

		JSONIFIER_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_INLINE serializer(const serializer& other)			= delete;

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_INLINE bool serializeJson(value_type&& object, buffer_type&& buffer) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			static constexpr serialize_options_internal optionsFinal{ .optionsReal = options };
			derivedRef.errors.clear();
			derivedRef.index = 0;
			serialize_impl<optionsFinal, derived_type, value_type>::impl(std::forward<value_type>(object), buffer, derivedRef.index);
			if (buffer.size() != derivedRef.index) {
				buffer.resize(derivedRef.index);
			}
			return true;
		}

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type> JSONIFIER_INLINE jsonifier::string serializeJson(value_type&& object) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			derivedRef.index = 0;
			derivedRef.errors.clear();
			jsonifier::string newString{};
			static constexpr serialize_options_internal optionsFinal{ .optionsReal = options };
			derivedRef.errors.clear();
			derivedRef.index = 0;
			serialize_impl<optionsFinal, derived_type, value_type>::impl(std::forward<value_type>(object), newString, derivedRef.index);
			if (derivedRef.index != minifyError) [[likely]] {
				newString.resize(derivedRef.index);
			}
			derivedRef.index = 0;
			return newString;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~serializer() noexcept = default;
	};

}