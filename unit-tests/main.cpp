/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
#include "common.hpp"
#include <jsonifier>

/*
template<typename failure_programmres_type, bool minified = false, bool knownOrder = false, bool partial = false>
JSONIFIER_INLINE static void failureProgrammersParseFunction(const std::string& dataToParse) {
	failure_programmres_type value{};
	std::string serializedJson{};
	jsonifier::jsonifier_core<> piece_of_shit_parser_written_by_trash_programmer{};
	piece_of_shit_parser_written_by_trash_programmer.parseJson<jsonifier::parse_options_new{ .partialRead = partial, .knownOrder = knownOrder, .minified = minified }>(value,
		dataToParse);
	for (auto& error: piece_of_shit_parser_written_by_trash_programmer.getErrors()) {
		std::cout << "Error: " << error << std::endl;
	}
	piece_of_shit_parser_written_by_trash_programmer.serializeJson(value, serializedJson);
	std::cout << "Failure Programmer's Shitty Attempt at Parsing: " << serializedJson << std::endl;
}*/

namespace jsonifier::internal {

	template<typename value_type, typename context_type, parse_options_new optionsNew> struct parse_partial_impl_new;
	template<typename value_type, typename context_type, parse_options_new optionsNew> struct parse_impl_new;

	template<concepts::string_t value_type, typename context_type, parse_options_new options> struct parse_impl_new<value_type, context_type, options> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseStringValue(value);
		}
	};

	template<concepts::num_t value_type, typename context_type, parse_options_new options> struct parse_impl_new<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseNumberValue(value);
		}
	};

	template<concepts::bool_t value_type, typename context_type, parse_options_new options> struct parse_impl_new<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseBoolValue(value);
		}
	};

	template<concepts::always_null_t value_type, typename context_type, parse_options_new options> struct parse_impl_new<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			context.parseNullValue();
		}
	};

	template<parse_options_new options> struct parse_new {
		template<typename value_type_new, typename context_type> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type&& context) noexcept {
			using value_type = remove_cvref_t<value_type_new>;
			if constexpr (options.partialRead) {
				parse_partial_impl_new<value_type, context_type, options>::impl(value, context);
			} else {
				parse_impl_new<value_type, context_type, options>::impl(value, context);
			}
		}
	};

	template<typename derived_type_new> class parser_new {
	  public:
		friend class jsonifier::raw_json_data;
		template<const auto options, typename context_type> friend struct derailleur;

		using derived_type = derived_type_new;

		parser_new& operator=(const parser_new& other) = delete;
		parser_new(const parser_new& other)		   = delete;

		template<parse_options_new options = parse_options_new{}, typename value_type, typename buffer_type> inline bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options_new optionsNew{ options };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
				jsonifier_iterator<options, structural_index_ptr> context{ this, derivedRef.section.begin(), derivedRef.section.end(), rootIter, endIter };
				auto newSize = context.getTotalLength() / 2;
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				context.errors.clear();
				if JSONIFIER_UNLIKELY (!context.anyInput()) {
					return false;
				}
				parse_new<optionsNew>::impl(object, context);
				return context.errors.size() == 0;
			} else {
				static constexpr parse_options_new optionsNew{ options };
				jsonifier_iterator<options, string_view_ptr> context{ getBeginIter(in), getEndIter(in) };
				auto newSize = context.getTotalLength() / 2;
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				context.errors.clear();
				if JSONIFIER_UNLIKELY (!context.anyInput()) {
					return false;
				}
				parse_new<optionsNew>::impl(object, context);
				return context.errors.size() == 0;
			}
		}

		const std::vector<internal::error>& getErrors() const noexcept {
			return derivedRef.getErrors();
		}

		JSONIFIER_INLINE auto& getStringBuffer() noexcept {
			return derivedRef.stringBuffer;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		parser_new () noexcept : derivedRef{ initializeSelfRef() } {
		}

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		~parser_new () noexcept = default;
	};
}

struct test_struct {
	std::vector<int32_t> values{ 2323 };
};

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<&test_struct::values>();
};

struct json_file {
	std::string fileContents{};
	std::string fileName{};
};

inline static std::vector<json_file> processFilesInFolder(const std::string& basePathNew) noexcept {
	std::vector<json_file> resultFileContents{};
	try {
		for (const auto& entry: std::filesystem::directory_iterator(std::string{ basePathNew })) {
			if (entry.is_regular_file()) {
				const std::string fileName = entry.path().filename().string();

				if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == std::string{ ".json" }) {
					std::ifstream file(entry.path());
					if (file.is_open()) {
						json_file jsonFile{};
						jsonFile.fileName	  = fileName;
						jsonFile.fileContents = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
						resultFileContents.emplace_back(jsonFile);
						file.close();
					} else {
						std::cerr << "Error opening file: " << fileName << std::endl;
					}
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error while processing files: " << e.what() << std::endl;
	}

	return resultFileContents;
}

static void testFunctionImpl(const json_file& fileContents) {
	std::cout << "File: " << fileContents.fileName << ", ";
	jsonifier::internal::jsonifier_iterator<jsonifier::internal::parse_options_new{}, jsonifier::string_view_ptr> jc{ fileContents.fileContents.data(),
		fileContents.fileContents.data() + fileContents.fileContents.size() };
	for (uint64_t x = 0; x < fileContents.fileContents.size(); ++x) {
		++jc;
		if (!jc) {
			std::cout << "Failed to validate: " << fileContents.fileName << std::endl;
			for (auto& error: jc.errors) {
				std::cout << error << std::endl;
			}
			return;
		}
	}
	if (!jc.jsonifierIterateDone()) {
		std::cout << "Failed to validate: " << fileContents.fileName << std::endl;
		for (auto& error: jc.errors) {
			std::cout << error << std::endl;
		}
		return;
	}
	std::cout << "Succesfully validated: " << fileContents.fileName << std::endl;
}

static void testFunctionImplPartial(const json_file& fileContents) {
	jsonifier::internal::simd_string_reader<1024 * 1024> stringReader{};
	stringReader.reset<false>(fileContents.fileContents.data(), fileContents.fileContents.size());
	std::cout << "File: " << fileContents.fileName << ", ";
	jsonifier::internal::jsonifier_iterator<jsonifier::internal::parse_options_new{}, jsonifier::structural_index_ptr> jc{ stringReader.begin(), stringReader.end(),
		fileContents.fileContents.data(), fileContents.fileContents.data() + fileContents.fileContents.size() };
	for (uint64_t x = 0; x < fileContents.fileContents.size(); ++x) {
		++jc;
		if (!jc) {
			std::cout << "Failed to validate: " << fileContents.fileName << std::endl;
			for (auto& error: jc.errors) {
				std::cout << error << std::endl;
			}
			return;
		}
	}
	if (!jc.jsonifierIterateDone()) {
		std::cout << "Failed to validate: " << fileContents.fileName << std::endl;
		for (auto& error: jc.errors) {
			std::cout << error << std::endl;
		}
		return;
	}
	std::cout << "Succesfully validated: " << fileContents.fileName << std::endl;
}

template<bool partial> static void testFunction(const std::string& basePathNew) {
	if constexpr (partial) {
		std::cout << "Running Partial (Structural Indexing) Tests" << std::endl;		
	} else {
		std::cout << "Running Non-Partial Tests" << std::endl;
	}
	auto results = processFilesInFolder(basePathNew);
	for (auto& value: results) {
		if constexpr (partial) {			
			testFunctionImplPartial(value);
		} else {
			testFunctionImpl(value);
		}
		
	}
}

static constexpr jsonifier::internal::string_literal jsonBasePath{ basePath + "/json" };
static constexpr jsonifier::internal::string_literal jsonCheckerBasePathPass{ basePath + "/jsonchecker/pass" };
static constexpr jsonifier::internal::string_literal jsonCheckerBasePathFail{ basePath + "/jsonchecker/fail" };
static constexpr jsonifier::internal::string_literal roundtripBasePath{ basePath + "/round_trip" };

template<jsonifier::internal::string_literal basePathNew> static void testFunction() {
	static constexpr jsonifier::internal::string_literal basePathNewer{ basePathNew };
	testFunction<false>(basePathNewer);
	testFunction<true>(basePathNewer);
}

static void testFunction() {
	testFunction<jsonBasePath>();
	testFunction<jsonCheckerBasePathFail>();
	testFunction<jsonCheckerBasePathPass>();
	testFunction<roundtripBasePath>();
}

int main() {
	try {
		testFunction();
	} catch (const std::runtime_error& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	} catch (const rt_ut::rt_ut_exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
