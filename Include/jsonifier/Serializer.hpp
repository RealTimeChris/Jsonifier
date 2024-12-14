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

namespace jsonifier_internal {

	enum class serialize_errors { Success = 0 };

	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct object_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct array_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct string_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct number_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct bool_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct null_val_serializer;
	template<jsonifier::serialize_options optionsNew, typename buffer_type, typename index_type, typename indent_type> struct accessor_val_serializer;

	template<jsonifier::serialize_options optionsNew> struct serialize {
		template<typename value_type, typename buffer_type, typename index_type, typename indent_type>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, indent_type&& indent) noexcept {
			if constexpr (jsonifier::concepts::jsonifier_object_t<value_type> || jsonifier::concepts::map_t<value_type>) {
				object_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else if constexpr (jsonifier::concepts::vector_t<value_type> || jsonifier::concepts::raw_array_t<value_type>) {
				array_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else if constexpr (jsonifier::concepts::string_t<value_type> || jsonifier::concepts::char_t<value_type>) {
				string_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else if constexpr (jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type>) {
				number_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else if constexpr (jsonifier::concepts::bool_t<value_type>) {
				bool_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
				null_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			} else {
				accessor_val_serializer<optionsNew, buffer_type, index_type, indent_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<buffer_type>(buffer), jsonifier_internal::forward<index_type>(index), jsonifier_internal::forward<indent_type>(indent));
			}
		}
	};

	template<typename derived_type> class serializer {
	  public:
		JSONIFIER_ALWAYS_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_ALWAYS_INLINE serializer(const serializer& other)			   = delete;

		template<jsonifier::serialize_options optionsNew = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_ALWAYS_INLINE bool serializeJson(value_type&& object, buffer_type&& buffer) noexcept {
			static constexpr jsonifier::serialize_options options{ optionsNew };
			indent = 0;
			index  = 0;
			serialize<options>::impl(std::forward<value_type>(object), stringBuffer, index, indent);
			buffer.resize(index);
			std::memcpy(buffer.data(), stringBuffer.data(), index);
			return true;
		}

		template<jsonifier::serialize_options optionsNew = jsonifier::serialize_options{}, typename value_type>
		JSONIFIER_ALWAYS_INLINE std::string_view serializeJson(value_type&& object) noexcept {
			static constexpr jsonifier::serialize_options options{ optionsNew };
			indent = 0;
			index  = 0;
			serialize<options>::impl(std::forward<value_type>(object), stringBuffer, index, indent);
			return std::string_view{ stringBuffer.data(), index };
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		size_t indent{};
		size_t index{};

		JSONIFIER_ALWAYS_INLINE serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE ~serializer() noexcept = default;
	};

}