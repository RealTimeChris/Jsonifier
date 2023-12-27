## Excluding Keys from Serialization at Runtime with Jsonifier
----
To exclude certain keys from being serialized at runtime using the Jsonifier library, you can create a member in your object called jsonifierExcludedKeys and add the keys you want to exclude to this set. You can then call the `serializeJson` member function of the `jsonifier::jsonifier_core` class with `true` passed into its first template parameter, to serialize the object to a JSON string, excluding the keys in the `jsonifierExcludedKeys` set.

Here's an example of how you can do this:
```c++
#include <jsonifier/Index.hpp>
#include <set>

class MyObject {
public:
  jsonifier::string name;
  int32_t age;
  std::set<jsonifier::string> jsonifierExcludedKeys;

  MyObject(const jsonifier::string& n, int32_t a) : name(n), age(a) {
    jsonifierExcludedKeys.insert("age"); // add "age" key to jsonifierExcludedKeys set
  }
};

int32_t main() {
  MyObject obj("John", 30);
  jsonifier::jsonifier_core jsonifier{};
  jsonifier::string jsonBuffer{};
  jsonifier.serializeJson<true>(obj, jsonBuffer); // {"name":"John"}
  return 0;
}
```

In this example, we have a class called `MyObject` with three member variables: `name`, `age`, and `jsonifierExcludedKeys`. The `jsonifierExcludedKeys` variable is a set of strings that will contain the keys we want to exclude from the serialized output.

In the constructor of `MyObject`, we add the key "age" to the `jsonifierExcludedKeys` set using the `insert` function. This means that when we serialize this object using the `serializeJson` member function of the `jsonifier::jsonifier_core` class, the "age" key will be excluded from the resulting JSON string.

In the `main` function, we create an instance of `MyObject` with the name "John" and age 30. We then create an instance of `jsonifier::jsonifier_core` and call its `serializeJson` member function to serialize the object to a JSON string. Since we added the "age" key to the `jsonifierExcludedKeys` set in the constructor, the resulting JSON string only contains the "name" key.

By using the `jsonifierExcludedKeys` member variable and adding keys to the set, you can easily exclude certain keys from being serialized at runtime using the Jsonifier library. And with the `serializeJson` member function of the `jsonifier::jsonifier_core` class, you can easily serialize objects with excluded keys to JSON strings.
