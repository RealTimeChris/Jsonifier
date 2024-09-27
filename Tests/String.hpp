#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace string_validation_tests {

	std::unordered_map<std::string_view, std::string> testValues = { { "test1.json", "" }, { "test2.json", "Hello" }, { "test3.json", "Hello\nWorld" },
		{ "test5.json", "Hello\u0000World" }, { "test6.json", "\"\\/\b\f\n\r\t" }, { "test8.json", "\u0024" }, { "test9.json", "\u00A2" }, { "test10.json", "\u20AC" },
		{ "test11.json", "\U0001D11E" } };

	class string_validation_test {
	  public:
		string_validation_test() noexcept = default;

		string_validation_test(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
			: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
		std::string fileContents{};
		bool areWeAFailingTest{};
		std::string testName{};
	};

	bool processFilesInFolder(std::unordered_map<std::string, string_validation_test>& resultFileContents) noexcept {
		try {
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "StringValidation" })) {
				if (entry.is_regular_file()) {
					const std::string fileName = entry.path().filename().string();

					if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
						std::ifstream file(entry.path());
						if (file.is_open()) {
							std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
							bool returnValue					= (fileName.find("test") != std::string::npos);
							resultFileContents[fileName.data()] = { fileName, fileContents, returnValue };
							file.close();
						} else {
							std::cerr << "Error opening file: " << fileName << std::endl;
							return false;
						}
					}
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Error while processing files: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	auto runTest(const std::string_view& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << testName << " Input: " << dataToParse << std::endl;
		std::vector<std::string> data;
		if (parser.parseJson(data, dataToParse.data()) && parser.getErrors().size() == 0) {
			if (data.size() == 1) {
				std::cout << testName << " Succeeded - Output: " << data[0] << std::endl;
				std::cout << testName << " Succeeded - Expected Output: " << testValues[testName] << std::endl;
			} else {
				std::cout << testName << " Failed." << std::endl;
			}
		} else {
			std::cout << testName << " Failed." << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return true;
	}

	bool stringTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, string_validation_test> jsonTests{};
		processFilesInFolder(jsonTests);
		std::cout << "String Tests: " << std::endl;
		runTest("test1.json", jsonTests["test1.json"].fileContents, parser);
		runTest("test2.json", jsonTests["test2.json"].fileContents, parser);
		runTest("test3.json", jsonTests["test3.json"].fileContents, parser);
		runTest("test5.json", jsonTests["test5.json"].fileContents, parser);
		runTest("test6.json", jsonTests["test6.json"].fileContents, parser);
		runTest("test8.json", jsonTests["test8.json"].fileContents, parser);
		runTest("test9.json", jsonTests["test9.json"].fileContents, parser);
		runTest("test10.json", jsonTests["test10.json"].fileContents, parser);
		runTest("test11.json", jsonTests["test11.json"].fileContents, parser);
		return true;
	}

}
