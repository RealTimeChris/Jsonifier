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

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	template<typename VTy> class VectorIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = VTy;
		using difference_type = std::ptrdiff_t;
		using pointer = VTy*;
		using reference = VTy&;

		__forceinline VectorIterator() noexcept {};

		__forceinline VectorIterator(pointer dataNew) noexcept {
			value = dataNew;
		}

		__forceinline bool operator==(VectorIterator other) noexcept {
			return value == other.value;
		}

		__forceinline VectorIterator operator++() noexcept {
			++value;
			return *this;
		}

		__forceinline pointer operator->() noexcept {
			return value;
		}

		__forceinline reference operator*() noexcept {
			return *value;
		}

	  protected:
		pointer value{};
	};

	template<typename VTy> class Vector {
	  public:
		__forceinline Vector& operator=(Vector&& other) noexcept {
			if (this != &other) {
				sizeVal = other.sizeVal;
				objects = std::move(other.objects);
				other.sizeVal = 0;
			}
			return *this;
		}

		__forceinline Vector(Vector&& other) noexcept {
			*this = std::move(other);
		}

		__forceinline Vector& operator=(const Vector& other) noexcept {
			if (this != &other) {
				this->objects = other.objects;
				this->sizeVal = other.sizeVal;
			}
			return *this;
		};

		__forceinline Vector(const Vector& other) noexcept {
			*this = other;
		};

		__forceinline Vector() noexcept {};

		__forceinline Vector(VTy& other) noexcept {
			emplaceBack(other);
		}

		__forceinline Vector(VTy&& other) noexcept {
			emplaceBack(std::move(other));
		}

		__forceinline void reserve(size_t newCapacity) {
			objects.resize(newCapacity);
		}

		__forceinline Vector(size_t size) noexcept {
			reserve(size);
		};

		__forceinline bool empty() const noexcept {
			return sizeVal == 0;
		}

		__forceinline VectorIterator<VTy> begin() const noexcept {
			return { objects.data() };
		}

		__forceinline VectorIterator<VTy> end() const noexcept {
			return { objects.data() + sizeVal };
		}

		__forceinline size_t capacity() const noexcept {
			return objects.size();
		}

		__forceinline VTy& back() const noexcept {
			return objects[sizeVal - 1];
		}

		__forceinline VTy& operator[](size_t index) const {
			return objects[index];
		}

		__forceinline VTy& emplaceBack(VTy& data) {
			if (sizeVal + 1 > capacity()) {
				reserve((sizeVal + 1) * 4);
			}
			objects[sizeVal] = data;
			++sizeVal;
			return objects[sizeVal - 1];
		}

		__forceinline VTy& emplaceBack(VTy&& data) {
			if (sizeVal + 1 > capacity()) {
				reserve((sizeVal + 1) * 4);
			}
			objects[sizeVal] = std::move(data);
			++sizeVal;
			return objects[sizeVal - 1];
		}

		__forceinline void resize(size_t newSize) noexcept {
			if (sizeVal < newSize) {
				auto oldData = std::move(objects);
				size_t oldSize = sizeVal;
				objects.reset(newSize);
				sizeVal = newSize;
				for (size_t x = 0; x < oldSize; ++x) {
					objects[x] = std::move(oldData[x]);
				}
				for (size_t x = oldSize; x < newSize; ++x) {
					objects[x] = VTy{};
				}
			}
		}

		__forceinline VTy* data() const noexcept {
			return objects.data();
		}

		__forceinline size_t size() const noexcept {
			return sizeVal;
		}

		__forceinline friend bool operator==(Vector lhs, Vector rhs) noexcept {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			for (size_t x = 0; x < lhs.size(); ++x) {
				if (lhs[x] != rhs[x]) {
					return false;
				}
			}
			return true;
		}

	  protected:
		MemoryCore<VTy> objects{};
		size_t sizeVal{};
	};
}