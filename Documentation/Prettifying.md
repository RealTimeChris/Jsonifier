## Prettifying JSON Data with Jsonifier

Jsonifier provides a `prettifyJson` function for quickly prettifying JSON data, which now supports two overloads.

#### Two Overloads
The `prettifyJson` function comes in two overloads to provide flexibility:

```cpp
template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type>
 auto prettifyJson(string_type&& in) noexcept;

template<jsonifier::prettify_options options = jsonifier::prettify_options{}, jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
 bool prettify(string_type01&& in, string_type02&& out) noexcept;
```

These overloads allow you to customize prettifying behavior and handle output flexibly.

#### Examples
Here are examples demonstrating the usage of the `prettifyJson` function:

#### Example 1: Prettifying JSON data
```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer = "{\"key\": \"value\"}";

// Prettify JSON data.
auto prettifiedBuffer = jsonifier::prettifyJson(buffer);
```

#### Example 2: Prettifying JSON data and outputting to another string
```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer = "{\"key\": \"value\"}";
jsonifier::string prettifiedBuffer;

// Prettify JSON data and output to another string.
jsonifier::prettifyJson(buffer, prettifiedBuffer);
```

These examples demonstrate how to prettifyJson JSON data using the `prettifyJson` function, both directly and by outputting to another string.

#### Prettify Options
The `prettify_options` struct allows customization of prettifying behavior. Here's the structure of the `prettify_options`:

```cpp
struct prettify_options {
    bool newLinesInArray{ true };
    uint64_t indentSize{ 4 };
    uint64_t maxDepth{ 100 };
    bool tabs{ false };
};
```

- `newLinesInArray`: Specifies whether new lines are allowed within arrays (default: `true`).
- `indentSize`: Sets the size of indentation (default: `4`).
- `maxDepth`: Specifies the maximum depth for nested objects (default: `100`).
- `tabs`: Indicates whether to use tabs for indentation (default: `false`).

You can specify these options when calling the `prettifyJson` function to customize the prettifying behavior.