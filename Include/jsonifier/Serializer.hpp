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

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct object_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct array_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct string_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct number_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct bool_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct null_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct accessor_val_serializer;

	template<typename value_type, typename context_type, jsonifier::serialize_options optionsNew, typename json_entity_type> struct custom_val_serializer;

	template<jsonifier::serialize_options options, typename json_entity_type> struct serialize {
		template<typename value_type, typename context_type> JSONIFIER_CLANG_INLINE static void impl(value_type&& value, context_type&& context) noexcept {
			if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::object) {
				object_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::array) {
				array_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::string) {
				string_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::number) {
				number_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::boolean) {
				bool_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::null) {
				null_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else if constexpr (getJsonTypeFromEntity<std::remove_cvref_t<json_entity_type>, std::remove_cvref_t<value_type>>() == jsonifier::json_type::custom) {
				custom_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			} else {
				accessor_val_serializer<std::remove_cvref_t<value_type>, context_type, options, json_entity_type>::impl(jsonifier_internal::forward<value_type>(value),
					jsonifier_internal::forward<context_type>(context));
			}
		}
	};

	template<typename buffer_type> struct serialize_context {
		JSONIFIER_INLINE serialize_context() noexcept = default;

		JSONIFIER_INLINE serialize_context(char* ptrNew, buffer_type& bufferNew) noexcept : buffer{ bufferNew }, bufferPtr{ ptrNew } {};

		buffer_type& buffer{};
		char* bufferPtr{};
		size_t indent{};
		size_t index{};
	};

	template<typename derived_type> class serializer {
	  public:
		serializer& operator=(const serializer& other) = delete;
		serializer(const serializer& other)			   = delete;

		template<jsonifier::serialize_options optionsNew = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
		bool serializeJson(value_type&& object, buffer_type&& buffer) noexcept {
			static constexpr jsonifier::serialize_options options{ optionsNew };
			serialize_context<decltype(stringBuffer)> context{ stringBuffer.data(), stringBuffer };
			serialize<options, jsonifier_internal::base_json_entity<value_type>>::impl(object, context);
			context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
			buffer.resize(context.index);
			std::memcpy(buffer.data(), stringBuffer.data(), context.index);
			return true;
		}

		template<jsonifier::serialize_options optionsNew = jsonifier::serialize_options{}, typename value_type> std::string_view serializeJson(value_type&& object) noexcept {
			static constexpr jsonifier::serialize_options options{ optionsNew };
			serialize_context<decltype(stringBuffer)> context{ stringBuffer.data(), stringBuffer };
			serialize<options, jsonifier_internal::base_json_entity<value_type>>::impl(object, context);
			context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
			return std::string_view{ stringBuffer.data(), context.index };
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		serializer() noexcept : derivedRef{ initializeSelfRef() } {};

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		~serializer() noexcept = default;
	};

}