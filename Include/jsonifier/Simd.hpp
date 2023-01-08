#pragma once

#include "Base.hpp"

namespace Jsonifier {

	class Jsonifier_Dll SimdBase128 {
	  public:
		inline SimdBase128() noexcept = default;

		inline SimdBase128& operator=(char other) {
			this->value = _mm_set1_epi8(other);
			return *this;
		}

		inline SimdBase128(char other) {
			*this = other;
		}

		inline operator __m128i() {
			return this->value;
		}

	  protected:
		__m128i value{};
	};

	class Jsonifier_Dll SimdBase256 {
	  public:
		inline SimdBase256() noexcept {};

		inline explicit operator bool() {
			for (size_t x = 0; x < 4; ++x) {
				if (this->getUint64(x) != 0) {
					return true;
				}
			}
			return false;
		}

		inline SimdBase256& operator=(const bool value) {
			if (value) {
				this->value = _mm256_insert_epi64(*this, 0x01, 0);
			} else {
				this->value = _mm256_set1_epi8('\0');
			}
			return *this;
		}

		inline SimdBase256(const bool value) {
			*this = value;
		}

		inline int32_t toBitMask() {
			return _mm256_movemask_epi8(*this);
		}

		inline void fromUint64(uint64_t data) {
			this->value = _mm256_set1_epi64x(static_cast<int64_t>(data));
		}

		explicit inline SimdBase256(uint64_t other) {
			this->fromUint64(other);
		}

		inline SimdBase256& operator=(char other) {
			this->value = _mm256_set1_epi8(other);
			return *this;
		}

		inline SimdBase256(char other) {
			*this = other;
		}

		inline SimdBase256& operator=(const char* values) {
			*this = _mm256_loadu_epi8(values);
			return *this;
		}

		explicit inline SimdBase256(const char* values) {
			*this = values;
		}

		inline SimdBase256& operator=(const uint8_t* values) {
			*this = _mm256_loadu_epi8(values);
			return *this;
		}

		explicit inline SimdBase256(const uint8_t* values) {
			*this = values;
		}

		explicit inline SimdBase256(int64_t value00, int64_t value01, int64_t value02, int64_t value03) {
			this->value = _mm256_set_epi64x(value03, value02, value01, value00);
		}

		explicit inline SimdBase256(uint64_t value00, uint64_t value01, uint64_t value02, uint64_t value03) {
			this->value = _mm256_set_epi64x(static_cast<int64_t>(value03), static_cast<int64_t>(value02), static_cast<int64_t>(value01),
				static_cast<int64_t>(value00));
		}

		inline SimdBase256& operator=(__m256i other) {
			this->value = other;
			return *this;
		}

		inline SimdBase256(__m256i other) {
			*this = other;
		}

		inline void store(uint8_t dst[32]) const {
			_mm256_storeu_epi8(dst, this->value);
		}

		inline uint64_t getUint64(size_t index) {
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
		}

		inline int64_t getInt64(size_t index) {
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
		}

		inline void insertUint64(uint64_t value, size_t index) {
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

		inline void insertInt64(int64_t value, size_t index) {
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

		inline operator __m256i&() {
			return this->value;
		}

		inline SimdBase256 operator|(SimdBase256& other) {
			return _mm256_or_si256(this->value, other);
		}

		inline SimdBase256 operator&(SimdBase256& other) {
			return _mm256_and_si256(this->value, other);
		}

		inline SimdBase256 operator^(SimdBase256& other) {
			return _mm256_xor_si256(this->value, other);
		}

		inline SimdBase256 operator+(SimdBase256& other) {
			return _mm256_add_epi8(this->value, other);
		}

		inline SimdBase256& operator|=(SimdBase256& other) {
			*this = *this | other;
			return *this;
		}

		inline SimdBase256& operator&=(SimdBase256& other) {
			*this = *this & other;
			return *this;
		}

		inline SimdBase256& operator^=(SimdBase256& other) {
			*this = *this ^ other;
			return *this;
		}

		inline SimdBase256 operator==(SimdBase256& other) {
			return _mm256_cmpeq_epi8(this->value, other);
		}

		inline SimdBase256 operator|(SimdBase256&& other) {
			return _mm256_or_si256(this->value, other);
		}

		inline SimdBase256 operator&(SimdBase256&& other) {
			return _mm256_and_si256(this->value, other);
		}

		inline SimdBase256 operator^(SimdBase256&& other) {
			return _mm256_xor_si256(this->value, other);
		}

		inline SimdBase256 operator+(SimdBase256&& other) {
			return _mm256_add_epi8(this->value, other);
		}

		inline SimdBase256& operator|=(SimdBase256&& other) {
			*this = *this | other;
			return *this;
		}

		inline SimdBase256& operator&=(SimdBase256&& other) {
			*this = *this & other;
			return *this;
		}

		inline SimdBase256& operator^=(SimdBase256&& other) {
			*this = *this ^ other;
			return *this;
		}

		inline SimdBase256 operator==(SimdBase256&& other) {
			return _mm256_cmpeq_epi8(this->value, other);
		}

		inline SimdBase256 operator==(uint8_t other) {
			return _mm256_cmpeq_epi8(this->value, _mm256_set1_epi8(other));
		}

		template<size_t amount> inline SimdBase256 shl() {
			SimdBase256 returnValueReal{};
			SimdBase256 returnValue{};
			returnValue = _mm256_slli_epi64(*this, (amount % 64));
			returnValueReal |= returnValue;
			returnValue = _mm256_permute4x64_epi64(*this, 0b10010011);
			returnValue = _mm256_srli_epi64(returnValue, 64 - (amount % 64));
			returnValueReal |= returnValue;
			return returnValueReal;
		}

		inline SimdBase256 copyLastBitToFirst(SimdBase256 dest) {
			SimdBase256 zeroesMask{};
			SimdBase256 onesMask{ '\xff' };
			onesMask = onesMask.shr<63>();
			onesMask = onesMask.shl<64>();
			zeroesMask = ~onesMask;
			dest &= zeroesMask;
			*this &= onesMask;
			dest |= *this;
			return dest;
		}

		template<size_t amount> inline SimdBase256 shr() {
			SimdBase256 returnValue{};
			returnValue = _mm256_srli_epi64(*this, (amount % 64));
			SimdBase256 returnValueReal{ _mm256_set_epi64x(0, 0, 0, static_cast<int64_t>((1ULL << 64 - amount) - (1ULL << 0))) };
			returnValue &= returnValueReal;
			return returnValue;
		}

		inline SimdBase256 operator~() {
			SimdBase256 newValues{};
			newValues = _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ull));
			return newValues;
		}

		inline SimdBase256 carrylessMultiplication(uint64_t& prevInString) {
			SimdBase128 allOnes{ '\xFF' };
			const uint64_t inString00 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, this->getInt64(0)), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString00) >> 63);
			const uint64_t inString01 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, this->getInt64(1)), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString01) >> 63);
			const uint64_t inString02 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, this->getInt64(2)), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString02) >> 63);
			const uint64_t inString03 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, this->getInt64(3)), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString03) >> 63);
			return SimdBase256{ inString00, inString01, inString02, inString03 };
		}

		inline bool collectCarries(SimdBase256 other1, SimdBase256& result) {
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

		inline SimdBase256& shiftLastBitToFirst() {
			*this = _mm256_permute4x64_epi64(*this, 0b10010011);
			*this = _mm256_srli_epi64(*this, 63);
			return *this;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			cout << std::bitset<64>{ values };
			cout << std::endl;
		}

		inline SimdBase256 printBits(const std::string& valuesTitle) {
			using std::cout;
			cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<int8_t*>(&this->value) + x)) >> y };
				}
			}
			cout << std::endl;
			return *this;
		}

		inline SimdBase256 bitAndNot(SimdBase256 other) {
			return _mm256_andnot_si256(other, this->value);
		}

		inline SimdBase256 shuffle(SimdBase256 other) {
			return _mm256_shuffle_epi8(other, this->value);
		}

	  protected:
		__m256i value{};
	};

	inline SimdBase256 convertSimdBytesToBits(SimdBase256 input00[8]) {
		SimdBase256 returnValue{};
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[0]), 0);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[1]), 1);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[2]), 2);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[3]), 3);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[4]), 4);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[5]), 5);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[6]), 6);
		returnValue = _mm256_insert_epi32(returnValue, _mm256_movemask_epi8(input00[7]), 7);
		return returnValue;
	}

	template<size_t StepSize> struct StringBlockReader {
	  public:
		inline StringBlockReader(const uint8_t* stringViewNew, size_t _len);
		inline size_t getRemainder(uint8_t* dst) const;
		inline const uint8_t* fullBlock() const;
		inline bool hasFullBlock() const;
		inline size_t blockIndex();
		inline void advance();

	  protected:
		const uint8_t* stringBuffer{};
		const size_t length{};
		const size_t lengthMinusStep{};
		size_t index{};
	};

	template<size_t StepSize>
	inline StringBlockReader<StepSize>::StringBlockReader(const uint8_t* stringViewNew, size_t _len)
		: stringBuffer{ stringViewNew }, length{ _len }, lengthMinusStep{ length < StepSize ? 0 : length - StepSize }, index{ 0 } {
	}

	template<size_t StepSize> inline size_t StringBlockReader<StepSize>::blockIndex() {
		return index;
	}

	template<size_t StepSize> inline bool StringBlockReader<StepSize>::hasFullBlock() const {
		return index < lengthMinusStep;
	}

	template<size_t StepSize> inline const uint8_t* StringBlockReader<StepSize>::fullBlock() const {
		return &stringBuffer[index];
	}

	template<size_t StepSize> inline size_t StringBlockReader<StepSize>::getRemainder(uint8_t* dst) const {
		if (length == index) {
			return 0;
		}
		std::memset(dst, 0x20, StepSize);
		std::memcpy(dst, this->stringBuffer + index, length - index);
		return length - index;
	}

	template<size_t StepSize> inline void StringBlockReader<StepSize>::advance() {
		index += StepSize;
	}

	class Jsonifier_Dll SimdStringSection {
	  public:
		inline SimdStringSection() noexcept = default;

		inline void packStringIntoValue(SimdBase256* theValue, const uint8_t string[32]) {
			*theValue = string;
		}

		inline void addTapeValues(uint32_t* tapePtrs, uint64_t* theBits, size_t currentIndexNew, size_t& currentIndexIntoTape, size_t stringLength) {
			int cnt = static_cast<int>(__popcnt64(*theBits));
			int64_t newValue{};
			for (int i = 0; i < cnt; i++) {
				newValue = _tzcnt_u64(*theBits) + (currentIndexNew * 64) + currentIndexIntoString;

				if (newValue > stringLength) {
					currentIndexIntoTape += i;
					return;

				} else {
					tapePtrs[i + currentIndexIntoTape] = newValue;
					*theBits = _blsr_u64(*theBits);
				}
			}
			currentIndexIntoTape += cnt;
			return;
		}

		inline void getStructuralIndices(uint32_t* currentPtr, size_t& currentIndexIntoTape, size_t stringLength) {
			for (size_t x = 0; x < 4; ++x) {
				auto newValue = this->structurals.getUint64(x);
				this->addTapeValues(currentPtr, &newValue, x, currentIndexIntoTape, stringLength);
			}
			this->currentIndexIntoString += 256;
			return;
		}

		inline SimdBase256 collectWhiteSpace() {
			char valuesNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase256 whitespaceTable{ valuesNew };
			SimdBase256 whiteSpaceReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				whiteSpaceReal[x] = this->values[x].shuffle(whitespaceTable) == this->values[x];
			}
			return convertSimdBytesToBits(whiteSpaceReal);
		}

		inline SimdBase256 collectStructuralCharacters() {
			char newValues[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ newValues };
			SimdBase256 structural[8]{};
			for (size_t x = 0; x < 8; ++x) {
				auto valuesNew00 = this->values[x] | char{ 0x20 };
				structural[x] = this->values[x].shuffle(opTable) == valuesNew00;
			}

			return convertSimdBytesToBits(structural);
		}

		inline SimdBase256 collectBackslashes() {
			SimdBase256 backslashes = _mm256_set1_epi8('\\');
			SimdBase256 backslashesReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				backslashesReal[x] = this->values[x] == backslashes;
			}

			return convertSimdBytesToBits(backslashesReal);
		}

		inline void collectEscapedCharacters() {
			this->backslash = this->collectBackslashes();
			this->backslash = this->backslash.bitAndNot(prevEscaped);
			SimdBase256 followsEscape = this->backslash.shl<1>() | this->prevEscaped;
			SimdBase256 evenBits{ _mm256_set1_epi8(0b01010101) };
			SimdBase256 oddSequenceStarts = this->backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
			SimdBase256 sequencesStartingOnEvenBits{};
			this->prevEscaped = this->backslash.collectCarries(oddSequenceStarts, sequencesStartingOnEvenBits);
			SimdBase256 invert_mask = sequencesStartingOnEvenBits.shl<1>();
			this->escaped = (evenBits ^ invert_mask) & followsEscape;
		}

		void collectJsonCharacters() {
			this->quote = this->collectQuotes().bitAndNot(this->escaped);
			this->inString = this->quote.carrylessMultiplication(this->prevInString);
			this->op = this->collectStructuralCharacters();
			this->whitespace = this->collectWhiteSpace();
		}

		inline SimdBase256 collectQuotes() {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			SimdBase256 quotesReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				quotesReal[x] = (this->values[x] == quotes);
			}

			return convertSimdBytesToBits(quotesReal);
		}

		inline SimdBase256 collectFinalStructurals() {
			this->collectEscapedCharacters();
			this->collectJsonCharacters();
			auto scalar = ~(this->op | this->whitespace);
			SimdBase256 nonQuoteScalar = ~(this->op | this->whitespace).bitAndNot(this->quote);
			auto prevInScalarNew = this->prevInScalar;
			SimdBase256 shiftMask{ _mm256_set_epi64x(static_cast<int64_t>(static_cast<uint64_t>(0ULL) - static_cast<uint64_t>(1ULL << 62)), 0ull,
				0ull, 0ull) };
			this->prevInScalar = (nonQuoteScalar & shiftMask).shiftLastBitToFirst();
			auto followsNonQuoteScalar = nonQuoteScalar.shl<1>();
			followsNonQuoteScalar = prevInScalarNew.copyLastBitToFirst(followsNonQuoteScalar);
			auto potentialScalarStart = scalar.bitAndNot(followsNonQuoteScalar);
			auto stringTail = this->inString ^ this->quote;
			auto potentialStructuralStart = this->op | potentialScalarStart;
			auto structuralStart = (potentialStructuralStart.bitAndNot(stringTail));
			return structuralStart;
		}

		void reset() {
			this->prevInScalar = SimdBase256{ '\x00' };
			this->prevEscaped = SimdBase256{ '\x00' };
			this->backslash = SimdBase256{ '\x00' };
			this->currentIndexIntoString = 0;
			this->prevInString = 0;
		}

		void submitDataForProcessing(const uint8_t* valueNew) {
			this->packStringIntoValue(&this->values[0], valueNew);
			this->packStringIntoValue(&this->values[1], valueNew + 32);
			this->packStringIntoValue(&this->values[2], valueNew + 64);
			this->packStringIntoValue(&this->values[3], valueNew + 96);
			this->packStringIntoValue(&this->values[4], valueNew + 128);
			this->packStringIntoValue(&this->values[5], valueNew + 160);
			this->packStringIntoValue(&this->values[6], valueNew + 192);
			this->packStringIntoValue(&this->values[7], valueNew + 224);
			this->structurals = this->collectFinalStructurals();
		}

	  protected:
		size_t currentIndexIntoString{};
		SimdBase256 prevInScalar{};
		SimdBase256 structurals{};
		SimdBase256 prevEscaped{};
		SimdBase256 whitespace{};
		uint64_t prevInString{};
		SimdBase256 values[8]{};
		SimdBase256 backslash{};
		SimdBase256 inString{};
		SimdBase256 escaped{};
		SimdBase256 quote{};
		SimdBase256 op{};
	};
}
