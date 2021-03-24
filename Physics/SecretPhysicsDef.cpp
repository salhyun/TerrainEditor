#include "SecretPhysicsDef.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

//#if !defined(TRACE) && !defined(ATLTRACE)
//void TRACE(char* lpFormatStr, ...)
//{
//#if defined(DEBUG) | defined(_DEBUG)
//
//	TCHAR   szBuffer[512];
//	va_list args;
//	va_start    (args, lpFormatStr);
//	//_vsntprintf (szBuffer, 512, lpFormatStr, args);
//	_vsntprintf_s (szBuffer, 512, 512, lpFormatStr, args);
//	va_end      (args);
//	OutputDebugString (szBuffer);
//#endif
//}
//#endif