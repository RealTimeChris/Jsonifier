#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace conformance_tests {

	class conformance_test {
	  public:
		conformance_test() noexcept = default;

		conformance_test(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
			: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
		std::string fileContents{};
		bool areWeAFailingTest{};
		std::string testName{};
	};

	bool processFilesInFolder(std::unordered_map<std::string, conformance_test>& resultFileContents) noexcept {
		try {
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "/ConformanceTests" })) {
				if (entry.is_regular_file()) {
					const std::string fileName = entry.path().filename().string();

					if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
						std::ifstream file(entry.path());
						if (file.is_open()) {
							std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
							bool returnValue					= (fileName.find("fail") != std::string::npos);
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

	template<typename test_type>
	test_type runTest(const std::string& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser, bool doWeFail = true) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		test_type valueNew{};
		auto result = parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(valueNew, dataToParse);
		if ((parser.getErrors().size() == 0 && result) && !doWeFail) {
			std::cout << "Test: " << testName << " = Succeeded 01" << std::endl;
		} else if ((parser.getErrors().size() != 0 || !result) && doWeFail) {
			std::cout << "Test: " << testName << " = Succeeded 02" << std::endl;
		} else {
			std::cout << "Test: " << testName << " = Failed" << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return valueNew;
	}

	bool conformanceTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, conformance_test> jsonTests{};
		processFilesInFolder(jsonTests);
		std::cout << "Conformance Tests: " << std::endl;
		runTest<std::unordered_map<std::string, std::string>>("fail02.json", jsonTests["fail02.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("fail03.json", jsonTests["fail03.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail04.json", jsonTests["fail04.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail05.json", jsonTests["fail05.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail06.json", jsonTests["fail06.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail07.json", jsonTests["fail07.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail08.json", jsonTests["fail08.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail09.json", jsonTests["fail09.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail10.json", jsonTests["fail10.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail11.json", jsonTests["fail11.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("fail12.json", jsonTests["fail12.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail13.json", jsonTests["fail13.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail14.json", jsonTests["fail14.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail15.json", jsonTests["fail15.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail16.json", jsonTests["fail16.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail17.json", jsonTests["fail17.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail19.json", jsonTests["fail19.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail20.json", jsonTests["fail20.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail21.json", jsonTests["fail21.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail22.json", jsonTests["fail22.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail23.json", jsonTests["fail23.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail24.json", jsonTests["fail24.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail25.json", jsonTests["fail25.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail26.json", jsonTests["fail26.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail27.json", jsonTests["fail27.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail28.json", jsonTests["fail28.json"].fileContents, parser);
		runTest<std::vector<double>>("fail29.json", jsonTests["fail29.json"].fileContents, parser);
		runTest<std::vector<double>>("fail30.json", jsonTests["fail30.json"].fileContents, parser);
		runTest<std::vector<double>>("fail31.json", jsonTests["fail31.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail32.json", jsonTests["fail32.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail33.json", jsonTests["fail33.json"].fileContents, parser);
		return true;
	}

}