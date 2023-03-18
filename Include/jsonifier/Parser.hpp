/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Simd.hpp>
#include <jsonifier/IteratorCore.hpp>
#include <jsonifier/String.hpp>

namespace Jsonifier {

	struct Read;

	class Parser {
	  public:
		inline Parser() = default;

		inline Parser& operator=(Parser&& other) noexcept {
			this->inString = std::move(other.inString);
			this->section = std::move(other.section);
			return *this;
		};

		inline Parser(Parser&& other) noexcept {
			*this = std::move(other);
		};

		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&) = delete;

		inline Parser(std::string_view string) {
			this->reset(string);
		}

		inline Parser(const std::string& string) {
			this->reset(string);
		}

		inline SimdIteratorCore begin() noexcept {
			return { &section };
		}

		inline SimdIteratorCore end() noexcept {
			return { &section };
		}

		template<typename OTy, typename OTy2> void parseJson(OTy& json, OTy2& inStringNew) {
			if (inString != inStringNew && inStringNew.size() != 0) {
				reset(inStringNew);
			}
			auto newIter = this->begin();
			auto endIter = this->end();
			Read::op<OTy>(json, newIter, endIter);
		}

	  protected:
		SimdStringReader section{};
		StringView inString{};

		void generateJsonIndices() noexcept {
			if (inString.data()) {
				StringBlockReader<256> stringReader{ reinterpret_cast<StringViewPtr>(inString.data()), inString.size() };
				while (stringReader.hasFullBlock()) {
					section.generateStructurals(stringReader.fullBlock());
				}
				uint8_t block[256];
				stringReader.getRemainder(block);
				section.generateStructurals(block);
			}
		}

		inline void reset(std::string_view string) {
			if (string.size() == 0) {
				return;
			}
			inString = StringView{ string.data(), string.size() };
			this->section.reset(inString.size(), reinterpret_cast<StringViewPtr>(inString.data()));
			this->generateJsonIndices();
		}

		inline void reset(const std::string& string) {
			if (string.size() == 0) {
				return;
			}
			inString = StringView{ string.data(), string.size() };
			this->section.reset(inString.size(), reinterpret_cast<StringViewPtr>(inString.data()));
			this->generateJsonIndices();
		}
	};

	inline void skipObject(SimdIteratorCore& it, SimdIteratorCore& end) noexcept {
		++it;
		size_t openCount{ 1 };
		size_t closeCount{};
		if (**it == '}') {
			++it;
			return;
		}
		while (openCount > closeCount && it != end) {
			++it;
			switch (**it) {
				case '{': {
					++openCount;
					break;
				}
				case '[': {
					++openCount;
					break;
				}
				case ']': {
					++closeCount;
					break;
				}
				case '}': {
					++closeCount;
					break;
				}
				default: {
					break;
				}
			}
		}
		++it;
	}

	inline void skipArray(SimdIteratorCore& it, SimdIteratorCore& end) noexcept {
		++it;
		size_t openCount{ 1 };
		size_t closeCount{};
		if (**it == ']') {
			++it;
			return;
		}
		while (openCount > closeCount && it != end) {
			++it;
			switch (**it) {
				case '{': {
					++openCount;
					break;
				}
				case '[': {
					++openCount;
					break;
				}
				case ']': {
					++closeCount;
					break;
				}
				case '}': {
					++closeCount;
					break;
				}
				default: {
					break;
				}
			}
		}
		++it;
	}

	inline void skipWhitespace(SimdIteratorCore& it, SimdIteratorCore& end) noexcept {
		while (std::isspace(**it)) {
			++it;
		}
	}

	inline void skipValue(SimdIteratorCore& it, SimdIteratorCore& end) noexcept {
		switch (**it) {
			case '{': {
				skipObject(it, end);
				break;
			}
			case '[': {
				skipArray(it, end);
				break;
			}
			case '"': {
				++it;
				break;
			}
			case 'n': {
				++it;
				break;
			}
			case 'f': {
				++it;
				break;
			}
			case 't': {
				++it;
				break;
			}
			case '\0': {
				break;
			}
			default: {
				++it;
				++it;
			}
		}
	}

};
