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
/// https://github.com/RealTimeChris/jsonifier
#include "Tests.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

void print_conversation_messages(const jsonifier::raw_json_data& conversation_data) {
	auto& conv_obj = conversation_data.getObject();

	// Get the title
	jsonifier::string title = "Untitled";
	for (auto& [key, value]: conv_obj) {
		if (key == "title") {
			//title = value.getString();
			break;
		}
	}

	//std::cout << "\n=== Conversation: " << title << " ===\n\n";

	// Get the mapping object
	const jsonifier::raw_json_data* mapping_ptr = nullptr;
	for (auto& [key, value]: conv_obj) {
		if (key == "mapping") {
			mapping_ptr = &value;
			break;
		}
	}

	if (!mapping_ptr) {
		std::cout << "NO MAPPING FOUND!" << std::endl;
		return;
	}

	auto& mapping = mapping_ptr->getObject();

	// Collect messages with timestamps
	struct TimestampedMessage {
		double timestamp;
		jsonifier::string role;
		jsonifier::string content;
	};

	std::vector<TimestampedMessage> messages;

	// Iterate through all nodes in the mapping
	for (auto& [node_id, node]: mapping) {
		auto& node_obj = node.getObject();

		// Find the message key
		const jsonifier::raw_json_data* message_ptr = nullptr;
		for (auto& [key, value]: node_obj) {
			if (key == "message") {
				if (value.getType() == jsonifier::json_type::null) {
					break;
				}
				message_ptr = &value;
				break;
			}
		}

		if (!message_ptr) {
			continue;
		}

		auto& message = message_ptr->getObject();

		// Extract author role
		jsonifier::string role;
		for (auto& [key, value]: message) {
			if (key == "author") {
				auto& author = value.getObject();
				for (auto& [author_key, author_value]: author) {
					if (author_key == "role") {
						role = author_value.getString();
						break;
					}
				}
				break;
			}
		}

		if (role.empty()) {
			continue;
		}

		// Only process user and assistant messages
		if (role != "user" && role != "assistant") {
			continue;
		}

		// Extract content parts
		jsonifier::string combined_content;
		for (auto& [key, value]: message) {
			if (key == "content") {
				auto& content_obj = value.getObject();
				for (auto& [content_key, content_value]: content_obj) {
					if (content_key == "parts") {
						auto& parts = content_value.getArray();
						for (auto& part: parts) {
							if (part.getType() == jsonifier::json_type::string) {
								jsonifier::string part_str = part.getString();
								if (!part_str.empty()) {
									if (!combined_content.empty()) {
										combined_content += "\n";
									}
									combined_content += part_str;
								}
							}
						}
						break;
					}
				}
				break;
			}
		}

		// Skip if no actual content
		if (combined_content.empty()) {
			continue;
		}

		// Get timestamp
		double timestamp = 0.0;
		for (auto& [key, value]: message) {
			if (key == "create_time") {
				if (value.getType() == jsonifier::json_type::number) {
					timestamp = value.getNumber().getDouble();
				}
				break;
			}
		}

		messages.push_back({ timestamp, role, std::move(combined_content) });
	}

	// Sort by timestamp
	std::sort(messages.begin(), messages.end(), [](const auto& a, const auto& b) {
		return a.timestamp < b.timestamp;
	});

	// Print messages
	for (const auto& msg: messages) {
		std::cout << "┌─────────────────────────────────────────────────────────\n";
		std::cout << "│ " << (msg.role == "user" ? "USER" : "ASSISTANT") << "\n";
		std::cout << "├─────────────────────────────────────────────────────────\n";
		std::cout << msg.content << "\n";
		std::cout << "└─────────────────────────────────────────────────────────\n\n";
	}
}

int32_t main() {
	try {
		auto file = bnch_swt::file_loader::loadFile("C:/Users/Chris/Downloads/conversations.json");

		jsonifier::jsonifier_core parser{};
		jsonifier::raw_json_data data{};
		parser.parseJson(data, file);

		// Check for errors
		for (auto& error: parser.getErrors()) {
			std::cout << error.reportError() << std::endl;
		}

		// Iterate through each conversation in the array
		auto& conversations = data.getArray();
		for (size_t x = 0; x < conversations.size(); ++x) {
			print_conversation_messages(conversations[x]);
			std::cout << "\n\n";
		}

	} catch (std::runtime_error& error) {
		std::cout << error.what() << std::endl;
	} catch (std::out_of_range& error) {
		std::cout << error.what() << std::endl;
	}
	return 0;
}