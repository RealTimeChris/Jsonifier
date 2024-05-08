## Validating JSON Data with Jsonifier

Jsonifier automatically validates JSON data against RFC standards when using the `parseJson` function. Below is an example of how to validateJson JSON using the `jsonifier_core::validateJson()` function.

### Using the `validateJson` Function
----
To use the `jsonifier_core::validateJson()` function, call it as follows. It returns true if successful or false if validation fails. Additionally, you can follow the [steps here](https://github.com/RealTimeChris/Jsonifier/blob/main/Documentation/Errors.md) to check possible validation errors.

```cpp
#include "jsonifier/Index.hpp"

jsonifier::jsonifier_core parser{};
jsonifier::string buffer = "{\"key\": \"value\"}";

// Validate JSON data
parser.validateJson(buffer);

// Print errors
for (auto& value : parser.getErrors()) {
    std::cout << "Jsonifier Error: " << value << std::endl;
}
```

### Using `parseJson` (Automatic Validation)
----
Jsonifier automatically validates JSON data during the parsing process. Simply call `jsonifier_core::parseJson()`.

```cpp
#include "jsonifier/Index.hpp"

jsonifier::jsonifier_core parser{};
twitter_message discordTest{};
jsonifier::string buffer = "{\"key\": \"value\"}";

// Parse and automatically validateJson JSON
parser.parseJson(discordTest, buffer);

// Print errors
for (auto& value : parser.getErrors()) {
    std::cout << "Jsonifier Error: " << value << std::endl;
}
```

In both examples, the `validateJson` function checks if the provided JSON data is valid according to RFC standards. Additionally, when using `parseJson`, JSON validation is performed automatically during the parsing process.