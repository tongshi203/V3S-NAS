#include <stdafx.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "MyString.h"

#if defined(_DEBUG) && defined(_WIN32)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char CMyString::m_NilString = 0;

CMyString::CMyString(const char* str)
{
#ifdef _DEBUG
	char * pAddr1 = (char*)&m_pString;
    char * pAddr2 = (char *)(&m_nLength);
    ASSERT( pAddr2 - pAddr1 >= 4 );
    char * pAddr3 = (char * )this;
    ASSERT( pAddr1 == pAddr3 );
#endif //_DEBUG

	m_nBufSize = 0;
	m_nLength = 0;
	m_pString = &m_NilString;
	m_nBufSize = 0;

	if(str == 0)
		return;

	m_nLength = strlen(str);
	GetBuffer( m_nLength + 1 );
	if( m_pString )
		strcpy(m_pString, str);
}

CMyString::CMyString(const CMyString& str)
{
	m_nBufSize = 0;
	m_nLength = 0;
	m_pString = &m_NilString;
	m_nBufSize = 0;

	if(str.m_nLength  == 0)
		return;

	m_nLength = str.m_nLength;
	GetBuffer( m_nLength + 1 );
	ASSERT( m_pString );
	if( m_pString )
		strcpy( m_pString, str.m_pString );
}

CMyString::CMyString(const char var)
{
	m_nBufSize = 0;
	m_nLength = 0;
	m_pString = &m_NilString;
	m_nBufSize = 0;

	m_nLength = 1;
	char * pszBuf = GetBuffer( m_nLength + 1 );
    if( pszBuf )
    {
    	*pszBuf++ = var;
        *pszBuf = 0;
    }
}

CMyString::~CMyString()
{
	if( NULL == m_pString )
		return;
	if( m_pString != &m_NilString )
		delete m_pString;
}

CMyString& CMyString::operator +=(const CMyString& str)
{
	m_nLength += str.m_nLength;
	char* pNew = GetBuffer( m_nLength + 1 );
	ASSERT(pNew != 0 && pNew  == m_pString );
	if( pNew )
		strcat(pNew, str.m_pString);

	return *this;
}

CMyString& CMyString::operator =(const char* str)
{
	if( NULL == str )
    {
		m_nLength = 0;
		ReleaseBuffer( 0 );
    }
	else
    {
		m_nLength = strlen(str);
		char * pNew = GetBuffer( m_nLength + 1 );
		if( pNew )
			strcpy(m_pString, str);
	}
	return *this;
}

CMyString& CMyString::operator =(const CMyString& str)
{
	ASSERT( str.m_pString );
	return operator = ( str.m_pString );
}

void CMyString::VarToString(const double var)
{
	char str[32];
#ifndef __FOR_MICROWIN_EMBED__
	gcvt(var, 16, str);
	m_nLength = strlen(str);
	if( str[m_nLength - 1] == '.' )
	{
		str[m_nLength - 1] = '\0';
		m_nLength --;
	}
	m_nLength = strlen( str );
	char * pszBuf = GetBuffer( m_nLength + 1 );
	ASSERT( pszBuf && pszBuf == m_pString );
	if( pszBuf )
		strcpy( m_pString, str );
#else
	Format("%d", var);
#endif //__FOR_MICROWIN_EMBED__
}

int CMyString::Format(const char* format, ...)
{
	ASSERT(format != 0);

	int len;
	char* MaxBuf;
	for(int i = 5; ; i ++)
	{
		len = 1 << i;
        int nBufMaxLen = len;
		MaxBuf = new char[len];
		if (!MaxBuf)
			return 0;

		// 2014.7.3 CYJ Modify, for Linux-64, va_list should be reset after calling vsnprintf
		va_list list;
		va_start(list,format);

		// some UNIX's do not support vsnprintf and snprintf
#ifdef _WIN32
		len = _vsnprintf(MaxBuf, len, format, (char*)(&format + 1));
#else		//	for Linux
		len = vsnprintf(MaxBuf, len, format, list );
#endif // _WIN32
		va_end(list);

		if( len > 0 && len < nBufMaxLen )
        	break;
		delete []MaxBuf;
		if (len == 0)
			return 0;
	}

	ASSERT( len == (int)strlen(MaxBuf) );
	m_nLength = len;
	GetBuffer( m_nLength + 1 );
	if( m_pString )
		strcpy(m_pString, MaxBuf);
	else
		len = 0;
	delete []MaxBuf;

	return len;
}

bool CMyString::Match(char* Wildcards, char* str)
{
	bool Yes = 1;

	//iterate and delete '?' and '*' one by one
	while(*Wildcards != '\0' && Yes && *str != '\0')
	{
		if (*Wildcards == '?') str ++;
		else if (*Wildcards == '*')
		{
			Yes = Scan(Wildcards, str);
			Wildcards --;
		}
		else
		{
			Yes = (*Wildcards == *str);
			str ++;
		}
		Wildcards ++;
	}
	while (*Wildcards == '*' && Yes)  Wildcards ++;

	return Yes && *str == '\0' && *Wildcards == '\0';
}

// scan '?' and '*'
bool CMyString::Scan(char*& Wildcards, char*& str)
{
	// remove the '?' and '*'
	for(Wildcards ++; *str != '\0' && (*Wildcards == '?' || *Wildcards == '*'); Wildcards ++)
		if (*Wildcards == '?') str ++;
	while ( *Wildcards == '*') Wildcards ++;

	// if str is empty and Wildcards has more characters or,
	// Wildcards is empty, return
	if (*str == '\0' && *Wildcards != '\0') return false;
	if (*str == '\0' && *Wildcards == '\0')	return true;
	// else search substring
	else
	{
		char* wdsCopy = Wildcards;
		char* strCopy = str;
		bool  Yes     = 1;
		do
		{
			if (!Match(Wildcards, str))	strCopy ++;
			Wildcards = wdsCopy;
			str		  = strCopy;
			while ((*Wildcards != *str) && (*str != '\0')) str ++;
			wdsCopy = Wildcards;
			strCopy = str;
		}while ((*str != '\0') ? !Match(Wildcards, str) : (Yes = false) != false);

		if (*str == '\0' && *Wildcards == '\0')	return true;

		return Yes;
	}
}

bool CMyString::NumericParse(void* pvar, char flag)
{
#ifndef __FOR_MICROWIN_EMBED__
	ASSERT(m_pString != 0);
	char* pTmpStr = m_pString;

	// remove the leading ' ' and '\t' at the beginning
	while (*pTmpStr == ' ' || *pTmpStr == '\t')
		pTmpStr++;

	// no desired character found
	if (strlen(pTmpStr) == 0)
		return false;

	char a = pTmpStr[0];
	if ((flag == 'b' || flag == 'C' || flag == 'S' ||
		flag == 'I' || flag == 'L') && a == '-')
		return false;

	if (flag == 'b')
	{
		bool var;
		if (strcmp(pTmpStr, "true") == 0 || strcmp(pTmpStr, "1") == 0 ||
			strcmp(pTmpStr, "TRUE") == 0) var = true;
		else if (strcmp(pTmpStr, "false") == 0 || strcmp(pTmpStr, "0") == 0 ||
			strcmp(pTmpStr, "FALSE") == 0) var = false;
		else // failed
			return false;
		memcpy(pvar, &var, sizeof(bool));
		return true;
	}
	else
	{
		double tmpvar = strtod(pTmpStr, (char**)&pTmpStr);
		if (tmpvar == 0.0 && a != '0')
			return false;   // convertion wrong

		if (flag == 'f' || flag == 'd')
		{
			// allow any float value with one 'f' or 'F' terminated
			if (*pTmpStr == 'f' || *pTmpStr == 'F')
				pTmpStr++;
		}
		else if (flag == 'l' || flag == 'L')
		{
			// allow any float value with one 'l' or 'L terminated
			if (*pTmpStr == 'l' || *pTmpStr == 'L')
				pTmpStr++;
		}

		switch(flag)
		{
		case 'c':
			{
				//if (tmpvar < -(0xff / 2 + 1) || tmpvar > 0xff / 2)
				if (tmpvar < -128 || tmpvar > 127)
					return false;   // onerflow
				char var = (char)tmpvar;
				memcpy(pvar, &var, sizeof(char));
			}
			break;
		case 's':
			{
				//if (tmpvar < -(0xffff / 2 + 1) || tmpvar > 0xffff / 2)
				if (tmpvar < -32768.0 || tmpvar > 32768.0)
					return false;   // onerflow
				short var = (short)tmpvar;
				memcpy(pvar, &var, sizeof(short));
			}
			break;
		case 'i':
			{
				//if (tmpvar < -(0xffffffff / 2 + 1) || tmpvar > 0xffffffff / 2)
				if (tmpvar < -2147483648.0 || tmpvar > 2147483647.0)
					return false;   // onerflow
				int var = (int)tmpvar;
				memcpy(pvar, &var, sizeof(int));
			}
			break;
		case 'l':
			{
				//if (tmpvar < -(0xffffffff / 2 + 1) || tmpvar > 0xffffffff / 2)
				if (tmpvar < -2147483648.0 || tmpvar > 2147483647.0)
					return false;   // onerflow
				long var = (long)tmpvar;
				memcpy(pvar, &var, sizeof(long));

			}
			break;
		case 'C':
			{
				//if (tmpvar < 0 || tmpvar > 0xff)
				if (tmpvar < 0.0 || tmpvar > 255)
					return false;   // onerflow
				unsigned char var = (unsigned char)tmpvar;
				memcpy(pvar, &var, sizeof(unsigned char));
			}
			break;
		case 'S':
			{
				//if (tmpvar < 0 || tmpvar > 0xffff)
				if (tmpvar < 0.0 || tmpvar > 65535.0)
					return false;   // onerflow
				unsigned short var = (unsigned short)tmpvar;
				memcpy(pvar, &var, sizeof(unsigned short));
			}
			break;
		case 'I':
			{
				//if (tmpvar < 0 || tmpvar > 0xffffffff)
				if (tmpvar < 0.0 || tmpvar > 4294967295.0)
					return false;   // onerflow
				unsigned int var = (unsigned int)tmpvar;
				memcpy(pvar, &var, sizeof(unsigned int));
			}
			break;
		case 'L':
			{
				//if (tmpvar < 0 || tmpvar > 0xffffffff)
				if (tmpvar < 0.0 || tmpvar > 4294967295.0)
					return false;   // onerflow
				unsigned long var = (unsigned long)tmpvar;
				memcpy(pvar, &var, sizeof(unsigned long));
			}
			break;
		case 'f':
			{
				if (tmpvar < -3.402823466e+38 || tmpvar > 3.402823466e+38)
					return false;   // onerflow
				float var = (float)tmpvar;
                            memcpy( pvar, &var, sizeof(float) );
			}
			break;
		case 'd':
			memcpy(pvar, &tmpvar, sizeof(double));
			break;
		}

		// remove the leading ' ' and '\t' at the end
		while (*pTmpStr == ' ' || *pTmpStr == '\t')
			pTmpStr++;

		if (*pTmpStr != '\0')
			return false;   // non digital character detected

		return true;
	}
#endif //__FOR_MICROWIN_EMBED__
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		整理左边的字符
/// 入口参数：
///		无
/// 返回参数：
///		无
void CMyString::TrimLeft()
{
	if(NULL == m_pString || m_pString == &m_NilString )
		return;
	const char* lpsz = m_pString;

#if 0		// 2010.4.6 Modify
	#ifdef _WIN32
		while (_istspace(*lpsz))
			lpsz = _tcsinc(lpsz);
	#else		//	Linux
		while( isspace(*lpsz) )
			lpsz ++;
	#endif // _WIN32
#else
	while( *lpsz && (*(PBYTE)lpsz) <= 0x20 )
		lpsz ++;
#endif //

	if (lpsz != m_pString)
	{
		// fix up data and length
		int nDataLength = m_nLength - (lpsz - m_pString);
#ifdef _WIN32
		memmove( m_pString, lpsz, (nDataLength+1)*sizeof(TCHAR) );
#else		//	Linux
		memmove( m_pString, lpsz, nDataLength+1 );
#endif // _WIN32
		m_nLength = nDataLength;
	}
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		整理右边的字符
/// 入口参数：
///		无
/// 返回参数：
///		无
void CMyString::TrimRight()
{
	if( m_pString == &m_NilString || NULL == m_pString )
    	return;

	char* lpsz = m_pString;
	char* lpszLast = NULL;

	while (*lpsz != '\0')
	{
#ifdef _WIN32
		if( _istspace(*lpsz) )
#else
		if( isspace(*lpsz) )
#endif // _WIN32
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
#ifdef _WIN32
		lpsz = _tcsinc(lpsz);
#else
		lpsz ++;
#endif // _WIN32
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		m_nLength = lpszLast - m_pString;
	}
}

void CMyString::MakeUpper()
{
	ASSERT( m_pString );
	if( m_pString == &m_NilString || NULL == m_pString )
    	return;
	if( m_pString )
    {
#ifdef _WIN32
		strupr( m_pString );
#else
		for(int i=0; i<m_nLength; i++)
        {
			m_pString[i] = toupper( m_pString[i] );
        }
#endif //_WIN32
    }
}

void CMyString::MakeLower()
{
	ASSERT( m_pString );
   	if( m_pString == &m_NilString || NULL == m_pString )
    	return;
	if( m_pString )
    {
#ifdef _WIN32
		strlwr( m_pString );
#else
		for(int i=0; i<m_nLength; i++)
        {
			m_pString[i] = tolower( m_pString[i] );
        }
#endif //_WIN32
    }
}

bool CMyString::IsEmpty()
{
	return ( NULL == m_pString || 0 == m_nLength || m_pString == &m_NilString );
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		获取指定位置的字符
/// 入口参数：
///		无
/// 返回参数：
///		无
char CMyString::GetAt(int nIndex) const
{
	ASSERT( nIndex >= 0 && nIndex < m_nLength );
	ASSERT( m_pString );
	if( nIndex < 0 || nIndex >= m_nLength )
		return 0;
	if( NULL == m_pString )
		return 0;
	return m_pString[ nIndex ];
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		设置字符
/// 入口参数：
///		nIndex			位置
///		newChar			新的字符
/// 返回参数：
///		无
void CMyString::SetAt(int nIndex, char newChar)
{
	ASSERT( nIndex >= 0 && nIndex < m_nLength );
	ASSERT( m_pString );
	if( nIndex < 0 || nIndex >= m_nLength || NULL == m_pString || m_pString == &m_NilString )
		return;

	m_pString[ nIndex ] = newChar;
}

int CMyString::GetLength() const
{
#ifdef _DEBUG
	if( NULL == m_pString )
		ASSERT( 0 == m_nLength );
#endif //_DEBUG
	return m_nLength;
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		不区分大小写进行比较
/// 入口参数：
///		pszValue		待比较的数据
/// 返回参数：
///		>0				大于
///		=0				相同
///		<0				小于
int CMyString::CompareNoCase(const char *pszValue) const
{
	ASSERT( pszValue && m_pString );
	if( NULL == pszValue || NULL == m_pString )
		return -1;
#ifdef _WIN32
	return stricmp( m_pString, pszValue );
#else		// Linux
	return strcasecmp( m_pString , pszValue );
#endif //_WIN32
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		大小写敏感的字符产比较
/// 入口参数：
///		pszValue		待比较的数据
/// 返回参数：
///		>0				大于
///		=0				相同
///		<0				小于
int CMyString::Compare(const char *pszValue) const
{
	ASSERT( pszValue && m_pString );
	if( NULL == pszValue || NULL == m_pString )
		return -1;
	return strcmp( m_pString, pszValue );
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		Find the offset of One Char
/// 入口参数：
///		DstChar			char to be found
///		nStart			Start position, default is 0
/// 返回参数：
///		>=0				offset of the char
///		<0				not found
int CMyString::Find(char DstChar, int nStart)
{
	ASSERT( m_pString );
	ASSERT( nStart >= 0 && nStart < m_nLength );
	if( nStart < 0 )
		nStart = 0;
	if( NULL == m_pString || nStart >= m_nLength )
		return -1;
	char * pszFound = strchr( m_pString+nStart, DstChar );
	if( NULL == pszFound )
		return -1;
	return pszFound - m_pString;
}

///-------------------------------------------------------
/// CYJ,2003-7-23
/// 功能：
///		Find the offset of One Char
/// 入口参数：
///		lpszDstString			char to be found
///		nStart			Start position, default is 0
/// 返回参数：
///		>=0				offset of the char
///		<0				not found
int CMyString::Find(const char * lpszDstString, int nStart)
{
	ASSERT( m_pString && lpszDstString );
	if( NULL == m_pString || NULL == lpszDstString )
		return -1;
	if( nStart < 0 )
		nStart = 0;
	if( nStart >= m_nLength )
		return -1;
	char * pszFound = strstr( m_pString+nStart, lpszDstString );
	if( NULL == pszFound )
		return -1;
	return pszFound - m_pString;
}

///-------------------------------------------------------
/// CYJ,2003-7-24
/// Function:
///		Get Left side string
/// Input parameter:
///		nCount		character count
/// Output parameter:
///		New String
CMyString CMyString::Left(int nCount)
{
	CMyString strRetVal;
	ASSERT( nCount >= 0 );
    if( nCount > m_nLength )
    	nCount = m_nLength;
	if( nCount <= 0 )
		return strRetVal;

	char * pBuf = strRetVal.GetBuffer( nCount + 1 );
	if( pBuf )
	{
		strncpy( pBuf, m_pString, nCount );
		pBuf[nCount] = 0;
		strRetVal.m_nLength = nCount;
	}
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2003-7-24
/// Function:
///		Allocate memory
/// Input parameter:
///		None
/// Output parameter:
///		None
char * CMyString::GetBuffer(int nSize)
{
	if( m_nBufSize && nSize <= m_nBufSize )
	{
		ASSERT( m_pString );
		return m_pString;
	}
	nSize = (nSize + 31) & (~31);	//	align by 32 bytes
	char * pszNewBuf = new char[nSize];
	if( NULL == pszNewBuf )
		return NULL;
    *pszNewBuf = 0;

	int nCountToMove = (nSize<m_nBufSize) ? nSize : m_nBufSize;
	m_nBufSize = nSize;				//	使用新的长度

	if( m_pString && m_pString != &m_NilString )
	{
		memmove( pszNewBuf, m_pString, nCountToMove );
		delete m_pString;
	}

	m_pString = pszNewBuf;
	return m_pString;
}

///-------------------------------------------------------
/// CYJ,2003-7-24
/// Function:
///		Release Buffer and set new length
/// Input parameter:
///		nNewLen		new string len, default is -1
/// Output parameter:
///		None
void CMyString::ReleaseBuffer(int nNewLen)
{
	if( NULL == m_pString || m_pString == &m_NilString )
		return;
	if( nNewLen < m_nBufSize )
	{
		if( nNewLen < 0 )
			nNewLen = strlen( m_pString );
		m_nLength = nNewLen;
		m_pString[nNewLen] = 0;
	}
}

//------------------------------------------
// Function:
//		Delete some char
// Input Parameter:
//		nIndex		the positon te be deleted
//		nCount		char count to delete
// Output Parameter:
//		none
void CMyString::Delete( int nIndex, int nCount /*=1*/ )
{
	ASSERT( m_pString );
	if( NULL == m_pString )
    	return;
    if( nIndex < 0 )
    	nIndex = 0;

    if( nCount < 0 || nIndex >= m_nLength )
    	return;
    int  nBytesToCopy = m_nLength - (nIndex + nCount ) + 1;
    ASSERT( nBytesToCopy > 0 );
    memmove( m_pString + nIndex, m_pString+nIndex+nCount, nBytesToCopy );

    m_nLength = nBytesToCopy + nIndex - 1;
    ASSERT( m_nLength == (int)strlen(m_pString) );
}

//------------------------------------------
// Function:
//		Replace char
// Input Parameter:
//		cOld		the char to be replaced
//		cNew		new char
// Output Parameter:
//
void CMyString::Replace( char cOld, char cNew )
{
    if( cOld == cNew || 0 == m_nLength )
    	return;
//    ASSERT( PBYTE(m_pString) != PBYTE(m_NilString) );
    char * pszBuf = m_pString;
    for(int i=0; i<m_nLength; i++)
    {
    	if( *pszBuf == cOld )
        	*pszBuf = cNew;
        pszBuf ++;
    }
}

//------------------------------------------
// Function:
//		Replace string
// Input Parameter:
//
// Output Parameter:
//
void CMyString::Replace( LPCSTR pszOld, LPCSTR pszNew )
{
    ASSERT( pszOld && pszNew );

	CMyString strTmp;
    LPCSTR lpszBuf = m_pString;
    LPCSTR lpszEnd = m_pString + m_nLength;
    ASSERT( m_nLength == (int)strlen(m_pString) );
    int nOldStringLen = (int)strlen(pszOld);

    char * pszStart;
    while( lpszBuf < lpszEnd )
    {
    	pszStart = (char*)strstr( lpszBuf,pszOld );
    	if( NULL == pszStart )
        {				// not found, not find any more
			strTmp += lpszBuf;
            break;
        }
        if( pszStart > lpszBuf )
        {				//  find
			*pszStart = 0;
            strTmp += lpszBuf;
        }
        strTmp += pszNew;
        lpszBuf = pszStart + nOldStringLen;
    }
	*this = strTmp;
}

//------------------------------------------
// Function:
//		Find One of char
// Input Parameter:
//		pszCharSet		one the char will be find
// Output Parameter:
//		position		 found
//		<0				 not found
int CMyString::FindOneOf( LPCSTR pszCharSet ) const
{
	ASSERT( pszCharSet );
    if( NULL == pszCharSet )
    	return -1;
    if( 0 == m_nLength )
    	return -1;
	char * pszRetVal = strpbrk(m_pString, pszCharSet );
    if( NULL == pszRetVal )
    	return -1;
    return (PBYTE)pszRetVal - (PBYTE)m_pString;
}

//------------------------------------------
// Function:
//		Copy Middle part string
// Input Parameter:
//		nFirst		postion
//		nCount		if < 0 the copy the remainder chars
// Output Parameter:
//		new string
CMyString CMyString::Mid( int nFirst, int nCount /* = -1*/ )
{
	CMyString strRetVal;
	if( nFirst < 0 )
    	nFirst = 0;
    if( nFirst >= m_nLength )
    	return strRetVal;

    if( nCount <= 0)
    	nCount = m_nLength - nFirst;

    char * pszBuf = strRetVal.GetBuffer( nCount + 1);
    if( NULL == pszBuf )
    	return strRetVal;

    strncpy( pszBuf, m_pString + nFirst, nCount );
    pszBuf[nCount] = 0;
    strRetVal.m_nLength = nCount;

	return strRetVal;
}

CMyString CMyString::Right( int nCount )
{
    return Mid( m_nLength-nCount, nCount );
}

//------------------------------------------
// Function:
//		Insert one char
// Input Parameter:
//
// Output Parameter:
//		>0		succ
//		<=0		failure
int CMyString::Insert( int nIndex, char NewChar )
{
	if (nIndex < 0)
		nIndex = 0;

	int nNewLength = m_nLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength++;

    if( NULL == GetBuffer(nNewLength+1))
    	return 0;

	// move existing bytes down
	memmove(m_pString + nIndex + 1,
		m_pString + nIndex, (nNewLength-nIndex) );
	m_pString[nIndex] = NewChar;
	m_nLength = nNewLength;

	return nNewLength;
}

//------------------------------------------
// Function:
//		Insert one new string
// Input Parameter:
//
// Output Parameter:
//
int CMyString::Insert( int nIndex, const char * pszNewStr )
{
	if (nIndex < 0)
		nIndex = 0;

	int nInsertLength = strlen(pszNewStr);
	int nNewLength = m_nLength;
    if( nInsertLength <= 0 )
    	return nNewLength;

    if (nIndex > nNewLength)
        nIndex = nNewLength;
    nNewLength += nInsertLength;

    if( NULL == GetBuffer( nNewLength + 1 ) )
    	return 0;
    // move existing bytes down
    memmove(m_pString + nIndex + nInsertLength,\
        m_pString + nIndex, (nNewLength-nIndex-nInsertLength+1));
    memcpy(m_pString + nIndex, pszNewStr, nInsertLength);
    m_nLength = nNewLength;

	return nNewLength;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		Remove char
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyString::Remove( char cChar )
{
    char * lpszBuf = GetBuffer( m_nLength );

    char * lpszEnd = lpszBuf + m_nLength;
	char * pszNewString = lpszBuf;
    while( lpszBuf <= lpszEnd )
    {
		if( *lpszBuf != cChar )
    		*pszNewString ++ = *lpszBuf;
		lpszBuf ++;
    }

	ReleaseBuffer();
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		reserv find
/// Input parameter:
///		None
/// Output parameter:
///		None
int CMyString::ReverseFind( char DstChar ) const
{
	if( NULL == m_pString || 0 == m_pString )
		return -1;
	char * pszFound = strrchr( m_pString, DstChar );
	if( NULL == pszFound )
		return -1;
	return pszFound - m_pString;
}
