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
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/IteratorCore.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Parse.hpp>
#include <jsonifier/Simd.hpp>

namespace Jsonifier {

	class Parser {
	  public:
		inline Parser() = default;

		inline Parser& operator=(Parser&& other) noexcept {
			std::swap(this->inString, other.inString);
			std::swap(this->section, other.section);
			return *this;
		};

		inline Parser(Parser&& other) noexcept {
			*this = std::move(other);
		};

		inline Parser& operator=(const Parser&) = delete;
		inline Parser(const Parser&) = delete;

		template<typename OTy, StringT OTy2> void parseJson(OTy& json, OTy2& inStringNew) {
			if (inString != inStringNew && inStringNew.size() != 0) {
				reset(inStringNew);
			} else if (inStringNew.size() == 0) {
				return;
			}
			auto newIter = this->begin();
			Parse::op<OTy, SimdIteratorCore>(json, newIter);
		}

	  protected:
		SimdStringReader section{};
		std::string_view inString{};

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

		template<typename OTy> inline void reset(OTy& string) {
			if (string.size() == 0) {
				return;
			}
			this->section.reset(string.size(), reinterpret_cast<StringViewPtr>(string.data()));
			inString = std::string_view{ string.data(), string.size() };
			this->generateJsonIndices();
		}

		inline SimdIteratorCore begin() noexcept {
			return { &section };
		}

		inline SimdIteratorCore end() noexcept {
			return { &section };
		}
	};
};
