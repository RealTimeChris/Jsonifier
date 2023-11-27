/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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
/// Note: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

namespace jsonifier_internal {

	enum class general_state : uint32_t { Normal, Escaped, String };

	template<jsonifier::concepts::string_t string_type, typename function_type>
	JSONIFIER_INLINE void prettifyImpl(const char c, string_type& out, uint32_t& indent, function_type newLine, general_state& state) noexcept {
		switch (state) {
			case general_state::Escaped: {
				state = general_state::String;
				out += c;
				break;
			}
			case general_state::String: {
				if (c == '\\') {
					state = general_state::Escaped;
				} else if (c == '"') {
					state = general_state::Normal;
				}
				out += c;
				break;
			}
			case general_state::Normal: {
				switch (c) {
					case ',': {
						out += c;
						newLine();
						break;
					}
					case '[': {
						++indent;
						out += c;
						newLine();
						break;
					}
					case ']': {
						--indent;
						newLine();
						out += c;
						break;
					}
					case '{': {
						++indent;
						out += c;
						newLine();
						break;
					}
					case '}': {
						--indent;
						newLine();
						out += c;
						break;
					}
					case '\"': {
						out += c;
						state = general_state::String;
						break;
					}
					case '/':
						[[fallthrough]];
					case ':':
						[[fallthrough]];
					case ' ':
						[[fallthrough]];
					case '\n':
						[[fallthrough]];
					case '\r':
						[[fallthrough]];
					case '\t':
						[[fallthrough]];
					default: {
						out += c;
						break;
					}
				}
				break;
			}
			default:
				out += c;
				break;
		}
	}
}

namespace jsonifier {

	template<concepts::string_t string_type> JSONIFIER_INLINE string_type prettify(const string_type& in, const bool tabs = false, const uint32_t indentSize = 3) noexcept {
		uint32_t indent{};
		string_type out{};
		out.reserve(in.size());

		auto newLine = [&]() {
			out += "\n";

			for (uint32_t i = 0; i < indent * (tabs ? 1 : indentSize); i++) {
				out += tabs ? "\t" : " ";
			}
		};

		jsonifier_internal::general_state state{ jsonifier_internal::general_state::Normal };

		for (auto& c: in) {
			jsonifier_internal::prettifyImpl(c, out, indent, newLine, state);
		}
		return out;
	}

}// namespace jsonifier_internal
