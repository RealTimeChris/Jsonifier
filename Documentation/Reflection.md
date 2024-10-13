
# Registering a Structure Using Reflection with Jsonifier

### Overview

Reflection is a powerful programming feature that enables a program to examine and modify its own structure and behavior at runtime. In the context of C++, reflection is not a built-in feature, but it can be emulated using techniques such as template metaprogramming.

Jsonifier leverages reflection to dynamically parse JSON data into C++ structures without requiring excessive explicit manual parsing code for each structure. This allows for more flexible and maintainable code, as changes to the structure do not necessitate corresponding changes to the parsing logic.

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
    static constexpr auto parseValue = createValue<&value_type::name, &value_type::moderated, &value_type::id>();
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

### Renaming Members in JSON Representation

If you need to rename members in the JSON output instead of having them reflected by their original member names, you can do so by specifying alternative names directly in the `createValue` function. This allows for more control over the serialization process.

Here’s how you can achieve this:

```cpp
// Define the structure with original member names
struct twitter_user_data {
    jsonifier::string name;
    jsonifier::string screen_name;
    jsonifier::string location;
    // other members...
};

// Register the structure with renamed JSON keys
template<> struct jsonifier::core<twitter_user_data> {
    using value_type = twitter_user_data;

    static constexpr auto parseValue =
        createValue<&value_type::id, &value_type::id_str, &value_type::name, &value_type::screen_name, &value_type::location, 
            &value_type::description, &value_type::url, &value_type::entities, &value_type::followers_count, 
            &value_type::friends_count, &value_type::listed_count, &value_type::created_at, &value_type::favourites_count, 
            &value_type::utc_offset, &value_type::time_zone, &value_type::geo_enabled, &value_type::verified, 
            &value_type::statuses_count, &value_type::lang, &value_type::contributors_enabled, 
            &value_type::is_translator, &value_type::is_translation_enabled, &value_type::profile_background_color, 
            &value_type::profile_background_image_url, &value_type::profile_background_image_url_https, 
            &value_type::profile_background_tile, &value_type::profile_image_url, &value_type::profile_image_url_https, 
            &value_type::profile_banner_url, &value_type::profile_link_color, 
            &value_type::profile_sidebar_border_color, &value_type::profile_sidebar_fill_color, 
            &value_type::profile_text_color, &value_type::profile_use_background_image, 
            &value_type::default_profile, &value_type::default_profile_image, 
            &value_type::following, &value_type::follow_request_sent, &value_type::notifications>(
            "protected", &value_type::protectedVal);
};
```

In the above example:
- The `twitter_user_data` structure has several members. When registering the structure, you can define how these members are represented in the JSON output.
- For instance, if you want to change how `protectedVal` appears in the JSON (for example, renaming it to `protected`, as protected is not available for a variable name in C++), you would specify this in the `createValue` function.

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
    static constexpr auto parseValue = createValue<&value_type::name, &value_type::moderated, &value_type::id>();
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
