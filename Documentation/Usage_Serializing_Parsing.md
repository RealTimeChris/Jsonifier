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
		constexpr auto parseValue = createValue("int_array", &value_type::int_array, "float_array", &value_type::float_array, "double_array", &value_type::double_array);
	};

	template<> struct core<TestNS::fixed_name_object_t> {
		using value_type = TestNS::fixed_name_object_t;
		constexpr auto parseValue = createValue("name0", &value_type::name0, "name1", &value_type::name1, "name2", &value_type::name2, "name3", &value_type::name3, "name4", &value_type::name4);
	};

	template<> struct core<TestNS::nested_object_t> {
		using value_type = TestNS::nested_object_t;
		constexpr auto parseValue = createValue("v3s", &value_type::v3s, "id", &value_type::id);
	};

	template<> struct core<TestNS::another_object_t> {
		using value_type = TestNS::another_object_t;
		constexpr auto parseValue =
			createValue("string", &value_type::string, "another_string", &value_type::another_string, "boolean", &value_type::boolean, "nested_object", &value_type::nested_object);
	};

	template<> struct core<TestNS::obj_t> {
		using value_type = TestNS::obj_t;
		constexpr auto parseValue =
			createValue("fixed_object", &value_type::fixed_object, "fixed_name_object", &value_type::fixed_name_object, "another_object", &value_type::another_object, "string_array",
				&value_type::string_array, "string", &value_type::string, "Number", &value_type::Number, "boolean", &value_type::boolean, "another_bool", &value_type::another_bool);
	};
}

```

### Usage - Parsing
----
Jsonifier provides flexible JSON parsing capabilities through the `parseJson` function, which now supports two overloads.

#### Two Overloads
The `parseJson` function now comes in two flavors:

```cpp
template<jsonifier::parse_options options = jsonifier::parse_options{}, typename value_type, jsonifier::concepts::string_t buffer_type>
 bool parseJson(value_type&& object, buffer_type&& in);

template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, jsonifier::concepts::string_t buffer_type>
 value_type parseJson(buffer_type&& in);
```

These overloads provide flexibility in parsing JSON data, allowing you to choose between parsing directly into an existing object or creating a new object and returning it.

#### Example - Parsing into an Existing Object
Here's an example demonstrating how to use `parseJson` to parse JSON data directly into an existing object:

```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer{ json_data };

obj_t obj{};

// Create an instance of the jsonifier_core class.
jsonifier::jsonifier_core<> parser{};

// Parse JSON data into obj.
parser.parseJson(obj, buffer);
```

#### Example - Parsing into a New Object
Here's an example demonstrating how to use `parseJson` to parse JSON data into a new object:

```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer{ json_data };

// Parse JSON data and obtain the parsed object directly.
jsonifier::parse_options options;
options.optionsReal.minified = true; // Set parse options if needed.
obj_t parsedObject = jsonifier::parseJson<obj_t, options>(buffer);
```

#### Parse Options
The `parse_options` struct allows customization of parsing behavior. Here's the structure of the `parse_options`:

```cpp
struct parse_options {
    bool refreshString{ true };
    bool minified{ false };
};
```

- `refreshString`: Indicates whether to refresh the parsing string before parsing (default: `true`).
- `minified`: Indicates whether the input JSON string is minified (default: `false`).

You can customize parsing behavior by setting these options in `parse_options` when calling the `parseJson` function.

### Usage - Serialization
----
Jsonifier offers flexibility in serializing JSON data through the `serializeJson` function, which now supports two overloads.

#### Two Overloads
The `serializeJson` function now comes in two flavors:

```cpp
template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type, jsonifier::concepts::buffer_like buffer_type>
 bool serializeJson(value_type&& object, buffer_type&& out);

template<jsonifier::serialize_options options = jsonifier::serialize_options{}, typename value_type>
 jsonifier::string serializeJson(value_type&& object);
```

These overloads provide flexibility in how you handle serialization output, allowing you to choose between directly serializing into a buffer or obtaining the serialized JSON string as a return value.

#### Example - Serializing into a Buffer
Here's an example demonstrating how to use `serializeJson` to serialize data directly into a buffer:

```cpp
#include "jsonifier/Index.hpp"

obj_t obj{};

// Create an instance of the jsonifier_core class.
jsonifier::jsonifier_core<> serializer{};

// Serialize obj into a buffer.
jsonifier::string buffer{};
serializer.serializeJson(obj, buffer);
```

#### Example - Obtaining Serialized JSON String
Here's an example demonstrating how to use `serializeJson` to obtain the serialized JSON string:

```cpp
#include "jsonifier/Index.hpp"

obj_t obj{};

// Serialize and obtain the serialized JSON string directly.
jsonifier::serialize_options options;
options.prettify = true; // Enable prettifying
options.prettifyOptions.indentSize = 2; // Set custom prettifyJson options if needed.
jsonifier::string serializedString = jsonifier::serializeJson<options>(obj);
```

#### Serialize Options
The `serialize_options` struct allows customization of serialization behavior. Here's the structure of the `serialize_options`:

```cpp
struct serialize_options {
    prettify_options prettifyOptions{};
    bool prettify{ false };
};
```

- `prettifyOptions`: Specifies prettifyJson options such as `indentSize`, `maxDepth`, etc.
- `prettifyJson`: Indicates whether to prettifyJson the JSON output (default: `false`).

You can enable prettifying by setting `options.prettifyJson` to `true` and customize prettifyJson options as needed in `options.prettifyOptions`.