// stdafx.cpp : source file that includes just the standard includes
//	IPRecDrv.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <stdio.h>

#if defined(_DEBUG) && defined( _WIN32 )

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
void TRACE(const char * lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[512];

	nBuf = _vsnprintf( szBuffer, sizeof(szBuffer), lpszFormat, args);

	if( nBuf > 0 )
		OutputDebugString( szBuffer );

	va_end(args);
}

#endif // defined(_DEBUG) && defined( _WIN32 )
