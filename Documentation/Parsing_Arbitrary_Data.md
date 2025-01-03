## Using `jsonifier` for Parsing Unknown or Arbitrary JSON Structures

`jsonifier` offers powerful tools for handling JSON data with flexibility and efficiency. This document explains how to use the library to parse JSON when the structure is unknown at runtime or to verify the presence of values at specific keys.

### Key Components

#### 1. **Custom Structure (`custom_struct_raw`)**

This structure models the data we wish to serialize or deserialize:

```cpp
struct custom_struct_raw {
    std::string testString{ "test_string" };
    int64_t testInt{ 234234 };
    bool testBool{true};
    std::vector<std::string> testVector{ "test01", "test02", "test03", "test04" };
};
```

#### 2. **Mapping the Structure**

The `jsonifier::core` template specialization enables automatic parsing and serialization of `custom_struct_raw`:

```cpp
template<> struct jsonifier::core<custom_struct_raw> {
    static constexpr auto parseValue = createValue<
        &custom_struct_raw::testBool,
        &custom_struct_raw::testInt,
        &custom_struct_raw::testString,
        &custom_struct_raw::testVector
    >();
};
```

This mapping specifies how the structure's fields map to JSON keys.

#### 3. **`jsonifier::raw_json_data`**

This flexible data structure can:
- Parse and hold arbitrary JSON data, allowing dynamic exploration of its contents.
- Be used to verify that certain keys or values exist, regardless of the schema.

#### 4. **`jsonifier::jsonifier_core`**

This is the core parser and serializer for handling JSON data. It:
- Serializes structures into JSON strings.
- Parses JSON strings into the target data structure or `raw_json_data` for dynamic exploration.

---

### Parsing Arbitrary JSON Structures

#### Example Workflow

The following example demonstrates how to parse JSON with an unknown structure or validate key existence.

#### Example 1: Serialize a Custom Structure

Use `jsonifier_core` to serialize a `custom_struct_raw` instance:

```cpp
custom_struct_raw testData{};
jsonifier::jsonifier_core parser{};
std::string testString{};
parser.serializeJson(testData, testString);
std::cout << "CURRENT DATA: " << testString << std::endl;
```

Output:

```json
{
    "testString": "test_string",
    "testInt": 234234,
    "testBool": true,
    "testVector": ["test01", "test02", "test03", "test04"]
}
```

#### Example 2: Parse JSON into `raw_json_data`

```cpp
jsonifier::raw_json_data testRawData{};
parser.parseJson(testRawData, testString);
std::cout << "CURRENT VALUES: " << testRawData.rawJson() << std::endl;
```

This populates `testRawData` with a dynamic representation of the JSON.

#### Example 3: Dynamic Exploration of JSON Data

Using `getObject` and `getArray`, explore the parsed JSON dynamically:

```cpp
for (auto& [key, value] : testRawData.getObject()) {
    if (value.getType() == jsonifier::json_type::array) {
        for (auto& valueNew : value.getArray()) {
            std::cout << "Array Value: " << valueNew.rawJson() << std::endl;
        }
    } else if (value.getType() != jsonifier::json_type::unset) {
        std::cout << "Key " << key << " was parsed successfully." << std::endl;
    }
    std::cout << "Key: " << key << ", Value: " << value.rawJson() << std::endl;
}
```

#### Output Verification

For the given JSON:

```json
{
    "testString": "test_string",
    "testInt": 234234,
    "testBool": true,
    "testVector": ["test01", "test02", "test03", "test04"]
}
```

The output might be:

```
Key: testString, Value: "test_string"
Key testString was parsed successfully.
Key: testInt, Value: 234234
Key testInt was parsed successfully.
Key: testBool, Value: true
Key testBool was parsed successfully.
Array Value: "test01"
Array Value: "test02"
Array Value: "test03"
Array Value: "test04"
Key: testVector, Value: ["test01", "test02", "test03", "test04"]
```

---

### Applications

#### 1. **Parsing Arbitrary JSON**

The `raw_json_data` structure is ideal when the JSON schema is unknown. It provides dynamic access to JSON contents and type information.

#### 2. **Validation of Key-Value Pairs**

This approach ensures that certain keys are present in the parsed JSON and allows verification of their types and values.

#### 3. **Interoperability**

Integrate `jsonifier` into systems that require flexible JSON handling without prior knowledge of the schema.

---

### Conclusion

The `jsonifier` library and its `raw_json_data` structure provide a robust way to parse, validate, and explore JSON data dynamically. By combining serialization with dynamic parsing, it allows developers to handle both known and unknown JSON structures efficiently.

--- 