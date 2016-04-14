#include "stdafx.h"
#include "Log.h"
#include <stdio.h>
#include <stdarg.h>

bool loggingEnabled = false;
void CreateDebugConsole() {
	FILE *conin, *conout;
	AllocConsole();
	freopen_s(&conin, "conin$", "r", stdin);
	freopen_s(&conout, "conout$", "w", stdout);
	freopen_s(&conout, "conout$", "w", stderr);
	loggingEnabled = true;
}

void Log(const char * format, ...)
{
	if (!loggingEnabled)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\r\n");
}