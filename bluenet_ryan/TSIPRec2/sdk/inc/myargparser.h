// myargparser.h: interface for the CMyArgParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYARGPARSER_H__05788285_7797_4664_9D13_F37A18F23210__INCLUDED_)
#define AFX_MYARGPARSER_H__05788285_7797_4664_9D13_F37A18F23210__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyArray.h>

#pragma pack(push,4)

class CMyArgParser  
{
public:
	CMyArgParser();
	CMyArgParser(const char * lpszArgument, char cSpliter = ';' );
	virtual ~CMyArgParser();

	int SetArgumentString( const char * lpszArgument, char cSpliter = ';' );
	int GetArgCount()const { return m_nArgCount; }

	const char * GetArgName(int nIndex);
	int GetArgNameIndex( const char * lpszArgName );

	const char * GetArgAsString(int nIndex, LPCSTR lpszDefValue = NULL );
	const char * GetArgAsString(const char * lpszArgName, LPCSTR lpszDefValue = NULL );

	int GetArgAsInt(int nIndex, int nDefValue = 0 );
	int GetArgAsInt(const char * lpszArgName, int nDefValue = 0 );

	unsigned long GetArgAsDWORD(int nIndex, bool bIsHex=true, DWORD dwDefValue = 0);
	unsigned long GetArgAsDWORD(const char * lpszArgName, bool bIsHex=true, DWORD dwDefValue = 0 );

private:
	void ParseOneArg( char * lpszArgLine );

private:	
	char * m_pszArgumentBuf;

	int m_nArgCount;
	CMyArray<LPCSTR>	m_aArgName;
	CMyArray<LPCSTR>	m_aArgValue;
};

#pragma pack(pop)

#endif // !defined(AFX_MYARGPARSER_H__05788285_7797_4664_9D13_F37A18F23210__INCLUDED_)
