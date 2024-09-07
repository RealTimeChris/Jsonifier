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

#include <jsonifier/Validate_Impl.hpp>
#include <jsonifier/Serializer.hpp>
#include <jsonifier/Prettifier.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier {

	template<bool doWeUseInitialBuffer = true> class jsonifier_core : public jsonifier_internal::prettifier<jsonifier_core<doWeUseInitialBuffer>>,
																	  public jsonifier_internal::serializer<jsonifier_core<doWeUseInitialBuffer>>,
																	  public jsonifier_internal::validator<jsonifier_core<doWeUseInitialBuffer>>,
																	  public jsonifier_internal::minifier<jsonifier_core<doWeUseInitialBuffer>>,
																	  public jsonifier_internal::parser<jsonifier_core<doWeUseInitialBuffer>> {
	  public:
		friend class jsonifier_internal::prettifier<jsonifier_core<doWeUseInitialBuffer>>;
		friend class jsonifier_internal::serializer<jsonifier_core<doWeUseInitialBuffer>>;
		friend class jsonifier_internal::validator<jsonifier_core<doWeUseInitialBuffer>>;
		friend class jsonifier_internal::minifier<jsonifier_core<doWeUseInitialBuffer>>;
		friend class jsonifier_internal::parser<jsonifier_core<doWeUseInitialBuffer>>;

		JSONIFIER_ALWAYS_INLINE jsonifier_core() noexcept = default;

		JSONIFIER_ALWAYS_INLINE jsonifier_core& operator=(jsonifier_core&& other) noexcept {
			if (this != &other) [[likely]] {
				errors		 = std::move(other.errors);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE jsonifier_core(jsonifier_core&& other) noexcept : prettifier{}, serializer{}, validator{}, minifier{}, parser{} {
			*this = std::move(other);
		};

		JSONIFIER_ALWAYS_INLINE jsonifier_core& operator=(const jsonifier_core& other) noexcept {
			if (this != &other) [[likely]] {
				errors		 = other.errors;
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE jsonifier_core(const jsonifier_core& other) noexcept : prettifier{}, serializer{}, validator{}, minifier{}, parser{} {
			*this = other;
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::vector<jsonifier_internal::error>& getErrors() noexcept {
			return errors;
		}

		JSONIFIER_ALWAYS_INLINE ~jsonifier_core() noexcept = default;

	  protected:
		using prettifier = jsonifier_internal::prettifier<jsonifier_core<doWeUseInitialBuffer>>;
		using serializer = jsonifier_internal::serializer<jsonifier_core<doWeUseInitialBuffer>>;
		using validator	 = jsonifier_internal::validator<jsonifier_core<doWeUseInitialBuffer>>;
		using minifier	 = jsonifier_internal::minifier<jsonifier_core<doWeUseInitialBuffer>>;
		using parser	 = jsonifier_internal::parser<jsonifier_core<doWeUseInitialBuffer>>;

		vector<jsonifier_internal::error> errors{};
		uint64_t index{};
	};

}

