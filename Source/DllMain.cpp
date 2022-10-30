#ifdef _WIN32

#include <Windows.h>

bool WINAPI DllMain(HINSTANCE hinstDLL, unsigned long fdwReason, void* lpvReserved) {
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