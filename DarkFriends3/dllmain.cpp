// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DarkFriends3.h"
#include "Log.h"
#include <easyhook.h>

const char* logFilePath = nullptr;
HANDLE hFile;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
	}

	return TRUE;
}

BYTE debug = 0;

extern "C" __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO *info) {
	debug = *info->UserData;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Bootstrap, &debug, 0, NULL);
	RhWakeUpProcess();
}

