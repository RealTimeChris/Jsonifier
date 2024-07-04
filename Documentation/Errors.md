## Handling Parsing Errors with Jsonifier

Jsonifier allows you to collect and handle possible parsing errors during the JSON parsing process. To check for and display these errors, follow these steps:

1. After parsing the JSON, you can call the `getErrors` method on the `jsonifier::jsonifier_core` instance to retrieve a vector of error objects.

```cpp
auto errors = jsonifier.getErrors();
```

2. Check if any errors were reported:

```cpp
for (const auto& error : errors) {
    std::cout << "Jsonifier Error: " << error.reportError() << std::endl;
}
```

This code snippet iterates through the error objects and prints a description of each error.

## Example

Here's a complete example of parsing JSON data and handling errors:

```cpp
#include <jsonifier/Index.hpp>
#include <iostream>

int32_t main() {
    jsonifier::string buffer{ your_json_string };
    obj_t obj;
    jsonifier::jsonifier_core jsonifier;

    jsonifier.parseJson(obj, buffer);

    auto errors = jsonifier.getErrors();
    for (const auto& error : errors) {
        std::cout << "Jsonifier Error: " << error.reportError() << std::endl;
    }

    // Process the parsed data in 'obj' here.

    return 0;
}
```

### Interpreting Error Messages:
----
With error message output enabled, Jsonifier will provide detailed information about parsing errors encountered during the process. When a parsing error occurs, Jsonifier will output an error message similar to the following:

```ruby
Failed to collect a 0x2Du, at index: 486 instead found a 'i', in file: C:\Users\Chris\source\repos\Jsonifier\Build\Windows-Release-Dev\_deps\jsonifier-src\Include\jsonifier/Parse_Impl.hpp, at: 182:44, in function: void __cdecl JsonifierImpl::ParseNoKeys::impl<true,struct DiscordCoreImpl::WebSocketMessage>(struct DiscordCoreImpl::WebSocketMessage &,class JsonifierImpl::SimdStructuralIterator &)().
```
In the provided error message:

 #### Failed to collect __**a**__:
 ----
 Indicates that a Comma was expected at a particular point in the JSON data.
 #### At __**index**__: 
 ----
 486 instead found a 'i': Specifies the index in the JSON data where the error occurred and the actual character found instead of the expected Comma.
 #### In __**file**__: 
 ----
 Gives the file path where the parsing error was encountered.
 #### At: __**182:44**__: 
 ----
 Specifies the line Number and column Number within the file where the error occurred.
 #### In __**function**__: 
 ----
 Provides information about the specific function where the parsing error occurred.

 When you receive an error message, carefully review the provided information to understand the cause of the parsing error. Use this information to identify the part of the JSON data that caused the issue and take appropriate steps to resolve it.
 
## Conclusion
----
Jsonifier makes parsing JSON in C++ easy and provides a convenient way to handle parsing errors. Refer to the [official documentation](https://github.com/RealTimeChris/jsonifier) for more details and advanced usage.

Feel free to explore Jsonifier and incorporate it into your projects for efficient JSON parsing and serialization.

Happy coding!
