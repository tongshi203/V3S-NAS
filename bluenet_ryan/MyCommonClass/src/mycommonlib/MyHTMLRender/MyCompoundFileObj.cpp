///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-26
///
///		用途：
///			符合文档
///=======================================================

// MyCompoundFileObj.cpp: implementation of the CMyCompoundFileObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyCompoundFileObj.h"
#ifdef _DEBUG
	#include <stdio.h>
#endif //_DEBUG

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyCompoundFileObj::CMyCompoundFileObj()
{
	Detach();
}

CMyCompoundFileObj::~CMyCompoundFileObj()
{

}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置缓冲区，并解释其中的文件。
/// Input parameter:
///		pBuf				复合文件内存地址
///		nLen				长度
/// Output parameter:
///		true				succ
///		false				failed
///	Note:
///		在调用 Detach 之前，pBuf不能被释放
bool CMyCompoundFileObj::Attach(PBYTE pBuf, int nLen)
{
	Detach();
	m_nFileCount = *pBuf ++;
	PBYTE pEndBuf = pBuf + nLen;
	
	if( 0 == m_nFileCount )
		return false;
	for(int i=0; i<m_nFileCount; i++)
	{
		ONE_COMPOUND_FILE & OneFile = m_aFileObjs[i];
		ToLittleEndian( pBuf, PBYTE(&OneFile.m_nFileLen), 3 );
		pBuf += 3;
		ToLittleEndian( pBuf, PBYTE(&OneFile.m_nLastModifyTime), 4 );
		pBuf += (4+1);		//	4 bytes modify time and 1 byte reserved
		OneFile.m_pszFileName = (LPCSTR)(pBuf+1);
		pBuf += (*pBuf)+1;	//	1 bytes FileName and N bytes FileName Len
		OneFile.m_pDataBuf = pBuf;
		pBuf += OneFile.m_nFileLen;
		if( pBuf >= pEndBuf && i<(m_nFileCount-1))
			return false;	//	overflow check
	}
	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		解除绑定
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyCompoundFileObj::Detach()
{
	m_nFileCount = 0;
	memset( m_aFileObjs, 0, sizeof(m_aFileObjs) );
}

#ifdef _DEBUG
void CMyCompoundFileObj::Dump()
{
	TRACE("-------------------------------------------------\n");
	TRACE("There are %d files.\n", m_nFileCount);
	for(int i=0; i<m_nFileCount; i++)
	{
		TRACE("Sub-File %d: ", i+1);
		TRACE("Len=%d, FileName=%s, Data=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",\
			m_aFileObjs[i].m_nFileLen, m_aFileObjs[i].m_pszFileName, \
			m_aFileObjs[i].m_pDataBuf[0], m_aFileObjs[i].m_pDataBuf[1],\
			m_aFileObjs[i].m_pDataBuf[2], m_aFileObjs[i].m_pDataBuf[3],\
			m_aFileObjs[i].m_pDataBuf[4], m_aFileObjs[i].m_pDataBuf[5],\
			m_aFileObjs[i].m_pDataBuf[6], m_aFileObjs[i].m_pDataBuf[7] );
	}
}
#endif //_DEBUG

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		根据文件名查找文件
/// Input parameter:
///		lpszFileName		待查找到文件名
///		bNoCase				忽略大小，缺省为 true
/// Output parameter:
///		None
PONE_COMPOUND_FILE CMyCompoundFileObj::Find( LPCSTR lpszFileName, bool bNoCase )
{
	for(int i=0; i<m_nFileCount; i++)
	{
		if( bNoCase )
		{
#ifdef _WIN32
			if( 0 == stricmp( lpszFileName, m_aFileObjs[i].m_pszFileName ) )
				return &m_aFileObjs[i];
#else
			if( 0 == strcasecmp( lpszFileName, m_aFileObjs[i].m_pszFileName ) )
				return &m_aFileObjs[i];
#endif //_WIN32				
		}
		else if( 0 == strcmp( lpszFileName, m_aFileObjs[i].m_pszFileName ) )
				return &m_aFileObjs[i];
	}
	return NULL;
}
