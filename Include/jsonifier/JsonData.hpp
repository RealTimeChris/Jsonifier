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

#include <jsonifier/Base.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/JsonDataBase.hpp>
#include <jsonifier/JsonData.hpp>

namespace Jsonifier {

	class JsonData {
	  public:
		__forceinline JsonData() noexcept = default;

		__forceinline JsonData& operator=(JsonData&& other) noexcept {
			if (this != &other) {
				if (other.ptr) {
					if (this->ptr) {
						delete this->ptr;
					}
					this->ptr = other.ptr;
					other.ptr = nullptr;
				}
			}
			return *this;
		}

		__forceinline JsonData(JsonData&& other) noexcept {
			*this = std::move(other);
		}

		__forceinline JsonData& operator=(const JsonData& other) noexcept {
			if (this != &other) {
				if (other.ptr) {
					if (this->ptr) {
						delete this->ptr;
					}
					switch (other.ptr->type()) {
						case JsonType::Object: {
							this->ptr = new Object{};
							*static_cast<Object*>(this->ptr) = *static_cast<Object*>(other.ptr);
							break;
						}
						case JsonType::Array: {
							this->ptr = new Array{};
							*static_cast<Array*>(this->ptr) = *static_cast<Array*>(other.ptr);
							break;
						}
						default: {
							this->ptr = new JsonDataBase{};
							*this->ptr = *other.ptr;
						}
					}
				}
			}
			return *this;
		}

		__forceinline JsonData(const JsonData& other) noexcept {
			*this = other;
		}
	
		__forceinline JsonData& operator=(Object* ptrNew) noexcept {
			ptr = ptrNew;
			return *this;
		}

		__forceinline JsonData(Object* ptrNew) noexcept {
			*this = ptrNew;
		}

		__forceinline JsonData& operator=(Array* ptrNew) noexcept {
			ptr = ptrNew;
			return *this;
		}

		__forceinline JsonData(Array* ptrNew) noexcept {
			*this = ptrNew;
		}

		__forceinline JsonData& operator=(JsonDataBase* ptrNew) noexcept {
			ptr = ptrNew;
			return *this;
		}

		__forceinline JsonData(JsonDataBase* ptrNew) noexcept {
			*this = ptrNew;
		}

		__forceinline JsonData& operator=(nullptr_t ptrNew) noexcept {
			ptr = ptrNew;
			return *this;
		}	

		__forceinline JsonData(nullptr_t ptrNew) noexcept {
			*this = ptrNew;
		}

		__forceinline JsonData& operator[](const char* key) noexcept {
			return static_cast<Object*>(this->ptr)->operator[](key);
		}

		__forceinline JsonData& operator[](size_t index) noexcept {
			return static_cast<Array*>(this->ptr)->operator[](index);
		}

		__forceinline JsonType type() noexcept {
			return this->ptr->type();
		}

		template<typename VTy> __forceinline ErrorCode get(VTy& out) noexcept;

		template<> __forceinline ErrorCode get<std::string_view>(std::string_view& value) noexcept {
			auto resultValue = this->ptr->getString();
			auto errorVal = this->ptr->reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<bool>(bool& value) noexcept {
			auto resultValue = this->ptr->getBool();
			auto errorVal = this->ptr->reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<double>(double& value) noexcept {
			auto resultValue = this->ptr->getDouble();
			auto errorVal = this->ptr->reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<int64_t>(int64_t& value) noexcept {
			auto resultValue = this->ptr->getInt64();
			auto errorVal = this->ptr->reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<Array>(Array& value) noexcept {
			auto resultValue = this->ptr->getArray();
			auto errorVal = resultValue.reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<Object>(Object& value) noexcept {
			auto resultValue = this->ptr->getObject();
			auto errorVal = resultValue.reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		template<> __forceinline ErrorCode get<uint64_t>(uint64_t& value) noexcept {
			auto resultValue = this->ptr->getUint64();
			auto errorVal = this->ptr->reportError();
			if (!errorVal) {
				value = std::move(resultValue);
			}
			return errorVal;
		}

		__forceinline operator std::string_view() noexcept {
			return this->ptr->operator std::string_view();
		}

		__forceinline operator std::string() noexcept {
			return this->ptr->operator std::string();
		}

		__forceinline operator uint64_t() noexcept {
			return this->ptr->operator uint64_t();
		}

		__forceinline operator int64_t() noexcept {
			return this->ptr->operator int64_t();
		}

		__forceinline operator double() noexcept {
			return this->ptr->operator double();
		}

		__forceinline operator ErrorCode() noexcept {
			return this->ptr->operator Jsonifier::ErrorCode();
		}

		__forceinline operator bool() noexcept {
			return this->ptr->operator bool();
		}

		__forceinline operator Array&() noexcept {
			return *static_cast<Array*>(this->ptr);
		}

		__forceinline operator Object&() noexcept {
			return *static_cast<Object*>(this->ptr);
		}

		__forceinline ~JsonData() noexcept {
			if (this->ptr) {
				delete this->ptr;
				ptr = nullptr;
			}
		}

	  protected:
		mutable JsonDataBase* ptr{};
	};

}
