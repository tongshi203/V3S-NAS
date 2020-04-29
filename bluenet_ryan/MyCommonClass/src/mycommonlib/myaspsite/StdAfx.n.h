#pragma once

#ifdef _WIN32
    // WIN32
    #define VC_EXTRALEAN

    #ifdef _MFC_VER
            #include <afx.h>
    #else
            #include <windows.h>
            #include <stdlib.h>
            #include <stdio.h>
            #include <MyString.h>
            #include <limits.h>
            #define CString CMyString

            #define TRACE
            #define TRACE0
            #ifndef _DEBUG					// RELEASE
                    #define VERIFY(f)          ((void)(f))
                    #define ASSERT
            #else	// _DEBUG
                    #include <assert.h>
                    #define ASSERT		assert
                    #define VERIFY(f)          ASSERT(f)
            #endif	// _DEBUG
    #endif //_MFC_VER

#else       // LINUX

    #include <stdlib.h>
    #include <stdio.h>
    #include <strings.h>
    #define _stricmp strcasecmp

    #include <assert.h>

    #ifdef _DEBUG
            #ifdef _WIN32
                    void TRACE(const char * lpszFormat, ...);
            #define ASSERT _ASSERT
	    #define VERIFY _ASSERT
            #else
               #ifndef TRACE
                    #define TRACE  printf
               #endif //TRACE
               #ifndef ASSERT
                    #define ASSERT assert
               #endif //
            #endif // _WIN32
    #else // Release
            #define ASSERT
            #define TRACE
	    #define VERIFY( a )		(void)a
    #endif // _DEBUG

    #if !( defined(_WIN32) || defined(__BORLANDC__) || defined(NULL) )
      #define NULL (void*)0
    #endif //NULL

    #include <MyComDef.h>

#endif //_WIN32

