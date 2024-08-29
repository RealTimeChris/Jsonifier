## Registering a Structure Using Reflection with Jsonifier

### Overview

Reflection is a powerful programming feature that enables a program to examine and modify its own structure and behavior at runtime. In the context of C++, reflection is not a built-in feature, but it can be emulated using techniques such as template metaprogramming.

Jsonifier leverages reflection to dynamically parse JSON data into C++ structures without requiring explicit manual parsing code for each structure. This allows for more flexible and maintainable code, as changes to the structure do not necessitate corresponding changes to the parsing logic.

### Example

Consider the following usage example:

```cpp
// Define the structure to be registered
struct available_tag {
    jsonifier::string name;
    bool moderated;
    int32_t id;
};

// Register the structure using reflection
template<> struct jsonifier::core<available_tag> {
    using value_type = available_tag;
    constexpr decltype(auto) parseValue = createValue<&value_type::name, &value_type::moderated, &value_type::id>();
};

// Define JSON data and the structure instance
jsonifier::string testData{};
available_tag tagsTest{};
jsonifier::jsonifier_core parser{};

// Parse JSON data to populate the structure
parser.parseJson(tagsTest, testData);
```

In this example:

- We define a structure named `available_tag` with members `name`, `moderated`, and `id`.
- We then specialize the `jsonifier::core` template for this structure, providing reflection capabilities to parse JSON data into the structure.
- Finally, we demonstrate how to parse JSON data into an instance of `available_tag` using the `jsonifier_core` instance's `parseJson` method.

### Usage

#### 1. Define Structure
----
Define the structure you want to register with Jsonifier, ensuring it contains the necessary members.

```cpp
struct available_tag {
    jsonifier::string name;
    bool moderated;
    int32_t id;
};
```

#### 2. Register Structure Using Reflection
----
Specialize the `jsonifier::core` template for the structure, providing the necessary reflection information for parsing.

```cpp
template<> struct jsonifier::core<available_tag> {
    using value_type = available_tag;
    constexpr decltype(auto) parseValue = createValue<&value_type::name, &value_type::moderated, &value_type::id>();
};
```

#### 3. Parse JSON Data into Structure
----
Instantiate a `jsonifier_core` object and use its `parseJson` method to populate the structure from JSON data.

```cpp
jsonifier::string testData{};
available_tag tagsTest{};
jsonifier::jsonifier_core parser{};
parser.parseJson(tagsTest, testData);
```

This completes the process of registering a structure using reflection with Jsonifier and parsing JSON data into the structure. Reflection empowers Jsonifier to dynamically adapt to changes in structure, enhancing code flexibility and maintainability.