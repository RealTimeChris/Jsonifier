
#include <jsonifier/AssemblyFunctions.hpp>

#if defined _WIN32

#include <Windows.h>
#include <cstdint>

bool __stdcall dll_main(void*, uint64_t fdwReason, void*) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH: {
			[[fallthrough]];
		}
		case DLL_THREAD_ATTACH: {
			[[fallthrough]];
		}
		case DLL_THREAD_DETACH: {
			[[fallthrough]];
		}
		case DLL_PROCESS_DETACH: {
			[[fallthrough]];
		}
		default: {
			return true;
		}
	}
}
#endif
