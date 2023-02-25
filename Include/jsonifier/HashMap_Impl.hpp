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
/// Feb 20, 2023
#pragma once

#include <jsonifier/HashMap.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/StringView.hpp>

namespace Jsonifier {

	__forceinline size_t Fnv1aHash::operator()(size_t string) noexcept {
		return string;
	}

	__forceinline size_t Fnv1aHash::operator()(const char* string, size_t size) noexcept {
		size_t value{ fnvOffsetBasis };
		for (size_t i = 0; i < size; ++i) {
			value ^= static_cast<std::uint64_t>(string[i]);
			value *= fnvPrime;
		}
		return value;
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy>::HashMapIterator(HashMap<VTy, KTy>& mapNew) noexcept : map(&mapNew) {
		currentTableIndex = 0;
		currentVectorIndex = 0;
		if (map->table.capacity() > 0) {
			currentVector = &map->table[currentTableIndex];
			while (currentVector->size() == 0 && currentTableIndex < map->table.capacity() - 1) {
				currentVector = &map->table[++currentTableIndex];
			}
		}
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy>::HashMapIterator(HashMap<VTy, KTy>& map, size_t currentTableIndex, size_t currentVectorIndex) noexcept
		: map(&map), currentTableIndex(currentTableIndex), currentVectorIndex(currentVectorIndex) {
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy>::reference HashMapIterator<VTy, KTy>::operator*() noexcept {
		return (*currentVector)[currentVectorIndex].accessFirst();
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy>::pointer HashMapIterator<VTy, KTy>::operator->() noexcept {
		return static_cast<VTy*>(&((*currentVector)[currentVectorIndex]).accessFirst());
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy>& HashMapIterator<VTy, KTy>::operator++() noexcept {
		++currentVectorIndex;
		++currentGlobalIndex;
		while (currentVectorIndex >= currentVector->size()) {
			++currentTableIndex;
			if (currentTableIndex >= map->table.capacity()) {
				return *this;
			}
			currentVector = &map->table[currentTableIndex];
			currentVectorIndex = 0;
			if (currentVector->size() == 0) {
				continue;
			}
		}
		return *this;
	}

	template<typename VTy, IsAKeyType KTy> __forceinline bool HashMapIterator<VTy, KTy>::operator==(HashMapIterator& other) noexcept {
		return (currentGlobalIndex >= map->size());
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMap<VTy, KTy>::HashMap(size_t sizeNew) noexcept {
		table.reserve(sizeNew);
		for (size_t x = 0; x < sizeNew; ++x) {
			table[x].reserve(1);
		}
	}

	template<typename VTy, IsAKeyType KTy> __forceinline void HashMap<VTy, KTy>::emplace(value_type valueNew) noexcept {
		size_t index = Fnv1aHash{}(valueNew.accessSecond().data(), valueNew.accessSecond().size()) % table.capacity();
		table[index].emplaceBack(std::move(valueNew));
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy> HashMap<VTy, KTy>::begin() noexcept {
		return HashMapIterator<VTy, KTy>(*this);
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMapIterator<VTy, KTy> HashMap<VTy, KTy>::end() noexcept {
		return HashMapIterator<VTy, KTy>(*this, table.capacity(), table.size());
	}

	template<typename VTy, IsAKeyType KTy> __forceinline bool HashMap<VTy, KTy>::contains(KTy keyPath) noexcept {
		size_t index = Fnv1aHash{}(keyPath.data(), keyPath.size()) % table.capacity();
		for (auto& value00: table[index]) {
			if (value00.accessSecond() == keyPath) {
				return true;
			}
		}
		return false;
	}

	template<typename VTy, IsAKeyType KTy> __forceinline HashMap<VTy, KTy>::reference HashMap<VTy, KTy>::at(KTy keyPath) noexcept {
		size_t index = Fnv1aHash{}(keyPath.data(), keyPath.size()) % table.capacity();
		for (auto& entry: table[index]) {
			if (entry.accessSecond() == keyPath) {
				return entry;
			}
		}
		return *table[index].end();
	}

	template<typename VTy, IsAKeyType KTy> __forceinline size_t HashMap<VTy, KTy>::size() noexcept {
		size_t sizeValNew{};
		for (size_t x = 0; x < table.capacity(); ++x) {
			sizeValNew += table[x].size();
		}
		return sizeValNew;
	}

};
