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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Parser.hpp>

#include <memory>

namespace jsonifier_internal {

	template<const jsonifier::parse_options& options, bool minified> struct index_processor;

	template<const jsonifier::parse_options& options> struct index_processor<options, false> {
		template<size_t index, typename value_type, typename parse_context_type>
		JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, parse_context_type& context) noexcept {
			if constexpr (index < std::tuple_size_v<core_tuple_t<value_type>>) {
				static constexpr auto ptr			= std::get<index>(coreTupleV<value_type>).ptr();
				static constexpr auto key			= std::get<index>(coreTupleV<value_type>).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if (comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(), context.iter)) [[likely]] {
					context.iter += keySizeNew;
					JSONIFIER_SKIP_WS();
					if (*context.iter == ':') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<decltype(value.*ptr)>;
						parse<false, options>::impl(value.*ptr, context);
						return true;
					} else [[unlikely]] {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
						derailleur<options>::skipToNextValue(context);
					}
				}
			}
			return false;
		}
	};

	template<const jsonifier::parse_options& options> struct index_processor<options, true> {
		template<size_t index, typename value_type, typename parse_context_type>
		JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, parse_context_type& context) noexcept {
			if constexpr (index < std::tuple_size_v<core_tuple_t<value_type>>) {
				static constexpr auto ptr			= std::get<index>(coreTupleV<value_type>).ptr();
				static constexpr auto key			= std::get<index>(coreTupleV<value_type>).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if (comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(), context.iter)) [[likely]] {
					context.iter += keySizeNew;
					if (*context.iter == ':') [[likely]] {
						++context.iter;
						using member_type = unwrap_t<decltype(value.*ptr)>;
						parse<true, options>::impl(value.*ptr, context);
						return true;
					} else [[unlikely]] {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
						derailleur<options>::skipToNextValue(context);
					}
				}
			}
			return false;
		}
	};

	template<bool minified, const jsonifier::parse_options& options, typename value_type, typename parse_context_type, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&index_processor<options, minified>::template processIndex<0, value_type, parse_context_type>);
		return std::array<function_type, sizeof...(indices)>{ &index_processor<options, minified>::template processIndex<indices, value_type, parse_context_type>... };
	}

	template<bool minified, const jsonifier::parse_options& options, typename value_type, typename parse_context_type>
	JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrs() {
		constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<value_type>>;
		return generateFunctionPtrsImpl<minified, options, value_type, parse_context_type>(std::make_index_sequence<tupleSize>{});
	}

	template<const jsonifier::parse_options& options, jsonifier::concepts::jsonifier_value_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			static constexpr auto memberCount = std::tuple_size_v<core_tuple_t<value_type>>;

			if (*context.iter == '{') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != '}') [[likely]] {
					if constexpr (memberCount > 0) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ size_t(context.iter - wsStart) };
						bool antihash{ true };

						if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
							const auto keySize = getKeyLength<options, value_type>(context);
							jsonifier::string_view key{ static_cast<const char*>(context.iter) + 1, keySize };
							auto& keys = value.jsonifierExcludedKeys;
							if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.context.endIter()) [[likely]] {
								derailleur<options>::skipToNextValue(context);
							}
						}

						if (*context.iter == '"') [[likely]] {
							++context.iter;
							static constexpr auto ptr			= std::get<0>(jsonifier::concepts::coreV<value_type>).ptr();
							static constexpr auto key			= std::get<0>(jsonifier::concepts::coreV<value_type>).view();
							static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
							static constexpr auto keySize		= key.size();
							static constexpr auto keySizeNew	= keySize + 1;
							if (*(context.iter + key.size()) == '"' &&
								comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(), context.iter))
								[[likely]] {
								context.iter += keySizeNew;
								JSONIFIER_SKIP_WS();
								if (*context.iter == ':') [[likely]] {
									++context.iter;
									JSONIFIER_SKIP_WS();
									using member_type = unwrap_t<decltype(value.*ptr)>;
									parse<false, options>::impl(value.*ptr, context);
								} else [[unlikely]] {
									static constexpr auto sourceLocation{ std::source_location::current() };
									context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
									derailleur<options>::skipToNextValue(context);
								}
							} else [[unlikely]] {
								antihash				= false;
								static constexpr auto N = std::tuple_size_v<core_tuple_t<value_type>>;
								if (auto index = hash_map<value_type, unwrap_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter); index < N) [[likely]] {
									static constexpr auto arrayOfPtrs = generateFunctionPtrs<false, options, value_type, parse_context_type>();
									if (arrayOfPtrs[index](value, context)) [[likely]] {
									} else [[unlikely]] {
										derailleur<options>::skipToNextValue(context);
									}
								} else {
									derailleur<options>::skipKey(context);
									++context.iter;
									derailleur<options>::skipToNextValue(context);
								}
							}
						} else [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
							derailleur<options>::skipToNextValue(context);
						}
						static constexpr auto parseLambda = [](const auto index, const auto newLines, const auto antiHashNew, auto&& parseLambda, value_type& value,
																parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
							if constexpr (index < memberCount) {
								if (*context.iter != '}') [[likely]] {
									if (*context.iter == ',') [[likely]] {
										++context.iter;
										JSONIFIER_SKIP_MATCHING_WS();
									} else [[unlikely]] {
										static constexpr auto sourceLocation{ std::source_location::current() };
										context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
										derailleur<options>::skipToNextValue(context);
										return false;
									}

									if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
										const auto keySize = getKeyLength<options, value_type>(context);
										jsonifier::string_view key{ static_cast<const char*>(context.iter) + 1, keySize };
										auto& keys = value.jsonifierExcludedKeys;
										if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.context.endIter()) [[likely]] {
											derailleur<options>::skipToNextValue(context);
											return false;
										}
									}

									if (*context.iter == '"') [[likely]] {
										++context.iter;
										if constexpr (antiHashNew) {
											static constexpr auto ptr			= std::get<index>(jsonifier::concepts::coreV<value_type>).ptr();
											static constexpr auto key			= std::get<index>(jsonifier::concepts::coreV<value_type>).view();
											static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
											static constexpr auto keySize		= key.size();
											static constexpr auto keySizeNew	= keySize + 1;
											if (*(context.iter + key.size()) == '"' &&
												comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(),
													context.iter)) [[likely]] {
												context.iter += keySizeNew;
												JSONIFIER_SKIP_WS();
												if (*context.iter == ':') [[likely]] {
													++context.iter;
													JSONIFIER_SKIP_WS();
													using member_type = unwrap_t<decltype(value.*ptr)>;
													parse<false, options>::impl(value.*ptr, context);
													return parseLambda(std::integral_constant<size_t, index + 1>{}, std::integral_constant<bool, newLines>{},
														std::integral_constant<bool, true>{}, parseLambda, value, context, wsStart, wsSize);
												} else [[unlikely]] {
													static constexpr auto sourceLocation{ std::source_location::current() };
													context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
													derailleur<options>::skipToNextValue(context);
													return false;
												}
											}
										}
										static constexpr auto N = std::tuple_size_v<core_tuple_t<value_type>>;
										if (auto indexNew = hash_map<value_type, unwrap_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter); indexNew < N)
											[[likely]] {
											static constexpr auto arrayOfPtrs = generateFunctionPtrs<false, options, value_type, parse_context_type>();
											if (arrayOfPtrs[indexNew](value, context)) [[likely]] {
												return parseLambda(std::integral_constant<size_t, index + 1>{}, std::integral_constant<bool, newLines>{},
													std::integral_constant<bool, false>{}, parseLambda, value, context, wsStart, wsSize);
											} else [[unlikely]] {
												derailleur<options>::skipToNextValue(context);
												return false;
											}
										} else {
											derailleur<options>::skipKey(context);
											++context.iter;
											derailleur<options>::skipToNextValue(context);
											return false;
										}
									} else [[unlikely]] {
										static constexpr auto sourceLocation{ std::source_location::current() };
										context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
										derailleur<options>::skipToNextValue(context);
										return false;
									}
								}
								return true;
							}
							return true;
						};
						if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
							if (antihash) {
								parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, std::integral_constant<bool, true>{}, parseLambda, value,
									context, wsStart, wsSize);
							} else {
								parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, std::integral_constant<bool, false>{}, parseLambda, value,
									context, wsStart, wsSize);
							}
						} else {
							if (antihash) {
								parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, std::integral_constant<bool, false>{}, parseLambda, value,
									context, wsStart, wsSize);
							} else {
								parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, false>{}, std::integral_constant<bool, false>{}, parseLambda, value,
									context, wsStart, wsSize);
							}
						}
					}
				}
				++context.iter;
				--context.currentObjectDepth;
				JSONIFIER_SKIP_WS();
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Object_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::jsonifier_value_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			static constexpr auto memberCount = std::tuple_size_v<core_tuple_t<value_type>>;

			if (*context.iter == '{') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != '}') [[likely]] {
					if constexpr (memberCount > 0) {
						bool antihash{ true };

						if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
							const auto keySize = getKeyLength<options, value_type>(context);
							jsonifier::string_view key{ static_cast<const char*>(context.iter) + 1, keySize };
							auto& keys = value.jsonifierExcludedKeys;
							if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.context.endIter()) [[likely]] {
								derailleur<options>::skipToNextValue(context);
							}
						}

						if (*context.iter == '"') [[likely]] {
							++context.iter;
							static constexpr auto ptr			= std::get<0>(jsonifier::concepts::coreV<value_type>).ptr();
							static constexpr auto key			= std::get<0>(jsonifier::concepts::coreV<value_type>).view();
							static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
							static constexpr auto keySize		= key.size();
							static constexpr auto keySizeNew	= keySize + 1;
							if (*(context.iter + key.size()) == '"' &&
								comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(), context.iter))
								[[likely]] {
								context.iter += keySizeNew;
								if (*context.iter == ':') [[likely]] {
									++context.iter;
									using member_type = unwrap_t<decltype(value.*ptr)>;
									parse<true, options>::impl(value.*ptr, context);
								} else [[unlikely]] {
									static constexpr auto sourceLocation{ std::source_location::current() };
									context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
									derailleur<options>::skipToNextValue(context);
								}
							} else [[unlikely]] {
								antihash				= false;
								static constexpr auto N = std::tuple_size_v<core_tuple_t<value_type>>;
								if (auto index = hash_map<value_type, unwrap_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter); index < N) [[likely]] {
									static constexpr auto arrayOfPtrs = generateFunctionPtrs<true, options, value_type, parse_context_type>();
									if (arrayOfPtrs[index](value, context)) [[likely]] {
									} else [[unlikely]] {
										derailleur<options>::skipToNextValue(context);
									}
								} else {
									derailleur<options>::skipKey(context);
									++context.iter;
									derailleur<options>::skipToNextValue(context);
								}
							}
						} else [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
							derailleur<options>::skipToNextValue(context);
						}

						static constexpr auto parseLambda = [](const auto index, const auto antiHashNew, auto&& parseLambda, value_type& value, parse_context_type& context) {
							if constexpr (index < memberCount) {
								if (*context.iter != '}') [[likely]] {
									if (*context.iter == ',') [[likely]] {
										++context.iter;
									} else [[unlikely]] {
										static constexpr auto sourceLocation{ std::source_location::current() };
										context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
										derailleur<options>::skipToNextValue(context);
										return false;
									}

									if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
										const auto keySize = getKeyLength<options, value_type>(context);
										jsonifier::string_view key{ static_cast<const char*>(context.iter) + 1, keySize };
										auto& keys = value.jsonifierExcludedKeys;
										if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.context.endIter()) [[likely]] {
											derailleur<options>::skipToNextValue(context);
											return false;
										}
									}

									if (*context.iter == '"') [[likely]] {
										++context.iter;
										if constexpr (antiHashNew) {
											static constexpr auto ptr			= std::get<index>(jsonifier::concepts::coreV<value_type>).ptr();
											static constexpr auto key			= std::get<index>(jsonifier::concepts::coreV<value_type>).view();
											static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
											static constexpr auto keySize		= key.size();
											static constexpr auto keySizeNew	= keySize + 1;
											if (*(context.iter + key.size()) == '"' &&
												comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.iter)>>::compare(stringLiteral.data(),
													context.iter)) [[likely]] {
												context.iter += keySizeNew;
												if (*context.iter == ':') [[likely]] {
													++context.iter;
													using member_type = unwrap_t<decltype(value.*ptr)>;
													parse<true, options>::impl(value.*ptr, context);
													return parseLambda(std::integral_constant<size_t, index + 1>{}, std::integral_constant<bool, true>{}, parseLambda, value,
														context);
												} else [[unlikely]] {
													static constexpr auto sourceLocation{ std::source_location::current() };
													context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
													derailleur<options>::skipToNextValue(context);
													return false;
												}
											}
										}
										static constexpr auto N = std::tuple_size_v<core_tuple_t<value_type>>;
										if (auto indexNew = hash_map<value_type, unwrap_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter); indexNew < N)
											[[likely]] {
											static constexpr auto arrayOfPtrs = generateFunctionPtrs<true, options, value_type, parse_context_type>();
											if (arrayOfPtrs[indexNew](value, context)) [[likely]] {
												return parseLambda(std::integral_constant<size_t, index + 1>{}, std::integral_constant<bool, false>{}, parseLambda, value, context);
											} else [[unlikely]] {
												derailleur<options>::skipToNextValue(context);
												return false;
											}
										} else {
											derailleur<options>::skipKey(context);
											++context.iter;
											derailleur<options>::skipToNextValue(context);
											return false;
										}
									} else [[unlikely]] {
										static constexpr auto sourceLocation{ std::source_location::current() };
										context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_String_Start>(context);
										derailleur<options>::skipToNextValue(context);
										return false;
									}
								}
							}
							return true;
						};
						if (antihash) {
							parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, parseLambda, value, context);
						} else {
							parseLambda(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, false>{}, parseLambda, value, context);
						}
					}
				}
				++context.iter;
				--context.currentObjectDepth;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Object_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto size{ std::tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = std::get<0>(coreTupleV<value_type>);
				using member_type			 = unwrap_t<decltype(getMember<newPtr>(value))>;
				parse<minified, options>::impl(getMember<newPtr>(value), context);
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::tuple_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			static constexpr auto memberCount = std::tuple_size_v<unwrap_t<value_type>>;
			if (*context.iter == '[') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != ']') [[likely]] {
					if constexpr (memberCount > 0) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ size_t(context.iter - wsStart) };
						auto newPtr		  = std::get<0>(value);
						using member_type = unwrap_t<decltype(getMember(newPtr, value))>;
						parse<false, options>::impl(getMember(newPtr, value), context);

						if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
							parseObjects<memberCount, 1, true>(value, context, wsStart, wsSize);
						} else {
							parseObjects<memberCount, 1, false>(value, context, wsStart, wsSize);
						}
					}
				}
				++context.iter;
				JSONIFIER_SKIP_WS();
				--context.currentArrayDepth;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}

		template<size_t n, size_t currentIndex, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if constexpr (currentIndex < n) {
				if (*context.iter != ']') [[likely]] {
					if (*context.iter == ',') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						auto newPtr		  = std::get<currentIndex>(value);
						using member_type = unwrap_t<decltype(getMember(newPtr, value))>;
						parse<false, options>::impl(getMember(newPtr, value), context);
						return parseObjects<n, currentIndex + 1, newLines>(value, context, wsStart, wsSize);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}
				} else {
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
			} else {
				++context.iter;
				JSONIFIER_SKIP_WS();
				--context.currentObjectDepth;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::tuple_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			static constexpr auto memberCount = std::tuple_size_v<unwrap_t<value_type>>;
			if (*context.iter == '[') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != ']') [[likely]] {
					if constexpr (memberCount > 0) {
						auto newPtr		  = std::get<0>(value);
						using member_type = unwrap_t<decltype(getMember(newPtr, value))>;
						parse<true, options>::impl(getMember(newPtr, value), context);
						parseObjects<memberCount, 1>(value, context);
					}
				}
				++context.iter;
				--context.currentArrayDepth;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}

		template<size_t n, size_t currentIndex> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (currentIndex < n) {
				if (*context.iter != ']') [[likely]] {
					if (*context.iter == ',') [[likely]] {
						++context.iter;
						auto newPtr		  = std::get<currentIndex>(value);
						using member_type = unwrap_t<decltype(getMember(newPtr, value))>;
						parse<true, options>::impl(getMember(newPtr, value), context);
						return parseObjects<n, currentIndex + 1>(value, context);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}
				} else {
					++context.iter;
					--context.currentArrayDepth;
				}
			} else {
				++context.iter;
				--context.currentObjectDepth;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::map_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter == '{') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != '}') [[likely]] {
					const auto wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize{ size_t(context.iter - wsStart) };
					static thread_local typename unwrap_t<value_type>::key_type key{};
					parse<false, options>::impl(key, context);

					if (*context.iter == ':') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<value_type>::mapped_type;
						parse<false, options>::impl(value[key], context);
					} else [[unlikely]] {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}
					if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
						return parseObjects<true>(value, context, wsStart, wsSize);
					} else {
						return parseObjects<false>(value, context, wsStart, wsSize);
					}
				} else {
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Object_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			while (*context.iter != '}') [[likely]] {
				if (*context.iter == ',') [[likely]] {
					++context.iter;
					JSONIFIER_SKIP_MATCHING_WS();
					static thread_local typename unwrap_t<value_type>::key_type key{};
					parse<false, options>::impl(key, context);

					if (*context.iter == ':') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<value_type>::mapped_type;
						parse<false, options>::impl(value[key], context);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
					derailleur<options>::skipToNextValue(context);
					return;
				}
			}
			++context.iter;
			--context.currentObjectDepth;
			JSONIFIER_SKIP_WS();
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::map_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter == '{') [[likely]] {
				++context.iter;
				++context.currentObjectDepth;
				if (*context.iter != '}') [[likely]] {
					static thread_local typename unwrap_t<value_type>::key_type key{};
					parse<true, options>::impl(key, context);

					if (*context.iter == ':') [[likely]] {
						++context.iter;
						using member_type = unwrap_t<value_type>::mapped_type;
						parse<true, options>::impl(value[key], context);
					} else [[unlikely]] {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}

					while (*context.iter != '}') [[likely]] {
						if (*context.iter == ',') [[likely]] {
							++context.iter;
							static thread_local typename unwrap_t<value_type>::key_type key{};
							parse<true, options>::impl(key, context);

							if (*context.iter == ':') [[likely]] {
								++context.iter;
								using member_type = unwrap_t<value_type>::mapped_type;
								parse<true, options>::impl(value[key], context);
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Colon>(context);
								derailleur<options>::skipToNextValue(context);
								return;
							}
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Comma_Or_Object_End>(context);
							derailleur<options>::skipToNextValue(context);
							return;
						}
					}
					++context.iter;
					--context.currentObjectDepth;
				} else {
					++context.iter;
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Missing_Object_Start>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::variant_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& value, auto&& context) {
				using member_type = decltype(valueNew);
				return parse<minified, options>::impl(value, context);
			};
			visit<lambda>(value, value, context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::optional_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter != 'n') [[likely]] {
				parse<false, options>::impl(value.emplace(), context);
			} else {
				context.iter += 4;
				JSONIFIER_SKIP_WS();
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::optional_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter != 'n') [[likely]] {
				parse<true, options>::impl(value.emplace(), context);
			} else {
				context.iter += 4;
				return;
			}
		}
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<const jsonifier::parse_options& options, jsonifier::concepts::vector_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			if (*context.iter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.iter;
				if (*context.iter != ']') [[likely]] {
					const auto wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize{ size_t(context.iter - wsStart) };
					if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
						parseObjects<true>(value, context, wsStart, wsSize);
					} else {
						parseObjects<false>(value, context, wsStart, wsSize);
					}
				} else {
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
			}
		}

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if (const size_t n = value.size(); n > 0) [[likely]] {
				auto iterNew = value.begin();

				for (size_t i = 0; i < n; ++i) {
					parse<false, options>::impl(*(iterNew++), context);

					if (*context.iter == ',') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
					} else [[unlikely]] {
						if (*context.iter == ']') [[likely]] {
							++context.iter;
							JSONIFIER_SKIP_WS()
							--context.currentArrayDepth;
							return (value.size() == i + 1) ? noop() : value.resize(i + 1);
						} else [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
							derailleur<options>::skipToNextValue(context);
							return;
						}
					}
				}
			}

			while (context.iter != context.endIter) {
				parse<false, options>::impl(value.emplace_back(), context);

				if (*context.iter == ',') [[likely]] {
					++context.iter;
					JSONIFIER_SKIP_MATCHING_WS();
				} else [[unlikely]] {
					if (*context.iter == ']') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_WS();
						--context.currentArrayDepth;
						return;
					} else [[unlikely]] {
						static constexpr auto sourceLocation{ std::source_location::current() };
						reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
						derailleur<options>::skipToNextValue(context);
						return;
					}
				}
			}
		}

	  protected:
		template<const auto& sourceLocation, auto parseError> JSONIFIER_ALWAYS_INLINE static void reportError(parse_context_type context) {
			context.parserPtr->template reportError<sourceLocation, parseError>(context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::vector_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.iter + (x * 64));
			}
			if (*context.iter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.iter;
				if (*context.iter != ']') [[likely]] {
					if (const size_t n = value.size(); n > 0) [[likely]] {
						auto iterNew = value.begin();

						for (size_t i = 0; i < n; ++i) {
							parse<true, options>::impl(*(iterNew++), context);

							if (*context.iter == ',') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								if (*context.iter == ']') [[likely]] {
									++context.iter;
									--context.currentArrayDepth;
									return (value.size() == i + 1) ? noop() : value.resize(i + 1);
								} else [[unlikely]] {
									static constexpr auto sourceLocation{ std::source_location::current() };
									reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
									derailleur<options>::skipToNextValue(context);
									return;
								}
							}
						}
					}

					while (context.iter != context.endIter) {
						parse<true, options>::impl(value.emplace_back(), context);

						if (*context.iter == ',') [[likely]] {
							++context.iter;
						} else [[unlikely]] {
							if (*context.iter == ']') [[likely]] {
								++context.iter;
								--context.currentArrayDepth;
								return;
							} else [[unlikely]] {
								static constexpr auto sourceLocation{ std::source_location::current() };
								reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
								derailleur<options>::skipToNextValue(context);
								return;
							}
						}
					}
				} else {
					++context.iter;
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
			}
		}

	  protected:
		template<const auto& sourceLocation, auto parseError> JSONIFIER_ALWAYS_INLINE static void reportError(parse_context_type context) {
			context.parserPtr->template reportError<sourceLocation, parseError>(context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::raw_array_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.iter;
				if (*context.iter != ']') [[likely]] {
					const auto wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize{ size_t(context.iter - wsStart) };
					if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
						return parseObjects<true>(value, context, wsStart, wsSize);
					} else {
						return parseObjects<false>(value, context, wsStart, wsSize);
					}
				} else {
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
			}
		}

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if (const size_t n = std::size(value); n > 0) [[likely]] {
				auto iterNew = std::begin(value);

				for (size_t i = 0; i < n; ++i) {
					parse<false, options>::impl(*(iterNew++), context);

					if (*context.iter == ',') [[likely]] {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
					} else [[unlikely]] {
						if (*context.iter == ']') [[likely]] {
							++context.iter;
							JSONIFIER_SKIP_WS()
							--context.currentArrayDepth;
							return;
						} else [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
							derailleur<options>::skipToNextValue(context);
							return;
						}
					}
				}
			}
		}

	  protected:
		template<const auto& sourceLocation, auto parseError> JSONIFIER_ALWAYS_INLINE static void reportError(parse_context_type context) {
			context.parserPtr->template reportError<sourceLocation, parseError>(context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::raw_array_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (*context.iter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.iter;
				if (*context.iter != ']') [[likely]] {
					if (const size_t n = std::size(value); n > 0) [[likely]] {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < n; ++i) {
							parse<false, options>::impl(*(iterNew++), context);

							if (*context.iter == ',') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								if (*context.iter == ']') [[likely]] {
									++context.iter;
									--context.currentArrayDepth;
									return;
								} else [[unlikely]] {
									static constexpr auto sourceLocation{ std::source_location::current() };
									reportError<sourceLocation, parse_errors::Imbalanced_Array_Brackets>(context);
									derailleur<options>::skipToNextValue(context);
									return;
								}
							}
						}
					}
				} else {
					++context.iter;
					--context.currentArrayDepth;
				}
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				reportError<sourceLocation, parse_errors::Missing_Array_Start>(context);
				derailleur<options>::skipToNextValue(context);
			}
		}

	  protected:
		template<const auto& sourceLocation, auto parseError> JSONIFIER_ALWAYS_INLINE static void reportError(parse_context_type context) {
			context.parserPtr->template reportError<sourceLocation, parseError>(context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::string_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			derailleur<options>::parseString(value, context);
			JSONIFIER_SKIP_WS();
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::string_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			derailleur<options>::parseString(value, context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::char_type value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			value = static_cast<value_type>(*(static_cast<const char*>(context.iter) + 1));
			++context.iter;
			JSONIFIER_SKIP_WS();
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::char_type value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			value = static_cast<value_type>(*(static_cast<const char*>(context.iter) + 1));
			++context.iter;
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::raw_json_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = static_cast<const char*>(context.iter);
			derailleur<options>::skipToNextValue(context);
			int64_t newSize = static_cast<const char*>(context.iter) - newPtr;
			if (newSize > 0) [[likely]] {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = newString;
			}
			return;
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::shared_ptr_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			using member_type = decltype(*value);
			if (!value) [[unlikely]] {
				value = std::make_shared<std::remove_pointer_t<unwrap_t<member_type>>>();
			}
			parse<minified, options>::impl(*value, context);
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::unique_ptr_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			using member_type = decltype(*value);
			if (!value) [[unlikely]] {
				value = std::make_unique<std::remove_pointer_t<unwrap_t<member_type>>>();
			}
			parse<minified, options>::impl(*value, context);
		}
	};

	template<bool minified, const jsonifier::parse_options& options, jsonifier::concepts::pointer_t value_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (!value) [[unlikely]] {
				value = new std::remove_pointer_t<unwrap_t<value_type>>{};
			}
			parse<minified, options>::impl(*value, context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::always_null_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&, parse_context_type& context) noexcept {
			if (parseNull(context.iter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Null_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::always_null_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&, parse_context_type& context) noexcept {
			if (parseNull(context.iter)) [[likely]] {
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Null_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::enum_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			size_t newValue{};
			if (parseNumber(newValue, context.iter, context.endIter)) [[likely]] {
				value = static_cast<value_type>(newValue);
				JSONIFIER_SKIP_WS();
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Number_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::enum_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			size_t newValue{};
			if (parseNumber(newValue, context.iter, context.endIter)) [[likely]] {
				value = static_cast<value_type>(newValue);
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Number_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::num_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (parseNumber(value, context.iter, context.endIter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Number_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::num_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (parseNumber(value, context.iter, context.endIter)) [[likely]] {
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Number_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::bool_t value_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (parseBool(value, context.iter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Bool_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::bool_t value_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, parse_context_type> {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if (parseBool(value, context.iter)) [[likely]] {
				return;
			} else [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserPtr->template reportError<sourceLocation, parse_errors::Invalid_Bool_Value>(context);
				derailleur<options>::skipToNextValue(context);
				return;
			}
		}
	};

}