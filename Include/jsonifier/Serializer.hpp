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
		size_t indentSize{ 3 };
		char indentChar{ ' ' };
		bool prettify{ false };
	};
}

namespace jsonifier_internal {

	enum class serialize_errors { Success = 0 };

	template<jsonifier::serialize_options options, typename value_type_new, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type> struct serialize_impl;

	template<const auto options> struct serialize {
		template<typename value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_context_type&& iter) {
			serialize_impl<options, std::remove_cvref_t<value_type>, std::remove_cvref_t<buffer_type>, serialize_context_type>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
				std::forward<serialize_context_type>(iter));
		}
	};

	template<typename derived_type> class serializer {
	  public:
		template<jsonifier::serialize_options options, typename value_type_new, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
		friend struct serialize_impl;

		JSONIFIER_ALWAYS_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_ALWAYS_INLINE serializer(const serializer& other)			   = delete;

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_ALWAYS_INLINE bool serializeJson(value_type&& object, buffer_type&& buffer) noexcept {
			static constexpr jsonifier::serialize_options optionsFinal{ options };
			derivedRef.errors.clear();
			serializePair.index	 = 0;
			serializePair.indent = 0;
			serialize<optionsFinal>::impl(std::forward<value_type>(object), stringBuffer, serializePair);
			if JSONIFIER_UNLIKELY ((buffer.size() != serializePair.index)) {
				buffer.resize(serializePair.index);
			}
			if (!comparison<0, std::remove_cvref_t<decltype(*buffer.data())>, std::remove_cvref_t<decltype(*stringBuffer.data())>>::compare(buffer.data(), stringBuffer.data(), serializePair.index)) {
				std::copy(stringBuffer.data(), stringBuffer.data() + serializePair.index, buffer.data());
			}
			return true;
		}

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type>
		JSONIFIER_ALWAYS_INLINE jsonifier::string serializeJson(value_type&& object) noexcept {
			derivedRef.errors.clear();
			serializePair.index	 = 0;
			serializePair.indent = 0;
			jsonifier::string newString{};
			static constexpr jsonifier::serialize_options optionsFinal{ options };
			serialize<optionsFinal>::impl(std::forward<value_type>(object), stringBuffer, serializePair);
			if JSONIFIER_UNLIKELY ((newString.size() != serializePair.index)) {
				newString.resize(serializePair.index);
			}
			std::memcpy(newString.data(), stringBuffer.data(), serializePair.index);
			return newString;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		struct serialize_pair {
			size_t indent{};
			size_t index{};
		} serializePair{};

		JSONIFIER_ALWAYS_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE ~serializer() noexcept = default;
	};

}