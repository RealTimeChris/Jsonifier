#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace success_tests {

	struct successTest01 {
		std::string value{};
	};

	struct successTest02 {
		double value{};
	};

	struct successTest03 {
		uint64_t value{};
	};

	struct successTest04 {
		std::vector<std::string> value{};
	};

	struct successTest05 {
		std::vector<double> value{};
	};

	struct successTest06 {
		std::vector<uint64_t> value{};
	};

	class conformance_test {
	  public:
		conformance_test() noexcept = default;

		conformance_test(const std::string& stringNew, const std::string& fileContentsNew, bool areWeASuccessingTestNew)
			: fileContents{ fileContentsNew }, areWeASuccessingTest{ areWeASuccessingTestNew }, testName{ stringNew } {};
		std::string fileContents{};
		bool areWeASuccessingTest{};
		std::string testName{};
	};

	bool processFilesInFolder(std::unordered_map<std::string, conformance_test>& resultFileContents) noexcept {
		try {
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "/SuccessTests" })) {
				if (entry.is_regular_file()) {
					const std::string fileName = entry.path().filename().string();

					if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
						std::ifstream file(entry.path());
						if (file.is_open()) {
							std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
							bool returnValue					= (fileName.find("success") != std::string::npos);
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

	template<typename test_type> void runTest(const std::string& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser, bool doWeSuccess = true) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		auto result = parser.parseJson<jsonifier::parse_options{ .minified = false }>(test_type{}, dataToParse);
		if ((parser.getErrors().size() == 0 && result) && doWeSuccess) {
			std::cout << "Test: " << testName << " = Succeeded" << std::endl;
		} else {
			std::cout << "Test: " << testName << " = Failed" << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
	}
}

template<> struct jsonifier::core<success_tests::successTest01> {
	using value_type				 = success_tests::successTest01;
	static constexpr auto parseValue = createValue("Testing_when_a_string_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<success_tests::successTest02> {
	using value_type				 = success_tests::successTest02;
	static constexpr auto parseValue = createValue("Testing_when_a_double_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<success_tests::successTest03> {
	using value_type				 = success_tests::successTest03;
	static constexpr auto parseValue = createValue("Testing_when_an_integer_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<success_tests::successTest04> {
	using value_type				 = success_tests::successTest04;
	static constexpr auto parseValue = createValue("Testing_when_a_string_ends_abruptly_in_an_array", &value_type::value);
};

template<> struct jsonifier::core<success_tests::successTest05> {
	using value_type				 = success_tests::successTest05;
	static constexpr auto parseValue = createValue("Testing_when_a_double_ends_abruptly_in_an_array", &value_type::value);
};

template<> struct jsonifier::core<success_tests::successTest06> {
	using value_type				 = success_tests::successTest06;
	static constexpr auto parseValue = createValue("Testing_when_an_integer_ends_abruptly_in_an_array", &value_type::value);
};

bool successTests() noexcept {
	jsonifier::jsonifier_core parser{};
	std::unordered_map<std::string, success_tests::conformance_test> jsonTests{};
	success_tests::processFilesInFolder(jsonTests);
	std::cout << "Success Tests: " << std::endl;
	success_tests::runTest<success_tests::successTest01>("success1.json", jsonTests["success1.json"].fileContents, parser);
	success_tests::runTest<success_tests::successTest02>("success2.json", jsonTests["success2.json"].fileContents, parser);
	success_tests::runTest<success_tests::successTest03>("success3.json", jsonTests["success3.json"].fileContents, parser);
	success_tests::runTest<success_tests::successTest04>("success4.json", jsonTests["success4.json"].fileContents, parser);
	success_tests::runTest<success_tests::successTest04>("success5.json", jsonTests["success5.json"].fileContents, parser);
	success_tests::runTest<success_tests::successTest04>("success6.json", jsonTests["success6.json"].fileContents, parser);
	return true;
}