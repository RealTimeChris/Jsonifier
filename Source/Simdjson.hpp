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

#include <BnchSwt/BenchmarkSuite.hpp>
#include "simdjson.h"
#include "Common.hpp"

#if !defined(ASAN_ENABLED)

namespace simdjson {

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<int64_t>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			int64_t newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			int64_t newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<std::string>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			std::string newVal;
			if (error = iter.operator*().get_string(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			std::string newVal;

			if (error = iter.operator*().get_string(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<uint64_t>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			uint64_t newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			uint64_t newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<bool>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			bool newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			bool newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<double>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			double newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			double newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, test_struct& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["testVals02"].get(data.testVals02); error) {
			return error;
		}

		if (error = obj["testVals05"].get(data.testVals05); error) {
			return error;
		}

		if (error = obj["testVals01"].get(data.testVals01); error) {
			return error;
		}

		if (error = obj["testVals03"].get(data.testVals03); error) {
			return error;
		}

		if (error = obj["testVals04"].get(data.testVals04); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, abc_test_struct& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["testVals04"].get(data.testVals04); error) {
			return error;
		}

		if (error = obj["testVals03"].get(data.testVals03); error) {
			return error;
		}

		if (error = obj["testVals01"].get(data.testVals01); error) {
			return error;
		}

		if (error = obj["testVals05"].get(data.testVals05); error) {
			return error;
		}

		if (error = obj["testVals02"].get(data.testVals02); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<test_struct>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			test_struct newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			test_struct newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<abc_test_struct>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			abc_test_struct newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			abc_test_struct newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value, typename value_type> auto tag_invoke(deserialize_tag, simdjson_value& val, test<value_type>& t) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["a"].get(t.a); error) {
			return error;
		}

		if (error = obj["b"].get(t.b); error) {
			return error;
		}

		if (error = obj["c"].get(t.c); error) {
			return error;
		}

		if (error = obj["d"].get(t.d); error) {
			return error;
		}

		if (error = obj["e"].get(t.e); error) {
			return error;
		}

		if (error = obj["f"].get(t.f); error) {
			return error;
		}

		if (error = obj["g"].get(t.g); error) {
			return error;
		}

		if (error = obj["h"].get(t.h); error) {
			return error;
		}

		if (error = obj["i"].get(t.i); error) {
			return error;
		}

		if (error = obj["j"].get(t.j); error) {
			return error;
		}

		if (error = obj["k"].get(t.k); error) {
			return error;
		}

		if (error = obj["l"].get(t.l); error) {
			return error;
		}

		if (error = obj["m"].get(t.m); error) {
			return error;
		}

		if (error = obj["n"].get(t.n); error) {
			return error;
		}

		if (error = obj["o"].get(t.o); error) {
			return error;
		}

		if (error = obj["p"].get(t.p); error) {
			return error;
		}

		if (error = obj["q"].get(t.q); error) {
			return error;
		}

		if (error = obj["r"].get(t.r); error) {
			return error;
		}

		if (error = obj["s"].get(t.s); error) {
			return error;
		}

		if (error = obj["t"].get(t.t); error) {
			return error;
		}

		if (error = obj["u"].get(t.u); error) {
			return error;
		}

		if (error = obj["v"].get(t.v); error) {
			return error;
		}

		if (error = obj["w"].get(t.w); error) {
			return error;
		}

		if (error = obj["x"].get(t.x); error) {
			return error;
		}

		if (error = obj["y"].get(t.y); error) {
			return error;
		}

		if (error = obj["z"].get(t.z); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value, typename value_type> auto tag_invoke(deserialize_tag, simdjson_value& val, abc_test<value_type>& t) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["z"].get(t.z); error) {
			return error;
		}

		if (error = obj["y"].get(t.y); error) {
			return error;
		}

		if (error = obj["x"].get(t.x); error) {
			return error;
		}

		if (error = obj["w"].get(t.w); error) {
			return error;
		}

		if (error = obj["v"].get(t.v); error) {
			return error;
		}

		if (error = obj["u"].get(t.u); error) {
			return error;
		}

		if (error = obj["t"].get(t.t); error) {
			return error;
		}

		if (error = obj["s"].get(t.s); error) {
			return error;
		}

		if (error = obj["r"].get(t.r); error) {
			return error;
		}

		if (error = obj["q"].get(t.q); error) {
			return error;
		}

		if (error = obj["p"].get(t.p); error) {
			return error;
		}

		if (error = obj["o"].get(t.o); error) {
			return error;
		}

		if (error = obj["n"].get(t.n); error) {
			return error;
		}

		if (error = obj["m"].get(t.m); error) {
			return error;
		}

		if (error = obj["l"].get(t.l); error) {
			return error;
		}

		if (error = obj["k"].get(t.k); error) {
			return error;
		}

		if (error = obj["j"].get(t.j); error) {
			return error;
		}

		if (error = obj["i"].get(t.i); error) {
			return error;
		}

		if (error = obj["h"].get(t.h); error) {
			return error;
		}

		if (error = obj["g"].get(t.g); error) {
			return error;
		}

		if (error = obj["f"].get(t.f); error) {
			return error;
		}

		if (error = obj["e"].get(t.e); error) {
			return error;
		}

		if (error = obj["d"].get(t.d); error) {
			return error;
		}

		if (error = obj["c"].get(t.c); error) {
			return error;
		}

		if (error = obj["b"].get(t.b); error) {
			return error;
		}

		if (error = obj["a"].get(t.a); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value, typename value_type> auto tag_invoke(deserialize_tag, simdjson_value& val, partial_test<value_type>& t) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["m"].get(t.m); error) {
			return error;
		}

		if (error = obj["s"].get(t.s); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<std::nullptr_t>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			if (!iter.operator*().is_null()) {
				return simdjson::UNEXPECTED_ERROR;
			}
		}

		for (; iter != arr.end(); ++iter) {
			if (!iter.operator*().is_null()) {
				return simdjson::UNEXPECTED_ERROR;
			}
			vector.emplace_back();
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::unordered_map<std::string, std::string>& car) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		for (auto field: obj) {
			std::string key;
			auto key_result = field.unescaped_key();
			if (error = key_result.error(); error) {
				return error;
			}

			key = static_cast<std::string>(key_result.value());

			ondemand::value field_value = field.value();
			std::string newValue;
			if (error = field_value.get_string(newValue); error) {
				return error;
			}
			car[key] = std::move(newValue);
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::unordered_map<std::string, std::vector<int64_t>>& car) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		for (auto field: obj) {
			std::string key;
			auto key_result = field.unescaped_key();
			if (error = key_result.error(); error) {
				return error;
			}

			key = static_cast<std::string>(key_result.value());

			ondemand::value field_value = field.value();
			std::vector<int64_t> newValue;
			if (error = field_value.get(newValue); error) {
				return error;
			}
			car[key] = std::move(newValue);
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<std::vector<double>>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			std::vector<double> newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			std::vector<double> newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<std::vector<std::vector<double>>>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			std::vector<std::vector<double>> newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			std::vector<std::vector<double>> newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, geometry_data& geom) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["type"].get_string(geom.type); error) {
			return error;
		}

		if (error = obj["coordinates"].get(geom.coordinates); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, properties_data& props) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["name"].get_string(props.name); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, feature& feat) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["type"].get_string(feat.type); error) {
			return error;
		}

		if (error = obj["properties"].get(feat.properties); error) {
			return error;
		}

		if (error = obj["geometry"].get(feat.geometry); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<feature>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			feature newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			feature newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, canada_message& msg) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["type"].get_string(msg.type); error) {
			return error;
		}

		if (error = obj["features"].get(msg.features); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, audience_sub_category_names& names) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["337100890"].get_string(names.the337100890); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, event& ev) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["description"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["id"].get(ev.id); error) {
			return error;
		}

		if (error = obj["logo"].get(ev.logo); error) {
			return error;
		}

		if (error = obj["name"].get_string(ev.name); error) {
			return error;
		}

		if (error = obj["subTopicIds"].get(ev.subTopicIds); error) {
			return error;
		}

		if (!obj["subjectCode"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["subtitle"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["topicIds"].get(ev.topicIds); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::unordered_map<std::string, event>& car) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		for (auto field: obj) {
			std::string key;
			auto key_result = field.unescaped_key();
			if (error = key_result.error(); error) {
				return error;
			}

			key = static_cast<std::string>(key_result.value());

			ondemand::value field_value = field.value();
			event newValue;
			if (error = field_value.get(newValue); error) {
				return error;
			}
			car[key] = std::move(newValue);
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, price& pr) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["amount"].get(pr.amount); error) {
			return error;
		}

		if (error = obj["audienceSubCategoryId"].get(pr.audienceSubCategoryId); error) {
			return error;
		}

		if (error = obj["seatCategoryId"].get(pr.seatCategoryId); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, area& ar) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["areaId"].get(ar.areaId); error) {
			return error;
		}

		if (error = obj["blockIds"].get(ar.blockIds); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<area>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			area newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			area newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, seat_category& sc) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["areas"].get(sc.areas); error) {
			return error;
		}

		if (error = obj["seatCategoryId"].get(sc.seatCategoryId); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<seat_category>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			seat_category newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			seat_category newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::optional<std::string>& car) {
		if (auto error = val.get_string(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::optional<bool>& car) {
		if (auto error = val.get(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value, jsonifier::concepts::optional_t optional_type> auto tag_invoke(deserialize_tag, simdjson_value& val, optional_type& car) {
		if (auto error = val.get(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::optional<int64_t>& car) {
		if (auto error = val.get(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<price>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			price newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			price newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, performance& perf) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["eventId"].get(perf.eventId); error) {
			return error;
		}

		if (error = obj["id"].get(perf.id); error) {
			return error;
		}

		if (error = obj["logo"].get(perf.logo); error != simdjson::NO_SUCH_FIELD && error != simdjson::SUCCESS) {
			return error;
		}

		if (!obj["name"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["prices"].get(perf.prices); error) {
			return error;
		}

		if (error = obj["seatCategories"].get(perf.seatCategories); error) {
			return error;
		}

		if (!obj["seatMapImage"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["start"].get(perf.start); error) {
			return error;
		}

		if (error = obj["venueCode"].get_string(perf.venueCode); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<performance>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			performance newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			performance newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, names&) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, venue_names& vnames) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["PLEYEL_PLEYEL"].get_string(vnames.PLEYEL_PLEYEL); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, citm_catalog_message& msg) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["areaNames"].get(msg.areaNames); error) {
			return error;
		}

		if (error = obj["audienceSubCategoryNames"].get(msg.audienceSubCategoryNames); error) {
			return error;
		}

		if (error = obj["blockNames"].get(msg.blockNames); error) {
			return error;
		}

		if (error = obj["events"].get(msg.events); error) {
			return error;
		}

		if (error = obj["performances"].get(msg.performances); error) {
			return error;
		}

		if (error = obj["seatCategoryNames"].get(msg.seatCategoryNames); error) {
			return error;
		}

		if (error = obj["subTopicNames"].get(msg.subTopicNames); error) {
			return error;
		}

		if (error = obj["subjectNames"].get(msg.subjectNames); error) {
			return error;
		}

		if (error = obj["topicNames"].get(msg.topicNames); error) {
			return error;
		}

		if (error = obj["topicSubTopics"].get(msg.topicSubTopics); error) {
			return error;
		}

		if (error = obj["venueNames"].get(msg.venueNames); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, icon_emoji_data& emoji) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["name"].get(emoji.name); error) {
			return error;
		}

		if (!obj["id"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, permission_overwrite& perm) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["allow"].get_string(perm.allow); error) {
			return error;
		}

		if (error = obj["type"].get(perm.type); error) {
			return error;
		}

		if (error = obj["deny"].get_string(perm.deny); error) {
			return error;
		}

		if (error = obj["id"].get_string(perm.id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<permission_overwrite>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			permission_overwrite newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			permission_overwrite newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, channel_data& channel) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["default_thread_rate_limit_per_user"].get(channel.default_thread_rate_limit_per_user); error) {
			return error;
		}

		if (error = obj["default_auto_archive_duration"].get(channel.default_auto_archive_duration); error) {
			return error;
		}

		if (error = obj["permission_overwrites"].get(channel.permission_overwrites); error) {
			return error;
		}

		if (error = obj["rate_limit_per_user"].get(channel.rate_limit_per_user); error) {
			return error;
		}

		if (error = obj["video_quality_mode"].get(channel.video_quality_mode); error) {
			return error;
		}

		if (error = obj["total_message_sent"].get(channel.total_message_sent); error) {
			return error;
		}

		if (error = obj["last_pin_timestamp"].get_string(channel.last_pin_timestamp); error) {
			return error;
		}

		if (error = obj["last_message_id"].get(channel.last_message_id); error) {
			return error;
		}

		if (error = obj["application_id"].get_string(channel.application_id); error) {
			return error;
		}

		if (error = obj["message_count"].get(channel.message_count); error) {
			return error;
		}

		if (error = obj["member_count"].get(channel.member_count); error) {
			return error;
		}

		if (error = obj["applied_tags"].get(channel.applied_tags); error) {
			return error;
		}

		if (error = obj["permissions"].get_string(channel.permissions); error) {
			return error;
		}

		if (error = obj["user_limit"].get(channel.user_limit); error) {
			return error;
		}

		if (error = obj["icon_emoji"].get(channel.icon_emoji); error) {
			return error;
		}

		if (error = obj["recipients"].get(channel.recipients); error) {
			return error;
		}

		if (error = obj["parent_id"].get_string(channel.parent_id); error) {
			return error;
		}

		if (error = obj["position"].get(channel.position); error) {
			return error;
		}

		if (error = obj["guild_id"].get_string(channel.guild_id); error) {
			return error;
		}

		if (error = obj["owner_id"].get_string(channel.owner_id); error) {
			return error;
		}

		if (error = obj["managed"].get(channel.managed); error) {
			return error;
		}

		if (error = obj["bitrate"].get(channel.bitrate); error) {
			return error;
		}

		if (error = obj["version"].get(channel.version); error) {
			return error;
		}

		if (!obj["status"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["flags"].get(channel.flags); error) {
			return error;
		}

		if (!obj["topic"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["nsfw"].get(channel.nsfw); error) {
			return error;
		}

		if (error = obj["type"].get(channel.type); error) {
			return error;
		}

		if (error = obj["icon"].get_string(channel.icon); error) {
			return error;
		}

		if (error = obj["name"].get_string(channel.name); error) {
			return error;
		}

		if (error = obj["id"].get_string(channel.id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<channel_data>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			channel_data newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			channel_data newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, user_data& user) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["avatar_decoration_data"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["discriminator"].get_string(user.discriminator); error) {
			return error;
		}

		if (error = obj["public_flags"].get(user.public_flags); error) {
			return error;
		}

		if (error = obj["premium_type"].get(user.premium_type); error) {
			return error;
		}

		if (error = obj["accent_color"].get(user.accent_color); error) {
			return error;
		}

		if (error = obj["display_name"].get(user.display_name); error) {
			return error;
		}

		if (error = obj["mfa_enabled"].get(user.mfa_enabled); error) {
			return error;
		}

		if (error = obj["global_name"].get(user.global_name); error) {
			return error;
		}

		if (error = obj["user_name"].get_string(user.user_name); error) {
			return error;
		}

		if (error = obj["verified"].get(user.verified); error) {
			return error;
		}

		if (error = obj["system"].get(user.system); error) {
			return error;
		}

		if (!obj["locale"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["banner"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["avatar"].get(user.avatar); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["flags"].get(user.flags); error) {
			return error;
		}

		if (error = obj["email"].get_string(user.email); error) {
			return error;
		}

		if (error = obj["bot"].get(user.bot); error) {
			return error;
		}

		if (error = obj["id"].get_string(user.id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, member_data& member) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["communication_disabled_until"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["premium_since"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["permissions"].get_string(member.permissions); error) {
			return error;
		}

		if (error = obj["joined_at"].get_string(member.joined_at); error) {
			return error;
		}

		if (error = obj["guild_id"].get_string(member.guild_id); error) {
			return error;
		}

		if (error = obj["pending"].get(member.pending); error) {
			return error;
		}

		if (!obj["avatar"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["flags"].get(member.flags); error) {
			return error;
		}

		if (error = obj["roles"].get(member.roles); error) {
			return error;
		}

		if (error = obj["mute"].get(member.mute); error) {
			return error;
		}

		if (error = obj["deaf"].get(member.deaf); error) {
			return error;
		}

		if (error = obj["user"].get(member.user); error) {
			return error;
		}

		if (error = obj["nick"].get(member.nick); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<member_data>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			member_data newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			member_data newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, tags_data& tags) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["premium_subscriber"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["bot_id"].get(tags.bot_id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, role_data& role) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["unicode_emoji"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["mentionable"].get(role.mentionable); error) {
			return error;
		}

		if (error = obj["permissions"].get_string(role.permissions); error) {
			return error;
		}

		if (error = obj["position"].get(role.position); error) {
			return error;
		}

		if (error = obj["managed"].get(role.managed); error) {
			return error;
		}

		if (error = obj["version"].get(role.version); error) {
			return error;
		}

		if (error = obj["hoist"].get(role.hoist); error) {
			return error;
		}

		if (error = obj["flags"].get(role.flags); error) {
			return error;
		}

		if (error = obj["color"].get(role.color); error) {
			return error;
		}

		if (error = obj["tags"].get(role.tags); error) {
			return error;
		}

		if (error = obj["name"].get_string(role.name); error) {
			return error;
		}

		if (!obj["icon"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["id"].get_string(role.id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<role_data>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			role_data newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			role_data newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, guild_data& guild) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (!obj["latest_on_boarding_question_id"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["max_stage_video_channel_users"].get(guild.max_stage_video_channel_users); error) {
			return error;
		}

		if (error = obj["default_message_notifications"].get(guild.default_message_notifications); error) {
			return error;
		}

		if (error = obj["premium_progress_bar_enabled"].get(guild.premium_progress_bar_enabled); error) {
			return error;
		}

		if (error = obj["approximate_presence_count"].get(guild.approximate_presence_count); error) {
			return error;
		}

		if (error = obj["premium_subscription_count"].get(guild.premium_subscription_count); error) {
			return error;
		}

		if (error = obj["public_updates_channel_id"].get_string(guild.public_updates_channel_id); error) {
			return error;
		}

		if (error = obj["approximate_member_count"].get(guild.approximate_member_count); error) {
			return error;
		}

		if (!obj["safety_alerts_channel_id"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["max_video_channel_users"].get(guild.max_video_channel_users); error) {
			return error;
		}

		if (error = obj["explicit_content_filter"].get(guild.explicit_content_filter); error) {
			return error;
		}

		if (error = obj["guild_scheduled_events"].get(guild.guild_scheduled_events); error) {
			return error;
		}

		if (error = obj["system_channel_flags"].get(guild.system_channel_flags); error) {
			return error;
		}

		if (error = obj["verification_level"].get(guild.verification_level); error) {
			return error;
		}

		if (!obj["inventory_settings"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["widget_channel_id"].get_string(guild.widget_channel_id); error) {
			return error;
		}

		if (error = obj["system_channel_id"].get_string(guild.system_channel_id); error) {
			return error;
		}

		if (error = obj["rules_channel_id"].get_string(guild.rules_channel_id); error) {
			return error;
		}

		if (error = obj["preferred_locale"].get_string(guild.preferred_locale); error) {
			return error;
		}

		if (!obj["discovery_splash"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["vanity_url_code"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["widget_enabled"].get(guild.widget_enabled); error) {
			return error;
		}

		if (!obj["afk_channel_id"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["application_id"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["max_presences"].get(guild.max_presences); error) {
			return error;
		}

		if (error = obj["premium_tier"].get(guild.premium_tier); error) {
			return error;
		}

		if (error = obj["member_count"].get(guild.member_count); error) {
			return error;
		}

		if (error = obj["voice_states"].get(guild.voice_states); error) {
			return error;
		}

		if (error = obj["unavailable"].get(guild.unavailable); error) {
			return error;
		}

		if (error = obj["afk_timeout"].get(guild.afk_timeout); error) {
			return error;
		}

		if (error = obj["max_members"].get(guild.max_members); error) {
			return error;
		}

		if (error = obj["permissions"].get_string(guild.permissions); error) {
			return error;
		}

		if (!obj["description"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["nsfw_level"].get(guild.nsfw_level); error) {
			return error;
		}

		if (error = obj["mfa_level"].get(guild.mfa_level); error) {
			return error;
		}

		if (error = obj["joined_at"].get_string(guild.joined_at); error) {
			return error;
		}

		if (error = obj["discovery"].get_string(guild.discovery); error) {
			return error;
		}

		if (error = obj["owner_id"].get_string(guild.owner_id); error) {
			return error;
		}

		if (!obj["hub_type"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["stickers"].get(guild.stickers); error) {
			return error;
		}

		if (error = obj["features"].get(guild.features); error) {
			return error;
		}

		if (error = obj["channels"].get(guild.channels); error) {
			return error;
		}

		if (error = obj["members"].get(guild.members); error) {
			return error;
		}

		if (error = obj["threads"].get(guild.threads); error) {
			return error;
		}

		if (error = obj["region"].get_string(guild.region); error) {
			return error;
		}

		if (!obj["banner"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["splash"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["owner"].get(guild.owner); error) {
			return error;
		}

		if (error = obj["large"].get(guild.large); error) {
			return error;
		}

		if (error = obj["flags"].get(guild.flags); error) {
			return error;
		}

		if (error = obj["roles"].get(guild.roles); error) {
			return error;
		}

		if (error = obj["lazy"].get(guild.lazy); error) {
			return error;
		}

		if (error = obj["nsfw"].get(guild.nsfw); error) {
			return error;
		}

		if (error = obj["icon"].get_string(guild.icon); error) {
			return error;
		}

		if (error = obj["name"].get_string(guild.name); error) {
			return error;
		}

		if (error = obj["id"].get_string(guild.id); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, discord_message& msg) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["op"].get(msg.op); error) {
			return error;
		}

		if (error = obj["s"].get(msg.s); error) {
			return error;
		}

		if (error = obj["d"].get(msg.d); error) {
			return error;
		}

		if (error = obj["t"].get_string(msg.t); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, search_metadata_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["completed_in"].get(data.completed_in); error) {
			return error;
		}

		if (error = obj["max_id"].get(data.max_id); error) {
			return error;
		}

		if (error = obj["max_id_str"].get_string(data.max_id_str); error) {
			return error;
		}

		if (error = obj["next_results"].get_string(data.next_results); error) {
			return error;
		}

		if (error = obj["query"].get_string(data.query); error) {
			return error;
		}

		if (error = obj["refresh_url"].get_string(data.refresh_url); error) {
			return error;
		}

		if (error = obj["count"].get(data.count); error) {
			return error;
		}

		if (error = obj["since_id"].get(data.since_id); error) {
			return error;
		}

		if (error = obj["since_id_str"].get_string(data.since_id_str); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, hashtag_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["text"].get_string(data.text); error) {
			return error;
		}

		if (error = obj["indices"].get(data.indices); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, large_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["w"].get(data.w); error) {
			return error;
		}

		if (error = obj["h"].get(data.h); error) {
			return error;
		}

		if (error = obj["resize"].get_string(data.resize); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, sizes_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["medium"].get(data.medium); error) {
			return error;
		}

		if (error = obj["small"].get(data.small); error) {
			return error;
		}

		if (error = obj["thumb"].get(data.thumb); error) {
			return error;
		}

		if (error = obj["large"].get(data.large); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, media_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["id"].get(data.id); error) {
			return error;
		}

		if (error = obj["id_str"].get_string(data.id_str); error) {
			return error;
		}

		if (error = obj["indices"].get(data.indices); error) {
			return error;
		}

		if (error = obj["media_url"].get_string(data.media_url); error) {
			return error;
		}

		if (error = obj["media_url_https"].get_string(data.media_url_https); error) {
			return error;
		}

		if (error = obj["url"].get_string(data.url); error) {
			return error;
		}

		if (error = obj["display_url"].get_string(data.display_url); error) {
			return error;
		}

		if (error = obj["expanded_url"].get_string(data.expanded_url); error) {
			return error;
		}

		if (error = obj["type"].get_string(data.type); error) {
			return error;
		}

		if (error = obj["sizes"].get(data.sizes); error) {
			return error;
		}

		if (error = obj["source_status_id"].get(data.source_status_id); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["source_status_id_str"].get_string(data.source_status_id_str); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, url_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["url"].get_string(data.url); error) {
			return error;
		}

		if (error = obj["expanded_url"].get_string(data.expanded_url); error) {
			return error;
		}

		if (error = obj["display_url"].get_string(data.display_url); error) {
			return error;
		}

		if (error = obj["indices"].get(data.indices); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, user_mention& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["screen_name"].get_string(data.screen_name); error) {
			return error;
		}

		if (error = obj["name"].get_string(data.name); error) {
			return error;
		}

		if (error = obj["id"].get(data.id); error) {
			return error;
		}

		if (error = obj["id_str"].get_string(data.id_str); error) {
			return error;
		}

		if (error = obj["indices"].get(data.indices); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<media_data>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			media_data newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			media_data newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::optional<std::vector<media_data>>& car) {
		if (auto error = val.get(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, status_entities& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["hashtags"].get(data.hashtags); error) {
			return error;
		}

		if (error = obj["symbols"].get(data.symbols); error) {
			return error;
		}

		if (error = obj["urls"].get(data.urls); error) {
			return error;
		}

		if (error = obj["user_mentions"].get(data.user_mentions); error) {
			return error;
		}

		if (error = obj["media"].get(data.media); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, metadata_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["result_type"].get_string(data.result_type); error) {
			return error;
		}

		if (error = obj["iso_language_code"].get_string(data.iso_language_code); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, description_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["urls"].get(data.urls); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::optional<description_data>& car) {
		if (auto error = val.get(car.emplace()); !val.is_null() && error != simdjson::SUCCESS) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, user_entities& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["description"].get(data.description); error) {
			return error;
		}

		if (error = obj["url"].get(data.url); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, twitter_user_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["id"].get(data.id); error) {
			return error;
		}

		if (error = obj["id_str"].get_string(data.id_str); error) {
			return error;
		}

		if (error = obj["name"].get_string(data.name); error) {
			return error;
		}

		if (error = obj["screen_name"].get_string(data.screen_name); error) {
			return error;
		}

		if (error = obj["location"].get_string(data.location); error) {
			return error;
		}

		if (error = obj["description"].get_string(data.description); error) {
			return error;
		}

		if (error = obj["url"].get(data.url); error) {
			return error;
		}

		if (error = obj["entities"].get(data.entities); error) {
			return error;
		}

		if (error = obj["protected"].get(data.protectedVal); error) {
			return error;
		}

		if (error = obj["followers_count"].get(data.followers_count); error) {
			return error;
		}

		if (error = obj["friends_count"].get(data.friends_count); error) {
			return error;
		}

		if (error = obj["listed_count"].get(data.listed_count); error) {
			return error;
		}

		if (error = obj["created_at"].get_string(data.created_at); error) {
			return error;
		}

		if (error = obj["favourites_count"].get(data.favourites_count); error) {
			return error;
		}

		if (error = obj["utc_offset"].get(data.utc_offset); error) {
			return error;
		}

		if (error = obj["time_zone"].get(data.time_zone); error) {
			return error;
		}

		if (error = obj["geo_enabled"].get(data.geo_enabled); error) {
			return error;
		}

		if (error = obj["verified"].get(data.verified); error) {
			return error;
		}

		if (error = obj["statuses_count"].get(data.statuses_count); error) {
			return error;
		}

		if (error = obj["lang"].get_string(data.lang); error) {
			return error;
		}

		if (error = obj["contributors_enabled"].get(data.contributors_enabled); error) {
			return error;
		}

		if (error = obj["is_translator"].get(data.is_translator); error) {
			return error;
		}

		if (error = obj["is_translation_enabled"].get(data.is_translation_enabled); error) {
			return error;
		}

		if (error = obj["profile_background_color"].get_string(data.profile_background_color); error) {
			return error;
		}

		if (error = obj["profile_background_image_url"].get_string(data.profile_background_image_url); error) {
			return error;
		}

		if (error = obj["profile_background_image_url_https"].get_string(data.profile_background_image_url_https); error) {
			return error;
		}

		if (error = obj["profile_background_tile"].get(data.profile_background_tile); error) {
			return error;
		}

		if (error = obj["profile_image_url"].get_string(data.profile_image_url); error) {
			return error;
		}

		if (error = obj["profile_image_url_https"].get_string(data.profile_image_url_https); error) {
			return error;
		}

		if (error = obj["profile_banner_url"].get(data.profile_banner_url); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["profile_link_color"].get_string(data.profile_link_color); error) {
			return error;
		}

		if (error = obj["profile_sidebar_border_color"].get_string(data.profile_sidebar_border_color); error) {
			return error;
		}

		if (error = obj["profile_sidebar_fill_color"].get_string(data.profile_sidebar_fill_color); error) {
			return error;
		}

		if (error = obj["profile_text_color"].get_string(data.profile_text_color); error) {
			return error;
		}

		if (error = obj["profile_use_background_image"].get(data.profile_use_background_image); error) {
			return error;
		}

		if (error = obj["default_profile"].get(data.default_profile); error) {
			return error;
		}

		if (error = obj["default_profile_image"].get(data.default_profile_image); error) {
			return error;
		}

		if (error = obj["following"].get(data.following); error) {
			return error;
		}

		if (error = obj["follow_request_sent"].get(data.follow_request_sent); error) {
			return error;
		}

		if (error = obj["notifications"].get(data.notifications); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, status_data& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["metadata"].get(data.metadata); error) {
			return error;
		}

		if (error = obj["created_at"].get_string(data.created_at); error) {
			return error;
		}

		if (error = obj["id"].get(data.id); error) {
			return error;
		}

		if (error = obj["id_str"].get_string(data.id_str); error) {
			return error;
		}

		if (error = obj["text"].get_string(data.text); error) {
			return error;
		}

		if (error = obj["source"].get_string(data.source); error) {
			return error;
		}

		if (error = obj["truncated"].get(data.truncated); error) {
			return error;
		}

		if (error = obj["in_reply_to_status_id"].get(data.in_reply_to_status_id); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["in_reply_to_status_id_str"].get(data.in_reply_to_status_id_str); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["in_reply_to_user_id"].get(data.in_reply_to_user_id); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["in_reply_to_user_id_str"].get(data.in_reply_to_user_id_str); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["in_reply_to_screen_name"].get(data.in_reply_to_screen_name); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["user"].get(data.user); error) {
			return error;
		}

		if (!obj["geo"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["coordinates"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["place"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (!obj["contributors"].is_null()) {
			return simdjson::UNEXPECTED_ERROR;
		}

		if (error = obj["retweet_count"].get(data.retweet_count); error) {
			return error;
		}

		if (error = obj["favorite_count"].get(data.favorite_count); error) {
			return error;
		}

		if (error = obj["entities"].get(data.entities); error) {
			return error;
		}

		if (error = obj["favorited"].get(data.favorited); error) {
			return error;
		}

		if (error = obj["retweeted"].get(data.retweeted); error) {
			return error;
		}

		if (error = obj["lang"].get_string(data.lang); error) {
			return error;
		}

		if (error = obj["retweeted_status"].get(data.retweeted_status); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		if (error = obj["possibly_sensitive"].get(data.possibly_sensitive); (error != simdjson::NO_SUCH_FIELD) && (error != simdjson::SUCCESS)) {
			return error;
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, std::vector<status_data>& vector) {
		ondemand::array arr;
		auto error = val.get_array().get(arr);
		if (error) {
			return error;
		}
		size_t currentIndex{};
		auto iter = arr.begin();
		for (; iter != arr.end() && currentIndex < vector.size(); ++currentIndex, ++iter) {
			status_data newVal;
			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector[currentIndex] = std::move(newVal);
		}

		for (; iter != arr.end(); ++iter) {
			status_data newVal;

			if (error = iter.operator*().get(newVal); error) {
				return error;
			}
			vector.emplace_back(std::move(newVal));
		}

		return simdjson::SUCCESS;
	}

	template<typename simdjson_value> auto tag_invoke(deserialize_tag, simdjson_value& val, twitter_message& data) {
		ondemand::object obj;
		auto error = val.get_object().get(obj);
		if (error) {
			return error;
		}

		if (error = obj["statuses"].get(data.statuses); error) {
			return error;
		}

		if (error = obj["search_metadata"].get(data.search_metadata); error) {
			return error;
		}

		return simdjson::SUCCESS;
	}

}

#endif