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
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Parse.hpp>
#include <iterator>

namespace Jsonifier {

	inline thread_local std::string currentStringBuffer{};

	template<std::forward_iterator ITy> inline void skipObject(ITy& it) noexcept {
		++it;
		size_t currentDepth{ 1 };
		if (**it == '}') {
			++it;
			return;
		}
		while (currentDepth > 0 && it != it) {
			++it;
			switch (**it) {
				case '{': {
					++currentDepth;
					break;
				}
				case '}': {
					--currentDepth;
					break;
				}
				default: {
					break;
				}
			}
		}
		++it;
	}

	template<std::forward_iterator ITy> inline void skipArray(ITy& it) noexcept {
		++it;
		size_t currentDepth{ 1 };
		if (**it == ']') {
			++it;
			return;
		}
		while (currentDepth > 0 && it != it) {
			++it;
			switch (**it) {
				case '[': {
					++currentDepth;
					break;
				}
				case ']': {
					--currentDepth;
					break;
				}
				default: {
					break;
				}
			}
		}
		++it;
	}

	template<std::forward_iterator ITy> inline void skipValue(ITy& it) noexcept {
		switch (**it) {
			case '{': {
				skipObject(it);
				break;
			}
			case '[': {
				skipArray(it);
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
			}
		}
	}

	template<char c, StringLiteral errorString, std::forward_iterator ITy> inline void match(ITy& it) {
		if (**it != c) [[unlikely]] {
			throw JsonifierError<"Parse">{ std::string{ "at function: " + static_cast<std::string>(errorString.sv()) +
											   std::string{ ", Failed to match a character: '" + std::string{ c } + "', it was: '" +
												   std::string{ *reinterpret_cast<const char*>(*it) } + "'" } }
											   .c_str(),
				it.getCurrentIndex() };
		} else [[likely]] {
			++it;
			return;
		}
	}

	template<char c, char c2, StringLiteral errorString, std::forward_iterator ITy> inline bool match(ITy& it) {
		switch (**it) {
			case c: {
				++it;
				return true;
			}
			case c2: {
				++it;
				return false;
			}
			default: {
				throw JsonifierError<"Parse">{ std::string{ "at function: " + static_cast<std::string>(errorString.sv()) +
												   std::string{ ", Failed to match a character: '" + std::string{ c } + "', it was: '" +
													   std::string{ *reinterpret_cast<const char*>(*it) } + "'" } }
												   .c_str(),
					it.getCurrentIndex() };
			}
		}
	}

	template<typename OTy>
		requires BoolT<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			value = parseBool(*it);
			++it;
			return;
		}
	};

	template<typename OTy>
		requires IsUniquePtrV<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			value = std::make_unique<OTy::element_type>();
			Parse::op(*value, it);
			return;
		}
	};

	template<typename OTy>
		requires IsEnum<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			auto newPtr = *it;
			auto newValue = std::to_underlying(value);
			auto newValueOld = std::to_underlying(value);
			parseNumber(newValue, newPtr);
			newValue |= newValueOld;
			value = static_cast<OTy>(newValue);
			++it;
			return;
		}
	};

	template<typename OTy>
		requires NumT<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			auto newPtr = *it;
			parseNumber(value, newPtr);
			++it;
			return;
		}
	};

	template<typename OTy>
		requires StringT<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			auto start = *it;
			++it;
			size_t newSize = static_cast<size_t>(*it - start) - 2;
			value.resize(newSize + (16 - (newSize % 16)));
			auto newPtr = parseString((start) + 1, reinterpret_cast<StringBufferPtr>(value.data()), newSize);
			newSize = reinterpret_cast<char*>(newPtr) - value.data();
			value.resize(newSize);
			return;
		}
	};

	inline size_t countArrayElements(auto& it) {
		if (**it == ']') [[unlikely]] {
			return 0;
		}
		size_t count = 1;
		while (true) {
			switch (**it) {
				case ',': {
					++count;
					++it;
					break;
				}
				case ']': {
					return count;
				}
				case '\0': {
					throw JsonifierError<"Parse">{ std::string{ "Unxpected end of buffer" }.c_str(), it.getCurrentIndex() };
				}
				default:
					++it;
			}
		}
		return {};
	}

	template<typename OTy>
		requires(ArrayT<OTy> && !EmplaceBackable<OTy> && !Resizeable<OTy> && Emplaceable<OTy>)
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			match<'[', "ArrayT 01">(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}

			while (it != it) {
				using VTy = typename OTy::value_type;
				if constexpr (sizeof(VTy) > 8) {
					thread_local VTy v;
					Parse::op(v, it);
					value.emplace(v);
				} else {
					VTy v{};
					Parse::op(v, it);
					value.emplace(std::move(v));
				}
				if (**it == ']') {
					++it;
					return;
				}
				match<','>(it);
			}
		}
	};

	template<typename OTy>
		requires(ArrayT<OTy> && ( EmplaceBackable<OTy> || !Resizeable<OTy> ) && !Emplaceable<OTy>)
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			if (!match<'[', 'n', "ArrayT 02, 01">(it)) {
				return;
			}
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = value.size();

			auto value_it = value.begin();

			for (size_t i = 0; i < n; ++i) {
				Parse::op(*value_it++, it);

				if (**it == ',') [[likely]] {
					++it;
				} else {
					match<']', "ArrayT 02, 02">(it);
					return;
				}
			}
			if constexpr (EmplaceBackable<OTy>) {
				while (it != it) {
					Parse::op(value.emplace_back(), it);
					if (**it == ',') [[likely]] {
						++it;
					} else {
						match<']', "ArrayT 02, 03">(it);
						return;
					}
				}
			}
		}
	};

	template<typename OTy>
		requires ArrayT<OTy> && ( !EmplaceBackable<OTy> && Resizeable<OTy> )
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			if (!match<'[', 'n', "ArrayT 03, 01">(it)) {
				return;
			}
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = countArrayElements(it);
			value.resize(n);
			size_t i = 0;
			for (auto& x: value) {
				Parse::op(x, it);
				if (i < n - 1) {
					match<',', "ArrayT 03, 02">(it);
				}
				++i;
			}
		}
	};

	template<typename OTy>
		requires ArrayT<OTy>
	struct ParseImpl<OTy> {
		template<std::forward_iterator ITy> inline static void op(OTy& value, ITy& it) {
			if (!match<'[', 'n', "ArrayT 04, 01">(it)) {
				return;
			}
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<OTy>) {
					return std::tuple_size_v<CoreT<OTy>>;
				} else {
					return std::tuple_size_v<OTy>;
				}
			}();

			forEach<N>([&](auto I) {
				if (**it == ']') {
					++it;
					return;
				}
				if constexpr (I != 0) {
					match<',', "ArrayT 04, 02">(it);
				}
				if constexpr (IsStdTuple<OTy>) {
					Parse::op(std::get<I>(value), it);
				} else if constexpr (JsonifierArrayT<OTy>) {
					Parse::op(getMember(value, Tuplet::get<I>(CoreV<OTy>)), it);
				} else {
					Parse::op(Tuplet::get<I>(value), it);
				}
			});
		}
	};

	template<std::forward_iterator ITy> std::string_view parseKey(ITy start, ITy& iterator) {
		auto newLength = static_cast<int64_t>(*iterator - *start);
		if (newLength > 0) {
			return std::string_view{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(newLength) - 2 };
		} else {
			return {};
		}
	}

	template<typename OTy>
		requires MapT<OTy> || JsonifierObjectT<OTy>
	struct ParseImpl<OTy> {
		inline static void op(OTy& value, auto& it) {
			if (!match<'{', 'n', "MapT 01, 01">(it)) {
				return;
			}
			bool first{ true };
			while (it != it) {
				if (**it == '}') [[unlikely]] {
					++it;
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					match<',', "MapT 01, 02">(it);
				}

				if constexpr (JsonifierObjectT<OTy>) {
					auto start = it;
					match<'"', "MapT 01, 03">(it);
					std::string_view key = parseKey(start, it);
					match<':', "MapT 01, 04">(it);
					constexpr auto frozen_map = makeMap<OTy>();
					const auto& memberIt = frozen_map.find(key);
					if (memberIt != frozen_map.end()) [[likely]] {
						std::visit(
							[&](auto& member_ptr) {
								Parse::op(getMember(value, member_ptr), it);
							},
							memberIt->second);
					} else [[unlikely]] {
						skipValue(it);
					}
				} else {
					Parse::op(currentStringBuffer, it);
					match<':', "MapT 01, 05">(it);

					if constexpr (std::same_as<typename OTy::key_type, std::string>) {
						Parse::op(value[currentStringBuffer], it);
					} else {
						static thread_local typename OTy::key_type key_value{};
						Parse::op(key_value, it);
						Parse::op(value[key_value], it);
					}
				}
			}
		};
	};

}