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
/// Feb 3, 2023
/// https://github.com/RealTimeChris/Jsonifier
/// \file DllMain.cpp

#ifdef _WIN32
	#include <jsonifier/Jsonifier.hpp>
	#include <Windows.h>

bool WINAPI DllMain(HINSTANCE hinstDLL, uint64_t fdwReason, void* lpvReserved) noexcept {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH: {
			break;
		}
		case DLL_THREAD_ATTACH: {
			break;
		}
		case DLL_THREAD_DETACH: {
			break;
		}
		case DLL_PROCESS_DETACH: {
			break;
		}
	}
	return TRUE;
}
#endif
