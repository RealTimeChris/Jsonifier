/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
#include "Tests.hpp"
#include <map>
#include <rt-ut>

enum class Color : uint8_t { Red, Green, Blue };

struct simple_struct {
	int32_t id{};
	std::string name{};
	double value{};
};

template<> struct jsonifier::core<simple_struct> {
	using value_type				 = simple_struct;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::name, &value_type::value>();
};

struct char_roundtrip {
	char char_val{};
	unsigned char uchar_val{};
	int32_t int_val{};
};

template<> struct jsonifier::core<char_roundtrip> {
	using value_type				 = char_roundtrip;
	static constexpr auto parseValue = createValue<&value_type::char_val, &value_type::uchar_val, &value_type::int_val>();
};

struct sub_thing {
	double a{ 3.14 };
	std::string b{ "stuff" };
};

template<> struct jsonifier::core<sub_thing> {
	using value_type				 = sub_thing;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b>();
};

struct sub_thing2 {
	double a{ 3.14 };
	std::string b{ "stuff" };
	double c{ 999.342494903 };
	double d{ 0.000000000001 };
	double e{ 203082348402.1 };
	float f{ 89.089f };
	double g{ 12380.00000013 };
	double h{ 1000000.000001 };
};

template<> struct jsonifier::core<sub_thing2> {
	using value_type = sub_thing2;
	static constexpr auto parseValue =
		createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h>();
};

struct V3 {
	double x{ 3.14 };
	double y{ 2.7 };
	double z{ 6.5 };

	bool operator==(const V3& rhs) const {
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
	}
};

template<> struct jsonifier::core<V3> {
	using value_type				 = V3;
	static constexpr auto parseValue = createValue<&value_type::x, &value_type::y, &value_type::z>();
};

struct nested_struct {
	simple_struct inner{};
	std::vector<int32_t> numbers{};
	bool flag{};
};

template<> struct jsonifier::core<nested_struct> {
	using value_type				 = nested_struct;
	static constexpr auto parseValue = createValue<&value_type::inner, &value_type::numbers, &value_type::flag>();
};

struct Thing {
	sub_thing thing{};
	std::array<sub_thing2, 1> thing2array{};
	V3 vec3{};
	std::vector<int32_t> numbers{ 6, 7, 8, 2 };
	std::array<std::string, 4> array{ "as\"df\\ghjkl", "pie", "42", "foo" };
	std::vector<V3> vector{ { 9.0, 6.7, 3.1 }, {} };
	int32_t i{ 8 };
	double d{ 2.0 };
	bool b{};
	char c{ 'W' };
	Color color{ Color::Green };
	std::vector<bool> vb{ true, false, false, true, true, true, true };
	std::shared_ptr<sub_thing> sptr = std::make_shared<sub_thing>();
	std::optional<V3> optional{};
	std::vector<double> doubles{ 9.0, 6.7, 3.1 };
	std::map<std::string, int32_t> map{ { "a", 4 }, { "f", 7 }, { "b", 12 } };
};

template<> struct jsonifier::core<Thing> {
	using value_type = Thing;
	static constexpr auto parseValue =
		createValue<&value_type::thing, &value_type::thing2array, &value_type::vec3, &value_type::numbers, &value_type::doubles, &value_type::vector, &value_type::i,
			&value_type::d, &value_type::b, &value_type::c, &value_type::color, &value_type::vb, &value_type::sptr, &value_type::optional, &value_type::array, &value_type::map>();
};

struct escaped_struct {
	int32_t escaped_key{};
	std::string escaped_key2{ "hi" };
	std::string escape_chars{};
};

template<> struct jsonifier::core<escaped_struct> {
	using value_type = escaped_struct;
	static constexpr auto parseValue =
		createValue<makeJsonEntity<&value_type::escaped_key, "escaped\"key">(), makeJsonEntity<&value_type::escaped_key2, "escaped\"\"key2">(), &value_type::escape_chars>();
};

enum class Vehicle : uint32_t { Car, Truck, Plane };

enum class TestData : uint8_t { None, A, B, C, D, ERROR_E = 0xFF };

struct dummy_data {
	uint32_t id{ 0 };
	int32_t a{ 0 };
	TestData b{ TestData::None };
	TestData c{ TestData::None };
	TestData d{ TestData::None };
	TestData e{ TestData::None };
	int64_t f{ 0 };
};

template<> struct jsonifier::core<dummy_data> {
	using value_type				 = dummy_data;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f>();
};

struct BasicStruct {
	int32_t i{};
	double d{};
	std::string str{};
	std::array<uint32_t, 3> arr{};
};

template<> struct jsonifier::core<BasicStruct> {
	using value_type				 = BasicStruct;
	static constexpr auto parseValue = createValue<&value_type::i, &value_type::d, &value_type::str, &value_type::arr>();
};

struct MetaStruct {
	int32_t count{};
	std::string name{};
};

template<> struct jsonifier::core<MetaStruct> {
	using value_type				 = MetaStruct;
	static constexpr auto parseValue = createValue<makeJsonEntity<&value_type::count, "cnt">(), makeJsonEntity<&value_type::name, "label">()>();
};

struct WithOptional {
	std::string required{ "default" };
	std::optional<double> maybe{};
};

template<> struct jsonifier::core<WithOptional> {
	using value_type				 = WithOptional;
	static constexpr auto parseValue = createValue<&value_type::required, &value_type::maybe>();
};

struct EnumHolder {
	Color c{ Color::Green };
};

template<> struct jsonifier::core<EnumHolder> {
	using value_type				 = EnumHolder;
	static constexpr auto parseValue = createValue<&value_type::c>();
};

struct ContainerStruct {
	std::vector<int32_t> vec{ 1, 2, 3 };
	std::array<std::string, 2> arr{ "Hello", "World" };
	std::tuple<int32_t, double, std::string> tup{ 42, 2.718, "pi?" };
};

template<> struct jsonifier::core<ContainerStruct> {
	using value_type				 = ContainerStruct;
	static constexpr auto parseValue = createValue<&value_type::vec, &value_type::arr, &value_type::tup>();
};

struct MapStruct {
	std::map<std::string, int32_t> str_map{ { "one", 1 }, { "two", 2 } };
	std::unordered_map<int32_t, std::string> umap{ { 5, "five" }, { 7, "seven" } };
};

template<> struct jsonifier::core<MapStruct> {
	using value_type				 = MapStruct;
	static constexpr auto parseValue = createValue<&value_type::str_map, &value_type::umap>();
};

struct PrettifyStruct {
	int32_t id{};
	std::string msg{};
};

template<> struct jsonifier::core<PrettifyStruct> {
	using value_type				 = PrettifyStruct;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::msg>();
};

struct FloatPrecision {
	double val{ 3.141592653589793 };
};

template<> struct jsonifier::core<FloatPrecision> {
	using value_type				 = FloatPrecision;
	static constexpr auto parseValue = createValue<&value_type::val>();
};

struct NestedStruct {
	BasicStruct inner{};
	std::vector<int32_t> nums{};
};

template<> struct jsonifier::core<NestedStruct> {
	using value_type				 = NestedStruct;
	static constexpr auto parseValue = createValue<&value_type::inner, &value_type::nums>();
};

struct SharedPtrStruct {
	std::shared_ptr<BasicStruct> ptr{};
};

template<> struct jsonifier::core<SharedPtrStruct> {
	using value_type				 = SharedPtrStruct;
	static constexpr auto parseValue = createValue<&value_type::ptr>();
};

int main() {
	size_t passed = 0;
	size_t total  = 0;

	auto test_basic_reflection = []() {
		jsonifier::jsonifier_core<> parser{};
		BasicStruct obj{ 42, 3.14, "Hello", { 1, 2, 3 } };
		std::string json{};
		parser.serializeJson(obj, json);
		BasicStruct obj2{};
		parser.parseJson(obj2, json);
		return std::make_tuple(json, obj2.i, obj2.d, obj2.str, obj2.arr[0]);
	};

	auto test_meta_struct = []() {
		jsonifier::jsonifier_core<> parser{};
		MetaStruct obj{ 5, "Gadget" };
		std::string json{};
		parser.serializeJson(obj, json);
		MetaStruct obj2{};
		std::string input = R"({"cnt":10,"label":"Widget"})";
		parser.parseJson(obj2, input);
		return std::make_tuple(json, obj2.count, obj2.name);
	};

	auto test_optional_fields = []() {
		jsonifier::jsonifier_core<> parser{};
		WithOptional obj{};
		std::string json{};
		parser.serializeJson(obj, json);
		std::string input = R"({"required":"changed","maybe":3.1415})";
		parser.parseJson(obj, input);
		obj.maybe.reset();
		json.clear();
		parser.serializeJson(obj, json);
		return obj.required == std::string{ "changed" } && !obj.maybe.has_value() && (json.find("maybe") == std::string::npos || json.find("null") != std::string::npos);
	};

	auto test_enum_as_integer = []() {
		jsonifier::jsonifier_core<> parser{};
		EnumHolder obj{};
		std::string json{};
		parser.serializeJson(obj, json);
		EnumHolder parsed{};
		std::string input = R"({"c":2})";
		parser.parseJson(parsed, input);
		return std::make_tuple(json, parsed.c == Color::Blue);
	};

	auto test_enum_map_key = []() {
		jsonifier::jsonifier_core<> parser{};
		std::map<std::string, int32_t> obj{ { "one", 1 } };
		std::string json{};
		parser.serializeJson(obj, json);
		std::map<std::string, int32_t> parsed{};
		parser.parseJson(parsed, json);
		return parsed["one"] == 1;
	};

	auto test_containers = []() {
		jsonifier::jsonifier_core<> parser{};
		ContainerStruct c{};
		std::string json{};
		parser.serializeJson(c, json);
		ContainerStruct c2{};
		parser.parseJson(c2, json);
		return std::make_tuple(c2.vec == std::vector<int32_t>{ 1, 2, 3 }, c2.arr[0], std::get<2>(c2.tup));
	};

	auto test_map_unordered = []() {
		jsonifier::jsonifier_core<> parser{};
		MapStruct ms{};
		std::string json{};
		parser.serializeJson(ms, json);
		MapStruct ms2{};
		parser.parseJson(ms2, json);
		return (json.find("one") != std::string::npos) && ms2.str_map["one"] == 1 && ms2.umap[5] == std::string{ "five" };
	};

	auto test_prettify = []() {
		jsonifier::jsonifier_core<> parser{};
		PrettifyStruct pd{ 123, "Hello" };
		std::string json{};
		parser.serializeJson(pd, json);
		std::string pretty{};
		parser.prettifyJson(json, pretty);
		std::string minified{};
		parser.minifyJson(pretty, minified);
		return (pretty.find('\n') != std::string::npos) && minified == json;
	};

	auto test_minify = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string prettified = "{\n    \"id\": 42,\n    \"msg\": \"test\"\n}";
		std::string minified{};
		parser.minifyJson(prettified, minified);
		return minified.find("\n") == std::string::npos && minified.size() < prettified.size();
	};

	auto test_validate_valid = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = R"({"i":42,"d":3.14,"str":"Hello","arr":[1,2,3]})";
		return parser.validateJson(json);
	};

	auto test_validate_invalid = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = R"({"i":42,"d":3.14,})";
		return !parser.validateJson(json);
	};

	auto test_float_precision = []() {
		jsonifier::jsonifier_core<> parser{};
		FloatPrecision fp{};
		std::string json{};
		parser.serializeJson(fp, json);
		return json.find("3.14159") != std::string::npos;
	};

	auto test_nested_struct = []() {
		jsonifier::jsonifier_core<> parser{};
		NestedStruct ns{};
		ns.inner = { 42, 3.14, "nested", { 1, 2, 3 } };
		ns.nums	 = { 10, 20, 30 };
		std::string json{};
		parser.serializeJson(ns, json);
		NestedStruct parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.inner.i, parsed.nums.size());
	};

	auto test_shared_ptr = []() {
		jsonifier::jsonifier_core<> parser{};
		SharedPtrStruct sps{};
		sps.ptr		 = std::make_shared<BasicStruct>();
		sps.ptr->i	 = 99;
		sps.ptr->str = "shared";
		std::string json{};
		parser.serializeJson(sps, json);
		SharedPtrStruct parsed{};
		parser.parseJson(parsed, json);
		return parsed.ptr && parsed.ptr->i == 99 && parsed.ptr->str == std::string{ "shared" };
	};

	auto test_vector_of_structs = []() {
		jsonifier::jsonifier_core<> parser{};
		std::vector<BasicStruct> vec{ { 1, 1.1, "a", { 1, 2, 3 } }, { 2, 2.2, "b", { 4, 5, 6 } } };
		std::string json{};
		parser.serializeJson(vec, json);
		std::vector<BasicStruct> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.size(), parsed[0].i, parsed[1].str);
	};

	auto test_array_of_enums = []() {
		jsonifier::jsonifier_core<> parser{};
		std::array<Color, 3> arr{ Color::Red, Color::Green, Color::Blue };
		std::string json{};
		parser.serializeJson(arr, json);
		std::array<Color, 3> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed[0] == Color::Red, parsed[2] == Color::Blue);
	};

	auto test_optional_with_value = []() {
		jsonifier::jsonifier_core<> parser{};
		WithOptional obj{ "test", 5.5 };
		std::string json{};
		parser.serializeJson(obj, json);
		WithOptional parsed{};
		parser.parseJson(parsed, json);
		return parsed.maybe.has_value() && *parsed.maybe == 5.5;
	};

	auto test_optional_without_value = []() {
		jsonifier::jsonifier_core<> parser{};
		WithOptional obj{ "test", std::nullopt };
		std::string json{};
		parser.serializeJson(obj, json);
		WithOptional parsed{};
		parser.parseJson(parsed, json);
		return !parsed.maybe.has_value();
	};

	auto test_empty_containers = []() {
		jsonifier::jsonifier_core<> parser{};
		ContainerStruct c{};
		c.vec.clear();
		c.arr = { "", "" };
		std::string json{};
		parser.serializeJson(c, json);
		ContainerStruct parsed{};
		parser.parseJson(parsed, json);
		return parsed.vec.empty();
	};

	auto test_large_numbers = []() {
		jsonifier::jsonifier_core<> parser{};
		BasicStruct obj{ 2147483647, 1.7976931348623157e+308, "max", { UINT32_MAX, UINT32_MAX - 1, UINT32_MAX - 2 } };
		std::string json{};
		parser.serializeJson(obj, json);
		BasicStruct parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.i, parsed.arr[0]);
	};

	auto test_special_string_chars = []() {
		jsonifier::jsonifier_core<> parser{};
		BasicStruct obj{ 1, 1.0, "test\"quote\\slash\nnewline", { 1, 2, 3 } };
		std::string json{};
		parser.serializeJson(obj, json);
		BasicStruct parsed{};
		parser.parseJson(parsed, json);
		return parsed.str.find("\"") != std::string::npos && parsed.str.find("\n") != std::string::npos;
	};

	auto test_unicode_string = []() {
		jsonifier::jsonifier_core<> parser{};
		BasicStruct obj{ 1, 1.0, "Hello 世界 🌍", { 1, 2, 3 } };
		std::string json{};
		parser.serializeJson(obj, json);
		BasicStruct parsed{};
		parser.parseJson(parsed, json);
		return parsed.str.find("世界") != std::string::npos;
	};

	auto test_tuple_roundtrip = []() {
		jsonifier::jsonifier_core<> parser{};
		std::tuple<int32_t, double, std::string> tup{ 123, 4.56, "test" };
		std::string json{};
		parser.serializeJson(tup, json);
		std::tuple<int32_t, double, std::string> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(std::get<0>(parsed), std::get<2>(parsed));
	};

	auto test_nested_maps = []() {
		jsonifier::jsonifier_core<> parser{};
		std::map<std::string, std::map<std::string, int32_t>> nested{ { "outer1", { { "inner1", 1 }, { "inner2", 2 } } }, { "outer2", { { "inner3", 3 } } } };
		std::string json{};
		parser.serializeJson(nested, json);
		std::map<std::string, std::map<std::string, int32_t>> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed["outer1"]["inner1"], parsed["outer2"]["inner3"]);
	};

	auto test_vector_of_vectors = []() {
		jsonifier::jsonifier_core<> parser{};
		std::vector<std::vector<int32_t>> vec{ { 1, 2 }, { 3, 4, 5 } };
		std::string json{};
		parser.serializeJson(vec, json);
		std::vector<std::vector<int32_t>> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.size(), parsed[1].size(), parsed[1][2]);
	};

	total++;
	if (rt_ut::unit_test<"Basic Reflection">::assert_eq(std::make_tuple(std::string{ R"({"i":42,"d":3.14,"str":"Hello","arr":[1,2,3]})" }, 42, 3.14, std::string{ "Hello" }, 1u),
			test_basic_reflection))
		passed++;
	total++;
	if (rt_ut::unit_test<"Meta Struct Renamed Fields">::assert_eq(std::make_tuple(std::string{ R"({"cnt":5,"label":"Gadget"})" }, 10, std::string{ "Widget" }), test_meta_struct))
		passed++;
	total++;
	if (rt_ut::unit_test<"Optional Fields">::assert_eq(true, test_optional_fields))
		passed++;
	total++;
	if (rt_ut::unit_test<"Enum as Integer">::assert_eq(std::make_tuple(std::string{ R"({"c":1})" }, true), test_enum_as_integer))
		passed++;
	total++;
	if (rt_ut::unit_test<"Enum Map Key">::assert_eq(true, test_enum_map_key))
		passed++;
	total++;
	if (rt_ut::unit_test<"Containers">::assert_eq(std::make_tuple(true, std::string{ "Hello" }, std::string{ "pi?" }), test_containers))
		passed++;
	total++;
	if (rt_ut::unit_test<"Map and Unordered Map">::assert_eq(true, test_map_unordered))
		passed++;
	total++;
	if (rt_ut::unit_test<"Prettify">::assert_eq(true, test_prettify))
		passed++;
	total++;
	if (rt_ut::unit_test<"Minify">::assert_eq(true, test_minify))
		passed++;
	total++;
	if (rt_ut::unit_test<"Validate Valid">::assert_eq(true, test_validate_valid))
		passed++;
	total++;
	if (rt_ut::unit_test<"Validate Invalid">::assert_eq(true, test_validate_invalid))
		passed++;
	total++;
	if (rt_ut::unit_test<"Float Precision">::assert_eq(true, test_float_precision))
		passed++;
	total++;
	if (rt_ut::unit_test<"Nested Struct">::assert_eq(std::make_tuple(42, std::size_t{ 3 }), test_nested_struct))
		passed++;
	total++;
	if (rt_ut::unit_test<"Shared Ptr">::assert_eq(true, test_shared_ptr))
		passed++;
	total++;
	if (rt_ut::unit_test<"Vector of Structs">::assert_eq(std::make_tuple(std::size_t{ 2 }, 1, std::string{ "b" }), test_vector_of_structs))
		passed++;
	total++;
	if (rt_ut::unit_test<"Array of Enums">::assert_eq(std::make_tuple(true, true), test_array_of_enums))
		passed++;
	total++;
	if (rt_ut::unit_test<"Optional With Value">::assert_eq(true, test_optional_with_value))
		passed++;
	total++;
	if (rt_ut::unit_test<"Optional Without Value">::assert_eq(true, test_optional_without_value))
		passed++;
	total++;
	if (rt_ut::unit_test<"Empty Containers">::assert_eq(true, test_empty_containers))
		passed++;
	total++;
	if (rt_ut::unit_test<"Large Numbers">::assert_eq(std::make_tuple(2147483647, UINT32_MAX), test_large_numbers))
		passed++;
	total++;
	if (rt_ut::unit_test<"Special String Chars">::assert_eq(true, test_special_string_chars))
		passed++;
	total++;
	if (rt_ut::unit_test<"Unicode String">::assert_eq(true, test_unicode_string))
		passed++;
	total++;
	if (rt_ut::unit_test<"Tuple Roundtrip">::assert_eq(std::make_tuple(123, std::string{ "test" }), test_tuple_roundtrip))
		passed++;
	total++;
	if (rt_ut::unit_test<"Nested Maps">::assert_eq(std::make_tuple(1, 3), test_nested_maps))
		passed++;
	total++;
	if (rt_ut::unit_test<"Vector of Vectors">::assert_eq(std::make_tuple(std::size_t{ 2 }, std::size_t{ 3 }, 5), test_vector_of_vectors))
		passed++;

	std::cout << "\n========================================\n";
	std::cout << "Test Results: " << passed << "/" << total << " passed\n";
	std::cout << "========================================\n";


	auto test_char_empty = []() {
		jsonifier::jsonifier_core<> parser{};
		char_roundtrip deserialized{ 'a', 'b', 1 };
		std::string buffer{};
		parser.parseJson(deserialized, buffer);
		return std::make_tuple(deserialized.char_val, deserialized.uchar_val, deserialized.int_val);
	};

	auto test_basic_serialize = []() {
		jsonifier::jsonifier_core<> parser{};
		simple_struct obj{ 42, "test", 3.14 };
		std::string result{};
		parser.serializeJson(obj, result);
		return !result.empty() && result.find("42") != std::string::npos;
	};

	auto test_basic_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = R"({"id":42,"name":"test","value":3.14})";
		simple_struct obj{};
		parser.parseJson(obj, json);
		return std::make_tuple(obj.id, obj.name);
	};

	auto test_roundtrip = []() {
		jsonifier::jsonifier_core<> parser{};
		simple_struct original{ 99, "roundtrip", 2.71828 };
		std::string serialized{};
		parser.serializeJson(original, serialized);
		simple_struct parsed{};
		parser.parseJson(parsed, serialized);
		return std::make_tuple(parsed.id, parsed.name);
	};

	auto test_nested = []() {
		jsonifier::jsonifier_core<> parser{};
		nested_struct obj{};
		obj.inner	= { 1, "nested", 1.5 };
		obj.numbers = { 1, 2, 3, 4, 5 };
		obj.flag	= true;
		std::string json{};
		parser.serializeJson(obj, json);
		nested_struct parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.inner.id, parsed.numbers.size(), parsed.flag);
	};

	auto test_double_write = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string buffer{};
		parser.serializeJson(3.14, buffer);
		std::string a = buffer;
		parser.serializeJson(0.0, buffer);
		std::string b = buffer;
		parser.serializeJson(-0.0, buffer);
		std::string c = buffer;
		return std::make_tuple(a, b, c);
	};

	auto test_double_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		double num{};
		parser.parseJson(num, "3.14");
		double a = num;
		parser.parseJson(num, "9.81");
		double b = num;
		parser.parseJson(num, "0");
		double c = num;
		return std::make_tuple(a, b, c);
	};

	auto test_int_write = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string buffer{};
		parser.serializeJson(0, buffer);
		std::string a = buffer;
		parser.serializeJson(999, buffer);
		std::string b = buffer;
		parser.serializeJson(-6, buffer);
		std::string c = buffer;
		return std::make_tuple(a, b, c);
	};

	auto test_int_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		int32_t num{};
		parser.parseJson(num, "-1");
		int32_t a = num;
		parser.parseJson(num, "0");
		int32_t b = num;
		parser.parseJson(num, "999");
		int32_t c = num;
		return std::make_tuple(a, b, c);
	};

	auto test_bool_write = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string buffer{};
		parser.serializeJson(true, buffer);
		std::string a = buffer;
		parser.serializeJson(false, buffer);
		std::string b = buffer;
		return std::make_tuple(a, b);
	};

	auto test_bool_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		bool val{};
		parser.parseJson(val, "true");
		bool a = val;
		parser.parseJson(val, "false");
		bool b = val;
		return std::make_tuple(a, b);
	};

	auto test_string_write = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string buffer{};
		parser.serializeJson(std::string{ "fish" }, buffer);
		std::string a = buffer;
		parser.serializeJson(std::string{ "as\"df\\ghjkl" }, buffer);
		std::string b = buffer;
		return std::make_tuple(a, b);
	};

	auto test_string_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string val{};
		parser.parseJson(val, "\"fish\"");
		std::string a = val;
		parser.parseJson(val, "\"as\\\"df\\\\ghjkl\"");
		std::string b = val;
		return std::make_tuple(a, b);
	};

	auto test_vector_serialize = []() {
		jsonifier::jsonifier_core<> parser{};
		std::vector<int32_t> vec{ 1, 2, 3, 4, 5 };
		std::string json{};
		parser.serializeJson(vec, json);
		return json.find("[") != std::string::npos && json.find("]") != std::string::npos;
	};

	auto test_vector_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = "[10,20,30,40,50]";
		std::vector<int32_t> vec{};
		parser.parseJson(vec, json);
		return std::make_tuple(vec.size(), vec[0], vec[4]);
	};

	auto test_array_serialize = []() {
		jsonifier::jsonifier_core<> parser{};
		std::array<int32_t, 3> arr{ 1, 2, 3 };
		std::string json{};
		parser.serializeJson(arr, json);
		return json.find("[") != std::string::npos && json.find("]") != std::string::npos;
	};

	auto test_array_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = "[10,20,30]";
		std::array<int32_t, 3> arr{};
		parser.parseJson(arr, json);
		return std::make_tuple(arr.size(), arr[0], arr[2]);
	};

	auto test_escaped_key = []() {
		jsonifier::jsonifier_core<> parser{};
		escaped_struct obj{ 5, "bye", "" };
		std::string json{};
		parser.serializeJson(obj, json);
		escaped_struct parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.escaped_key, parsed.escaped_key2);
	};

	auto test_escaped_chars_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = R"({"escaped\"key":0,"escaped\"\"key2":"hi","escape_chars":"\b\f\n\r\t"})";
		escaped_struct obj{};
		parser.parseJson(obj, json);
		return obj.escape_chars;
	};

	auto test_enum_serialize = []() {
		jsonifier::jsonifier_core<> parser{};
		Color color = Color::Green;
		std::string json{};
		parser.serializeJson(color, json);
		return json;
	};

	auto test_enum_parse = []() {
		jsonifier::jsonifier_core<> parser{};
		std::string json = "0";
		Color color{};
		parser.parseJson(color, json);
		return color == Color::Red;
	};

	auto test_enum_array = []() {
		jsonifier::jsonifier_core<> parser{};
		std::array<Color, 3> arr{};
		std::string json = "[1,0,2]";
		parser.parseJson(arr, json);
		return std::make_tuple(arr[0] == Color::Green, arr[1] == Color::Red, arr[2] == Color::Blue);
	};

	auto test_vehicle_enum = []() {
		jsonifier::jsonifier_core<> parser{};
		Vehicle vehicle = Vehicle::Plane;
		std::string json{};
		parser.serializeJson(vehicle, json);
		Vehicle parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(json, parsed == Vehicle::Plane);
	};

	auto test_complex_struct = []() {
		jsonifier::jsonifier_core<> parser{};
		Thing obj{};
		std::string json{};
		parser.serializeJson(obj, json);
		Thing parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.i, parsed.d, parsed.c);
	};

	auto test_optional_empty = []() {
		jsonifier::jsonifier_core<> parser{};
		Thing obj{};
		obj.optional = std::nullopt;
		std::string json{};
		parser.serializeJson(obj, json);
		Thing parsed{};
		parser.parseJson(parsed, json);
		return !parsed.optional.has_value();
	};

	auto test_optional_value = []() {
		jsonifier::jsonifier_core<> parser{};
		Thing obj{};
		obj.optional = V3{ 1.0, 2.0, 3.0 };
		std::string json{};
		parser.serializeJson(obj, json);
		Thing parsed{};
		parser.parseJson(parsed, json);
		return parsed.optional.has_value() && parsed.optional->x == 1.0;
	};

	auto test_map = []() {
		jsonifier::jsonifier_core<> parser{};
		std::map<std::string, int32_t> map{ { "a", 4 }, { "f", 7 }, { "b", 12 } };
		std::string json{};
		parser.serializeJson(map, json);
		std::map<std::string, int32_t> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed["a"], parsed["f"], parsed["b"]);
	};

	auto test_dummy_data = []() {
		jsonifier::jsonifier_core<> parser{};
		std::vector<dummy_data> test_data = { { 0, 0, TestData::None, TestData::None, TestData::None, TestData::None, 0 },
			{ 1, 1, TestData::A, TestData::B, TestData::A, TestData::B, 0xDDDDDDDD }, { 2, 6, TestData::A, TestData::B, TestData::C, TestData::D, 0xEEEEEEEE },
			{ 3, -1, TestData::ERROR_E, TestData::ERROR_E, TestData::ERROR_E, TestData::ERROR_E, 0xFFFFFFFF } };
		std::string json{};
		parser.serializeJson(test_data, json);
		std::vector<dummy_data> parsed{};
		parser.parseJson(parsed, json);
		return std::make_tuple(parsed.size(), parsed[1].b == TestData::A);
	};

	total++;
	if (rt_ut::unit_test<"Char Empty String">::assert_eq(std::make_tuple(char{ 'a' }, static_cast<unsigned char>('b'), 1), test_char_empty))
		passed++;
	total++;
	if (rt_ut::unit_test<"Basic Serialize">::assert_eq(true, test_basic_serialize))
		passed++;
	total++;
	if (rt_ut::unit_test<"Basic Parse">::assert_eq(std::make_tuple(42, std::string{ "test" }), test_basic_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Roundtrip">::assert_eq(std::make_tuple(99, std::string{ "roundtrip" }), test_roundtrip))
		passed++;
	total++;
	if (rt_ut::unit_test<"Nested Structures">::assert_eq(std::make_tuple(1, std::size_t{ 5 }, true), test_nested))
		passed++;
	total++;
	if (rt_ut::unit_test<"Double Write">::assert_eq(std::make_tuple(std::string{ "3.14" }, std::string{ "0" }, std::string{ "-0" }), test_double_write))
		passed++;
	total++;
	if (rt_ut::unit_test<"Double Parse">::assert_eq(std::make_tuple(3.14, 9.81, 0.0), test_double_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Int Write">::assert_eq(std::make_tuple(std::string{ "0" }, std::string{ "999" }, std::string{ "-6" }), test_int_write))
		passed++;
	total++;
	if (rt_ut::unit_test<"Int Parse">::assert_eq(std::make_tuple(-1, 0, 999), test_int_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Bool Write">::assert_eq(std::make_tuple(std::string{ "true" }, std::string{ "false" }), test_bool_write))
		passed++;
	total++;
	if (rt_ut::unit_test<"Bool Parse">::assert_eq(std::make_tuple(true, false), test_bool_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"String Write">::assert_eq(std::make_tuple(std::string{ "\"fish\"" }, std::string{ "\"as\\\"df\\\\ghjkl\"" }), test_string_write))
		passed++;
	total++;
	if (rt_ut::unit_test<"String Parse">::assert_eq(std::make_tuple(std::string{ "fish" }, std::string{ "as\"df\\ghjkl" }), test_string_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Vector Serialize">::assert_eq(true, test_vector_serialize))
		passed++;
	total++;
	if (rt_ut::unit_test<"Vector Parse">::assert_eq(std::make_tuple(std::size_t{ 5 }, 10, 50), test_vector_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Array Serialize">::assert_eq(true, test_array_serialize))
		passed++;
	total++;
	if (rt_ut::unit_test<"Array Parse">::assert_eq(std::make_tuple(std::size_t{ 3 }, 10, 30), test_array_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Prettify JSON">::assert_eq(true, test_prettify))
		passed++;
	total++;
	if (rt_ut::unit_test<"Minify JSON">::assert_eq(true, test_minify))
		passed++;
	total++;
	if (rt_ut::unit_test<"Validate Valid">::assert_eq(true, test_validate_valid))
		passed++;
	total++;
	if (rt_ut::unit_test<"Validate Invalid">::assert_eq(true, test_validate_invalid))
		passed++;
	total++;
	if (rt_ut::unit_test<"Escaped Key">::assert_eq(std::make_tuple(5, std::string{ "bye" }), test_escaped_key))
		passed++;
	total++;
	if (rt_ut::unit_test<"Escaped Chars Parse">::assert_eq(std::string{ "\b\f\n\r\t" }, test_escaped_chars_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Enum Serialize">::assert_eq(std::string{ "1" }, test_enum_serialize))
		passed++;
	total++;
	if (rt_ut::unit_test<"Enum Parse">::assert_eq(true, test_enum_parse))
		passed++;
	total++;
	if (rt_ut::unit_test<"Enum Array">::assert_eq(std::make_tuple(true, true, true), test_enum_array))
		passed++;
	total++;
	if (rt_ut::unit_test<"Vehicle Enum">::assert_eq(std::make_tuple(std::string{ "2" }, true), test_vehicle_enum))
		passed++;
	total++;
	if (rt_ut::unit_test<"Complex Struct">::assert_eq(std::make_tuple(8, 2.0, 'W'), test_complex_struct))
		passed++;
	total++;
	if (rt_ut::unit_test<"Optional Empty">::assert_eq(true, test_optional_empty))
		passed++;
	total++;
	if (rt_ut::unit_test<"Optional Value">::assert_eq(true, test_optional_value))
		passed++;
	total++;
	if (rt_ut::unit_test<"Map">::assert_eq(std::make_tuple(4, 7, 12), test_map))
		passed++;
	total++;
	if (rt_ut::unit_test<"Shared Ptr">::assert_eq(true, test_shared_ptr))
		passed++;
	total++;
	if (rt_ut::unit_test<"Dummy Data Vector">::assert_eq(std::make_tuple(std::size_t{ 4 }, true), test_dummy_data))
		passed++;

	std::cout << "\n========================================\n";
	std::cout << "Test Results: " << passed << "/" << total << " passed\n";
	std::cout << "========================================\n";

	try {
		tests::testFunction();
	} catch (std::runtime_error& error) {
		std::cout << error.what() << std::endl;
	} catch (std::out_of_range& error) {
		std::cout << error.what() << std::endl;
	}
	return 0;
}