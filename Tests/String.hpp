#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace string_validation_tests {

	struct test_action {
		using value_type = std::vector<std::string> ;
		std::vector<std::string> data{};
	};

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

	template<typename test_type> auto runTest(const std::string_view& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		std::vector<std::string> data;
		if (parser.parseJson(data, dataToParse.data()) && parser.getErrors().size() == 0) {
			if (data.size() == 1) {
				std::cout << "Test: " << testName << " = Succeeded - Output: " << data[0] << std::endl;
			} else {
				std::cout << "Test: " << testName << " = Failed." << std::endl;
			}
		} else {
			std::cout << "Test: " << testName << " = Failed." << std::endl;
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
		runTest<test_action>("test1.json", jsonTests["test1.json"].fileContents, parser);
		runTest<test_action>("test2.json", jsonTests["test2.json"].fileContents, parser);
		runTest<test_action>("test3.json", jsonTests["test3.json"].fileContents, parser);
		runTest<test_action>("test4.json", jsonTests["test4.json"].fileContents, parser);
		runTest<test_action>("test5.json", jsonTests["test5.json"].fileContents, parser);
		runTest<test_action>("test6.json", jsonTests["test6.json"].fileContents, parser);
		runTest<test_action>("test7.json", jsonTests["test7.json"].fileContents, parser);
		runTest<test_action>("test8.json", jsonTests["test8.json"].fileContents, parser);
		runTest<test_action>("test9.json", jsonTests["test9.json"].fileContents, parser);
		runTest<test_action>("test10.json", jsonTests["test10.json"].fileContents, parser);
		runTest<test_action>("test11.json", jsonTests["test11.json"].fileContents, parser);
		return true;
	}

}