
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>

static void __cdecl TRACE(const char *szString, ...)
{
	char szDebugString[1024];
	va_list varList;

	va_start(varList, szString);

	vsprintf(szDebugString, szString, varList);
	OutputDebugString(szDebugString);
}
