// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__1AA352BB_6367_4A7E_AB23_02E609C3FA23__INCLUDED_)
#define AFX_STDAFX_H__1AA352BB_6367_4A7E_AB23_02E609C3FA23__INCLUDED_

#ifdef _WIN32

	#if _MSC_VER > 1000
		#pragma once
	#endif // _MSC_VER > 1000

	#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

	#include <windows.h>
	#include <limits.h>
	#ifdef _DEBUG
		#include <assert.h>
		#define ASSERT assert
	#else
		#define ASSERT
	#endif //_DEBUG

#else       // LINUX

    #include <stdlib.h>
    #include <stdio.h>
    #include <strings.h>
    #include <limits.h>	
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

#if defined(_WIN32) && defined(_DEBUG)
	void * __cdecl operator new(unsigned int,int,const char *,int );
	inline void* __cdecl operator new(unsigned int s, const char*pszFileName, int nLine)
        { return ::operator new(s, 1, pszFileName, nLine); }
	inline void operator delete( void * p, const char *, int )
		{	::delete( p ); }
	#define DEBUG_NEW new(__FILE__, __LINE__)
#endif //defined(_WIN32) && defined(_DEBUG)


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__1AA352BB_6367_4A7E_AB23_02E609C3FA23__INCLUDED_)
