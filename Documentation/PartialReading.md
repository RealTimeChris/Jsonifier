## Enabling Partial JSON Reads with Jsonifier

Jsonifier allows you to configure the parser to handle partial JSON data or unordered keys without throwing errors. This is particularly useful when you expect JSON with a flexible structure or if you are only interested in specific parts of the data. Here's how to enable this feature.

### Configuration

To enable partial reading in Jsonifier, you set the `partialRead` option in the `parse_options` structure.

#### Option Details

- **partialRead**: Allows the parser to read JSON partially without requiring a fully matching structure.

### Example Usage

Here is an example demonstrating how to configure these options when setting up Jsonifier for partial reading:

```cpp
#include <jsonifier/Index.hpp>

int32_t main() {
    jsonifier::jsonifier_core parser{};
    test_data_type testData{};
    jsonifier::string newBuffer{ your_json_string };

    parser.parseJson<jsonifier::parse_options{ .partialRead = true }>(testData, newBuffer);

    // Process 'testData' here.

    return 0;
}
```
