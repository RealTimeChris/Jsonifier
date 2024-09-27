#include <unordered_map>
#include <vector>
#include <string>

namespace round_trip_tests {

	struct Obj2 {
		std::string foo;
	};

	struct Obj3 {
		std::unique_ptr<int32_t> a;
		std::string* foo;
	};
}

template<> struct jsonifier::core<round_trip_tests::Obj2> {
	using value_type = round_trip_tests::Obj2;
	static constexpr auto parseValue = createValue<&value_type::foo>();
};

template<> struct jsonifier::core<round_trip_tests::Obj3> {
	using value_type				 = round_trip_tests::Obj3;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::foo>();
};

namespace round_trip_tests {

	class round_trip_test {
	  public:
		round_trip_test() noexcept = default;

		round_trip_test(const std::string& stringNew, const std::string& fileContentsNew, bool areWeAFailingTestNew)
			: fileContents{ fileContentsNew }, areWeAFailingTest{ areWeAFailingTestNew }, testName{ stringNew } {};
		std::string fileContents{};
		bool areWeAFailingTest{};
		std::string testName{};
	};

	bool processFilesInFolder(std::unordered_map<std::string, round_trip_test>& resultFileContents) noexcept {
		try {
			for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH + std::string{ "RoundTrip" })) {
				if (entry.is_regular_file()) {
					const std::string fileName = entry.path().filename().string();

					if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
						std::ifstream file(entry.path());
						if (file.is_open()) {
							std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
							bool returnValue					= (fileName.find("roundtrip") != std::string::npos);
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
		std::string newString{};
		if (parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(valueNew, dataToParse)) {
			auto result = parser.serializeJson(valueNew, newString);
			if (parser.getErrors().size() == 0) {
				std::cout << "Test: " << testName << " = Succeeded - Output: " << newString << std::endl;
			} else {
				std::cout << "Test: " << testName << " = Failed - Output: " << newString << std::endl;
				for (auto& value: parser.getErrors()) {
					std::cout << "Jsonifier Error: " << value << std::endl;
				}
			}
		} else {
			std::cout << "Test: " << testName << " = Failed - Output: " << newString << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return valueNew;
	}

	bool roundTripTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, round_trip_test> jsonTests{};
		processFilesInFolder(jsonTests);
		std::cout << "RoundTrip Tests: " << std::endl;
		runTest<std::vector<int32_t*>>("roundtrip01.json", jsonTests["roundtrip01.json"].fileContents, parser);
		runTest<std::vector<bool>>("roundtrip02.json", jsonTests["roundtrip02.json"].fileContents, parser);
		runTest<std::vector<bool>>("roundtrip03.json", jsonTests["roundtrip03.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip04.json", jsonTests["roundtrip04.json"].fileContents, parser);
		runTest<std::vector<std::string>>("roundtrip05.json", jsonTests["roundtrip05.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip06.json", jsonTests["roundtrip06.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("roundtrip07.json", jsonTests["roundtrip07.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip08.json", jsonTests["roundtrip08.json"].fileContents, parser);
		runTest<Obj2>("roundtrip09.json", jsonTests["roundtrip09.json"].fileContents, parser);
		runTest<Obj3>("roundtrip10.json", jsonTests["roundtrip10.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip11.json", jsonTests["roundtrip11.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip12.json", jsonTests["roundtrip12.json"].fileContents, parser);
		runTest<std::vector<long>>("roundtrip13.json", jsonTests["roundtrip13.json"].fileContents, parser);
		runTest<std::vector<long>>("roundtrip14.json", jsonTests["roundtrip14.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip15.json", jsonTests["roundtrip15.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip16.json", jsonTests["roundtrip16.json"].fileContents, parser);
		runTest<std::vector<long>>("roundtrip17.json", jsonTests["roundtrip17.json"].fileContents, parser);
		runTest<std::vector<long>>("roundtrip18.json", jsonTests["roundtrip18.json"].fileContents, parser);
		runTest<std::vector<long>>("roundtrip19.json", jsonTests["roundtrip19.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip20.json", jsonTests["roundtrip20.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip21.json", jsonTests["roundtrip21.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip22.json", jsonTests["roundtrip22.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip23.json", jsonTests["roundtrip23.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip24.json", jsonTests["roundtrip24.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip25.json", jsonTests["roundtrip25.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip26.json", jsonTests["roundtrip26.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip27.json", jsonTests["roundtrip27.json"].fileContents, parser);

		return true;
	}
}