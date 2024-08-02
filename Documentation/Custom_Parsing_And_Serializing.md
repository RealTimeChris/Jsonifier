# Custom JSON Parsing/Serializing Behavior

## Overview

The `jsonifier` library provides a flexible approach to customizing JSON parsing and serialization. This is achieved through the `parser::impl` and `serialize_impl` structures, allowing developers to define specific behaviors based on their requirements.

## Parsing with `parser::impl`

### Structure

The `parser::impl` structure is designed to handle the parsing of JSON data with a focus on customization. It utilizes template parameters such as `value_type_new` and `derived_type`.

### Example

```cpp
template<jsonifier::concepts::bool_t value_type_new, typename derived_type>
struct parser::impl<value_type_new, derived_type> : public derailleur {
    template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator>
     void impl(value_type&& value, iterator&& iter) {
        // Custom parsing logic goes here
    }
};
```

## Serialization with `serialize_impl`

### Structure

The `serialize_impl` structure complements parsing by defining how JSON data should be serialized. It also uses template parameters like `value_type_new` and `derived_type`.

### Example

```cpp
template<jsonifier::concepts::bool_t value_type_new, typename derived_type>
struct serialize_impl<value_type_new, derived_type> {
    template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
     void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
        // Custom serialization logic goes here
    }
};
```

Certainly! The idea of creating your own specialization of `parser::impl` and `serialize_impl` is about tailoring the behavior of the JSON parsing and serialization process for specific types or situations that may not be adequately covered by the generic implementations.

### Specialization for Time Stamps

In the provided example, there's a concept named `time_stamp_t`, which represents a specific type of data, presumably a timestamp from a Discord API. This concept is then used to create a specialization for the `serialize_impl` and `parser::impl` structures.

#### Serialization Specialization

Let's break down the `serialize_impl` specialization for time stamps:

```cpp
template<time_stamp_t value_type_new, typename derived_type>
struct serialize_impl<value_type_new, derived_type> {
    template<time_stamp_t value_type, jsonifier::concepts::buffer_like iterator>
     void impl(value_type&& value, iterator&& iter, uint64_t& index) {
        jsonifier::string newString{ static_cast<jsonifier::string>(value) };
        serializer<derived_type>::impl(newString, iter, index);
    }
};
```

Here, the `serialize_impl` specialization is created for the `time_stamp_t` concept. The `impl` method inside this specialization is where you can define how time stamps should be serialized. This allows you to have precise control over the JSON representation of time stamp data.

#### Parsing Specialization

Similarly, the `parser::impl` specialization for time stamps is structured as follows:

```cpp
template<time_stamp_t value_type_new, typename derived_type>
struct parser::impl<value_type_new, derived_type> {
    template<time_stamp_t value_type, jsonifier::concepts::is_fwd_iterator iterator>
     void impl(value_type&& value, iterator&& iter) {
        jsonifier::string newString{};
        parser<derived_type>::impl(newString, iter);
        value = static_cast<jsonifier::string>(newString);
    };
};
```

In this case, the `impl` method within the `parser::impl` specialization enables you to define how time stamps should be parsed from the JSON representation. This customization ensures that the library handles time stamp data according to your specific requirements.

### Practical Use

For practical use, developers might want to create specializations for types that require non-trivial handling during JSON parsing and serialization. It allows for a more fine-grained control over the conversion process, especially when dealing with complex data structures or types that deviate from standard JSON conventions.

By creating these specializations, you can adapt the `jsonifier` library to accommodate the unique characteristics of your data types, ensuring a seamless integration with the broader JSON handling framework.