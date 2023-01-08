
#pragma once

#include <jsonifier/Object.hpp>

namespace Jsonifier {

	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool has_value;
		JsonifierTry(iterator.find_field_unordered_raw(key).get(has_value));
		if (!has_value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool has_value;
		JsonifierTry(iterator.find_field_unordered_raw(key).get(has_value));
		if (!has_value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::operator[](const std::string_view key) & noexcept {
		return findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Object::operator[](const std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}
	inline JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool has_value;
		JsonifierTry(iterator.find_field_raw(key).get(has_value));
		if (!has_value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}
	inline JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool has_value;
		JsonifierTry(iterator.find_field_raw(key).get(has_value));
		if (!has_value) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	inline JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.start_object().error());
		return Object(iterator);
	}
	inline JsonifierResult<Object> Object::start_root(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.start_root_object().error());
		return Object(iterator);
	}
	inline ErrorCode Object::consume() noexcept {
		if (iterator.is_at_key()) {
			/**
     * whenever you are pointing at a key, calling skip_child() is
     * unsafe because you will hit a string and you will assume that
     * it is string Value, and this mistake will lead you to make bad
     * depth computation.
     */
			/**
     * We want to 'consume' the key. We could really
     * just do jsonIterator->return_current_and_advance(); at this
     * point, but, for clarity, we will use the high-level API to
     * eat the key. We assume that the compiler optimizes away
     * most of the work.
     */
			RawJsonString actual_key;
			auto error = iterator.field_key().get(actual_key);
			if (error) {
				iterator.abandon();
				return error;
			};
			// Let us move to the Value while we are at it.
			if ((error = iterator.field_value())) {
				iterator.abandon();
				return error;
			}
		}
		auto error_skip = iterator.json_iter().skip_child(iterator.depth() - 1);
		if (error_skip) {
			iterator.abandon();
		}
		return error_skip;
	}

	inline JsonifierResult<std::string_view> Object::raw_json() noexcept {
		const uint8_t* starting_point{ iterator.peek_start() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* final_point{ iterator.jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.started_object().error());
		return Object(iterator);
	}

	inline Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Object::Object(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(iterator);
	}
	inline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(iterator);
	}

	inline JsonifierResult<Value> Object::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		size_t slash = json_pointer.find('/');
		std::string_view key = json_pointer.substr(0, slash);
		// Grab the child with the given key
		JsonifierResult<Value> child;

		// If there is an escape character in the key, unescape it and then get the child.
		size_t escape = key.find('~');
		if (escape != std::string_view::npos) {
			// Unescape the key
			std::string unescaped(key);
			do {
				switch (unescaped[escape + 1]) {
					case '0':
						unescaped.replace(escape, 2, "~");
						break;
					case '1':
						unescaped.replace(escape, 2, "/");
						break;
					default:
						return Invalid_Json_Pointer;// "Unexpected ~ escape character in JSON pointer");
				}
				escape = unescaped.find('~', escape + 1);
			} while (escape != std::string::npos);
			child = findField(unescaped);// Take note findField does not unescape keys when matching
		} else {
			child = findField(key);
		}
		if (child.error()) {
			return child;// we do not continue if there was an error
		}
		// If there is a /, we have to recurse and look up more of the path
		if (slash != std::string_view::npos) {
			child = child.atPointer(json_pointer.substr(slash));
		}
		return child;
	}

	inline JsonifierResult<size_t> Object::countFields() & noexcept {
		size_t count{ 0 };
		// Important: we do not consume any of the values.
		for (auto v: *this) {
			count++;
		}
		// The above loop will always succeed, but we want to report errors.
		if (iterator.error()) {
			return iterator.error();
		}
		// We need to move back at the start because we expect users to iterate through
		// the Object after counting the number of elements.
		iterator.reset_object();
		return count;
	}

	inline JsonifierResult<bool> Object::is_empty() & noexcept {
		bool is_not_empty;
		auto error = iterator.reset_object().get(is_not_empty);
		if (error) {
			return error;
		}
		return !is_not_empty;
	}

	inline JsonifierResult<bool> Object::reset() & noexcept {
		return iterator.reset_object();
	}


		inline JsonifierResult<Object>::JsonifierResult(Object&& Value) noexcept
			: ImplementationJsonifierResultBase<Object>(std::forward<Object>(Value)) {
		}
		inline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Object>(error) {
		}

		inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
			if (error()) {
				return error();
			}
			return first.begin();
		}
		inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
			if (error()) {
				return error();
			}
			return first.end();
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
			if (error()) {
				return error();
			}
			return first.findFieldUnordered(key);
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
			if (error()) {
				return error();
			}
			return std::forward<Object>(first).findFieldUnordered(key);
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
			if (error()) {
				return error();
			}
			return first[key];
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
			if (error()) {
				return error();
			}
			return std::forward<Object>(first)[key];
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) & noexcept {
			if (error()) {
				return error();
			}
			return first.findField(key);
		}
		inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) && noexcept {
			if (error()) {
				return error();
			}
			return std::forward<Object>(first).findField(key);
		}

		inline JsonifierResult<Value> JsonifierResult<Object>::atPointer(std::string_view json_pointer) noexcept {
			if (error()) {
				return error();
			}
			return first.atPointer(json_pointer);
		}

		inline JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
			if (error()) {
				return error();
			}
			return first.reset();
		}

		inline JsonifierResult<bool> JsonifierResult<Object>::is_empty() noexcept {
			if (error()) {
				return error();
			}
			return first.is_empty();
		}

		inline JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
			if (error()) {
				return error();
			}
			return first.countFields();
		}

	}
