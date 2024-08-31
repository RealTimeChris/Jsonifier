#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace fail_tests {

	enum class parse_errors {
		Success						= 0,
		Missing_Object_Start		= 1,
		Imbalanced_Object_Braces	= 2,
		Missing_Array_Start			= 3,
		Imbalanced_Array_Brackets	= 4,
		Missing_String_Start		= 5,
		Missing_Colon				= 6,
		Missing_Comma_Or_Object_End = 7,
		Missing_Comma_Or_Array_End	= 8,
		Missing_Comma				= 9,
		Invalid_Number_Value		= 10,
		Invalid_Null_Value			= 11,
		Invalid_Bool_Value			= 12,
		Invalid_String_Characters	= 13,
		No_Input					= 14,
		Unfinished_Input			= 15,
		Unexpected_String_End		= 16,
	};

	using enum jsonifier_internal::parse_errors;

	struct failTest01 {
		static constexpr auto failCode = Unexpected_String_End;
		std::string value{};
	};

	struct failTest02 {
		static constexpr auto failCode = Invalid_Bool_Value;
		bool value{};
	};

	struct failTest03 {
		static constexpr auto failCode = Imbalanced_Array_Brackets;
		std::vector<std::string> value{};
	};

	struct test_object {};

	struct failTest04 {
		static constexpr auto failCode = static_cast<uint64_t>(Imbalanced_Object_Braces) | static_cast<uint64_t>(Missing_Comma_Or_Object_End);
		test_object value{};
	};

	struct failTest05 {
		static constexpr auto failCode = Unexpected_String_End;
		std::vector<std::string> value{};
	};

	struct failTest06 {
		static constexpr auto failCode = Invalid_Bool_Value;
		std::vector<bool> value{};
	};

	struct failTest07 {
		static constexpr auto failCode = static_cast<uint64_t>(Imbalanced_Array_Brackets) | static_cast<uint64_t>(Missing_Comma_Or_Array_End);
		std::vector<std::vector<bool>> value{};
	};

	struct failTest08 {
		static constexpr auto failCode = static_cast<uint64_t>(Imbalanced_Object_Braces) | static_cast<uint64_t>(Missing_Comma_Or_Object_End);
		std::vector<test_object> value{};
	};

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
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "/FailTests" })) {
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

	template<typename test_type> void runTest(const std::string& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		auto result = parser.parseJson<jsonifier::parse_options{ .minified = false }>(test_type{}, dataToParse);
		if ((parser.getErrors().size() == 0)) {
			std::cout << "Test: " << testName << " = Failed 01" << std::endl;
		} else {
			if (static_cast<uint64_t>(test_type::failCode) & static_cast<uint64_t>(parser.getErrors()[0])) {
				std::cout << "Test: " << testName << " = Succeeded 02" << std::endl;
			} else {
				std::cout << "Test: " << testName << " = Failed, Expected Type: " << static_cast<uint64_t>(test_type::failCode)
						  << ", instead got: " << static_cast<uint64_t>(parser.getErrors()[0]) << std::endl;
			}
		}
	}
}

template<> struct jsonifier::core<fail_tests::failTest01> {
	using value_type				 = fail_tests::failTest01;
	static constexpr auto parseValue = createValue("Testing_when_a_string_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::test_object> {
	using value_type				 = fail_tests::test_object;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<fail_tests::failTest02> {
	using value_type				 = fail_tests::failTest02;
	static constexpr auto parseValue = createValue("Testing_when_a_bool_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest03> {
	using value_type				 = fail_tests::failTest03;
	static constexpr auto parseValue = createValue("Testing_when_an_array_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest04> {
	using value_type				 = fail_tests::failTest04;
	static constexpr auto parseValue = createValue("Testing_when_an_object_ends_abruptly_in_an_object", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest05> {
	using value_type				 = fail_tests::failTest05;
	static constexpr auto parseValue = createValue("Testing_when_a_string_ends_abruptly_in_an_array", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest06> {
	using value_type				 = fail_tests::failTest06;
	static constexpr auto parseValue = createValue("Testing_when_a_bool_ends_abruptly_in_an_array", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest07> {
	using value_type				 = fail_tests::failTest07;
	static constexpr auto parseValue = createValue("Testing_when_an_array_ends_abruptly_in_an_array", &value_type::value);
};

template<> struct jsonifier::core<fail_tests::failTest08> {
	using value_type				 = fail_tests::failTest08;
	static constexpr auto parseValue = createValue("Testing_when_an_object_ends_abruptly_in_an_array", &value_type::value);
};

bool failTests() noexcept {
	jsonifier::jsonifier_core parser{};
	std::unordered_map<std::string, fail_tests::conformance_test> jsonTests{};
	fail_tests::processFilesInFolder(jsonTests);
	std::cout << "Fail Tests: " << std::endl;
	fail_tests::runTest<fail_tests::failTest01>("fail1.json", jsonTests["fail1.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest02>("fail2.json", jsonTests["fail2.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest03>("fail3.json", jsonTests["fail3.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest04>("fail4.json", jsonTests["fail4.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest05>("fail5.json", jsonTests["fail5.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest06>("fail6.json", jsonTests["fail6.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest07>("fail7.json", jsonTests["fail7.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest08>("fail8.json", jsonTests["fail8.json"].fileContents, parser); /*
	fail_tests::runTest<fail_tests::failTest04>("fail9.json", jsonTests["fail9.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest04>("fail10.json", jsonTests["fail10.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest04>("fail11.json", jsonTests["fail11.json"].fileContents, parser);
	fail_tests::runTest<fail_tests::failTest04>("fail12.json", jsonTests["fail12.json"].fileContents, parser);*/
	return true;
}