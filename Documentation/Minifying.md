## Minifying JSON Data with Jsonifier

Jsonifier provides a `minifyJson` function for quickly minifying JSON data, which supports two overloads.

#### Two Overloads
The `minifyJson` function comes in two overloads to provide flexibility:

```cpp
template<jsonifier::concepts::string_t string_type>  auto minifyJson(string_type&& in) noexcept;

template<jsonifier::concepts::string_t string_type01, jsonifier::concepts::string_t string_type02>
 bool minifyJson(string_type01&& in, string_type02&& out) noexcept;
```

These overloads allow you to choose between directly minifying a string or outputting the minified JSON data to another string.

#### Examples
Here are examples demonstrating the usage of the `minifyJson` function:

#### Example 1: Minifying JSON data
```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer = "{ \"key\": \"value\" }";

// Minify JSON data.
auto minifiedBuffer = jsonifier::minifyJson(buffer);
```

#### Example 2: Minifying JSON data and outputting to another string
```cpp
#include "jsonifier/Index.hpp"

jsonifier::string buffer = "{ \"key\": \"value\" }";
jsonifier::string minifiedBuffer;

// Minify JSON data and output to another string.
jsonifier::minifyJson(buffer, minifiedBuffer);
```

These examples demonstrate how to minifyJson JSON data using the `minifyJson` function, both directly and by outputting to another string.

---
