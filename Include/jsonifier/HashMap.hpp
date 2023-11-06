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
/// Feb 20, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/CTimeArray.hpp>
#include <jsonifier/CTimeVector.hpp>
#include <jsonifier/Expected.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Base.hpp>
#include <algorithm>
#include <span>

namespace jsonifier_internal {

	template<typename value_type> constexpr uint64_t toUint64(const value_type* bytes, const uint64_t n) {
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (uint64_t x = 0; x < n; ++x) {
				res |= static_cast<uint64_t>(bytes[x]) << (8 * x);
			}
			return res;
		} else {
			uint64_t res{};
			std::memcpy(&res, bytes, n);
			return res;
		}
	}

	struct string_compare_helper {
		template<typename T0, typename T1> constexpr bool operator()(T0&& lhs, T1&& rhs) const {
			if (std::is_constant_evaluated()) {
				return stringConstCompare(std::forward<T0>(lhs), std::forward<T1>(rhs));
			} else {
				if (lhs.size() == rhs.size()) {
					return jsonifier_core_internal::compare(lhs.data(), rhs.data(), rhs.size());
				}
				return false;
			}
		}
	};

	constexpr auto nextHighestPowerOfTwo(uint64_t v) {
		// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		constexpr auto tripCount = std::numeric_limits<decltype(v)>::digits;
		v--;
		for (uint64_t x = 1; x < tripCount; x <<= 1)
			v |= v >> x;
		++v;
		return v;
	}

	template<typename value_type> constexpr auto log(value_type v) {
		uint64_t n = 0;
		while (v > 1) {
			n += 1;
			v >>= 1;
		}
		return n;
	}

	constexpr uint64_t bitWeight(uint64_t n) {
		return static_cast<uint64_t>(n <= 8ULL) * sizeof(uint32_t) + (static_cast<uint64_t>(n <= 8ULL) * sizeof(uint32_t)) + (static_cast<uint64_t>(n <= 8ULL) * sizeof(uint64_t)) +
			(n <= 128ULL);
	}

	template<uint64_t n> inline uint64_t selectUintLeast(std::integral_constant<uint64_t, n>) {
		static_assert(n < 2, "Unsupported type size.");
		return {};
	}

	template<uint64_t n> using SelectUintLeastT = decltype(selectUintLeast(std::integral_constant<uint64_t, bitWeight(n)>()));

	template<typename value_type> constexpr void primitiveSwap(value_type&& a, value_type&& b) {
		auto tmp = std::move(a);
		a		 = std::move(b);
		b		 = std::move(tmp);
	}

	template<typename value_type, typename u> constexpr void primitiveSwap(pair<value_type, u>&& a, pair<value_type, u>&& b) {
		primitiveSwap(std::forward<pair<value_type, u>>(a.first), std::forward<pair<value_type, u>>(b.first));
		primitiveSwap(std::forward<pair<value_type, u>>(a.second), std::forward<pair<value_type, u>>(b.second));
	}

	template<typename... Tys, uint64_t... Is> constexpr void primitiveSwap(std::tuple<Tys...>&& a, std::tuple<Tys...>&& b, std::index_sequence<Is...>) {
		using swallow = int32_t[];
		( void )swallow{ (primitiveSwap(std::forward<Tys...>(get<Is>(a)), std::forward<Tys...>(get<Is>(b))), 0)... };
	}

	template<typename... Tys> constexpr void primitiveSwap(std::tuple<Tys...>& a, std::tuple<Tys...>& b) {
		primitiveSwap(a, b, std::make_index_sequence<sizeof...(Tys)>());
	}

	template<typename iterator, typename compare> constexpr iterator partition(iterator&& left, iterator&& right, const compare& compareNew) {
		auto pivot = left + (right - left) / 2;
		auto value = *pivot;
		primitiveSwap(*right, *pivot);
		for (auto it = left; 0 < right - it; ++it) {
			if (compareNew(*it, value)) {
				primitiveSwap(*it, *left);
				++left;
			}
		}
		primitiveSwap(*right, *left);
		return left;
	}

	template<typename iterator, typename compare> constexpr void quicksort(iterator&& left, iterator&& right, const compare& compareNew) {
		while (0 < right - left) {
			auto newPivot = jsonifier_internal::partition(left, right, compareNew);
			quicksort(left, newPivot, compareNew);
			left = newPivot + 1;
		}
	}

	template<typename value_type, uint64_t n, typename compare> constexpr ctime_array<value_type, n> quicksort(ctime_array<value_type, n> const& array, const compare& compareNew) {
		ctime_array<value_type, n> res = array;
		quicksort(res.begin(), res.end() - 1, compareNew);
		return res;
	}

	template<typename uint_type, uint_type multiplier, uint_type increment, uint_type modulus> class linear_congruential_engine {
		static_assert(std::is_unsigned<uint_type>::value, "uint_type must be an unsigned integral type.");
		template<typename value_type> static constexpr uint_type modulo(value_type value, std::integral_constant<uint_type, 0>) {
			return static_cast<uint_type>(value);
		}
		template<typename value_type, uint_type m2> static constexpr uint_type modulo(value_type value, std::integral_constant<uint_type, m2>) {
			return static_cast<uint_type>(value % m2);
		}

	  public:
		using result_type						  = uint_type;
		static constexpr result_type default_seed = 1u;

		constexpr linear_congruential_engine() = default;

		constexpr linear_congruential_engine(result_type S) {
			seed(S);
		}

		constexpr void seed(result_type S = default_seed) const {
			state_ = S;
		}

		constexpr result_type operator()() {
			using uint_least_t = SelectUintLeastT<log(multiplier) + log(modulus) + 4>;
			uint_least_t tmp   = static_cast<uint_least_t>(multiplier) * state_ + increment;

			state_ = modulo(tmp, std::integral_constant<uint_type, modulus>());
			return state_;
		}

		constexpr void discard(uint64_t n) const {
			while (n--)
				operator()();
		}

		constexpr bool operator==(linear_congruential_engine const& other) const {
			return state_ == other.state_;
		}

	  protected:
		result_type state_ = default_seed;
	};

	using default_prg_t = linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647>;

	// from
	// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-std::tuple-c
	template<typename value_type, typename... value_types> struct unique {
		using type = value_type;
	};

	template<template<typename...> class value_type, typename... value_types, typename u, typename... Us> struct unique<value_type<value_types...>, u, Us...>
		: std::conditional_t<(std::same_as<u, value_types> || ...), unique<value_type<value_types...>, Us...>, unique<value_type<value_types..., u>, Us...>> {};

	template<typename value_type> struct tuple_variant;

	template<typename... value_types> struct tuple_variant<std::tuple<value_types...>> : unique<std::variant<>, value_types...> {};

	template<typename value_type> struct tuple_ptr_variant;

	template<typename... value_types> struct tuple_ptr_variant<std::tuple<value_types...>> : unique<std::variant<>, std::add_pointer_t<value_types>...> {};

	template<typename... value_types> struct tuple_ptr_variant<pair<value_types...>> : unique<std::variant<>, std::add_pointer_t<value_types>...> {};

	template<typename tuple_t, typename = std::make_index_sequence<std::tuple_size<tuple_t>::value>> struct value_tuple_variant;

	template<typename tuple_t, size_t... I> struct value_tuple_variant<tuple_t, std::index_sequence<I...>> {
		using type = typename tuple_variant<decltype(std::tuple_cat(std::declval<std::tuple<std::tuple_element_t<1, std::tuple_element_t<I, tuple_t>>>>()...))>::type;
	};

	template<typename tuple_t> using value_tuple_variant_t = typename value_tuple_variant<tuple_t>::type;

	struct bucket_size_compare {
		template<typename b> bool constexpr operator()(b const& b0, b const& b1) const {
			return b0.size() > b1.size();
		}
	};

	template<uint64_t m> struct pmh_buckets {
		static constexpr auto bucket_max = 2 * (1u << (log(m) / 2));

		using bucket_t = ctime_vector<uint64_t, bucket_max>;
		ctime_array<bucket_t, m> buckets;
		uint64_t seed;

		struct bucket_ref {
			unsigned hash;
			const bucket_t* values;
			using value_type	 = typename bucket_t::value_type;
			using const_iterator = typename bucket_t::const_iterator;

			constexpr auto size() const {
				return values->size();
			}

			constexpr const auto& operator[](uint64_t idx) const {
				return (*values)[idx];
			}

			constexpr auto begin() const {
				return values->begin();
			}

			constexpr auto end() const {
				return values->end();
			}
		};

		template<size_t... Is> ctime_array<bucket_ref, m> constexpr makeBucketRefs(std::index_sequence<Is...>) const {
			return { { bucket_ref{ Is, &buckets[Is] }... } };
		}

		ctime_array<bucket_ref, m> constexpr getSortedBuckets() const {
			ctime_array<bucket_ref, m> result{ makeBucketRefs(std::make_index_sequence<m>()) };
			quicksort(result.begin(), result.end() - 1, bucket_size_compare{});
			return result;
		}
	};

	struct get_key {
		template<typename KV> constexpr auto const& operator()(KV const& kv) const {
			return kv.first;
		}
	};

	template<size_t m, typename Item, size_t n> pmh_buckets<m> constexpr makePmhBuckets(const ctime_array<Item, n>& items) {
		get_key keyNew{};
		default_prg_t prg{};
		using result_t = pmh_buckets<m>;
		using hasher   = hash<jsonifier::string_view>;
		result_t result{};
		bool rejected = false;
		while (true) {
			for (auto& b: result.buckets) {
				b.clear();
			}
			result.seed = prg();
			rejected	= false;
			for (uint64_t x = 0; x < n; ++x) {
				auto& bucket = result.buckets[hasher{}(keyNew(items[x]), static_cast<uint64_t>(result.seed)) % m];
				if (bucket.size() >= result_t::bucket_max) [[unlikely]] {
					rejected = true;
					break;
				}
				bucket.pushBack(x);
			}
			if (!rejected) [[likely]] {
				return result;
			}
		}
	}

	template<typename value_type, uint64_t n> constexpr bool allDifferentFrom(ctime_vector<value_type, n>& data, value_type& a) {
		for (uint64_t x = 0; x < data.size(); ++x)
			if (data[x] == a) [[unlikely]] {
				return false;
			}
		return true;
	}

	struct seed_or_index {
		using value_type = uint64_t;

	  protected:
		static constexpr value_type MinusOne = std::numeric_limits<value_type>::max();
		static constexpr value_type HighBit	 = ~(MinusOne >> 1);

		value_type val{ 0 };

	  public:
		constexpr seed_or_index(const value_type& valueNew) {
			val = valueNew;
		}

		constexpr value_type value() const {
			return val;
		}

		constexpr bool isSeed() const {
			return val & HighBit;
		}

		constexpr seed_or_index(bool isSeed, value_type value) : val(isSeed ? (value | HighBit) : (value & ~HighBit)) {
		}

		constexpr seed_or_index() = default;
	};

	template<uint64_t m> struct pmh_tables {
		constexpr pmh_tables() = default;
		constexpr pmh_tables(uint64_t firstSeedNew, ctime_array<seed_or_index, m> firstTableNew, ctime_array<uint64_t, m> secondTableNew) {
			firstSeed	= firstSeedNew;
			firstTable	= firstTableNew;
			secondTable = secondTableNew;
		}
		uint64_t firstSeed;
		ctime_array<seed_or_index, m> firstTable;
		ctime_array<uint64_t, m> secondTable;

		template<typename KeyType> using hasher = hash<KeyType>;

		template<typename value_type> constexpr const hasher<value_type> getHasher() const {
			return hasher<value_type>{};
		}

		template<typename KeyType> constexpr uint64_t lookup(const KeyType& key) const {
			auto const d = firstTable[getHasher<KeyType>()(key, static_cast<uint64_t>(firstSeed)) % m];
			if (!d.isSeed()) [[unlikely]] {
				return static_cast<uint64_t>(d.value());
			} else [[likely]] {
				return secondTable[getHasher<KeyType>()(key, static_cast<uint64_t>(d.value())) % m];
			}
		}
	};

	template<size_t m, typename Item, size_t n> pmh_tables<m> constexpr makePmhTables(const ctime_array<Item, n>& items) {
		get_key keyNew{};
		default_prg_t prg{};
		auto step_one = makePmhBuckets<m>(items);
		auto buckets  = step_one.getSortedBuckets();

		ctime_array<seed_or_index, m> G;
		using hasher = hash<jsonifier::string_view_base<uint8_t>>;

		constexpr uint64_t UNUSED = std::numeric_limits<uint64_t>::max();
		ctime_array<uint64_t, m> H;
		H.fill(UNUSED);

		for (const auto& bucket: buckets) {
			auto const bsize = bucket.size();

			if (bsize == 1) {
				G[bucket.hash] = { false, static_cast<uint64_t>(bucket[0]) };
			} else if (bsize > 1) {
				seed_or_index d{ true, prg() };
				ctime_vector<uint64_t, decltype(step_one)::bucket_max> bucket_slots;

				while (bucket_slots.size() < bsize) {
					auto slot = hasher{}(keyNew(items[bucket[bucket_slots.size()]]), d.value()) % m;

					if (H[slot] != UNUSED || !allDifferentFrom(bucket_slots, slot)) {
						bucket_slots.clear();
						d = { true, prg() };
						continue;
					}

					bucket_slots.pushBack(slot);
				}

				G[bucket.hash] = d;
				for (uint64_t x = 0; x < bsize; ++x)
					H[bucket_slots[x]] = bucket[x];
			}
		}

		for (uint64_t x = 0; x < m; ++x)
			if (H[x] == UNUSED)
				H[x] = 0;

		return { step_one.seed, G, H };
	}

	template<typename key, typename Value, size_t n>
	class unordered_map : protected string_compare_helper, protected ctime_array<pair<key, Value>, n>, protected pmh_tables<nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1)> {
	  public:
		static constexpr auto storageSize = nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1);
		using container_type			  = ctime_array<pair<key, Value>, n>;
		using tables_type				  = pmh_tables<storageSize>;

		using key_type		  = key;
		using mapped_type	  = Value;
		using value_type	  = typename container_type::value_type;
		using size_type		  = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;
		using key_equal		  = string_compare_helper;
		using reference		  = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using pointer		  = typename container_type::pointer;
		using const_pointer	  = typename container_type::const_pointer;
		using iterator		  = typename container_type::iterator;
		using const_iterator  = typename container_type::const_iterator;

		constexpr unordered_map() = default;

		constexpr unordered_map(container_type items) : container_type{ items }, tables_type{ makePmhTables<storageSize>(items) } {
		}

		constexpr const_iterator begin() const {
			return container_type::begin();
		}

		constexpr const_iterator end() const {
			return container_type::end();
		}

		template<typename key_type_new> constexpr const_iterator find(const key_type_new& keyNew) const {
			auto& kv = lookup(keyNew);
			if (keyEq()(kv.first, keyNew)) {
				return const_iterator{ &kv };
			} else {
				return end();
			}
		}

	  protected:
		template<typename key_type_new> constexpr const auto& lookup(const key_type_new& keyNew) const {
			return container_type::operator[](tables_type::lookup(keyNew));
		}

		constexpr const key_equal& keyEq() const {
			return *this;
		}
	};

	template<typename value_type, typename u, uint64_t n> constexpr auto makeUnorderedMap(pair<value_type, u> const (&items)[n]) {
		return unordered_map<value_type, u, n>{ items };
	}

	template<typename value_type, uint64_t I> struct core_sv {
		static constexpr jsonifier::string_view value = get<0>(get<I>(jsonifier::concepts::coreV<value_type>));
	};

	template<typename value_type, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t		= value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto n	= std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		constexpr auto size = sizeof...(I);
		static_assert(size == n);
		return unordered_map<jsonifier::string_view, value_t, n>({ pair<jsonifier::string_view, value_t>(
			jsonifier::string_view(get<0>(get<I>(jsonifier::concepts::coreV<value_type>))), get<1>(get<I>(jsonifier::concepts::coreV<value_type>)))... });
	}

	template<typename value_type> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<jsonifier::concepts::unwrap<value_type>>(indices);
	}
}