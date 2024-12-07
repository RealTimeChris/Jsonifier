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
#pragma once

#include <unordered_map>
#include <optional>
#include <string>
#include <vector>

 struct audience_sub_category_names {
	std::string the337100890{};
};

struct names {};

struct event {
	std::nullptr_t description{};
	int64_t id{};
	std::optional<std::string> logo{};
	std::string name{};
	std::vector<int64_t> subTopicIds{};
	std::nullptr_t subjectCode{};
	std::nullptr_t subtitle{};
	std::vector<int64_t> topicIds{};
};

struct price {
	int64_t amount{};
	int64_t audienceSubCategoryId{};
	int64_t seatCategoryId{};
};

struct area {
	int64_t areaId{};
	std::vector<std::nullptr_t> blockIds{};
};

struct seat_category {
	std::vector<area> areas{};
	int64_t seatCategoryId{};
};

struct venue_names {
	std::string PLEYEL_PLEYEL{};
};

struct performance {
	int64_t eventId{};
	int64_t id{};
	std::optional<std::string> logo{};
	std::nullptr_t name{};
	std::vector<price> prices{};
	std::vector<seat_category> seatCategories{};
	std::nullptr_t seatMapImage{};
	int64_t start{};
	std::string venueCode{};
};

struct citm_catalog_message {
	std::unordered_map<std::string, std::string> areaNames{};
	audience_sub_category_names audienceSubCategoryNames{};
	names blockNames{};
	std::unordered_map<std::string, event> events{};
	std::vector<performance> performances{};
	std::unordered_map<std::string, std::string> seatCategoryNames{};
	std::unordered_map<std::string, std::string> subTopicNames{};
	names subjectNames{};
	std::unordered_map<std::string, std::string> topicNames{};
	std::unordered_map<std::string, std::vector<int64_t>> topicSubTopics{};
	venue_names venueNames{};
};
