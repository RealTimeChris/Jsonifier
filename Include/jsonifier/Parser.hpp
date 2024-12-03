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
/// Feb 3, 2023
#pragma once

#include <jsonifier/Validator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<size_t currentIndex, size_t maxIndex, typename value_type> constexpr size_t collectLongestKeyLength(size_t currentMax = 0) {
		if constexpr (currentIndex < maxIndex) {
			auto newMember = get<currentIndex>(coreTupleV<value_type>);
			if constexpr (jsonifier::concepts::has_member_t<decltype(newMember)>) {
				using member_type = decltype(newMember)::member_type;
				if constexpr (core_tuple_type<std::remove_cvref_t<member_type>>) {
					constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<std::remove_cvref_t<member_type>>>;
					currentMax				 = collectLongestKeyLength<0, tupleSize, std::remove_cvref_t<member_type>>(currentMax);
				}
			}
			currentMax = currentMax > keyStatsVal<value_type>.maxLength ? currentMax : keyStatsVal<value_type>.maxLength;
			return collectLongestKeyLength<currentIndex + 1, maxIndex, value_type>(currentMax);
		}
		return currentMax;
	}

	template<typename value_type> constexpr size_t collectLongestStringLength(size_t currentMax = 0) {
		if constexpr (core_tuple_type<std::remove_cvref_t<value_type>>) {
			constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<value_type>>;
			currentMax				 = collectLongestKeyLength<0, tupleSize, value_type>(currentMax);
		}
		return currentMax;
	}

	template<typename derived_type> class parser;

	template<typename derived_type> struct parse_context {
		JSONIFIER_ALWAYS_INLINE constexpr parse_context() noexcept = default;
		JSONIFIER_ALWAYS_INLINE constexpr parse_context(const char* iterNew, const char* endNew) noexcept {
			rootIter = iterNew;
			iter	 = iterNew;
			endIter	 = endNew;
		}
		parser<derived_type>* parserPtr{};
		int64_t currentObjectDepth{};
		int64_t currentArrayDepth{};
		const char* rootIter{};
		const char* endIter{};
		const char* iter{};
	};

	template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE const char* getEndIter(value_type value) noexcept {
		return reinterpret_cast<const char*>(char_comparison<'\0', decltype(*value)>::memchar(value, std::numeric_limits<size_t>::max()));
	}

	template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE const char* getBeginIter(value_type value) noexcept {
		return reinterpret_cast<const char*>(value);
	}

	template<jsonifier::concepts::has_data value_type> JSONIFIER_ALWAYS_INLINE const char* getEndIter(value_type& value) noexcept {
		return reinterpret_cast<const char*>(value.data() + value.size());
	}

	template<jsonifier::concepts::has_data value_type> JSONIFIER_ALWAYS_INLINE const char* getBeginIter(value_type& value) noexcept {
		return reinterpret_cast<const char*>(value.data());
	}

	template<bool minified, jsonifier::parse_options, typename value_type, typename buffer_type, typename parse_context_type> struct parse_impl;

	template<bool minified, jsonifier::parse_options options> struct parse {
		template<typename buffer_type, typename value_type, typename parse_context_type>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, parse_context_type&& iter) noexcept {
			parse_impl<minified, options, std::remove_cvref_t<value_type>, buffer_type, parse_context_type>::impl(value, iter);
		}
	};

	template<typename derived_type_new> class parser {
	  public:
		using derived_type = derived_type_new;
		template<bool minified, jsonifier::parse_options, typename value_type, typename buffer_type, typename parse_context_type> friend struct parse_impl;

		JSONIFIER_INLINE parser& operator=(const parser& other) = delete;
		JSONIFIER_INLINE parser(const parser& other)			= delete;

		template<jsonifier::parse_options options = jsonifier::parse_options{}, typename value_type, typename buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr jsonifier::parse_options optionsNew{ options };
			parse_context<derived_type> optionsReal{};
			optionsReal.rootIter  = getBeginIter(in);
			optionsReal.iter	  = optionsReal.rootIter;
			optionsReal.endIter	  = getEndIter(in);
			optionsReal.parserPtr = this;
			auto newSize		  = static_cast<uint64_t>((optionsReal.endIter - optionsReal.iter) / 2);
			if (stringBuffer.size() < newSize) {
				stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (!optionsReal.iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::No_Input>(optionsReal);
				return false;
			}
			parse<options.minified, optionsNew>::template impl<buffer_type>(object, optionsReal);
			static constexpr auto sourceLocation{ std::source_location::current() };
			return (optionsReal.currentObjectDepth != 0)							  ? (reportError<sourceLocation, parse_errors::Imbalanced_Object_Braces>(optionsReal), false)
				: (optionsReal.currentArrayDepth != 0)								  ? (reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(optionsReal), false)
				: (optionsReal.iter < optionsReal.endIter && !optionsNew.partialRead) ? (reportError<sourceLocation, parse_errors::Unfinished_Input>(optionsReal), false)
				: derivedRef.errors.size() > 0										  ? false
																					  : true;
		}
		
		template<jsonifier::parse_options options = jsonifier::parse_options{}, typename value_type, jsonifier::concepts::is_resizable buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr jsonifier::parse_options optionsNew{ options };
			static constexpr auto longestKeyLength{ collectLongestStringLength<value_type>() };
			size_t originalSize{ in.size() };
			in.reserve(in.size() + longestKeyLength);
			parse_context<derived_type> optionsReal{};
			optionsReal.rootIter  = getBeginIter(in);
			optionsReal.iter	  = optionsReal.rootIter;
			optionsReal.endIter	  = getEndIter(in);
			optionsReal.parserPtr = this;
			auto newSize		  = static_cast<uint64_t>((optionsReal.endIter - optionsReal.iter) / 2);
			if (stringBuffer.size() < newSize) {
				stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					in.resize(originalSize);
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (!optionsReal.iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::No_Input>(optionsReal);
				in.resize(originalSize);
				return false;
			}
			parse<options.minified, optionsNew>::template impl<buffer_type>(object, optionsReal);
			static constexpr auto sourceLocation{ std::source_location::current() };
			return (optionsReal.currentObjectDepth != 0) ? (reportError<sourceLocation, parse_errors::Imbalanced_Object_Braces>(optionsReal), in.resize(originalSize), false)
				: (optionsReal.currentArrayDepth != 0)	 ? (reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(optionsReal), in.resize(originalSize), false)
				: (optionsReal.iter < optionsReal.endIter && !optionsNew.partialRead)
				? (reportError<sourceLocation, parse_errors::Unfinished_Input>(optionsReal), in.resize(originalSize), false)
				: derivedRef.errors.size() > 0 ? (in.resize(originalSize), false)
											   : (in.resize(originalSize), true);
		}

		template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_INLINE value_type parseJson(buffer_type&& in) noexcept {
			static constexpr jsonifier::parse_options optionsNew{ options };
			parse_context<derived_type> optionsReal{};
			optionsReal.rootIter  = getBeginIter(in);
			optionsReal.iter	  = optionsReal.rootIter;
			optionsReal.endIter	  = getEndIter(in);
			optionsReal.parserPtr = this;
			auto newSize		  = static_cast<uint64_t>((optionsReal.endIter - optionsReal.iter) / 2);
			if (stringBuffer.size() < newSize) {
				stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if JSONIFIER_UNLIKELY (!derivedRef.validateJson(in)) {
					return value_type{};
				}
			}
			derivedRef.errors.clear();
			std::remove_cvref_t<value_type> object{};
			if JSONIFIER_UNLIKELY (!optionsReal.iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::No_Input>(optionsReal);
				return std::move(object);
			}
			parse<options.minified, optionsNew>::template impl<buffer_type>(object, optionsReal);
			static constexpr auto sourceLocation{ std::source_location::current() };
			return (optionsReal.currentObjectDepth != 0) ? (reportError<sourceLocation, parse_errors::Imbalanced_Object_Braces>(optionsReal), std::remove_cvref_t<value_type>{})
				: (optionsReal.currentArrayDepth != 0)	 ? (reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(optionsReal), std::remove_cvref_t<value_type>{})
				: (optionsReal.iter < optionsReal.endIter && !optionsNew.partialRead)
				? (reportError<sourceLocation, parse_errors::Unfinished_Input>(optionsReal), std::remove_cvref_t<value_type>{})
				: derivedRef.errors.size() > 0 ? std::remove_cvref_t<value_type>{}
											   : object;
		}

		template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, jsonifier::concepts::is_resizable buffer_type>
		JSONIFIER_INLINE value_type parseJson(buffer_type&& in) noexcept {
			static constexpr jsonifier::parse_options optionsNew{ options };
			static constexpr auto longestKeyLength{ collectLongestStringLength<value_type>() };
			size_t originalSize{ in.size() };
			in.reserve(in.size() + longestKeyLength);
			parse_context<derived_type> optionsReal{};
			optionsReal.rootIter  = getBeginIter(in);
			optionsReal.iter	  = optionsReal.rootIter;
			optionsReal.endIter	  = getEndIter(in);
			optionsReal.parserPtr = this;
			auto newSize		  = static_cast<uint64_t>((optionsReal.endIter - optionsReal.iter) / 2);
			if (stringBuffer.size() < newSize) {
				stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if JSONIFIER_UNLIKELY (!derivedRef.validateJson(in)) {
					in.resize(originalSize);
					return value_type{};
				}
			}
			derivedRef.errors.clear();
			std::remove_cvref_t<value_type> object{};
			if JSONIFIER_UNLIKELY (!optionsReal.iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::No_Input>(optionsReal);
				in.resize(originalSize);
				return object;
			}
			parse<options.minified, optionsNew>::template impl<buffer_type>(object, optionsReal);
			static constexpr auto sourceLocation{ std::source_location::current() };
			return (optionsReal.currentObjectDepth != 0) ? (reportError<sourceLocation, parse_errors::Imbalanced_Object_Braces>(optionsReal), std::remove_cvref_t<value_type>{})
				: (optionsReal.currentArrayDepth != 0)	 ? (reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(optionsReal), std::remove_cvref_t<value_type>{})
				: (optionsReal.iter < optionsReal.endIter && !optionsNew.partialRead)
				? (reportError<sourceLocation, parse_errors::Unfinished_Input>(optionsReal), std::remove_cvref_t<value_type>{})
				: derivedRef.errors.size() > 0 ? (in.resize(originalSize), std::remove_cvref_t<value_type>{})
											   : (in.resize(originalSize), object);
		}

		template<const auto& sourceLocation, auto parseError> JSONIFIER_INLINE void reportError(parse_context<derived_type>& context) noexcept {
			derivedRef.errors.emplace_back(
				error::constructError<sourceLocation, error_classes::Parsing, parseError>(context.iter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~parser() noexcept = default;
	};
};