#pragma once

#include "Base.hpp"

namespace Jsonifier {

	class Jsonifier_Dll SimdBase128 {
	  public:
		__forceinline SimdBase128() noexcept = default;
		__forceinline SimdBase128& operator=(const SimdBase128&) noexcept = delete;
		__forceinline SimdBase128(const SimdBase128&) noexcept = delete;

		__forceinline SimdBase128& operator=(SimdBase128&& other) noexcept {
			this->value = _mm_load_si128(&other.value);
			return *this;
		};

		__forceinline SimdBase128(SimdBase128&& other) noexcept {
			*this = std::move(other);
		};

		__forceinline SimdBase128& operator=(const char* other) noexcept {
			this->value = _mm_loadu_epi8(other);
			return *this;
		};

		__forceinline SimdBase128 operator==(SimdBase128& other) {
			return _mm_cmpeq_epi8(this->value, other);
		}

		__forceinline SimdBase128& operator=(SimdBase128* values) {
			*this = _mm_insert_epi16(__m128i{}, _mm_movemask_epi8(values[0]), 0);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[1]), 1);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[2]), 2);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[3]), 3);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[4]), 4);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[5]), 5);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[6]), 6);
			*this = _mm_insert_epi16(*this, _mm_movemask_epi8(values[7]), 7);
			return *this;
		}

		__forceinline SimdBase128(SimdBase128* values) {
			*this = value;
		}

		__forceinline SimdBase128(const char* other) noexcept {
			*this = std::move(other);
		};

		__forceinline SimdBase128(int64_t valueOne, int64_t valueTwo) {
			*this = _mm_set_epi64x(valueOne, valueTwo);
		}

		__forceinline SimdBase128(uint64_t valueOne, uint64_t valueTwo) {
			*this = _mm_set_epi64x(static_cast<int64_t>(valueOne), static_cast<int64_t>(valueTwo));
		}

		__forceinline void insertUint64(uint64_t value, size_t index) {
			switch (index) {
				case 0: {
					this->value = _mm_insert_epi64(this->value, static_cast<int64_t>(value), 0);
					break;
				}
				case 1: {
					this->value = _mm_insert_epi64(this->value, static_cast<int64_t>(value), 1);
					break;
				}
				default: {
					this->value = _mm_insert_epi64(this->value, static_cast<int64_t>(value), 0);
					break;
				}
			}
		}

		__forceinline void insertInt64(int64_t value, size_t index) {
			switch (index) {
				case 0: {
					this->value = _mm_insert_epi64(this->value, value, 0);
					break;
				}
				case 1: {
					this->value = _mm_insert_epi64(this->value, value, 1);
					break;
				}
				default: {
					this->value = _mm_insert_epi64(this->value, value, 0);
					break;
				}
			}
		}

		__forceinline SimdBase128& operator=(__m128i& other) {
			this->value = other;
			return *this;
		}

		__forceinline SimdBase128(__m128i& other) {
			*this = other;
		}

		__forceinline SimdBase128& operator=(__m128i&& other) {
			this->value = std::move(other);
			return *this;
		}

		__forceinline SimdBase128(__m128i&& other) {
			*this = std::move(other);
		}

		__forceinline SimdBase128& operator=(char other) {
			this->value = _mm_set1_epi8(other);
			return *this;
		}

		__forceinline SimdBase128(char other) {
			*this = other;
		}

		__forceinline operator __m128i() {
			return this->value;
		}

		__forceinline SimdBase128 operator|(SimdBase128 other) {
			return _mm_or_si128(this->value, other);
		}

		__forceinline SimdBase128 operator&(SimdBase128 other) {
			return _mm_and_si128(this->value, other);
		}

		__forceinline SimdBase128 operator^(SimdBase128 other) {
			return _mm_xor_si128(this->value, other);
		}

		__forceinline SimdBase128 operator+(SimdBase128 other) {
			return _mm_add_epi8(this->value, other);
		}

		__forceinline SimdBase128& operator|=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		__forceinline SimdBase128& operator&=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		__forceinline SimdBase128& operator^=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		__forceinline SimdBase128 printBits(const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			for (size_t x = 0; x < 16; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<int8_t*>(&this->value) + x)) >> y };
				}
			}
			cout << std::endl;
			return std::move(*this);
		}

		__forceinline int32_t toBitMask() {
			return _mm_movemask_epi8(*this);
		}

		template<size_t amount> __forceinline SimdBase128 shl() {
			SimdBase128 returnValueReal{};
			SimdBase128 returnValue{};
			returnValue = _mm_slli_epi64(*this, (amount % 64));
			returnValueReal |= std::move(returnValue);
			returnValue = _mm_shuffle_epi32(*this, 0b001100101);
			returnValue = _mm_srli_epi64(returnValue, 64 - (amount % 64));
			returnValueReal |= std::move(returnValue);
			return returnValueReal;
		}

		__forceinline SimdBase128 operator~() {
			SimdBase128 newValues{ _mm_xor_si128(*this, _mm_set1_epi64x(-1ULL)) };
			return newValues;
		}

		__forceinline void printBits(uint64_t values, const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			cout << std::bitset<64>{ values };
			cout << std::endl;
		}

		__forceinline SimdBase128 bitAndNot(SimdBase128&& other) {
			return _mm_andnot_si128(other, this->value);
		}

		__forceinline SimdBase128 shuffle(SimdBase128&& other) {
			return _mm_shuffle_epi8(other, this->value);
		}

		__forceinline SimdBase128 bitAndNot(SimdBase128& other) {
			return _mm_andnot_si128(other, this->value);
		}

		__forceinline SimdBase128 shuffle(SimdBase128& other) {
			return _mm_shuffle_epi8(other, this->value);
		}

	  protected:
		__m128i value{};
	};

	class Jsonifier_Dll SimdBase256 {
	  public:
		__forceinline SimdBase256() noexcept {};
		__forceinline SimdBase256& operator=(const SimdBase256&) noexcept = delete;
		__forceinline SimdBase256(const SimdBase256&) noexcept = delete;

		__forceinline SimdBase256& operator=(SimdBase256&& other) noexcept {
			this->value = _mm256_load_si256(&other.value);
			return *this;
		};

		__forceinline SimdBase256(SimdBase256&& other) noexcept {
			*this = std::move(other);
		}

		__forceinline SimdBase256(SimdBase128&& other, size_t index) noexcept {
			switch (index) {
				case 0: {
					this->value = _mm256_insertf128_si256(this->value, other, 0);
					break;
				}
				case 1: {
					this->value = _mm256_insertf128_si256(this->value, other, 1);
					break;
				}
			}
		};

		__forceinline void insert128iValue(__m128i value, size_t index) {
			switch (index) {
				case 0: {
					this->value = _mm256_insertf128_si256(this->value, value, 0);
					break;
				}
				case 1: {
					this->value = _mm256_insertf128_si256(this->value, value, 1);
					break;
				}
			}
		}

		__forceinline SimdBase256& operator=(SimdBase256* values) {
			*this = _mm256_insert_epi32(__m256i{}, _mm256_movemask_epi8(values[0]), 0);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[1]), 1);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[2]), 2);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[3]), 3);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[4]), 4);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[5]), 5);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[6]), 6);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(values[7]), 7);
			return *this;
		}

		__forceinline SimdBase256(SimdBase256* values) {
			*this = values;
		}

		__forceinline explicit operator bool() {
			for (size_t x = 0; x < 4; ++x) {
				if (this->getUint64(x) != 0) {
					return true;
				}
			}
			return false;
		}

		__forceinline int32_t toBitMask() {
			return _mm256_movemask_epi8(*this);
		}

		__forceinline SimdBase256& operator=(char other) {
			this->value = _mm256_set1_epi8(other);
			return *this;
		}

		__forceinline SimdBase256(char other) {
			*this = other;
		}

		__forceinline SimdBase256& operator=(const char values[32]) {
			this->value = _mm256_loadu_epi8(values);
			return *this;
		}

		__forceinline SimdBase256(const char values[32]) {
			*this = values;
		}

		explicit __forceinline SimdBase256(int64_t value00, int64_t value01, int64_t value02, int64_t value03) {
			this->value = _mm256_set_epi64x(value03, value02, value01, value00);
		}

		explicit __forceinline SimdBase256(uint64_t value00, uint64_t value01, uint64_t value02, uint64_t value03) {
			this->value = _mm256_set_epi64x(value03, value02, value01, value00);
		}

		__forceinline SimdBase256& operator=(__m256i other) {
			this->value = other;
			return *this;
		}

		__forceinline SimdBase256(__m256i other) {
			*this = other;
		}

		__forceinline void store(uint8_t dst[32]) const {
			_mm256_storeu_epi8(dst, this->value);
		}

		__forceinline uint64_t getUint64(size_t index) {
#ifdef _WIN32
			return this->value.m256i_u64[index];
#else
			switch (index) {
				case 0: {
					return static_cast<uint64_t>(_mm256_extract_epi64(this->value, 0));
				}
				case 1: {
					return static_cast<uint64_t>(_mm256_extract_epi64(this->value, 1));
				}
				case 2: {
					return static_cast<uint64_t>(_mm256_extract_epi64(this->value, 2));
				}
				case 3: {
					return static_cast<uint64_t>(_mm256_extract_epi64(this->value, 3));
				}
				default: {
					return static_cast<uint64_t>(_mm256_extract_epi64(this->value, 0));
				}
			}
#endif
		}

		__forceinline int64_t getInt64(size_t index) {
#ifdef _WIN32
			return this->value.m256i_i64[index];
#else
			switch (index) {
				case 0: {
					return _mm256_extract_epi64(this->value, 0);
				}
				case 1: {
					return _mm256_extract_epi64(this->value, 1);
				}
				case 2: {
					return _mm256_extract_epi64(this->value, 2);
				}
				case 3: {
					return _mm256_extract_epi64(this->value, 3);
				}
				default: {
					return _mm256_extract_epi64(this->value, 0);
				}
			}
#endif
		}

		__forceinline void insertUint64(uint64_t value, size_t index) {
			switch (index) {
				case 0: {
					this->value = _mm256_insert_epi64(this->value, static_cast<int64_t>(value), 0);
					break;
				}
				case 1: {
					this->value = _mm256_insert_epi64(this->value, static_cast<int64_t>(value), 1);
					break;
				}
				case 2: {
					this->value = _mm256_insert_epi64(this->value, static_cast<int64_t>(value), 2);
					break;
				}
				case 3: {
					this->value = _mm256_insert_epi64(this->value, static_cast<int64_t>(value), 3);
					break;
				}
				default: {
					this->value = _mm256_insert_epi64(this->value, static_cast<int64_t>(value), 0);
					break;
				}
			}
		}

		__forceinline void insertInt64(int64_t value, size_t index) {
			switch (index) {
				case 0: {
					this->value = _mm256_insert_epi64(this->value, value, 0);
					break;
				}
				case 1: {
					this->value = _mm256_insert_epi64(this->value, value, 1);
					break;
				}
				case 2: {
					this->value = _mm256_insert_epi64(this->value, value, 2);
					break;
				}
				case 3: {
					this->value = _mm256_insert_epi64(this->value, value, 3);
					break;
				}
				default: {
					this->value = _mm256_insert_epi64(this->value, value, 0);
					break;
				}
			}
		}

		__forceinline operator __m256i&() {
			return this->value;
		}

		__forceinline SimdBase256 operator|(SimdBase256& other) {
			return _mm256_or_si256(this->value, other);
		}

		__forceinline SimdBase256 operator&(SimdBase256& other) {
			return _mm256_and_si256(this->value, other);
		}

		__forceinline SimdBase256 operator^(SimdBase256& other) {
			return _mm256_xor_si256(this->value, other);
		}

		__forceinline SimdBase256 operator+(SimdBase256& other) {
			return _mm256_add_epi8(this->value, other);
		}

		__forceinline SimdBase256& operator|=(SimdBase256& other) {
			*this = *this | other;
			return *this;
		}

		__forceinline SimdBase256& operator&=(SimdBase256& other) {
			*this = *this & other;
			return *this;
		}

		__forceinline SimdBase256& operator^=(SimdBase256& other) {
			*this = *this ^ other;
			return *this;
		}

		__forceinline SimdBase256 operator==(SimdBase256& other) {
			return _mm256_cmpeq_epi8(this->value, other);
		}

		__forceinline SimdBase256 operator==(uint8_t other) {
			return _mm256_cmpeq_epi8(this->value, _mm256_set1_epi8(other));
		}

		__forceinline SimdBase256 operator|(SimdBase256&& other) {
			return _mm256_or_si256(this->value, other);
		}

		__forceinline SimdBase256 operator&(SimdBase256&& other) {
			return _mm256_and_si256(this->value, other);
		}

		__forceinline SimdBase256 operator^(SimdBase256&& other) {
			return _mm256_xor_si256(this->value, other);
		}

		__forceinline SimdBase256 operator+(SimdBase256&& other) {
			return _mm256_add_epi8(this->value, other);
		}

		__forceinline SimdBase256& operator|=(SimdBase256&& other) {
			*this = *this | other;
			return *this;
		}

		__forceinline SimdBase256& operator&=(SimdBase256&& other) {
			*this = *this & other;
			return *this;
		}

		__forceinline SimdBase256& operator^=(SimdBase256&& other) {
			*this = *this ^ other;
			return *this;
		}

		__forceinline SimdBase256 operator==(SimdBase256&& other) {
			return _mm256_cmpeq_epi8(this->value, other);
		}

		template<size_t amount> __forceinline SimdBase256 shl() {
			SimdBase256 returnValueReal{};
			returnValueReal = _mm256_slli_epi64(*this, (amount % 64));
			auto returnValue = _mm256_permute4x64_epi64(*this, 0b10010011);
			returnValue = _mm256_srli_epi64(returnValue, 64 - (amount % 64));
			returnValueReal |= returnValue;
			return returnValueReal;
		}

		__forceinline void setFirstBit(bool onOrOff) {
			if (onOrOff) {
				this->insertInt64(this->getInt64(0) | 1LL << 0, 0);
			} else {
				this->insertInt64(this->getInt64(0) & ~(1LL << 0), 0);
			}
		}

		__forceinline bool checkLastBit() {
			return ((this->getUint64(3) >> 63) & 1) << 63;
		}

		__forceinline SimdBase256 operator~() {
			SimdBase256 newValues{ _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ULL)) };
			return newValues;
		}

		__forceinline SimdBase256 carrylessMultiplication(uint64_t& prevInString) {
			SimdBase128 allOnes{ '\xFF' };
			const uint64_t inString00 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 0), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString00) >> 63);
			const uint64_t inString01 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 0), allOnes, 1)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString01) >> 63);
			const uint64_t inString02 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 1), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString02) >> 63);
			const uint64_t inString03 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 1), allOnes, 1)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString03) >> 63);
			return SimdBase256{ inString00, inString01, inString02, inString03 };
		}

		__forceinline bool collectCarries(SimdBase256& other1, SimdBase256& result) {
			bool returnValue{};
			uint64_t returnValue64{};
			for (size_t x = 0; x < 4; ++x) {
				if (_addcarry_u64(0, this->getUint64(x), other1.getUint64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		__forceinline void printBits(uint64_t values, const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			cout << std::bitset<64>{ values };
			cout << std::endl;
		}

		__forceinline SimdBase256 printBits(const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<int8_t*>(&this->value) + x)) >> y };
				}
			}
			cout << std::endl;
			return std::move(*this);
		}

		__forceinline SimdBase256 bitAndNot(SimdBase256&& other) {
			return _mm256_andnot_si256(other, this->value);
		}

		__forceinline SimdBase256 shuffle(SimdBase256&& other) {
			return _mm256_shuffle_epi8(other, this->value);
		}

		__forceinline SimdBase256 bitAndNot(SimdBase256& other) {
			return _mm256_andnot_si256(other, this->value);
		}

		__forceinline SimdBase256 shuffle(SimdBase256& other) {
			return _mm256_shuffle_epi8(other, this->value);
		}

	  protected:
		__m256i value{};
	};

	class Jsonifier_Dll SimdBase256x8 {
	  public:
		__forceinline SimdBase256x8() noexcept = default;
		__forceinline SimdBase256x8& operator=(const SimdBase256x8&) noexcept = delete;
		__forceinline SimdBase256x8(const SimdBase256x8&) noexcept = delete;

		__forceinline SimdBase256x8& operator=(SimdBase256&& other) noexcept {
			_mm256_storeu_epi8(this->values, other);
			return *this;
		};

		__forceinline SimdBase256x8(SimdBase256&& other) noexcept {
			*this = std::move(other);
		};

		operator SimdBase256() {
			return _mm256_loadu_epi8(this->values);
		}

	  protected:
		uint8_t values[32];
	};

	template<size_t StepSize> struct StringBlockReader {
	  public:

		__forceinline void addNewString(const uint8_t* stringViewNew, size_t _len) noexcept {
			this->stringBuffer = stringViewNew;
			this->length = _len;
			this->lengthMinusStep = length < StepSize ? 0 : length - StepSize;
			this->index = 0;
		}

		__forceinline size_t blockIndex() {
			return index;
		}

		__forceinline bool hasFullBlock() const {
			return index < lengthMinusStep;
		}

		__forceinline const uint8_t* fullBlock() const {
			return &stringBuffer[index];
		}

		__forceinline size_t getRemainder(uint8_t* dst) const {
			if (length == index) {
				return 0;
			}
			memset(dst, 0x20, StepSize);
			std::copy(this->stringBuffer + index, this->stringBuffer + index + (length - index), dst);
			return length - index;
		}

		__forceinline void advance() {
			index += StepSize;
		}

	  protected:
		const uint8_t* stringBuffer{};
		size_t length{};
		size_t lengthMinusStep{};
		size_t index{};
	};

	template<size_t BlockStringCount>
	class Jsonifier_Dll SimdStringSection {
	  public:

		__forceinline SimdStringSection& operator=(SimdStringSection&& other) noexcept {
			this->structuralIndexes = std::move(other.structuralIndexes);
			this->stringLength = other.stringLength;
			this->currentIndexIntoString = 0;
			this->currentTapeIndex = 0;
			this->currentBlock = 0;
			return *this;
		}
		
		__forceinline SimdStringSection(SimdStringSection&& other) noexcept {
			*this = std::move(other);
		}

		__forceinline SimdStringSection() noexcept = default;

		__forceinline SimdStringSection(size_t stringLengthNew, uint32_t* tapePtrsNew) noexcept : tapePtrs{ tapePtrsNew } {
			this->stringLength = stringLengthNew;
		};

		__forceinline void packStringIntoValue(SimdBase256* theValue, const char* string) {
			*theValue = string;
		}

		__forceinline void addTapeValues() {
			for (size_t x = 0; x < 4; ++x) {
				auto newBits = *(reinterpret_cast<uint64_t*>(&structurals) + x);
				if (newBits == 0) {
					return;
				}
				int cnt = static_cast<int>(__popcnt64(newBits));
				for (int i=0; i<8; i++) {
					auto newValue = _tzcnt_u64(newBits) + (x * 64) + currentIndexIntoString;
					this->tapePtrs[i + this->currentTapeIndex] = newValue;
					newBits = _blsr_u64(newBits);
				}
				if (cnt > 8){
					for (int i=8; i<16; i++) {
						auto newValue = _tzcnt_u64(newBits) + (x * 64) + currentIndexIntoString;
						this->tapePtrs[i + this->currentTapeIndex] = newValue;
						newBits = _blsr_u64(newBits);
					}

					if (cnt > 16) {
						int i = 16;
						do {
							auto newValue = _tzcnt_u64(newBits) + (x * 64) + currentIndexIntoString;
							this->tapePtrs[i + this->currentTapeIndex] = newValue;
							newBits = _blsr_u64(newBits);
							i++;		
						} while (i < cnt);
					}
				}

				this->currentTapeIndex += cnt;
			}
			return;
		}

		__forceinline void getStructuralIndices() {
			this->addTapeValues();
			this->currentIndexIntoString += 256;
			return;
		}

		__forceinline SimdBase256 collectWhiteSpace() {
			char valuesNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase256 whitespaceTable{ valuesNew };
			SimdBase256 whiteSpaceReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				whiteSpaceReal[x] = this->values[x + (8 * this->currentBlock)].shuffle(whitespaceTable) == this->values[x + (8 * this->currentBlock)];
			}
			return whiteSpaceReal;
		}

		__forceinline SimdBase256 collectStructuralCharacters() {
			char newValues[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ newValues };
			SimdBase256 structural[8];
			for (size_t x = 0; x < 8; ++x) {
				SimdBase256 charVals{ char{ 0x20 } };
				auto valuesNew00 = this->values[x + (8 * this->currentBlock)] | std::move(charVals);
				structural[x] = this->values[x + (8 * this->currentBlock)].shuffle(opTable) == valuesNew00;
			}
			return structural;
		}

		__forceinline SimdBase256 collectBackslashes() {
			SimdBase256 backslashes = _mm256_set1_epi8('\\');
			SimdBase256 backslashesReal[8];
			for (size_t x = 0; x < 8; ++x) {
				backslashesReal[x] = this->values[x + (8 * this->currentBlock)] == backslashes;
			}
			return backslashesReal;
		}

		__forceinline void collectEscapedCharacters() {
			auto backslash = this->collectBackslashes();
			backslash = backslash.bitAndNot(SimdBase256{ std::move(this->prevEscaped) });
			SimdBase256 followsEscape = backslash.template shl<1>() | std::move(this->prevEscaped);
			SimdBase256 evenBits{ _mm256_set1_epi8(0b01010101) };
			auto newBits = evenBits.bitAndNot(followsEscape);
			SimdBase256 oddSequenceStarts = backslash.bitAndNot(newBits);
			SimdBase256 sequencesStartingOnEvenBits{};
			this->prevEscaped = backslash.collectCarries(oddSequenceStarts, sequencesStartingOnEvenBits);
			SimdBase256 invert_mask = sequencesStartingOnEvenBits.template shl<1>();
			this->escaped = (evenBits ^ std::move(invert_mask)) & std::move(followsEscape);
		}

		__forceinline void collectJsonCharacters() {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			SimdBase256 quotesReal[8];
			for (size_t x = 0; x < 8; ++x) {
				quotesReal[x] = (this->values[x + (8 * this->currentBlock)] == quotes);
			}
			SimdBase256 quotesFinal = quotesReal;
			this->quote = quotesFinal.bitAndNot(this->escaped);
			auto newPrevInString = this->prevInString;
			this->inString = quotesFinal.carrylessMultiplication(newPrevInString);
			this->prevInString = newPrevInString;
			this->op = this->collectStructuralCharacters();
			this->whitespace = this->collectWhiteSpace();
		}

		__forceinline SimdBase256 collectFinalStructurals() {
			this->collectEscapedCharacters();
			this->collectJsonCharacters();
			auto scalar = ~(SimdBase256{ std::move(this->op) } | SimdBase256{ std::move(this->whitespace) });
			SimdBase256 nonQuoteScalar = ~(SimdBase256{ std::move(this->op) } | SimdBase256{ std::move(this->whitespace) }).bitAndNot(this->quote);
			auto prevInScalarNew = std::move(this->prevInScalar);
			SimdBase256 shiftMask{ _mm256_set_epi64x(static_cast<int64_t>(static_cast<uint64_t>(0ULL) - static_cast<uint64_t>(1ULL << 62)), 0ull,
				0ull, 0ull) };
			this->prevInScalar = (nonQuoteScalar & shiftMask);
			auto followsNonQuoteScalar = nonQuoteScalar.template shl<1>();
			followsNonQuoteScalar.setFirstBit(prevInScalarNew.checkLastBit());
			auto potentialScalarStart = scalar.bitAndNot(followsNonQuoteScalar);
			auto stringTail = this->inString ^ this->quote;
			auto potentialStructuralStart = SimdBase256{ std::move(this->op) } | potentialScalarStart;
			auto structuralStart = (potentialStructuralStart.bitAndNot(stringTail));
			//this->inString.printBits("IN STRING: ");
			//SimdBase256{}.printBits(this->prevInString, "PREV IN STRING: ");
			//this->prevInScalar.printBits("PREV IN SCALAR: ");
			//structuralStart.printBits("FINAL BITS: ");
			return structuralStart;
		}

		template<size_t StringBlockCount> __forceinline void submitDataForProcessing(const uint8_t* valueNew) {
			for (size_t x = 0; x < StringBlockCount * 8; ++x) {
				this->packStringIntoValue(&this->values[x], reinterpret_cast<const char*>(valueNew + (32 * x)));
			}
		}

		template<size_t StringBlockCount>
		__forceinline size_t generateStructurals() {
			for (size_t x = 0; x < StringBlockCount; ++x) {
				this->currentBlock = x;
				this->structurals = this->collectFinalStructurals();
				this->getStructuralIndices();
			}
			this->currentBlock = 0;
			//this->structurals.printBits("FINAL BITS: ");
			return this->currentTapeIndex;
		}

	  protected:
		SimdBase256 values[BlockStringCount * 8];
		size_t currentIndexIntoString{};
		SimdBase256 prevInScalar{};
		SimdBase256 structurals{};
		size_t currentTapeIndex{};
		SimdBase256 prevEscaped{};
		SimdBase256 whitespace{};
		int64_t iterationCount{};
		SimdBase256 backslash{};
		uint64_t prevInString{};
		SimdBase256 inString{};
		SimdBase256 escaped{};
		int8_t currentBlock{};
		size_t stringLength{};
		int64_t passedTime{};
		uint32_t* tapePtrs;
		SimdBase256 quote{};
		SimdBase256 op{};
	};
};