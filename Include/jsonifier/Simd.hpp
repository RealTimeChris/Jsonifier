#pragma once

#include "Base.hpp"

namespace Jsonifier {

	class Jsonifier_Dll SimdBase128 {
	  public:
		inline SimdBase128() noexcept {};
		inline SimdBase128& operator=(const SimdBase128&) noexcept = delete;
		inline SimdBase128(const SimdBase128&) noexcept = delete;

		inline SimdBase128& operator=(SimdBase128&& other) noexcept {
			this->value = other.value;
			return *this;
		};

		inline SimdBase128(SimdBase128&& other) noexcept {
			*this = std::move(other);
		};

		inline SimdBase128& operator=(__m128i& other) {
			this->value = other;
			return *this;
		}

		inline SimdBase128(__m128i& other) {
			*this = other;
		}

		inline SimdBase128& operator=(__m128i&& other) {
			this->value = std::move(other);
			return *this;
		}

		inline SimdBase128(__m128i&& other) {
			*this = std::move(other);
		}

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

		inline SimdBase128 operator|(SimdBase128 other) {
			return _mm_or_si128(this->value, other);
		}

		inline SimdBase128 operator&(SimdBase128 other) {
			return _mm_and_si128(this->value, other);
		}

		inline SimdBase128 operator^(SimdBase128 other) {
			return _mm_xor_si128(this->value, other);
		}

		inline SimdBase128 operator+(SimdBase128 other) {
			return _mm_add_epi8(this->value, other);
		}

		inline SimdBase128& operator|=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		inline SimdBase128& operator&=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		inline SimdBase128& operator^=(SimdBase128 other) {
			*this = *this | std::move(other);
			return *this;
		}

		inline SimdBase128 printBits(const std::string& valuesTitle) {
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

	  protected:
		__m128i value{};
	};

	class Jsonifier_Dll SimdBase256 {
	  public:
		inline SimdBase256() noexcept {};

		inline SimdBase256& operator=(const SimdBase256&) noexcept = delete;
		inline SimdBase256(const SimdBase256&) noexcept = delete;

		inline SimdBase256& operator=(SimdBase256&& other) noexcept {
			this->value = _mm256_load_si256(&other.value);
			return *this;
		};

		inline SimdBase256(SimdBase256&& other) noexcept {
			*this = std::move(other);
		};

		void insert128iValue(__m128i value, size_t index) {
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

		inline explicit operator bool() {
			for (size_t x = 0; x < 4; ++x) {
				if (this->getUint64(x) != 0) {
					return true;
				}
			}
			return false;
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

		inline SimdBase256& operator=(const char values[32]) {
			this->value = _mm256_load_si256(( __m256i* )values);
			return *this;
		}

		inline SimdBase256(const char values[32]) {
			*this = values;
		}

		explicit inline SimdBase256(int64_t value00, int64_t value01, int64_t value02, int64_t value03) {
			this->value = _mm256_set_epi64x(value03, value02, value01, value00);
		}

		explicit inline SimdBase256(uint64_t value00, uint64_t value01, uint64_t value02, uint64_t value03) {
			this->value = _mm256_set_epi64x(value03, value02, value01, value00);
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

		inline SimdBase256 operator|(SimdBase256* other) {
			return _mm256_or_si256(this->value, *other);
		}

		inline SimdBase256 operator&(SimdBase256* other) {
			return _mm256_and_si256(this->value, *other);
		}

		inline SimdBase256 operator^(SimdBase256* other) {
			return _mm256_xor_si256(this->value, *other);
		}

		inline SimdBase256 operator+(SimdBase256* other) {
			return _mm256_add_epi8(this->value, *other);
		}

		inline SimdBase256& operator|=(SimdBase256* other) {
			*this = *this | other;
			return *this;
		}

		inline SimdBase256& operator&=(SimdBase256* other) {
			*this = *this & other;
			return *this;
		}

		inline SimdBase256& operator^=(SimdBase256* other) {
			*this = *this ^ other;
			return *this;
		}

		inline SimdBase256 operator==(SimdBase256* other) {
			return _mm256_cmpeq_epi8(this->value, *other);
		}

		inline SimdBase256 operator==(uint8_t other) {
			return _mm256_cmpeq_epi8(this->value, _mm256_set1_epi8(other));
		}

		template<size_t amount> inline SimdBase256 shl() {
			SimdBase256 returnValueReal{};
			SimdBase256 returnValue{};
			returnValue = _mm256_slli_epi64(*this, (amount % 64));
			returnValueReal |= &returnValue;
			returnValue = _mm256_permute4x64_epi64(*this, 0b10010011);
			returnValue = _mm256_srli_epi64(returnValue, 64 - (amount % 64));
			returnValueReal |= &returnValue;
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

		inline SimdBase256 carrylessMultiplication(bool& prevInString) {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase256 returnValue{};
			SimdBase128 inString00 = _mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 0), allOnes, 0);
			inString00 |= _mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 1), allOnes, 0);
			returnValue.insert128iValue(inString00, 0);
			SimdBase128 inString01 = _mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 0), allOnes, 1);
			inString01 |= _mm_clmulepi64_si128(_mm256_extracti128_si256(this->value, 1), allOnes, 1);
			returnValue.insert128iValue(inString01, 1);
			prevInString = returnValue.checkLastBit();
			return returnValue;
		}

		inline bool collectCarries(SimdBase256* other1, SimdBase256* result) {
			bool returnValue{};
			uint64_t returnValue64{};
			for (size_t x = 0; x < 4; ++x) {
				if (_addcarry_u64(0, this->getUint64(x), other1->getUint64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result->insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		inline void convertSimdBytesToBits(SimdBase256* input00) {
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[0]), 0);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[1]), 1);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[2]), 2);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[3]), 3);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[4]), 4);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[5]), 5);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[6]), 6);
			*this = _mm256_insert_epi32(*this, _mm256_movemask_epi8(input00[7]), 7);
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
			return std::move(*this);
		}

		inline SimdBase256 bitAndNot(SimdBase256* other) {
			return _mm256_andnot_si256(*other, this->value);
		}

		inline SimdBase256 shuffle(SimdBase256* other) {
			return _mm256_shuffle_epi8(*other, this->value);
		}

	  protected:
		__m256i value{};
	};

	template<size_t StepSize> struct StringBlockReader {
	  public:
		inline StringBlockReader() noexcept = default;
		inline void addNewString(const uint8_t* stringViewNew, size_t _len) noexcept;
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

	template<size_t StepSize> inline void StringBlockReader<StepSize>::addNewString(const uint8_t* stringViewNew, size_t _len) noexcept {
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
		inline SimdStringSection(uint32_t* tapePtrsNew) noexcept {
			this->tapePtrs = tapePtrsNew;
		};

		inline void packStringIntoValue(SimdBase256* theValue, const char string[32]) {
			*theValue = string;
		}

			
		inline void addTapeValues(uint64_t* theBits, size_t currentIndexNew, size_t& currentIndexIntoTape, size_t stringLength) {
			uint64_t newValue{};
			int cnt = static_cast<int>(__popcnt64(*theBits));
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

		inline void getStructuralIndices(size_t& currentIndexIntoTape, size_t stringLength) {
			for (size_t x = 0; x < 4; ++x) {
				auto newValue = this->structurals.getUint64(x);
				this->addTapeValues(&newValue, x, currentIndexIntoTape, stringLength);
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
				whiteSpaceReal[x] = this->values[x].shuffle(&whitespaceTable) == &this->values[x];
			}
			SimdBase256 returnValue{};
			returnValue.convertSimdBytesToBits(whiteSpaceReal);
			return returnValue;
		}

		inline SimdBase256 collectStructuralCharacters() {
			char newValues[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ newValues };
			SimdBase256 structural[8]{};
			for (size_t x = 0; x < 8; ++x) {
				SimdBase256 charVals{ char{ 0x20 } };
				auto valuesNew00 = this->values[x] | &charVals;
				structural[x] = this->values[x].shuffle(&opTable) == &valuesNew00;
			}
			SimdBase256 returnValue{};
			returnValue.convertSimdBytesToBits(structural);
			return returnValue;
		}

		inline SimdBase256 collectBackslashes() {
			SimdBase256 backslashes = _mm256_set1_epi8('\\');
			SimdBase256 backslashesReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				backslashesReal[x] = this->values[x] == &backslashes;
			}
			SimdBase256 returnValue{};
			returnValue.convertSimdBytesToBits(backslashesReal);
			return returnValue;
		}

		inline void collectEscapedCharacters() {
			this->backslash = this->collectBackslashes();
			this->backslash = this->backslash.bitAndNot(&prevEscaped);
			SimdBase256 followsEscape = this->backslash.shl<1>() | &this->prevEscaped;
			SimdBase256 evenBits{ _mm256_set1_epi8(0b01010101) };
			auto newBits = evenBits.bitAndNot(&followsEscape);
			SimdBase256 oddSequenceStarts = this->backslash.bitAndNot(&newBits);
			SimdBase256 sequencesStartingOnEvenBits{};
			this->prevEscaped = this->backslash.collectCarries(&oddSequenceStarts, &sequencesStartingOnEvenBits);
			SimdBase256 invert_mask = sequencesStartingOnEvenBits.shl<1>();
			this->escaped = (evenBits ^ &invert_mask) & &followsEscape;
		}

		void collectJsonCharacters() {
			this->quote = this->collectQuotes().bitAndNot(&this->escaped);
			this->inString = this->quote.carrylessMultiplication(this->prevInString);
			this->op = this->collectStructuralCharacters();
			this->whitespace = this->collectWhiteSpace();
		}

		inline SimdBase256 collectQuotes() {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			SimdBase256 quotesReal[8]{};
			for (size_t x = 0; x < 8; ++x) {
				quotesReal[x] = (this->values[x] == &quotes);
			}
			SimdBase256 returnValue{};
			returnValue.convertSimdBytesToBits(quotesReal);
			return returnValue;
		}

		inline void reset() {
			this->currentIndexIntoString = 0;
			//this->backslash = SimdBase256{};
			//this->prevEscaped = SimdBase256{};
			this->prevInScalar = false;
		}

		inline SimdBase256 collectFinalStructurals() {
			this->collectEscapedCharacters();
			this->collectJsonCharacters();
			auto scalar = ~(this->op | &this->whitespace);
			SimdBase256 nonQuoteScalar = ~(this->op | &this->whitespace).bitAndNot(&this->quote);
			auto prevInScalarNew = this->prevInScalar;
			SimdBase256 shiftMask{ _mm256_set_epi64x(static_cast<int64_t>(static_cast<uint64_t>(0ULL) - static_cast<uint64_t>(1ULL << 62)), 0ull,
				0ull, 0ull) };
			this->prevInScalar = (nonQuoteScalar & &shiftMask).checkLastBit();
			auto followsNonQuoteScalar = nonQuoteScalar.shl<1>();
			followsNonQuoteScalar.setFirstBit(prevInScalarNew);
			auto potentialScalarStart = scalar.bitAndNot(&followsNonQuoteScalar);
			auto stringTail = this->inString ^ &this->quote;
			auto potentialStructuralStart = this->op | &potentialScalarStart;
			auto structuralStart = (potentialStructuralStart.bitAndNot(&stringTail));
			//structuralStart.printBits("FINAL BITS: ");
			return structuralStart;
		}

		void submitDataForProcessing(const uint8_t* valueNew) {
			//iterationCount++;
			//StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
			for (size_t x = 0; x < 8; ++x) {
				this->packStringIntoValue(&this->values[x], reinterpret_cast<const char*>(valueNew + (32 * x)));
			}
			//passedTime += stopWatch.totalTimePassed().count();
			//std::cout << "TIME TO PACK THE VALUES: " << passedTime / iterationCount << std::endl;
			this->structurals = this->collectFinalStructurals();
			this->structurals.printBits("FINAL BITS: ");
		}

	  protected:
		size_t currentIndexIntoString{};
		SimdBase256 structurals{};
		SimdBase256 prevEscaped{};
		SimdBase256 whitespace{};
		SimdBase256 values[8]{};
		SimdBase256 backslash{};
		SimdBase256 inString{};
		SimdBase256 escaped{};
		uint32_t* tapePtrs{};
		SimdBase256 quote{};
		bool prevInScalar{};
		bool prevInString{};
		SimdBase256 op{};
	};
}
