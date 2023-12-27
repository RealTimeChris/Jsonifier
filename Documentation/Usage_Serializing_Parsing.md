## Serialization/Parsing with Jsonifier
----
- Create a specialization of the `jsonifier::core` class template for whichever data structure you would like to parse/serialize, within the Jsonifier namespace as follows...
----
```cpp
namespace TestNS {

	struct fixed_object_t {
		std::vector<int32_t> int_array;
		std::vector<float> float_array;
		std::vector<double> double_array;
	};

	struct fixed_name_object_t {
		jsonifier::string name0{};
		jsonifier::string name1{};
		jsonifier::string name2{};
		jsonifier::string name3{};
		jsonifier::string name4{};
	};

	struct nested_object_t {
		std::vector<std::array<double, 3>> v3s{};
		jsonifier::string id{};
	};

	struct another_object_t {
		jsonifier::string string{};
		jsonifier::string another_string{};
		bool boolean{};
		nested_object_t nested_object{};
	};

	struct obj_t {
		fixed_object_t fixed_object{};
		fixed_name_object_t fixed_name_object{};
		another_object_t another_object{};
		std::vector<jsonifier::string> string_array{};
		jsonifier::string string{};
		double Number{};
		bool boolean{};
		bool another_bool{};
	};
}

namespace jsonifier {

	template<> struct core<TestNS::fixed_object_t> {
		using value_type = TestNS::fixed_object_t;
		static constexpr auto parseValue = createValue("int_array", &value_type::int_array, "float_array", &value_type::float_array, "double_array", &value_type::double_array);
	};

	template<> struct core<TestNS::fixed_name_object_t> {
		using value_type = TestNS::fixed_name_object_t;
		static constexpr auto parseValue = createValue("name0", &value_type::name0, "name1", &value_type::name1, "name2", &value_type::name2, "name3", &value_type::name3, "name4", &value_type::name4);
	};

	template<> struct core<TestNS::nested_object_t> {
		using value_type = TestNS::nested_object_t;
		static constexpr auto parseValue = createValue("v3s", &value_type::v3s, "id", &value_type::id);
	};

	template<> struct core<TestNS::another_object_t> {
		using value_type = TestNS::another_object_t;
		static constexpr auto parseValue =
			createValue("string", &value_type::string, "another_string", &value_type::another_string, "boolean", &value_type::boolean, "nested_object", &value_type::nested_object);
	};

	template<> struct core<TestNS::obj_t> {
		using value_type = TestNS::obj_t;
		static constexpr auto parseValue =
			createValue("fixed_object", &value_type::fixed_object, "fixed_name_object", &value_type::fixed_name_object, "another_object", &value_type::another_object, "string_array",
				&value_type::string_array, "string", &value_type::string, "Number", &value_type::Number, "boolean", &value_type::boolean, "another_bool", &value_type::another_bool);
	};
}

```

### Usage - Parsing
----
- Create an instance of the `jsonifier::jsonifier_core` class, and pass to its function `parseJson()` a reference to the intended parsing target, along with a reference to a `jsonifier::string` or equivalent, to be parsed from, as follows...
- Note: You can save parsing time by reusing a previously-allocated object, that has been used for previous parses.
```cpp
jsonifier::string buffer{ json0 };

obj_t obj{};

jsonifier::jsonifier_core<> parser{};
parser.parseJson(obj, buffer);
```

### Usage - Serialization
----
- Create an instance of the `jsonifier::jsonifier_core` class, and pass to its function `serializeJson()` a reference to the intended serialization target, along with a reference to a `jsonifier::string` or equivalent, to be serialized into, as follows...
- Note: You can save serialization time by reusing a previously-allocated buffer, that has been used for previous serializations.
```cpp
jsonifier::string buffer{};

obj_t obj{};

jsonifier::jsonifier_core<> serializer{};
serializer.serializeJson(obj, buffer);
```