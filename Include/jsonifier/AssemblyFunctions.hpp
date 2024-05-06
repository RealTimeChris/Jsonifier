#pragma once

#include <cstdint>
#include <cstring>

namespace jsonifier_asm {

	extern "C" bool compareLong(const char* lhs, const char* rhs, uint64_t count);

	template<typename char_type01, typename char_type02> bool compare(const char_type01* lhs, const char_type02* rhs, uint64_t count) {
		return compareLong(reinterpret_cast<const char*>(lhs), reinterpret_cast<const char*>(rhs), count);
	}

}
