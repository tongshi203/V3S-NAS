// BScriptEngine.h: interface for the CBScriptEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BSCRIPTENGINE_H__06EB8799_5136_499F_A551_93254F524EE9__INCLUDED_)
#define AFX_BSCRIPTENGINE_H__06EB8799_5136_499F_A551_93254F524EE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(_MSC_VER) && (!defined(_LIB))
	#ifdef _AFXDLL
		#ifdef _DEBUG
			#pragma comment(lib, "BScriptD.lib")
		#else
			#pragma comment(lib, "BScript.lib")
		#endif //_DEBUG
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "BScriptSD.lib")
		#else
			#pragma comment(lib, "BScriptS.lib")
		#endif //_DEBUG
	#endif //_AFXDLL
#endif // _LIB

#ifndef __BSCRIPT_H_20050428__
	typedef struct _BSInterpreter BSInterpreter;
	typedef struct _BSModule BSModule;
	typedef struct _BSContext BSContext;
#endif // __BSCRIPT_H_20050428__

#ifndef __BLIB_H_20050428__
	typedef struct _BVariant BVariant;
	typedef struct _BList BList;
#endif // __BLIB_H_20050428__

class CBSAutoReleaseString
{
public:
	CBSAutoReleaseString();
	CBSAutoReleaseString(const CBSAutoReleaseString & strText);
	CBSAutoReleaseString(const char * pszText );
	~CBSAutoReleaseString();
	CBSAutoReleaseString & operator = (const char * pszText );
	CBSAutoReleaseString & operator = (const CBSAutoReleaseString & strText );
	operator const char *()
	{
		return m_pString;
	}
	char *	m_pString;
};

class CBSExternFunctionTemplete
{
public:
	CBSExternFunctionTemplete( BSModule * pModule, const char * lpszFunctionName );
	virtual ~CBSExternFunctionTemplete();
	static BVariant* FunctionCallBackLink( BSContext *context, BList *args );
	long AddRef();
	long Release();

	virtual void OnFunctionCall() = 0;
	virtual void SafeDelete() = 0;
	int GetArgCount();

	bool IsArgTypeString( int nIndex );

	bool GetArgAsBool( int nIndex );
	int GetArgAsInt( int nIndex );
	double GetArgAsDouble( int nIndex );
	CBSAutoReleaseString GetArgAsString( int nIndex );
	void * GetArgAsPointer( int nIndex );

	void SetRetValue( int nValue );
	void SetRetValue( double fValue );
	void SetRetValue( const char * pszValue );
private:
	BSModule * m_pModule;
	char * m_pszFunctionName;
	long m_nRef;
	BList * m_pArgList;
	BSContext * m_pContext;
	BVariant * m_pRetVal;
};

class CBScriptEngine  
{
public:
	virtual void OnModuleCreated( BSModule * pModule );
	virtual void OnDeleteModule( BSModule * pModule );
	CBScriptEngine();
	virtual ~CBScriptEngine();
	bool IsValid();
	void Run( char * pszCode );
	void RunFile( char * pszFileName );
	virtual void Print( void * pData, bool bIsString );

private:
	BSInterpreter * m_pEngine;
};

#endif // !defined(AFX_BSCRIPTENGINE_H__06EB8799_5136_499F_A551_93254F524EE9__INCLUDED_)


















