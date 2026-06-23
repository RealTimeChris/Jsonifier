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

#include <jsonifier-incl/parsing/validate_impl.hpp>
#include <jsonifier-incl/serializing/serializer.hpp>
#include <jsonifier-incl/serializing/prettifier.hpp>
#include <jsonifier-incl/parsing/parser.hpp>
#include <jsonifier-incl/utilities/error.hpp>
#include <jsonifier-incl/utilities/comparitor_impl.hpp>

namespace jsonifier {

	template<uint64_t initialBufferSize = 1024 * 1024> class jsonifier_core : public internal::comparator<jsonifier_core<initialBufferSize>>,
																			  public internal::prettifier<jsonifier_core<initialBufferSize>>,
																			  public internal::serializer<jsonifier_core<initialBufferSize>>,
																			  public internal::validator<jsonifier_core<initialBufferSize>>,
																			  public internal::minifier<jsonifier_core<initialBufferSize>>,
																			  public internal::parser<jsonifier_core<initialBufferSize>> {
	  public:
		friend class internal::comparator<jsonifier_core<initialBufferSize>>;
		friend class internal::prettifier<jsonifier_core<initialBufferSize>>;
		friend class internal::serializer<jsonifier_core<initialBufferSize>>;
		friend class internal::validator<jsonifier_core<initialBufferSize>>;
		friend class internal::minifier<jsonifier_core<initialBufferSize>>;
		friend class internal::parser<jsonifier_core<initialBufferSize>>;

		jsonifier_core() noexcept = default;

		jsonifier_core& operator=(jsonifier_core&& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				errors = internal::move(other.errors);
			}
			return *this;
		}

		jsonifier_core(jsonifier_core&& other) noexcept : prettifier{}, serializer{}, validator{}, minifier{}, parser{} {
			*this = internal::move(other);
		}

		jsonifier_core& operator=(const jsonifier_core& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				errors = other.errors;
			}
			return *this;
		}

		jsonifier_core(const jsonifier_core& other) noexcept : prettifier{}, serializer{}, validator{}, minifier{}, parser{} {
			*this = other;
		}

		const std::vector<internal::error>& getErrors() const noexcept {
			return errors;
		}

		~jsonifier_core() noexcept = default;

	  protected:
		using comparator = internal::comparator<jsonifier_core<initialBufferSize>>;
		using prettifier = internal::prettifier<jsonifier_core<initialBufferSize>>;
		using serializer = internal::serializer<jsonifier_core<initialBufferSize>>;
		using validator	 = internal::validator<jsonifier_core<initialBufferSize>>;
		using minifier	 = internal::minifier<jsonifier_core<initialBufferSize>>;
		using parser	 = internal::parser<jsonifier_core<initialBufferSize>>;

		internal::simd_string_reader<initialBufferSize> section{};
		string_base<char, initialBufferSize> stringBuffer{};
		std::vector<internal::error> errors{};
	};

}
