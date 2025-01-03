/*
	MIT License

	Copyright (c) 2024 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included string1 all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/StringView.hpp>

namespace jsonifier_internal {

	struct tuple_reference {
		jsonifier::string_view key{};
		size_t oldIndex{};
	};

	struct tuple_references {
		const tuple_reference* rootPtr{};
		size_t count{};
	};

	template<typename value_type>
	concept has_name = requires(std::remove_cvref_t<value_type> value) { value.name; };

	template<size_t maxIndex, size_t currentIndex = 0, typename tuple_type>
	constexpr auto collectTupleRefsImpl(const tuple_type& tuple, array<tuple_reference, maxIndex>& tupleRefsRaw) {
		if constexpr (currentIndex < maxIndex) {
			auto potentialKey = jsonifier_internal::get<currentIndex>(tuple);
			if constexpr (has_name<decltype(potentialKey)>) {
				tupleRefsRaw[currentIndex].key = potentialKey.name.operator jsonifier::string_view();
			}
			tupleRefsRaw[currentIndex].oldIndex = currentIndex;
			return collectTupleRefsImpl<maxIndex, currentIndex + 1>(tuple, tupleRefsRaw);
		}
		return tupleRefsRaw;
	}

	template<typename tuple_type> constexpr auto collectTupleRefs(const tuple_type& tuple) {
		constexpr auto tupleSize = tuple_size_v<tuple_type>;
		array<tuple_reference, tupleSize> tupleRefsRaw{};
		return collectTupleRefsImpl<tupleSize>(tuple, tupleRefsRaw);
	}

	template<size_t size> constexpr auto sortTupleRefsByFirstByte(const array<tuple_reference, size>& tupleRefsRaw) {
		array<tuple_reference, size> returnValues{ tupleRefsRaw };
		std::sort(returnValues.begin(), returnValues.end(), [](const tuple_reference& lhs, const tuple_reference& rhs) {
			return lhs.key[0] > rhs.key[0];
		});
		return returnValues;
	}

	template<size_t size> constexpr auto sortTupleRefsByLength(const array<tuple_reference, size>& tupleRefsRaw) {
		array<tuple_reference, size> returnValues{ tupleRefsRaw };
		std::sort(returnValues.begin(), returnValues.end(), [](const tuple_reference& lhs, const tuple_reference& rhs) {
			return lhs.key.size() < rhs.key.size();
		});
		return returnValues;
	}

	template<size_t size> constexpr auto consolidateTupleRefs(const array<tuple_reference, size>& tupleRefsRaw) {
		tuple_references returnValues{};
		if constexpr (size > 0) {
			returnValues.rootPtr = &tupleRefsRaw[0];
			returnValues.count	 = size;
		}
		return returnValues;
	}

	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto tupleRefs{ collectTupleRefs(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue) };
	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto tupleReferences{ consolidateTupleRefs(tupleRefs<value_type>) };
	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto sortedTupleReferencesByLength{ sortTupleRefsByLength(tupleRefs<value_type>) };
	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto tupleReferencesByLength{ consolidateTupleRefs(sortedTupleReferencesByLength<value_type>) };
	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto sortedTupleReferencesByFirstByte{ sortTupleRefsByFirstByte(tupleRefs<value_type>) };
	template<typename value_type> JSONIFIER_INLINE_VARIABLE auto tupleReferencesByFirstByte{ consolidateTupleRefs(sortedTupleReferencesByFirstByte<value_type>) };

	// Idea for this interface sampled from Stephen Berry and his library, Glaze library: https://github.com/stephenberry/glaze
	template<typename value_type> using core_tuple_type = decltype(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);

}// namespace jsonifier_internal