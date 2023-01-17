/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/IteratorCore.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	__forceinline void IteratorCore::setPosition(StructuralIndex newIndex) noexcept {
		currentStructural = newIndex;
	}

	__forceinline OutStringPtr& IteratorCore::getStringBuffer() noexcept {
		return outString;
	}

	__forceinline StructuralIndex IteratorCore::lastPosition() noexcept {
		return endStructural;
	}

	__forceinline InStringPtr IteratorCore::getStringView() noexcept {
		return inString;
	}

	__forceinline StructuralIndex IteratorCore::position() noexcept {
		return currentStructural;
	}

	__forceinline void IteratorCore::setError(ErrorCode error) noexcept {
		errorVal = error;
	}

	__forceinline ErrorCode IteratorCore::reportError() noexcept {
		return errorVal;
	}

	__forceinline void IteratorCore::reset(Parser* parserNew) noexcept {
		endStructural = &parserNew->structuralIndices[parserNew->tapeLength - 1];
		currentStructural = parserNew->structuralIndices;
		rootStructural = parserNew->structuralIndices;
		outString = parserNew->outString;
		inString = parserNew->inString;
	}

}
