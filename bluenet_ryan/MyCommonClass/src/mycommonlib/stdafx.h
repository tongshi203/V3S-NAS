// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B64D15D0_892D_4F52_B1C2_403040A9110B__INCLUDED_)
#define AFX_STDAFX_H__B64D15D0_892D_4F52_B1C2_403040A9110B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __FOR_MICROWIN_EMBED__
	#include <assert.h>
#else
	#include <windows.h>
#endif // __FOR_MICROWIN_EMBED__

#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
    #ifdef _WIN32
		void TRACE(const char * lpszFormat, ...);
       	#define ASSERT _ASSERT
    #else
    #ifdef __FOR_MICROWIN_EMBED__
	    #define TRACE  printf
      	#define ASSERT(a) 
    #else // linux
        #define TRACE(format, ...) {fprintf(stderr, format, ## __VA_ARGS__); fprintf(stderr,"\n");}
        #include <assert.h>
        #define ASSERT(stat) assert(stat)
    #endif //
	#endif // _WIN32
#else // Release
    #define TRACE(format, ...) 
    #define ASSERT(stat)
#endif // _DEBUG

#if !( defined(_WIN32) || defined(__BORLANDC__) || defined(NULL) )
  #define NULL (void*)0
#endif //NULL

#include "MyComDef.h"
#include "my_errno.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B64D15D0_892D_4F52_B1C2_403040A9110B__INCLUDED_)
