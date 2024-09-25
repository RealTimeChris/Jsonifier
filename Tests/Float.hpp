#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace float_validation_tests {

	struct test_action {
		using value_type = std::vector<std::string>;
		std::vector<std::string> data{};
	};

	class float_validation_test {
	  public:
		float_validation_test() noexcept = default;

		float_validation_test(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
			: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
		std::string fileContents{};
		bool areWeAFailingTest{};
		std::string testName{};
	};

	bool processFilesInFolder(std::unordered_map<std::string, float_validation_test>& resultFileContents) noexcept {
		try {
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "FloatValidation" })) {
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
		std::vector<double> data;
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

	bool floatTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, float_validation_test> jsonTests{};
		processFilesInFolder(jsonTests);
		std::cout << "Float Tests: " << std::endl;
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
		runTest<test_action>("test12.json", jsonTests["test12.json"].fileContents, parser);
		runTest<test_action>("test13.json", jsonTests["test13.json"].fileContents, parser);
		runTest<test_action>("test14.json", jsonTests["test14.json"].fileContents, parser);
		runTest<test_action>("test15.json", jsonTests["test15.json"].fileContents, parser);
		runTest<test_action>("test16.json", jsonTests["test16.json"].fileContents, parser);
		runTest<test_action>("test17.json", jsonTests["test17.json"].fileContents, parser);
		runTest<test_action>("test18.json", jsonTests["test18.json"].fileContents, parser);
		runTest<test_action>("test19.json", jsonTests["test19.json"].fileContents, parser);
		runTest<test_action>("test20.json", jsonTests["test20.json"].fileContents, parser);
		runTest<test_action>("test21.json", jsonTests["test21.json"].fileContents, parser);
		runTest<test_action>("test22.json", jsonTests["test22.json"].fileContents, parser);
		runTest<test_action>("test23.json", jsonTests["test23.json"].fileContents, parser);
		runTest<test_action>("test24.json", jsonTests["test24.json"].fileContents, parser);
		runTest<test_action>("test25.json", jsonTests["test25.json"].fileContents, parser);
		runTest<test_action>("test26.json", jsonTests["test26.json"].fileContents, parser);
		runTest<test_action>("test27.json", jsonTests["test27.json"].fileContents, parser);
		runTest<test_action>("test28.json", jsonTests["test28.json"].fileContents, parser);
		runTest<test_action>("test29.json", jsonTests["test29.json"].fileContents, parser);
		runTest<test_action>("test30.json", jsonTests["test30.json"].fileContents, parser);
		runTest<test_action>("test31.json", jsonTests["test31.json"].fileContents, parser);
		runTest<test_action>("test32.json", jsonTests["test32.json"].fileContents, parser);
		runTest<test_action>("test33.json", jsonTests["test33.json"].fileContents, parser);
		runTest<test_action>("test34.json", jsonTests["test34.json"].fileContents, parser);
		runTest<test_action>("test35.json", jsonTests["test35.json"].fileContents, parser);
		runTest<test_action>("test36.json", jsonTests["test36.json"].fileContents, parser);
		runTest<test_action>("test37.json", jsonTests["test37.json"].fileContents, parser);
		runTest<test_action>("test38.json", jsonTests["test38.json"].fileContents, parser);
		runTest<test_action>("test39.json", jsonTests["test39.json"].fileContents, parser);
		runTest<test_action>("test40.json", jsonTests["test40.json"].fileContents, parser);
		runTest<test_action>("test41.json", jsonTests["test41.json"].fileContents, parser);
		runTest<test_action>("test42.json", jsonTests["test42.json"].fileContents, parser);
		runTest<test_action>("test43.json", jsonTests["test43.json"].fileContents, parser);
		runTest<test_action>("test44.json", jsonTests["test44.json"].fileContents, parser);
		runTest<test_action>("test45.json", jsonTests["test45.json"].fileContents, parser);
		runTest<test_action>("test46.json", jsonTests["test46.json"].fileContents, parser);
		runTest<test_action>("test47.json", jsonTests["test47.json"].fileContents, parser);
		runTest<test_action>("test48.json", jsonTests["test48.json"].fileContents, parser);
		runTest<test_action>("test49.json", jsonTests["test49.json"].fileContents, parser);
		runTest<test_action>("test50.json", jsonTests["test50.json"].fileContents, parser);
		runTest<test_action>("test51.json", jsonTests["test51.json"].fileContents, parser);
		runTest<test_action>("test52.json", jsonTests["test52.json"].fileContents, parser);
		runTest<test_action>("test53.json", jsonTests["test53.json"].fileContents, parser);
		runTest<test_action>("test54.json", jsonTests["test54.json"].fileContents, parser);
		runTest<test_action>("test55.json", jsonTests["test55.json"].fileContents, parser);
		runTest<test_action>("test56.json", jsonTests["test56.json"].fileContents, parser);
		runTest<test_action>("test57.json", jsonTests["test57.json"].fileContents, parser);
		runTest<test_action>("test58.json", jsonTests["test58.json"].fileContents, parser);
		runTest<test_action>("test59.json", jsonTests["test59.json"].fileContents, parser);
		runTest<test_action>("test60.json", jsonTests["test60.json"].fileContents, parser);
		runTest<test_action>("test61.json", jsonTests["test61.json"].fileContents, parser);
		runTest<test_action>("test62.json", jsonTests["test62.json"].fileContents, parser);
		runTest<test_action>("test63.json", jsonTests["test63.json"].fileContents, parser);
		runTest<test_action>("test64.json", jsonTests["test64.json"].fileContents, parser);
		runTest<test_action>("test65.json", jsonTests["test65.json"].fileContents, parser);
		runTest<test_action>("test66.json", jsonTests["test66.json"].fileContents, parser);
		runTest<test_action>("test67.json", jsonTests["test67.json"].fileContents, parser);
		runTest<test_action>("test68.json", jsonTests["test68.json"].fileContents, parser);
		runTest<test_action>("test69.json", jsonTests["test69.json"].fileContents, parser);
		return true;
	}

}