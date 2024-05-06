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
		prettify_options prettifyOptions{};
		bool prettify{ false };
	};
}

namespace jsonifier_internal {

	enum class serialize_errors { Success = 0 };

	template<uint64_t maxDepth> struct prettify_arguments {
		json_structural_type state[maxDepth]{};
		int64_t indent{};
	};

	template<jsonifier::serialize_options options, typename derived_type, typename value_type> struct serialize_impl;

	template<typename derived_type> class serializer {
	  public:
		template<jsonifier::serialize_options options, typename derived_type_new, typename value_type> friend struct serialize_impl;

		JSONIFIER_INLINE serializer& operator=(const serializer& other) = delete;
		JSONIFIER_INLINE serializer(const serializer& other)			= delete;

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
		JSONIFIER_INLINE bool serializeJson(value_type&& object, buffer_type&& out) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			derivedRef.errors.clear();
			derivedRef.index = 0;
			if constexpr (options.prettify) {
				prettify_arguments<options.prettifyOptions.maxDepth> prettifyArgs{};
				impl<options>(std::forward<value_type>(object), out, derivedRef.index, &prettifyArgs);
			} else {
				impl<options>(std::forward<value_type>(object), out, derivedRef.index, nullptr);
			}
			if (out.size() != derivedRef.index) {
				out.resize(derivedRef.index);
			}
			return true;
		}

		template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type>
		JSONIFIER_INLINE jsonifier::string serializeJson(value_type&& object) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			derivedRef.index = 0;
			derivedRef.errors.clear();
			jsonifier::string newString{};
			if constexpr (options.prettify) {
				prettify_arguments<options.prettifyOptions.maxDepth> prettifyArgs{};
				impl<options>(std::forward<value_type>(object), newString, derivedRef.index, &prettifyArgs);
			} else {
				impl<options>(std::forward<value_type>(object), newString, derivedRef.index, nullptr);
			}
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

		template<jsonifier::serialize_options options, typename value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				serialize_impl<options, derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index), prettifyArgs, value.jsonifierExcludedKeys);
			} else {
				serialize_impl<options, derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index), prettifyArgs);
			}
		}

		JSONIFIER_INLINE ~serializer() noexcept = default;
	};

}