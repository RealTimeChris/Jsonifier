## Optimizing For Minified Json

Jsonifier allows you to optimize parsing speed if you know that the data will be minified, here is how:

1. When parsing the JSON, you can set the `minified` member of the `parse_options` structure, which gets passed as the first or second template argument of `parseJson` to true.

## Example

Here's a complete example of parsing JSON data with minified optimization enabled:

```cpp
#include <jsonifier/Index.hpp>
#include <iostream>

int32_t main() {
    jsonifier::string buffer{ your_json_string };
    obj_t obj;
    jsonifier::jsonifier_core jsonifier;

    jsonifier.parseJson<jsonifier::parse_options{ .minified = true }>(obj, buffer);

    // Process the parsed data in 'obj' here.

    return 0;
}
```

## Conclusion
----
Jsonifier makes parsing JSON in C++ easy and provides a convenient way to optimize when parsing minified json data. Refer to the [official documentation](https://github.com/RealTimeChris/jsonifier) for more details and advanced usage.

Feel free to explore Jsonifier and incorporate it into your projects for efficient JSON parsing and serialization.

Happy coding!
