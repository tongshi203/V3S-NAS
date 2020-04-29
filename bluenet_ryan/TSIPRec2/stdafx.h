// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__20040401__INCLUDED_)
#define AFX_STDAFX_H__20040401__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//-----------------------------------------
//	若为 Linux 操作系统使用，则定义如下宏为
#define __USE_FOR_LINUX__

//-----------------------------------------
//	只有Linux下，才有必要release时也打印文字
#ifdef __USE_FOR_LINUX__
	#define __PRINT_DEBUG_MSG_RELEASE__
#endif // __USE_FOR_LINUX__


#ifdef _WIN32

    #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

    #include <afxwin.h>         // MFC core and standard components
    #include <afxext.h>         // MFC extensions

    #ifndef _AFX_NO_OLE_SUPPORT
    #include <afxole.h>         // MFC OLE classes
    #include <afxodlgs.h>       // MFC OLE dialog classes
    #include <afxdisp.h>        // MFC Automation classes
    #endif // _AFX_NO_OLE_SUPPORT


    #ifndef _AFX_NO_DB_SUPPORT
    #include <afxdb.h>			// MFC ODBC database classes
    #endif // _AFX_NO_DB_SUPPORT

    #ifndef _AFX_NO_DAO_SUPPORT
    #include <afxdao.h>			// MFC DAO database classes
    #endif // _AFX_NO_DAO_SUPPORT

    #include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
    #ifndef _AFX_NO_AFXCMN_SUPPORT
    #include <afxcmn.h>			// MFC support for Windows Common Controls
    #endif // _AFX_NO_AFXCMN_SUPPORT

	#include <afxtempl.h>
	#include <afxmt.h>

    //#include <afxsock.h>		// MFC socket extensions
    #include "MyRegKey.h"

#else  // Linux

	#include "sdk/inc/stdafx.h"

    #include <MyComDef.h>
    #include <time.h>
    #include <MyString.h>
    #include <MyFile.h>
    #include <MyArray.h>
    #include <MySyncObj.h>
	typedef CMyArray<DWORD>	CDWordArray;
    typedef CMyArray<BYTE>	CByteArray;

    #define CString CMyString
    #define TRY	try
    #define CATCH_ALL( e )	catch( ... )
    #define CATCH( a, e )	catch( ... )
    #define	END_CATCH
    #define END_CATCH_ALL
    #define _MAX_PATH	260
	#define HANDLE		unsigned long
    #define	HRESULT		DWORD
    #define WINAPI	__stdcall
    #define HMODULE		DWORD

#ifdef __cplusplus
    #define EXTERN_C	extern "C"
#else
	#define EXTERN_C	extern
#endif //__cplusplus

    #define IUnknown	IMyUnknown

#endif //


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // AFX_STDAFX_H__20040401__INCLUDED_
