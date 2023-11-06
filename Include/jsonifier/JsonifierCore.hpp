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

#include <jsonifier/Serializer.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier {

	class jsonifier_core : public jsonifier_internal::serializer<jsonifier_core>, public jsonifier_internal::parser<jsonifier_core> {
	  public:
		friend class jsonifier_internal::serialization_iterator<serializer<jsonifier_core>, jsonifier_core>;
		friend class jsonifier_internal::simd_structural_iterator<parser<jsonifier_core>, jsonifier_core>;
		friend class serializer<jsonifier_core>;
		friend class parser<jsonifier_core>;

		using serializer = jsonifier_internal::serializer<jsonifier_core>;
		using parser	 = jsonifier_internal::parser<jsonifier_core>;

		JSONIFIER_INLINE jsonifier_core() noexcept = default;

		JSONIFIER_INLINE jsonifier_core& operator=(jsonifier_core&& other) noexcept {
			stringBuffer = std::move(other.stringBuffer);
			section		 = std::move(other.section);
			currentIndex = other.currentIndex;
			currentSize	 = other.currentSize;
			rootIndex	 = other.rootIndex;
			errors		 = other.errors;
			return *this;
		}

		JSONIFIER_INLINE jsonifier_core(jsonifier_core&& other) noexcept : serializer{}, parser{} {
			*this = std::move(other);
		}

		JSONIFIER_INLINE jsonifier_core& operator=(const jsonifier_core& other) {
			stringBuffer = other.stringBuffer;
			currentIndex = other.currentIndex;
			currentSize	 = other.currentSize;
			rootIndex	 = other.rootIndex;
			section		 = other.section;
			errors		 = other.errors;
			return *this;
		}

		JSONIFIER_INLINE jsonifier_core(const jsonifier_core& other) : serializer{}, parser{} {
			*this = other;
		}

		JSONIFIER_INLINE jsonifier::vector<jsonifier_internal::error>& getErrors() {
			return errors;
		}

		JSONIFIER_INLINE ~jsonifier_core() noexcept = default;

	  protected:
		jsonifier_internal::buffer_string<uint8_t> stringBuffer{};
		jsonifier::vector<jsonifier_internal::error> errors{};
	};

}
