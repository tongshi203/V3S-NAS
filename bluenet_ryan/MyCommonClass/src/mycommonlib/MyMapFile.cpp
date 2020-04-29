// MyMapFile.cpp: implementation of the CMyMapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyMapFile.h"
#include <sys/mman.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyMapFile::CMyMapFile()
{
	m_bReadOnly = TRUE;
	m_pMappedBuffer = NULL;
}

CMyMapFile::~CMyMapFile()
{
	Close();
}

///-----------------------------------------------------------
/// 功能：
///		打开映射功能
/// 入口参数：
///		lpszFileName			目标文件名，全路径，输入文件名为NULL，或""时，表示使用内存共享文件，此时 lpszShareName 不能为 0
///		lpszShareName			共享文件名，一般在 lpszFileName = NULL 时才使用
/// 返回参数：
///		TRUE					成功
///		FALSE					失败
BOOL CMyMapFile::MapFileForReadOnly(LPCSTR lpszFileName, LPCSTR lpszShareName)
{
	ASSERT( lpszFileName || lpszShareName );		//	其中必有一个不能为 NULL

	return MapFile( lpszFileName, lpszShareName, MAPFILE_MODE_READONLY );
}

///-------------------------------------------------------
/// CYJ,2003-10-25
/// Function:
///		Open File and map it, with user defined parameter
/// Input parameter:
///		lpszFileName		file name to be mapped
///		lpszShareName		share mapped file name
///		nMode				map mode
///		pbIsExist			output is exist or not, default is NULL
///		dwLowPos			low part 
///		dwHightPos			hight pos
/// Output parameter:
///		None
BOOL CMyMapFile::MapFile(LPCSTR lpszFileName, LPCSTR lpszShareName, int nMode, BOOL * pbIsExist, DWORD dwLowPos, DWORD dwHightPos)
{
	ASSERT( lpszFileName || lpszShareName );		//	其中必有一个不能为 NULL

	Close();

	DWORD dwOpenFileFlags = 0;
	DWORD dwMapFlags = 0;
	int   nMapProt = 0;
	if( MAPFILE_MODE_READONLY == nMode  )
	{									// read only
		dwOpenFileFlags = CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = TRUE;
		nMapProt = PROT_READ;
		dwMapFlags = MAP_SHARED;
	}
	else if( MAPFILE_MODE_WRITEONLY == nMode )
	{									//	write only
		dwOpenFileFlags = CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = FALSE;
		nMapProt = PROT_WRITE;
		dwMapFlags = MAP_SHARED;
	}
	else
	{									// read write
		dwOpenFileFlags = CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = FALSE;
		nMapProt = PROT_WRITE|PROT_READ;
		dwMapFlags = MAP_SHARED;
	}

	if( lpszFileName && strlen(lpszFileName) )
	{
		if( FALSE == m_file.Open( lpszFileName, dwOpenFileFlags ) )
			return FALSE;
		if( 0 == m_file.GetLength() )
		{
			m_file.Abort();
			return FALSE;
		}
	}
	else
		m_file.m_hFile = CFile::hFileNull;
 
	m_pMappedBuffer = (PBYTE) mmap( 0, m_file.GetLength(), nMapProt, dwMapFlags, m_file.m_hFile, 0 );
	if( NULL == m_pMappedBuffer )
	{
		Close();
		return FALSE;
	}	

	return TRUE;
}

///-----------------------------------------------------------
/// 功能：
///		判断是否有效
/// 入口参数：
///		无
/// 返回参数：
///		TRUE					有效，可以操作
///		FAKSE					无效
BOOL CMyMapFile::IsValid()
{
	return NULL != m_pMappedBuffer;
}

///-----------------------------------------------------------
/// 功能：
///		获取内存地址
/// 入口参数：
///		无
/// 返回参数：
///		内存地址
PBYTE CMyMapFile::GetBuffer()
{
	ASSERT( IsValid() );
	return m_pMappedBuffer;
}

///-----------------------------------------------------------
/// 功能：
///		关闭映射
/// 入口参数：
///		无
/// 返回参数：
///		无
void CMyMapFile::Close()
{
	if( m_pMappedBuffer )
		munmap( (void*)m_pMappedBuffer, GetFileLen() );
	m_pMappedBuffer = NULL;
	m_file.Abort();
}

///-----------------------------------------------------------
/// 功能：
///		获取文件长度
/// 入口参数：
///		无
/// 返回参数：
///		文件长度
DWORD CMyMapFile::GetFileLen()
{
	ASSERT( IsValid() );
	if( FALSE == IsValid() )
		return 0;
	return m_file.GetLength();
}

