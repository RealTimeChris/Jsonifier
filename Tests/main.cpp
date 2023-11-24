#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//	#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVXF|JSONIFIER_POPCNT|JSONIFIER_BMI|JSONIFIER_BMI2|JSONIFIER_LZCNT)
#endif
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <jsonifier/Index.hpp>
#include <unordered_set>
#include <unordered_map>
#include "fmt/format.h"
#include <iostream>
#include <chrono>


static constexpr jsonifier::string_view discordData{
	"{\"t\":\"READY\",\"d\":{\"_trace\":[\""
	"\"],\"application\":null,"
	"\"auth\":{},\"geo_ordered_rtc_"
	"regions\":[\"newark\",\"us-east\",\"us-central\",\"atlanta\",\"us-south\"],\"guild_join_requests\":[],\"guilds\":[{\"id\":"
	"318872312596267018,\"unavailable\":true},{"
	"\"id\":931640556814237706,\"unavailable\":true},{\"id\":991025447875784714,\"unavailable\":true},{\"id\":995048955215872071,"
	"\"unavailable\":true},{\"id\":"
	"1022405038922006538,\"unavailable\":true},{\"id\":1032783776184533022,\"unavailable\":true},{\"id\":1078501504119476282,\"unavailable\":"
	"true},{\"id\":"
	"1131853763506880522,\"unavailable\":true}],\"presences\":[],\"private_channels\":[],\"relationships\":[],\"resume_gateway_url\":\"wss://"
	"gateway-us-east1-d.discord.gg\",\"session_id\":\"5b405a8282550f72114b460169cd08f6\",\"session_type\":\"normal\",\"shard\":\"01\","
	"\"user\":{\"avatar\":"
	"\"88bd9ce7bf889c0d36fb4afd3725900b\",\"bot\":true,\"discriminator\":\"3055\",\"email\":\"\",\"flags\":0,\"global_name\":\"\",\"id\":"
	"1142733646600614004,\"mfa_"
	"enabled\":false,\"username\":\"MBot-MusicHouse-2\",\"verified\":true},\"user_settings\":{},\"v\":-10},\"op\":0,\"s\":1}"
};

struct Application {
	uint32_t flags;
	uint64_t id;
};

struct Guild {
	uint64_t id;
	bool unavailable;
};

struct auth {
	bool emptyVal{};
};

struct User {
	std::string avatar;
	bool bot;
	std::string discriminator;
	std::string email;
	uint32_t flags;
	std::string global_name;
	uint64_t id;
	bool mfa_enabled;
	std::string username;
	bool verified;
};

struct UserSettings {
	bool emptyVal{};
};

struct ReadyData {
	std::vector<std::string> _trace;
	std::nullptr_t application;
	std::vector<std::string> geo_ordered_rtc_regions;
	std::vector<Guild> guild_join_requests;
	auth authVal{};
	std::vector<Guild> guilds;
	std::vector<std::string> presences;
	std::vector<std::string> private_channels;
	std::vector<std::string> relationships;
	std::string resume_gateway_url;
	User user{};
	std::string session_id;
	std::string session_type;
	std::string shard;
	UserSettings user_settings;
	int v;
};

struct ReadyMessage {
	ReadyData d;
	int op;
	int s;
	std::string t{};
};

template<> struct jsonifier::core<Application> {
	using OTy						 = Application;
	static constexpr auto parseValue = createObject();
};

template<> struct jsonifier::core<Guild> {
	using OTy						 = Guild;
	static constexpr auto parseValue = createObject("id", &OTy::id, "unavailable", &OTy::unavailable);
};

template<> struct jsonifier::core<auth> {
	using OTy						 = auth;
	static constexpr auto parseValue = createObject("emptyVal", &OTy::emptyVal);
};

template<> struct jsonifier::core<UserSettings> {
	using OTy						 = UserSettings;
	static constexpr auto parseValue = createObject("emptyVal", &OTy::emptyVal);
};

template<> struct jsonifier::core<User> {
	using OTy						 = User;
	static constexpr auto parseValue = createObject("avatar", &OTy::avatar, "bot", &OTy::bot, "discriminator", &OTy::discriminator, "email", &OTy::email, "flags", &OTy::flags,
		"global_name", &OTy::global_name, "id", &OTy::id, "mfa_enabled", &OTy::mfa_enabled, "username", &OTy::username, "verified", &OTy::verified);
};

template<> struct jsonifier::core<ReadyData> {
	using OTy						 = ReadyData;
	static constexpr auto parseValue = createObject("_trace", &OTy::_trace, "application", &OTy::application, "geo_ordered_rtc_regions", &OTy::geo_ordered_rtc_regions,
		"guild_join_requests", &OTy::guild_join_requests, "guilds", &OTy::guilds, "presences", &OTy::presences, "private_channels", &OTy::private_channels, "relationships",
		&OTy::relationships, "resume_gateway_url", &OTy::resume_gateway_url, "session_id", &OTy::session_id, "session_type", &OTy::session_type, "shard", &OTy::shard, "user",
		&OTy::user, "user_settings", &OTy::user_settings, "v", &OTy::v, "auth", &OTy::authVal);
};

template<> struct jsonifier::core<ReadyMessage> {
	using OTy						 = ReadyMessage;
	static constexpr auto parseValue = createObject("d", &OTy::d, "op", &OTy::op, "s", &OTy::s, "t", &OTy::t);
};

template<> struct glz::meta<Application> {
	using OTy					= Application;
	static constexpr auto value = object("id", &OTy::id, "flags", &OTy::flags);
};

template<> struct glz::meta<UserSettings> {
	using OTy					= UserSettings;
	static constexpr auto value = object();
};

template<> struct glz::meta<auth> {
	using OTy					= auth;
	static constexpr auto value = object();
};

template<> struct glz::meta<Guild> {
	using OTy					= Guild;
	static constexpr auto value = object("id", &OTy::id, "unavailable", &OTy::unavailable);
};

template<> struct glz::meta<User> {
	using OTy					= User;
	static constexpr auto value = object("avatar", &OTy::avatar, "bot", &OTy::bot, "discriminator", &OTy::discriminator, "email", &OTy::email, "flags", &OTy::flags, "global_name",
		&OTy::global_name, "id", &OTy::id, "mfa_enabled", &OTy::mfa_enabled, "username", &OTy::username, "verified", &OTy::verified);
};

template<> struct glz::meta<ReadyData> {
	using OTy					= ReadyData;
	static constexpr auto value = object("_trace", &OTy::_trace, "application", &OTy::application, "geo_ordered_rtc_regions", &OTy::geo_ordered_rtc_regions, "guild_join_requests",
		&OTy::guild_join_requests, "guilds", &OTy::guilds, "presences", &OTy::presences, "private_channels", &OTy::private_channels, "relationships", &OTy::relationships,
		"resume_gateway_url", &OTy::resume_gateway_url, "session_id", &OTy::session_id, "session_type", &OTy::session_type, "shard", &OTy::shard, "user", &OTy::user,
		"user_settings", &OTy::user_settings, "v", &OTy::v, "auth", &OTy::authVal);
};

template<> struct glz::meta<ReadyMessage> {
	using OTy					= ReadyMessage;
	static constexpr auto value = object("d", &OTy::d, "op", &OTy::op, "s", &OTy::s, "t", &OTy::t);
};

struct test_struct {
	jsonifier::vector<std::string> testStrings{};
	jsonifier::vector<uint64_t> testUints{};
	jsonifier::vector<double> testDoubles{};
	jsonifier::vector<int64_t> testInts{};
	jsonifier::vector<bool> testBools{};
};

struct json_data {
	std::string theData{};
	jsonifier::vector<int32_t> arraySizes{};
};

template<typename OTy> struct Test {
	jsonifier::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename OTy> struct TestGenerator {
	jsonifier::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937 gen{ randomEngine() };
	jsonifier::vector<int32_t> arraySizes{};

	static constexpr jsonifier::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\\\r\b\f\t\n" };

	double randomizeNumber(double mean, double stdDeviation) {
		std::normal_distribution<> normalDistributionTwo{ mean, stdDeviation };
		auto theResult = normalDistributionTwo(this->randomEngine);
		if (theResult < 0) {
			theResult = -theResult;
		}
		return static_cast<double>(theResult);
	}

	inline static json_data generateJsonData() {
		std::string buffer{};
		TestGenerator generator{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson(generator, buffer);
		//std::cout << "SERIALIZED JSON: " << buffer << std::endl;
		json_data returnData{};
		returnData.arraySizes = generator.arraySizes;
		returnData.theData	  = buffer;
		return returnData;
	}

	std::string generateString() {
		auto length{ randomizeNumber(35.0f, 10.0f) };
		static int32_t charsetSize = charset.size();
		std::mt19937 generator(std::random_device{}());
		std::uniform_int_distribution<int32_t> distribution(0, charsetSize - 1);
		std::string result{};
		for (int32_t x = 0; x < length; ++x) {
			result += charset[distribution(generator)];
		}
		return result;
	}

	double generateDouble() {
		auto newValue = static_cast<double>(randomizeNumber(1000000.34342f, 10000.3435454f));
		return generateBool() ? newValue : -newValue;
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumber(50.0f, 50.0f) >= 50.0f);
	};

	uint64_t generateUint() {
		return static_cast<uint64_t>(randomizeNumber(1000000000000000, 10000.0f));
	};

	int64_t generateInt() {
		auto newValue = static_cast<int64_t>(randomizeNumber(1000000000000000, 10000.0f));
		return generateBool() ? newValue : -newValue;
	};

	TestGenerator() {
		auto fill = [&](auto& v) {
			v.resize(35);
			for (uint64_t x = 0; x < 35; ++x) {
				if constexpr (std::same_as<OTy, test_struct>) {
					auto arraySize01 = randomizeNumber(35, 15);
					arraySizes.emplace_back(arraySize01);
					for (uint64_t y = 0; y < arraySize01; ++y) {
						v[x].testStrings.emplace_back(generateString());
						v[x].testBools.emplace_back(generateBool());
						v[x].testUints.emplace_back(generateUint());
						v[x].testInts.emplace_back(generateInt());
						v[x].testDoubles.emplace_back(generateDouble());
					}
				}
			}
		};

		fill(a);
		fill(b);
		fill(c);
		fill(d);
		fill(e);
		fill(f);
		fill(g);
		fill(h);
		fill(i);
		fill(j);
		fill(k);
		fill(l);
		fill(m);
		fill(n);
		fill(o);
		fill(p);
		fill(q);
		fill(r);
		fill(s);
		fill(t);
		fill(u);
		fill(v);
		fill(w);
		fill(x);
		fill(y);
		fill(z);
	}
};

GLZ_META(test_struct, testBools, testInts, testUints, testDoubles, testStrings);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(TestGenerator<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
template<typename OTy> struct AbcTest {
	jsonifier::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using OTy = test_struct;
	static constexpr auto parseValue =
		createObject("testBools", &OTy::testBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles", &OTy::testDoubles, "testStrings", &OTy::testStrings);
};

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy						 = Test<test_struct>;
	static constexpr auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};


template<> struct jsonifier::core<TestGenerator<test_struct>> {
	using OTy						 = TestGenerator<test_struct>;
	static constexpr auto parseValue = createObject("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using OTy						 = AbcTest<test_struct>;
	static constexpr auto parseValue = createObject("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r", &OTy::r,
		"q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e",
		&OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

GLZ_META(AbcTest<test_struct>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);

#if defined(NDEBUG)
constexpr uint64_t iterations = 100;
#else
constexpr uint64_t iterations = 1;
#endif

struct results {
	std::string name{};
	std::string test{};
	std::string url{};
	uint64_t iterations{};

	std::optional<uint64_t> json_byte_length{};
	std::optional<uint64_t> json_read{};
	std::optional<uint64_t> json_write{};

	void print() {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";

		if (json_byte_length) {
			auto mbCount		 = static_cast<double>(*json_byte_length) / (1024.0 * 1024.0);
			auto readSecondCount = static_cast<double>(*json_read) / (1000000000.0);
			std::cout << "Length: " << *json_byte_length << std::endl;
			std::cout << "Read: " << static_cast<int64_t>(mbCount / readSecondCount) << " MB/s\n";
			if (json_write) {
				auto writeSecondCount = static_cast<double>(*json_write) / (1000000000.0);
				std::cout << "Write: " << static_cast<int64_t>(mbCount / writeSecondCount) << " MB/s\n";
			}
		}

		std::cout << "\n---" << std::endl;
	}

	std::string json_stats() {
		std::string write{};
		std::string read{};
		std::string finalstring{};
		auto mbCount		 = static_cast<double>(*json_byte_length) / (1024.0 * 1024.0);
		auto readSecondCount = static_cast<double>(*json_read) / (1000000000.0);
		double writeSecondCount{};
		if (json_write) {
			writeSecondCount = static_cast<double>(*json_write) / (1000000000.0);
			write			 = fmt::format("{}", static_cast<int64_t>(mbCount / writeSecondCount));
		} else {
			write = "N/A";
		}
		read		= fmt::format("{}", static_cast<int64_t>(mbCount / readSecondCount));
		finalstring = fmt::format("| [**{}**]({}) | **{}** | **{}** |", name, url, write, read);
		return finalstring;
	}
};

class FileLoader {
  public:
	FileLoader(const char* filePathNew) {
		filePath	   = filePathNew;
		auto theStream = std::ofstream{ filePath.data(), std::ios::binary | std::ios::out | std::ios::in };
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(std::string fileToSave) {
		auto theStream = std::ofstream{ filePath.data(), std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc };
		theStream << "";
		theStream.write(fileToSave.data(), fileToSave.size());
		theStream.close();
	}

	operator std::string() {
		return fileContents;
	}

	~FileLoader() {
	}

  protected:
	std::string fileContents{};
	std::string filePath{};
};

template<typename Function> uint64_t benchmark(Function function, int64_t iterationCount) {
	uint64_t currentLowestTime{ std::numeric_limits<uint64_t>::max() };
	for (int64_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::nano>>(std::chrono::system_clock::now().time_since_epoch());
		function();
		auto endTime = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::nano>>(std::chrono::system_clock::now().time_since_epoch());
		auto newTime = endTime - startTime;
		if (static_cast<uint64_t>(newTime.count()) < currentLowestTime) {
			currentLowestTime = static_cast<uint64_t>(newTime.count());
		}
	}
	return currentLowestTime;
}

auto jsonifier_single_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Single Test", "https://github.com/realtimechris/jsonifier", 1 };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};
	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		1);
	r.json_byte_length = buffer.size();
	r.json_write	   = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Multi Test", "https://github.com/realtimechris/jsonifier", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read		   = result;
	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	//std::cout << "CURRENT BUFFER: " << buffer << std::endl;

	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_abc_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Abc Test", "https://github.com/realtimechris/jsonifier", iterations };
	AbcTest<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_discord_test(bool doWePrint = true) {
	std::string buffer{ discordData };
	auto newSize = buffer.size();

	results r{ "jsonifier", "Discord Test", "https://github.com/realtimechris/jsonifier", iterations };
	ReadyMessage discordDataTest{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}


auto glaze_single_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Single Test", "https://github.com/stephenberry/glaze", 1 };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		1);

	r.json_read = result;
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		1);
	r.json_byte_length = buffer.size();
	r.json_write	   = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Multi Test", "https://github.com/stephenberry/glaze", iterations };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_abc_test(const std::string bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Abc Test", "https://github.com/stephenberry/glaze", iterations };
	AbcTest<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(uint64Test, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_discord_test(bool doWePrint = true) {
	std::string buffer{ discordData };

	results r{ "glaze", "Discord Test", "https://github.com/stephenberry/glaze", iterations };
	ReadyMessage discordDataTest{};

	auto result = benchmark(
		[&]() {
			if (auto error = glz::read_json(discordDataTest, buffer); error) {
				std::cout << "glaze Error: " << glz::format_error(error, buffer) << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(discordDataTest, buffer);
		},
		iterations);
	r.json_write = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}
	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(Test<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

template<typename OTy2> void simdPullArray(ondemand::array newX, jsonifier::vector<OTy2>& newVector);

template<> void simdPullArray<double>(ondemand::array newX, jsonifier::vector<double>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_double());
	}
}

template<> void simdPullArray<int64_t>(ondemand::array newX, jsonifier::vector<int64_t>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_int64());
	}
}

template<> void simdPullArray<uint64_t>(ondemand::array newX, jsonifier::vector<uint64_t>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_uint64());
	}
}

template<> void simdPullArray<bool>(ondemand::array newX, jsonifier::vector<bool>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(value.get_bool());
	}
}

template<> void simdPullArray<std::string>(ondemand::array newX, jsonifier::vector<std::string>& newVector) {
	for (ondemand::value value: newX) {
		newVector.emplace_back(static_cast<std::string>(value.get_string().value()));
	}
}

#define SIMD_Pull(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		ondemand::array newArray{}; \
		ondemand::object newObject{}; \
		for (ondemand::value value: newX) { \
			test_struct newStruct{}; \
			newObject = value.get_object(); \
			newArray  = newObject["testInts"].get_array().value(); \
			simdPullArray(newArray, newStruct.testInts); \
			newArray = newObject["testDoubles"].get_array().value(); \
			simdPullArray(newArray, newStruct.testDoubles); \
			newArray = newObject["testStrings"].get_array().value(); \
			simdPullArray(newArray, newStruct.testStrings); \
			newArray = newObject["testUints"].get_array().value(); \
			simdPullArray(newArray, newStruct.testUints); \
			newArray = newObject["testBools"].get_array().value(); \
			simdPullArray(newArray, newStruct.testBools); \
			obj.x.emplace_back(std::move(newStruct)); \
		} \
	}

bool on_demand::read_in_order(Test<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(a);
	SIMD_Pull(b);
	SIMD_Pull(c);
	SIMD_Pull(d);
	SIMD_Pull(e);
	SIMD_Pull(f);
	SIMD_Pull(g);
	SIMD_Pull(h);
	SIMD_Pull(i);
	SIMD_Pull(j);
	SIMD_Pull(k);
	SIMD_Pull(l);
	SIMD_Pull(m);
	SIMD_Pull(n);
	SIMD_Pull(o);
	SIMD_Pull(p);
	SIMD_Pull(q);
	SIMD_Pull(r);
	SIMD_Pull(s);
	SIMD_Pull(t);
	SIMD_Pull(u);
	SIMD_Pull(v);
	SIMD_Pull(w);
	SIMD_Pull(x);
	SIMD_Pull(y);
	SIMD_Pull(z);
	return false;
}

auto simdjson_single_test(const std::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();
	uint64_t result{};
	result = benchmark(
		[&]() {
			parser.read_in_order(uint64Test, padded_string{ buffer });
		},
		1);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_test(const std::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Multi Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};

	r.json_byte_length = buffer.size();
	uint64_t result{};
	result = benchmark(
		[&]() {
			parser.read_in_order(uint64Test, padded_string{ buffer });
		},
		iterations);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(z);
	SIMD_Pull(y);
	SIMD_Pull(x);
	SIMD_Pull(w);
	SIMD_Pull(v);
	SIMD_Pull(u);
	SIMD_Pull(t);
	SIMD_Pull(s);
	SIMD_Pull(r);
	SIMD_Pull(q);
	SIMD_Pull(p);
	SIMD_Pull(o);
	SIMD_Pull(n);
	SIMD_Pull(m);
	SIMD_Pull(l);
	SIMD_Pull(k);
	SIMD_Pull(j);
	SIMD_Pull(i);
	SIMD_Pull(h);
	SIMD_Pull(g);
	SIMD_Pull(f);
	SIMD_Pull(e);
	SIMD_Pull(d);
	SIMD_Pull(c);
	SIMD_Pull(b);
	SIMD_Pull(a);
	return false;
}

auto simdjson_abc_test(const std::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };
	uint64_t result{};

	result = benchmark(
		[&]() {
			parser.read_out_of_order(obj, padded_string{ buffer });
		},
		iterations);

	r.json_byte_length = buffer.size();
	r.json_read		   = result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

static std::string table_header = R"(
| Library | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |)";

std::string regular_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string abc_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_abc_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_abc_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_abc_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_abc_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string discord_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_discord_test(false);
	}
	results.emplace_back(glaze_discord_test());
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_discord_test(false);
	}
	results.emplace_back(jsonifier_discord_test());

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string single_test(const json_data& jsonData) {
	jsonifier::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_single_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_single_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_single_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_single_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
};

struct testStruct {
	double testDuoble{ 2444.0444344f };
};

template<typename value_type>
concept convertible_to_or_same_as = std::convertible_to<value_type, jsonifier::string_view> || std::same_as<value_type, jsonifier::string_view>;

template<typename value_type> auto convSv(value_type&& value) {
	if constexpr (convertible_to_or_same_as<value_type>) {
		return jsonifier::string_view{ std::forward<value_type>(value) };
	} else {
		return std::forward<value_type>(value);
	}
}

template<size_t sizeNew, typename arg_type> struct tupleContainer {
	std::tuple<arg_type> value{};
	void printSize() {
		std::cout << "CURRENT SIZE: " << sizeNew << std::endl;
	}
};

//template<size_t sizeNew, typename arg_type> tupleContainer(arg_type&&) -> tupleContainer<sizeNew, std::unwrap_ref_decay_t<arg_type>>;

template<typename... arg_types> auto createObjectNew(arg_types&&... args) {
	static constexpr auto sizeNew = sizeof...(arg_types);
	auto tupleNew				  = std::make_tuple(::convSv(args)...);
	using tuple_t				  = decltype(tupleNew);
	return tupleContainer<sizeNew, tuple_t>{ std::move(tupleNew) };
}

template<typename... arg_types> auto createObjectVariant(arg_types&&... args) {
	static constexpr auto sizeNew = sizeof...(arg_types);
	auto tupleNew				  = std::make_tuple(::convSv(args)...);
	using tuple_t				  = decltype(tupleNew);
	return tuple_t{};
}

struct TestStructNew {
	uint64_t value{ 1142732033978814564 };
};

template<> struct jsonifier::core<TestStructNew> {
	using OTy						 = TestStructNew;
	static constexpr auto parseValue = createObject("value", &OTy::value);
};

template<typename value_type> struct TestStructNew02 {
	uint64_t value{ 1142732033978814564 };
};

struct TestStructNew03 : public TestStructNew02<TestStructNew03> {
	uint64_t value{ 1142732033978814564 };
};


int32_t main() {
	try {
		TestStructNew testValue{};
		jsonifier::string stringNewer10{};

		jsonifier::jsonifier_core parser{};
		parser.serializeJson(testValue, stringNewer10);
		std::cout << "CURRENT BUFFER: " << stringNewer10 << std::endl;
		stringNewer10.clear();
		parser.parseJson(testValue, stringNewer10);
		std::cout << "CURRENT BUFFER: " << testValue.value << std::endl;
		auto newResult = ::createObjectVariant("TESTING", &test_struct::testBools);
		newResult;
		json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
#if defined(_WIN32)
		FileLoader fileLoader01{ "../../../ReadMe.md" };
		FileLoader fileLoader02{ "../../../JsonData.json" };
		fileLoader02.saveFile(glz::prettify(jsonData.theData));
#else
		FileLoader fileLoader01{ "../ReadMe.md" };
		FileLoader fileLoader02{ "../JsonData.json" };
		fileLoader02.saveFile(glz::prettify(jsonData.theData));
#endif
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t result = std::time(nullptr);
		resultTwo		   = *localtime(&result);
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		auto discordTestResults = discord_test(jsonData);
		auto singlTestResults	= single_test(jsonData);
		auto multiTestResults	= regular_test(jsonData);
		auto abcTestResults		= abc_test(jsonData);
		std::string newString	= fileLoader01;
		uint64_t currentStart{ 0 };
		uint64_t currentEnd{ 0 };
		currentEnd			  = newString.find("Latest results (") + std::string{ "Latest results (" }.size();
		std::string dateLine  = newString.substr(currentStart, currentEnd);
		currentStart		  = currentEnd + 2 + std::string{ "Jan 01, 2022" }.size();
		currentEnd			  = newString.find("Single Iteration Test Results:") + std::string{ "Single Iteration Test Results:" }.size();
		std::string section01 = newString.substr(currentStart, (currentEnd - currentStart));
		currentStart		  = newString.find("Multi Iteration Test Results:");
		currentEnd			  = newString.find("Multi Iteration Test Results:") + std::string{ "Multi Iteration Test Results:" }.size();
		std::string section02 = newString.substr(currentStart, (currentEnd - currentStart));
		currentStart		  = newString.find("## ABC Test (Out of Sequence Performance)");
		currentEnd			  = newString.find("In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining "
														  "optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.") +
			std::string{ "In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal "
						 "performance regardless of the JSON document's scale, or ordering of the keys being parsed." }
				.size();
		std::string section03	= newString.substr(currentStart, (currentEnd - currentStart));
		std::string newerString = dateLine + newTimeString + "):" + section01 + "\n" + singlTestResults + "\n\n" + section02 + "\n" + multiTestResults + "\n" + "> " +
			std::to_string(iterations) + " iterations on a 6 core (Intel i7 8700k)\n\n" + section03 + "\n" + abcTestResults + "\n" + "> " + std::to_string(iterations) +
			" iterations on a 6 core (Intel i7 8700k)";
		fileLoader01.saveFile(newerString);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}