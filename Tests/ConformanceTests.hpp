#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

struct failTest01 {
	std::string failTestString{};
};

template<> struct jsonifier::core<failTest01> {
	using value_type				 = failTest01;
	static constexpr auto parseValue = createValue(&value_type::failTestString);
};

struct failTest02 {
	std::vector<std::string> testVal{};
};

template<> struct jsonifier::core<failTest02> {
	using value_type				 = failTest02;
	static constexpr auto parseValue = createValue(&value_type::testVal);
};

struct failTest03 {
	std::string failTestVal{};
};

template<> struct jsonifier::core<failTest03> {
	using value_type				 = failTest03;
	static constexpr auto parseValue = createValue("unquoted_key", &value_type::failTestVal);
};

struct failTest04 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest04> {
	using value_type				 = failTest04;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest05 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest05> {
	using value_type				 = failTest05;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest06 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest06> {
	using value_type				 = failTest06;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest07 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest07> {
	using value_type				 = failTest07;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest08 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest08> {
	using value_type				 = failTest08;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest09 {
	bool failTestVal{};
};

template<> struct jsonifier::core<failTest09> {
	using value_type				 = failTest09;
	static constexpr auto parseValue = createValue("extra comma", &value_type::failTestVal);
};

struct failTest10 {
	bool failTestVal{};
};

template<> struct jsonifier::core<failTest10> {
	using value_type				 = failTest10;
	static constexpr auto parseValue = createValue("Extra value after close", &value_type::failTestVal);
};

struct failTest11 {
	uint64_t failTestVal{};
};

template<> struct jsonifier::core<failTest11> {
	using value_type				 = failTest11;
	static constexpr auto parseValue = createValue("Illegal expression", &value_type::failTestVal);
};

struct failTest12 {
	uint64_t failTestVal{};
};

template<> struct jsonifier::core<failTest12> {
	using value_type				 = failTest12;
	static constexpr auto parseValue = createValue("Illegal invocation", &value_type::failTestVal);
};

struct failTest13 {
	uint64_t failTestVal{};
};

template<> struct jsonifier::core<failTest13> {
	using value_type				 = failTest13;
	static constexpr auto parseValue = createValue("Numbers cannot have leading zeroes", &value_type::failTestVal);
};

struct failTest14 {
	uint64_t failTestVal{};
};

template<> struct jsonifier::core<failTest14> {
	using value_type				 = failTest14;
	static constexpr auto parseValue = createValue("Numbers cannot be hex", &value_type::failTestVal);
};

struct failTest15 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest15> {
	using value_type				 = failTest15;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest16 {
	std::vector<uint64_t> failTestVal{};
};

template<> struct jsonifier::core<failTest16> {
	using value_type				 = failTest16;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest17 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest17> {
	using value_type				 = failTest17;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest18 {
	std::string failTestVal{};
};

template<> struct jsonifier::core<failTest18> {
	using value_type				 = failTest18;
	static constexpr auto parseValue = createValue("Missing colon", &value_type::failTestVal);
};

struct failTest19 {
	std::string failTestVal{};
};

template<> struct jsonifier::core<failTest19> {
	using value_type				 = failTest19;
	static constexpr auto parseValue = createValue("Double colon", &value_type::failTestVal);
};

struct failTest20 {
	std::string failTestVal{};
};

template<> struct jsonifier::core<failTest20> {
	using value_type				 = failTest20;
	static constexpr auto parseValue = createValue("Comma instead of colon", &value_type::failTestVal);
};

struct failTest21 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest21> {
	using value_type				 = failTest21;
	static constexpr auto parseValue = createValue("Colon instead of comma", &value_type::failTestVal);
};

struct failTest22 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest22> {
	using value_type				 = failTest22;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest23 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest23> {
	using value_type				 = failTest23;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest24 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest24> {
	using value_type				 = failTest24;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest25 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest25> {
	using value_type				 = failTest25;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest26 {
	std::vector<double> failTestVal{};
};

template<> struct jsonifier::core<failTest26> {
	using value_type				 = failTest26;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest27 {
	std::vector<double> failTestVal{};
};

template<> struct jsonifier::core<failTest27> {
	using value_type				 = failTest27;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest28 {
	std::vector<double> failTestVal{};
};

template<> struct jsonifier::core<failTest28> {
	using value_type				 = failTest28;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct failTest29 {
	bool failTestVal{};
};

template<> struct jsonifier::core<failTest29> {
	using value_type				 = failTest29;
	static constexpr auto parseValue = createValue("Comma instead if closing brace", &value_type::failTestVal);
};

struct failTest30 {
	std::vector<std::string> failTestVal{};
};

template<> struct jsonifier::core<failTest30> {
	using value_type				 = failTest30;
	static constexpr auto parseValue = createValue(&value_type::failTestVal);
};

struct object {};

using array = std::vector<std::string>;

struct ObjectWith1Member {
	std::vector<std::string> value{};
};

template<> struct jsonifier::core<ObjectWith1Member> {
	using value_type				 = ObjectWith1Member;
	static constexpr auto parseValue = createValue("object with 1 member", &value_type::value);
};

struct message_class {
	message_class() noexcept = default;
	std::string uCafEuBabEuAb98UFcdEubcdauef4Abfnrt1_{};
	std::vector<std::string> array{};
	std::vector<int64_t> compact{};
	std::vector<int64_t> sPACED{};
	std::string the0123456789{};
	std::string backslash{};
	std::string controls{};
	std::string jsontext{};
	std::nullptr_t null{};
	std::string special{};
	std::string address{};
	std::string comment{};
	std::string message{};
	std::string quotes{};
	std::string space{};
	std::string quote{};
	std::string slash{};
	std::string alpha{};
	std::string ALPHA{};
	std::string digit{};
	bool messageFalse{};
	bool messageTrue{};
	object objectVal{};
	std::string hex{};
	std::string url{};
	int64_t integer{};
	double empty{};
	int64_t zero{};
	double real{};
	int64_t one{};
	double E{};
	double e{};
};

using message_element = std::variant<std::string, ObjectWith1Member, object, array, int64_t, bool, std::nullptr_t, message_class, double>;

template<> struct jsonifier::core<object> {
	using value_type				 = object;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<message_class> {
	using value_type				 = message_class;
	static constexpr auto parseValue = createValue("integer", &value_type::integer, "real", &value_type::real, "E", &value_type::E, "e", &value_type::e, "", &value_type::empty,
		"zero", &value_type::zero, "one", &value_type::one, "space", &value_type::space, "quote", &value_type::quote, "backslash", &value_type::backslash, "controls",
		&value_type::controls, "slash", &value_type::slash, "alpha", &value_type::alpha, "ALPHA", &value_type::alpha, "digit", &value_type::digit, "0123456789",
		&value_type::the0123456789, "special", &value_type::special, "hex", &value_type::hex, "true", &value_type::messageTrue, "false", &value_type::messageFalse, "null",
		&value_type::null, "array", &value_type::array, "object", &value_type::objectVal, "address", &value_type::address, "url", &value_type::url, "comment", &value_type::comment,
		"# -- --> */", &value_type::message, " s p a c e d ", &value_type::sPACED, "compact", &value_type::compact, "jsontext", &value_type::jsontext, "quotes",
		&value_type::quotes, "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?", &value_type::uCafEuBabEuAb98UFcdEubcdauef4Abfnrt1_);
};

struct passTest01 {
	passTest01() noexcept = default;
	std::vector<message_element> valueNew{ "", ObjectWith1Member{}, object{}, array{}, int64_t{}, bool{}, bool{}, std::nullptr_t{}, message_class{}, double{}, double{}, double{},
		int64_t{}, double{}, double{}, double{}, double{}, double{}, double{} };
};

template<> struct jsonifier::core<passTest01> {
	using value_type				 = passTest01;
	static constexpr auto parseValue = createValue(&value_type::valueNew);
};

struct json_test_pattern_pass2 {
	std::string theOutermostValue;
	std::string inThisTest;
};

template<> struct jsonifier::core<json_test_pattern_pass2> {
	using value_type				 = json_test_pattern_pass2;
	static constexpr auto parseValue = createValue("In this test", &value_type::inThisTest, "The outermost value", &value_type::theOutermostValue);
};

struct passTest02 {
	json_test_pattern_pass2 jsonTestPatternPass3;
};

template<> struct jsonifier::core<passTest02> {
	using value_type				 = passTest02;
	static constexpr auto parseValue = createValue("JSON Test Pattern pass2", &value_type::jsonTestPatternPass3);
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

bool processFilesInFolder(std::unordered_map<std::string, conformance_test>& resultFileContents) {
	try {
		for (const auto& entry: std::filesystem::directory_iterator(JSON_TEST_PATH)) {
			if (entry.is_regular_file()) {
				const std::string fileName = entry.path().filename().string();

				if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == ".json") {
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

template<typename test_type> void runTest(const std::string& testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser, bool doWeFail = true) {
	std::cout << "Running Test: " << testName << std::endl;
	auto result = parser.parseJson<jsonifier::parse_options{ .validateJson = true, .minified = false }>(test_type{}, parser.minifyJson(dataToParse));
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
}

bool conformanceTests() {
	jsonifier::jsonifier_core parser{};
	std::unordered_map<std::string, conformance_test> jsonTests{};
	processFilesInFolder(jsonTests);
	std::cout << "Conformance Tests: " << std::endl;
	runTest<failTest01>("fail1.json", jsonTests["fail1.json"].fileContents, parser);
	runTest<failTest02>("fail2.json", jsonTests["fail2.json"].fileContents, parser);
	runTest<failTest03>("fail3.json", jsonTests["fail3.json"].fileContents, parser);
	runTest<failTest04>("fail4.json", jsonTests["fail4.json"].fileContents, parser);
	runTest<failTest05>("fail5.json", jsonTests["fail5.json"].fileContents, parser);
	runTest<failTest06>("fail6.json", jsonTests["fail6.json"].fileContents, parser);
	runTest<failTest07>("fail7.json", jsonTests["fail7.json"].fileContents, parser);
	runTest<failTest08>("fail8.json", jsonTests["fail8.json"].fileContents, parser);
	runTest<failTest09>("fail9.json", jsonTests["fail9.json"].fileContents, parser);
	runTest<failTest10>("fail10.json", jsonTests["fail10.json"].fileContents, parser);
	runTest<failTest11>("fail11.json", jsonTests["fail11.json"].fileContents, parser);
	runTest<failTest12>("fail12.json", jsonTests["fail12.json"].fileContents, parser);
	runTest<failTest13>("fail13.json", jsonTests["fail13.json"].fileContents, parser);
	runTest<failTest14>("fail14.json", jsonTests["fail14.json"].fileContents, parser);
	runTest<failTest15>("fail15.json", jsonTests["fail15.json"].fileContents, parser);
	runTest<failTest16>("fail16.json", jsonTests["fail16.json"].fileContents, parser);
	runTest<failTest17>("fail17.json", jsonTests["fail17.json"].fileContents, parser);
	runTest<failTest18>("fail18.json", jsonTests["fail18.json"].fileContents, parser);
	runTest<failTest19>("fail19.json", jsonTests["fail19.json"].fileContents, parser);
	runTest<failTest20>("fail20.json", jsonTests["fail20.json"].fileContents, parser);
	runTest<failTest21>("fail21.json", jsonTests["fail21.json"].fileContents, parser);
	runTest<failTest22>("fail22.json", jsonTests["fail22.json"].fileContents, parser);
	runTest<failTest23>("fail23.json", jsonTests["fail23.json"].fileContents, parser);
	runTest<failTest24>("fail24.json", jsonTests["fail24.json"].fileContents, parser);
	runTest<failTest25>("fail25.json", jsonTests["fail25.json"].fileContents, parser);
	runTest<failTest26>("fail26.json", jsonTests["fail26.json"].fileContents, parser);
	runTest<failTest27>("fail27.json", jsonTests["fail27.json"].fileContents, parser);
	runTest<failTest28>("fail28.json", jsonTests["fail28.json"].fileContents, parser);
	runTest<failTest29>("fail29.json", jsonTests["fail29.json"].fileContents, parser);
	runTest<failTest30>("fail30.json", jsonTests["fail30.json"].fileContents, parser);
	runTest<passTest01>("pass1.json", jsonTests["pass1.json"].fileContents, parser, false);
	runTest<passTest02>("pass2.json", jsonTests["pass2.json"].fileContents, parser, false);
	return true;
}