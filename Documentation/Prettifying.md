## Prettifying JSON Data with Jsonifier

Jsonifier can prettify JSON data rapidly using simd instructions. It can do this either explicitly through the prettify function or using the template parameter of the serializeJson function.

### Using the `prettify` Function
----
To use the `jsonifier_core::prettify()` function, call it as follows:

```cpp
#include "jsonifier/Index.hpp"

jsonifier::jsonifier_core parser{};
jsonifier::string buffer = "{\"key\": \"value\"}";

// Prettify JSON data.
buffer = parser.prettify(buffer);
```

### Using `serializeJson`'s  template parameter
----
Simply call `jsonifier_core::serializeJson()` with its first template parameter set to `true`.

```cpp
#include "jsonifier/Index.hpp"

jsonifier::jsonifier_core parser{};
discord_test discordTest{};
jsonifier::string buffer{};

// Serialize and automatically prettify JSON data.
parser.serializeJson<true>(discordTest, buffer);
```
