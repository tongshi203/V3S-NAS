// myargparser.cpp: implementation of the CMyArgParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include "myargparser.h"
#include <MyString.h>

#ifdef _WIN32
	#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
	#endif
#endif // _WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyArgParser::CMyArgParser()
{
	m_pszArgumentBuf = NULL;
	m_nArgCount = 0;
}

CMyArgParser::~CMyArgParser()
{
	if( m_pszArgumentBuf )
		free( m_pszArgumentBuf );
}

CMyArgParser::CMyArgParser(const char * lpszArgument, char cSpliter )
{
	m_pszArgumentBuf = NULL;
	m_nArgCount = 0;

	SetArgumentString( lpszArgument, cSpliter );
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		Set Argument string
// Input:
//		lpszArgument		argumen string
//		cSpliter			spliter char
// Output:
//		>0					argument count
//		<=0					error
int CMyArgParser::SetArgumentString( const char * lpszArgument, char cSpliter )
{
	if( NULL == lpszArgument )
	{
		fprintf( stderr, "CMyArgParser::SetArgumentString, lpszArgument = NULL\n" );
		return -1;
	}

	m_aArgName.RemoveAll();
	m_aArgValue.RemoveAll();

	if( m_pszArgumentBuf )
		free( m_pszArgumentBuf );
	m_pszArgumentBuf = strdup( lpszArgument );
	if( NULL == m_pszArgumentBuf )
	{
		fprintf( stderr, "CMyArgParser::SetArgumentString, strdup failed.\n" );
		return -1;
	}

	int nLen = strlen(m_pszArgumentBuf);

	char * pszArgName = m_pszArgumentBuf;

	for( int i=0; i<nLen; i++ )
	{
		if( m_pszArgumentBuf[i] != cSpliter && m_pszArgumentBuf[i] )
			continue;

		m_pszArgumentBuf[i] = 0;

		ParseOneArg( pszArgName );		

		pszArgName = m_pszArgumentBuf + i + 1;
	}

	if( pszArgName )
		ParseOneArg( pszArgName );

	m_nArgCount = m_aArgName.GetSize();

	return m_nArgCount;
}

//-----------------------------------------
// my trim left
static char * MyTrimLeft( char * lpszLine )
{
	// skip space
	while( *lpszLine && *(PBYTE)lpszLine <= 0x20  )
	{
		lpszLine ++;
	}
	return lpszLine;
}

//------------------------------------------
// my trim right
static void MyTrimRight( char * lpszLine )
{
	int nLen = strlen(lpszLine);
	char * pszRight = lpszLine + nLen;
	
	while( pszRight > lpszLine )
	{
		if( *(PBYTE)pszRight > 0x20 )
			return;
		*pszRight = 0;
		pszRight --;
	}	
}

//-------------------------------------------------------
// CYJ,2008-7-14
// Function:
//
// Input:
//		lpszArg			argument line
// Out:
//		None	
void CMyArgParser::ParseOneArg( char * lpszArgLine )
{
	if( NULL == lpszArgLine )
		return;

	// skip space
	char * pszArgName = MyTrimLeft( lpszArgLine );
	MyTrimRight( pszArgName );
		
	char * pszArgValue = strchr( pszArgName, '=' );
	if( NULL == pszArgValue )
	{
		fprintf( stderr, "ParseOneArg, Only ArgName=%s\n", pszArgName );
		return;
	}
	*pszArgValue = 0;
	pszArgValue = MyTrimLeft( pszArgValue+1 );
	MyTrimRight( pszArgName );
	
	if( pszArgValue && pszArgName && *pszArgValue && *pszArgName )
	{
		const char * pszName = pszArgName;
		const char * pszValue = pszArgValue;
		m_aArgName.Add( pszName );
		m_aArgValue.Add( pszValue );
	}
	else
	{
		fprintf(stderr, "Find one bad argument pair, ArgName=%p, Value=%p, ", pszArgName, pszArgValue );
		if( pszArgName )
			fprintf( stderr, "pszArgName = %s, \n", pszArgName );
		if( pszArgValue )
			fprintf( stderr, "pszArgValue = %s, \n", pszArgValue );
		fprintf( stderr, "\n" );				
	}
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		Get Argument name by Index
// Input:
//		nIndex			index	
// Output:
//		NULL			failed
//		else			argument name
const char * CMyArgParser::GetArgName(int nIndex)
{
	if( nIndex < 0 || nIndex >= m_nArgCount )
		return NULL;

	return m_aArgName[nIndex];
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		Get Argument index by name
// Input:
//		lpszArgName		argument name
// Output:
//		>=0				index
//		<0				failed
int CMyArgParser::GetArgNameIndex( const char * lpszArgName )
{
	int nCount = m_nArgCount;
	for(int i=0; i<nCount; i++ )
	{
#ifdef _WIN32
		if( stricmp(lpszArgName, m_aArgName[i] ) == 0 )
			return i;
#else
		if( strcasecmp(lpszArgName, m_aArgName[i] ) == 0 )
			return i;
#endif //_WIN32
	}
	return -1;
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		Get Argument as string by index
// Input:
//		nIndex
// Output:
//		NULL			failed
//		else			argument value
const char * CMyArgParser::GetArgAsString(int nIndex, LPCSTR lpszDefValue)
{
	if( nIndex < 0 || nIndex >= m_nArgCount )
		return lpszDefValue;

	return m_aArgValue[nIndex];
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		
// Input:
//		
// Output:
//		
const char * CMyArgParser::GetArgAsString(const char * lpszArgName, LPCSTR lpszDefValue)
{
	int nIndex = GetArgNameIndex(lpszArgName);
	if( nIndex < 0 )
		return lpszDefValue;

	return m_aArgValue[nIndex];
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		
// Input:
//		
// Output:
//		
int CMyArgParser::GetArgAsInt(int nIndex, int nDefValue)
{
	if( nIndex < 0 || nIndex >= m_nArgCount )
		return nDefValue;

	return atoi( (LPCSTR)m_aArgValue[nIndex] );
}	

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		
// Input:
//		
// Output:
//		
int CMyArgParser::GetArgAsInt(const char * lpszArgName, int nDefValue)
{
	int nIndex = GetArgNameIndex(lpszArgName);
	if( nIndex < 0 )
		return nDefValue;

	return atoi( (LPCSTR)m_aArgValue[nIndex] );
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		
// Input:
//		
// Output:
//		
unsigned long CMyArgParser::GetArgAsDWORD(int nIndex, bool bIsHex, DWORD dwDefValue)
{
	if( nIndex < 0 || nIndex >= m_nArgCount )
		return dwDefValue;

	return strtoul( (LPCSTR)m_aArgValue[nIndex], NULL, bIsHex ? 16 : 10 );
}

//----------------------------------------------------
// CYJ, 2008.7.14
// Function:
//		
// Input:
//		
// Output:
//		
unsigned long CMyArgParser::GetArgAsDWORD(const char * lpszArgName, bool bIsHex, DWORD dwDefValue)
{
	int nIndex = GetArgNameIndex(lpszArgName);
	if( nIndex < 0 )
		return dwDefValue;

	return strtoul( (LPCSTR)m_aArgValue[nIndex], NULL, bIsHex ? 16 : 10 );
}

