## Minifying JSON Data with Jsonifier

### Example

Consider the following example code snippet:

```cpp
json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };

FileLoader fileLoader{ "../../../JsonData.json" };
fileLoader.saveFile(parser.minify(jsonData.theData));
```

In this example:

- JSON data is generated using `TestGenerator<test_struct>::generateJsonData()` and stored in an object of type `json_data` named `jsonData`.
- An instance of `FileLoader` is created, and the minified JSON data is saved to a file using `parser.minify(jsonData.theData)`.

### Usage

#### 1. Generate JSON Data
----
Generate JSON data using your preferred method and store it in an object. In the example, it uses a test data generator:

```cpp
json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
```

#### 2. Minify JSON Data
----
Use the `minify` method of the `jsonifier_core` instance to minify the JSON data:

```cpp
std::string minifiedJson = parser.minify(jsonData.theData);
```

#### 3. Save Minified JSON Data
----
Save the minified JSON data to a file or use it as needed in your application:

```cpp
FileLoader fileLoader{ "../../JsonData.json" };
fileLoader.saveFile(minifiedJsonJson);
```