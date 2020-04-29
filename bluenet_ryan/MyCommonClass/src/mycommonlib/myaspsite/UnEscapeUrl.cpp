// UnEscapeUrl.cpp: implementation of the CUnEscapeUrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnEscapeUrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

//	解码
CMyString CUnEscapeUrl::Decode(const CMyString &strUrl, BOOL bQuery)
{
	CMyString strDecoded = strUrl;
	strDecoded.TrimLeft();
	strDecoded.TrimRight();
	if( strDecoded.IsEmpty() )
		return strDecoded;

	// special processing or query strings....
	if ( bQuery )
		strDecoded.Replace('+',' ');

	// first see if there are any %s to decode....
	if ( strDecoded.Find( '%' ) != -1 )
	{
		char * pszSrc;
		char * pszDst;
		int nLen = strDecoded.GetLength();
		pszSrc = strDecoded.GetBuffer( nLen );
		pszDst = pszSrc;
		// iterate through the string, changing %dd to special char....
		while( *pszSrc )
		{
			char ch = *pszSrc ++;
			if ( ch == '%' )
			{
				if ( *pszSrc == '%' )
				{								//	就是为了编码 %
					*pszDst = '%';				//	%
					pszSrc ++;
					pszDst ++;
					nLen --;					//	减少 1 个字符的编码	, %% ==> %
					continue;
				}
				else
				{
					WORD wWideChar = 0;
					BYTE ch1 = *pszSrc++;
					if( ch1 == 'u' || ch1 == 'U' )
					{							//	IE 新的编码, 直接使用WideChar编码,如: 我==>%u6211
						for(int i=0; i<4; i++)
						{
							ch1 = *pszSrc++;
							wWideChar <<= 4;
							wWideChar |= (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
						}
						wWideChar = WideToMultiByte( wWideChar );		//	转换成MultiBytes
						*((PWORD)pszDst) = wWideChar;
						pszDst += 2;
						nLen -= 4;
						continue;
					}
					else
					{
						*pszDst = DecodeOneByte( pszSrc-1 );
						pszSrc ++;
						pszDst ++;
						nLen -= 2;					
					}
				}
			}
			else
			{
				*pszDst = ch;				//	普通的数据
				pszDst ++;
			}
		}
		strDecoded.ReleaseBuffer( nLen );	//	释放缓冲区
		UpdateWideChar( strDecoded );
	}
	return strDecoded;
}

//	解码一个字节
//	入口参数
//		pszEsc				待解码的字符, 格式如: %EA
//	返回一个字节的数据
BYTE CUnEscapeUrl::DecodeOneByte(LPCSTR pszEsc)
{
	BYTE ch1 = *pszEsc++;
	ch1 = (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
	BYTE ch2 = *pszEsc;
	ch2 = (ch2 >= 'A') ? ((ch2&0xdf)-'A'+0xa) : (ch2-'0');	
	return ch1 = ch1*16 + ch2;
}

//	多字节
WORD CUnEscapeUrl::WideToMultiByte(WORD dwWideChar)
{
	return dwWideChar;
}

//	整理Wide Char to MultiChar
void CUnEscapeUrl::UpdateWideChar(CMyString &strUrl)
{
	int nLen = strUrl.GetLength();
	char * pszSrc = strUrl.GetBuffer( nLen+2 );
	pszSrc[nLen] = 0;					//	扩充部分清 0
	pszSrc[nLen+1] = 0;
	char * pszDst = pszSrc;
	BYTE ch = *pszSrc;
	while( ch )
	{
		ch = *pszSrc ++;
		if( ch < 0x80 )
		{								//	普通的字符
			*pszDst = ch;
			pszDst ++;
			continue;
		}
		else
		{
			if( (ch&0xF0)==0xE0 )
			{
				WORD wWideChar = ch;
				if( pszSrc[0] >= 0x60 && (pszSrc[0]&0xE0)<=0xA0 && pszSrc[1]>= 0x60 && (pszSrc[1]&0xC0)<=0xA0 )
				{							//	目前我已知的IE的编码方式
					wWideChar = ch<<12;
					wWideChar |= ( (pszSrc[0]&0x3F) << 6 );
					wWideChar |= ( pszSrc[1]&0x3F );
					wWideChar = WideToMultiByte( wWideChar );
					*((PWORD)pszDst) = wWideChar;
					pszDst += 2;
					pszSrc += 2;
					nLen --;
					continue;
				}
/*				else if( (pszSrc[0]&0xE0) == 0xC0 )
				{
					wWideChar = ((pszSrc[0]&0xF)<<6)|(pszSrc[1]&0x3f);
					wWideChar = WideToMultiByte( wWideChar );
					*((PWORD)pszDst) = wWideChar;
					pszDst += 2;
					pszSrc += 2;
					nLen --;
					continue;
				}
*/
			}
			*pszDst = ch;
			pszDst ++;
		}
	}
	strUrl.ReleaseBuffer( nLen );
}

//	解密
CMyString CUnEscapeUrl::Decode(LPCSTR lpszUrl, BOOL bQuery)
{
	CMyString strTmp = lpszUrl;
	return Decode( strTmp, bQuery );
}
