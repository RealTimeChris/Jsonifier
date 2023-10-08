
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <jsonifier/Index.hpp>

template<size_t strLength> struct string_literal {
  public:
	static constexpr size_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

	constexpr string_literal() = default;

	constexpr string_literal(const char (&str)[strLength]) {
		std::copy(str, str + strLength, string);
	}

	constexpr const char* data() const {
		return string;
	}

	constexpr const char* begin() const {
		return string;
	}

	constexpr const char* end() const {
		return string + sizeVal;
	}

	constexpr size_t size() const {
		return sizeVal;
	}

	constexpr const jsonifier::string_view stringView() const {
		return { string, sizeVal };
	}

	char string[strLength];
};

template<uint64_t index = 0> string_literal(const char (&str)[index]) -> string_literal<index>;


template<typename T>
concept IsStringLiteral = std::same_as<T, const char*>;


// Base case: Define a structure for an empty Tuple-like object.
template<std::size_t Index, typename... Types> struct CustomTuple;

// Recursive case: Define a structure for a Tuple-like object that holds the current type and recursively constructs the rest.
template<std::size_t Index, typename First, typename... Rest> struct CustomTuple<Index, First, Rest...> {
	First value;
	CustomTuple<Index + 1, Rest...> rest;

	constexpr CustomTuple(First first, Rest... rest) : value(first), rest(rest...) {
		if constexpr (Index % 2 == 0) {
			static_assert(IsStringLiteral<First>, "Please use a correct string literal type for this value!");
		}
		std::cout << "TYPE: " << typeid(first).name() << ", INDEX: " << Index << std::endl;
	}
};

// Specialization for the last element.
template<std::size_t Index, typename Last> struct CustomTuple<Index, Last> {
	Last value;

	constexpr CustomTuple(Last last) : value(last) {
		if constexpr (Index % 2 == 0) {
			static_assert(IsStringLiteral<Last>, "Please use a correct string literal type for this value!");
		}
		std::cout << "TYPE: " << typeid(last).name() << ", INDEX: " << Index << std::endl;
	}
};
template<uint64_t index = 0, typename... value_type> CustomTuple(value_type...) -> CustomTuple<index, value_type...>;

int main() {
	// Create a vector to store pairs
	//variant_pair_collection pairCollection{ StringLiteralPair{ "TESTING", 2323 } };
	CustomTuple tuple{ "Test", 2, "22" };
	return 0;
}
