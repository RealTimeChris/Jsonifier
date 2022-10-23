#ifdef _WIN32

#include <Windows.h>

bool __stdcall DllMain(void*, unsigned long fdwReason, void*) {
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
			return TRUE;
		}
	}
}
#endif