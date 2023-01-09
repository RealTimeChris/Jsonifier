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

		inline SimdBase256& operator=(char other) {
			this->value = _mm256_set1_epi8(other);
			return *this;
		}

		inline SimdBase256(char other) {
			*this = other;
		}

		inline SimdBase256& operator=(const uint8_t* values) {
			*this = _mm256_loadu_epi8(values);
			return *this;
		}

		inline SimdBase256(const uint8_t* values) {
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

		inline void setFirstBit(bool onOrOff) {
			if (onOrOff) {
				this->insertInt64(this->getInt64(0) | 1L << 0, 0);
			} else {
				this->insertInt64(this->getInt64(0) & ~(1L << 0), 0);
			}
		}

		bool checkLastBit() {
			return ((this->getUint64(3) >> 63) & 1) << 63;
		}

		inline SimdBase256 operator~() {
			SimdBase256 newValues{};
			newValues = _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ull));
			return newValues;
		}

		inline uint64_t carrylessMultiplication(uint64_t& prevInString) {
			SimdBase128 allOnes{ '\xFF' };
			const uint64_t inString00 =
				_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, this->getInt64(0)), allOnes, 0)) ^ static_cast<int64_t>(prevInString);
			prevInString = uint64_t(static_cast<int64_t>(inString00) >> 63);
			return inString00;
		}

		inline bool collectCarries(uint64_t other00,uint64_t other1, uint64_t& result) {
			bool returnValue{};
			uint64_t returnValue64{};
			if (_addcarry_u64(0, other00, other1, &returnValue64)) {
				returnValue = true;
			} else {
				returnValue = false;
			}
			result = returnValue64;
			return returnValue;
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

	inline uint64_t convertSimdBytesToBits(SimdBase256 input00[2]) {
		uint64_t returnValue{};
		returnValue = _mm256_movemask_epi8(input00[0]);
		returnValue |= (uint64_t(_mm256_movemask_epi8(input00[1])) << 32);
		return returnValue;
	}

	template<size_t StepSize> struct StringBlockReader {
	  public:
		inline StringBlockReader(const uint8_t* stringViewNew, size_t _len) noexcept;
		inline size_t getRemainder(uint8_t* dst) const;
		inline const uint8_t* fullBlock() const;
		inline bool hasFullBlock() const;
		inline size_t blockIndex();
		inline void advance();

	  protected:
		const uint8_t* stringBuffer{};
		size_t length{};
		size_t lengthMinusStep{};
		size_t index{};
	};

	template<size_t StepSize>
	inline StringBlockReader<StepSize>::StringBlockReader(const uint8_t* stringViewNew, size_t _len) noexcept {
		this->stringBuffer = stringViewNew;
		this->length = _len;
		this->lengthMinusStep = length < StepSize ? 0 : length - StepSize;
		this->index = 0;
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
			auto newValue = this->structurals;
			this->addTapeValues(currentPtr, &newValue, 0, currentIndexIntoTape, stringLength);
			this->currentIndexIntoString += 64;
			return;
		}

		inline uint64_t collectWhiteSpace() {
			uint8_t valuesNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase256 whitespaceTable{ valuesNew };
			SimdBase256 whiteSpaceReal[2]{};
			for (size_t x = 0; x < 2; ++x) {
				whiteSpaceReal[x] = this->values[x].shuffle(whitespaceTable) == this->values[x];
			}
			return convertSimdBytesToBits(whiteSpaceReal);
		}

		inline uint64_t collectStructuralCharacters() {
			uint8_t newValues[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ newValues };
			SimdBase256 structural[2]{};
			for (size_t x = 0; x < 2; ++x) {
				auto valuesNew00 = this->values[x] | char{ 0x20 };
				structural[x] = this->values[x].shuffle(opTable) == valuesNew00;
			}

			return convertSimdBytesToBits(structural);
		}

		inline uint64_t collectBackslashes() {
			SimdBase256 backslashes = _mm256_set1_epi8('\\');
			SimdBase256 backslashesReal[2]{};
			for (size_t x = 0; x < 2; ++x) {
				backslashesReal[x] = this->values[x] == backslashes;
			}

			return convertSimdBytesToBits(backslashesReal);
		}

		inline void collectEscapedCharacters() {
			this->backslash = this->collectBackslashes();
			this->backslash = this->backslash & ~prevEscaped;
			uint64_t followsEscape = this->backslash << 1 | this->prevEscaped;
			uint64_t evenBits{ 0b0101010101010101010101010101010101010101010101010101010101010101 };
			uint64_t oddSequenceStarts = this->backslash & ~evenBits & ~followsEscape;
			uint64_t sequencesStartingOnEvenBits{};
			this->prevEscaped = SimdBase256{}.collectCarries(this->backslash, oddSequenceStarts, sequencesStartingOnEvenBits);
			uint64_t invert_mask = sequencesStartingOnEvenBits << 1;
			this->escaped = (evenBits ^ invert_mask) & followsEscape;
		}

		void collectJsonCharacters() {
			this->quote = this->collectQuotes() & ~this->escaped;
			this->inString = SimdBase256 {
			}.carrylessMultiplication(this->prevInString);
			this->op = this->collectStructuralCharacters();
			this->whitespace = this->collectWhiteSpace();
		}

		inline uint64_t collectQuotes() {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			SimdBase256 quotesReal[2]{};
			for (size_t x = 0; x < 2; ++x) {
				quotesReal[x] = (this->values[x] == quotes);
			}

			return convertSimdBytesToBits(quotesReal);
		}		

		inline uint64_t collectFinalStructurals() {
			this->collectEscapedCharacters();
			this->collectJsonCharacters();
			auto scalar = ~(this->op | this->whitespace);
			uint64_t nonQuoteScalar = ~(this->op | this->whitespace) & ~this->quote;
			auto prevInScalarNew = this->prevInScalar;
			uint64_t shiftMask{ static_cast<uint64_t>(static_cast<uint64_t>(0ULL) - static_cast<uint64_t>(1ULL << 62)) };
			this->prevInScalar = SimdBase256{}.checkLastBit();
			auto followsNonQuoteScalar = nonQuoteScalar << 1;
			SimdBase256 {
			}.setFirstBit(prevInScalarNew);
			auto potentialScalarStart = scalar & ~followsNonQuoteScalar;
			auto stringTail = this->inString ^ this->quote;
			auto potentialStructuralStart = this->op | potentialScalarStart;
			auto structuralStart = (potentialStructuralStart & ~stringTail);
			return structuralStart;
		}

		void submitDataForProcessing(const uint8_t* valueNew) {
			for (size_t x = 0; x < 2; ++x) {
				this->packStringIntoValue(&this->values[x], valueNew + (32 * x));
			}
			this->structurals = this->collectFinalStructurals();
		}

	  protected:
		size_t currentIndexIntoString{};
		uint64_t structurals{};
		uint64_t prevEscaped{};
		uint64_t whitespace{};
		uint64_t prevInString{};
		SimdBase256 values[2]{};
		uint64_t backslash{};
		uint64_t inString{};
		uint64_t escaped{};
		uint64_t quote{};
		bool prevInScalar{};
		uint64_t op{};
	};
}
