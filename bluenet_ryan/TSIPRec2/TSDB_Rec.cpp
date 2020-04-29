// TSDBMultiFileHeader.cpp: implementation of the CTSDBMultiFileHeader class.
//
//////////////////////////////////////////////////////////////////////
// 2002.5.9		修改 IsFileHead 函数，添加参数 nBufLen 以防止 m_cbSize 太大以致内存越界
//

#include "stdafx.h"
#include "TSDB_Rec.h"
#include "crc.h"
#include <stddef.h>

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	多文件头
CTSDBMultiFileHeader::CTSDBMultiFileHeader()
{
	m_pHeader = NULL;
}

CTSDBMultiFileHeader::CTSDBMultiFileHeader(CTSDBMultiFileHeader &RefMulHead)
{
	m_pHeader = RefMulHead.m_pHeader;
}

CTSDBMultiFileHeader::CTSDBMultiFileHeader(PTSDBMULFILEHEAD pRefHead)
{
	ASSERT( pRefHead );
	m_pHeader = pRefHead;
}

CTSDBMultiFileHeader::~CTSDBMultiFileHeader()
{

}

//	判断是否为多文件数据头
BOOL CTSDBMultiFileHeader::IsMultiFileHeader()
{
	ASSERT( m_pHeader );
	return IsMultiFileHeader( m_pHeader );
}

//	判断是否多文件头
//	入口参数
//		pBuf						数据头缓冲区
BOOL CTSDBMultiFileHeader::IsMultiFileHeader(PBYTE pBuf)
{
	ASSERT( pBuf );
	return IsMultiFileHeader( (PTSDBMULFILEHEAD)pBuf );
}

//	判断是否多文件头
//	入口参数
//		pRefHead					数据头
BOOL CTSDBMultiFileHeader::IsMultiFileHeader(PTSDBMULFILEHEAD pRefHead)
{
	ASSERT( pRefHead );
	if( !pRefHead )
		return FALSE;
	if( pRefHead->m_CLSID != CLSID_TSDBMULFILEHEAD )		//	GUID 不同
		return FALSE;	
	if( pRefHead->m_cbSize >= MULFILEHEAD_MAXSIZE || pRefHead->m_cbSize < sizeof(TSDBMULFILEHEAD))	
		return FALSE;										//	文件头太大
	if( CCRC::GetCRC32( pRefHead->m_cbSize - offsetof(TSDBMULFILEHEAD,m_wVersion),\
		(PBYTE)&pRefHead->m_wVersion) != pRefHead->m_dwHeaderCRC32 )
	{
		return FALSE;
	}
	return TRUE;
}


//	取多文件中的包含的文件个数
int CTSDBMultiFileHeader::GetFileNum()
{
	ASSERT( m_pHeader && IsMultiFileHeader() );
	return( m_pHeader->m_cFileNum );
}

//	取指定序号的单个文件头
//	入口参数
//		nIndex				文件序号
//	返回参数
//		单个文件头
PTSDBFILEHEADER CTSDBMultiFileHeader::GetFileHeader(int nIndex)
{
	ASSERT( m_pHeader && nIndex < GetFileNum() );
	if( nIndex >= m_pHeader->m_cFileNum )
		return NULL;					//	失败
	return( (PTSDBFILEHEADER)((PBYTE)m_pHeader + GetFileOfs(nIndex)) );
}

//	取指定文件序号的文件头的偏移
//	入口参数
//		nIndex				文件序号
//	返回参数
//		单个文件头
int CTSDBMultiFileHeader::GetFileOfs(int nIndex)
{
	ASSERT( m_pHeader && nIndex < GetFileNum() );
	return m_pHeader->m_wFileDataOfs[nIndex];
}

//	取指定序号的单个文件头
//	入口参数
//		nIndex				文件序号
//	返回参数
//		单个文件头
TSDBFILEHEADER& CTSDBMultiFileHeader::operator []( int nIndex )
{
	return( *GetFileHeader(nIndex) );
}

CTSDBMultiFileHeader & CTSDBMultiFileHeader::operator=(CTSDBMultiFileHeader& RefMulHeader)
{
	m_pHeader = RefMulHeader.m_pHeader;
	return *this;
}

CTSDBMultiFileHeader & CTSDBMultiFileHeader::operator=(PTSDBMULFILEHEAD pRefMulHead)
{
	ASSERT( pRefMulHead );
	m_pHeader = pRefMulHead;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CTSDBFileHeader Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBFileHeader::CTSDBFileHeader()
{
	m_pHeader = NULL;
}

CTSDBFileHeader::~CTSDBFileHeader()
{

}

CTSDBFileHeader::CTSDBFileHeader(CTSDBFileHeader &refHeader)
{
	m_pHeader = refHeader.m_pHeader;
	ASSERT( IsFileHead() );
}

CTSDBFileHeader::CTSDBFileHeader(PTSDBFILEHEADER pRefHeader)
{
	ASSERT( pRefHeader );
	m_pHeader = pRefHeader;
	ASSERT( IsFileHead() );
}

CTSDBFileHeader::CTSDBFileHeader(TSDBFILEHEADER &RefHeader)
{
	m_pHeader = &RefHeader;
	ASSERT( IsFileHead() );
}

//	判断是否为文件头
BOOL CTSDBFileHeader::IsFileHead()
{
	ASSERT( m_pHeader );
	if( NULL == m_pHeader || m_pHeader->m_cbSize >= 20480 )
		return FALSE;											//	2002.5.9 添加，超过 20K 字节
#ifdef _WIN32
	if( ::IsBadReadPtr( PBYTE(m_pHeader) + m_pHeader->m_cbSize, 1 ) )	//	2002.5.9 添加，不可读
		return FALSE;
#endif//_WIN32        
	return IsFileHead( m_pHeader, m_pHeader->m_cbSize );		//	兼容，应该已经判断过
}

//	判断是否为文件头
BOOL CTSDBFileHeader::IsFileHead(PBYTE pBuf, int nBufLen )
{
	return IsFileHead( PTSDBFILEHEADER(pBuf), nBufLen );
}

//	判断是否为文件头
BOOL CTSDBFileHeader::IsFileHead(PTSDBFILEHEADER pHeader, int nBufLen)
{
	ASSERT( pHeader );
	if( pHeader == NULL )
		return FALSE;
	if( pHeader->m_CLSID != CLSID_TSDBFILEHEADER )
		return FALSE;
	if( pHeader->m_cbSize <= sizeof(TSDBFILEHEADER) || pHeader->m_cbSize > nBufLen ) //	2002.5.9 添加 nBufLen 判断
		return FALSE;
#ifdef _WIN32
	__try
	{
		if( pHeader->m_dwHeaderCRC32 != \
			CCRC::GetCRC32( pHeader->m_cbSize - offsetof(TSDBFILEHEADER,m_wVersion), (PBYTE)&pHeader->m_wVersion ) )
		{
			return FALSE;
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return FALSE;
	}
#else
	if( pHeader->m_dwHeaderCRC32 != \
		CCRC::GetCRC32( pHeader->m_cbSize - offsetof(TSDBFILEHEADER,m_wVersion), (PBYTE)&pHeader->m_wVersion ) )
    {
        return FALSE;
    }
#endif //_WIN32
	return TRUE;
}


//	取文件长度
int CTSDBFileHeader::GetFileLen()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_dwFileLen;
}

//	取文件名
//	若文件名长度 = 0, 则表示没有文件名
CString CTSDBFileHeader::GetFileName()
{
	CString	strRetVal;
	ASSERT( m_pHeader );
	int nLen = m_pHeader->m_cbFileNameLenCount;
	if( nLen )
	{							//	有文件名
		char * pBuf = strRetVal.GetBuffer( nLen+1 );
		memcpy( pBuf, ((PBYTE)m_pHeader) + sizeof(TSDBFILEHEADER), nLen );
		strRetVal.ReleaseBuffer( nLen );
	}
	return strRetVal;
}

//	判断是否为大文件
BOOL CTSDBFileHeader::IsHugeFile()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHugeFile;
}

//	是否有文件属性参数
BOOL CTSDBFileHeader::HasFileAttarib()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHasAttrib;
}

//	是否有 Socket 参数属性
BOOL CTSDBFileHeader::HasSocket()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bWinSock;
}

//	是否有 Multicast 参数属性
BOOL CTSDBFileHeader::HasMulticast()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHasMuiticast;
}

//	是否有附加数据
BOOL CTSDBFileHeader::HasExtData()
{
	ASSERT( m_pHeader );
	return ExtDataLen();
}

//	取附加数据长度
int CTSDBFileHeader::ExtDataLen()
{
	ASSERT( m_pHeader );
	int nLen = GetParamHeader( 32 );
	if( m_pHeader->m_cbSize <= nLen )
		return 0;
	return ( m_pHeader->m_cbSize - nLen );
}

//	拷贝附加数据
//	入口参数
//		pDstBuf				输出缓冲区
//		nBufSize			缓冲区大小
//	返回参数
//		实际拷贝的数据长度
int CTSDBFileHeader::CopyExtData(PBYTE pDstBuf, int nBufSize)
{
	int nLen = ExtDataLen();
	ASSERT( nLen >= 0 );
	if( nLen >= nBufSize )
		nLen = nBufSize;
	PBYTE pBuf = (PBYTE) m_pHeader;
	pBuf += m_pHeader->m_cbSize - nLen;
	memcpy( pDstBuf, pBuf, nLen );
	return nLen;
}

//	取数据缓冲区地址
PBYTE CTSDBFileHeader::GetDataBuf()
{
	ASSERT( m_pHeader );
	return ((PBYTE)m_pHeader) + m_pHeader->m_cbSize;
}

//	拷贝数据
//	入口参数
//		pDstBuf				输出缓冲区
//		nBufSize			缓冲区大小
//	返回参数
//		实际拷贝的数据长度
int CTSDBFileHeader::CopyData(PBYTE pDstBuf, int nBufSize)
{
	ASSERT( m_pHeader && pDstBuf );
	if( m_pHeader->m_dwFileLen < (DWORD)nBufSize )
		nBufSize = m_pHeader->m_dwFileLen;
	memcpy( pDstBuf, GetDataBuf(), nBufSize );
	return 0;
}

//	取大文件数据头
//	返回参数
//		NULL				失败
PTSDBHUGEFILEHEAD CTSDBFileHeader::GetHugeFileHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHugeFile == FALSE )
		return NULL;
	int nLen = sizeof( TSDBFILEHEADER ) + m_pHeader->m_cbFileNameLenCount;
	return (PTSDBHUGEFILEHEAD) ( ((PBYTE)m_pHeader) + nLen );
}

//	取文件属性数据头
//	返回参数
//		NULL				失败
PTSDBFILEATTRIBHEAD CTSDBFileHeader::GetFileAttribHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHasAttrib == FALSE )
		return FALSE;
	int nLen = GetParamHeader(1);
	return (PTSDBFILEATTRIBHEAD)( ((PBYTE)m_pHeader) + nLen );
}

//	取文件 Socket 数据头
//	返回参数
//		NULL				失败
PTSDBSOCKETHEAD CTSDBFileHeader::GetSocketHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bWinSock == FALSE )
		return NULL;
	int nLen = GetParamHeader(2);
	return (PTSDBSOCKETHEAD)(  ((PBYTE)m_pHeader) + nLen );
}

//	取多值传送数据头
//	返回参数
//		NULL				失败
PTSDBMULTICAST CTSDBFileHeader::GetMulticastHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHasMuiticast == FALSE )
		return FALSE;
	int nLen = GetParamHeader(3);
	return (PTSDBMULTICAST)( ((PBYTE)m_pHeader) + nLen );
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( CTSDBFileHeader & RefHead)
{
	m_pHeader = RefHead.m_pHeader;
	return *this;
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( PTSDBFILEHEADER pRefHeader )
{
	ASSERT( pRefHeader );
	m_pHeader = pRefHeader;
	return *this;
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( TSDBFILEHEADER & RefHeader )
{
	m_pHeader = &RefHeader;
	return *this;
}

//	取参数数据项偏移的字节偏移
//	入口参数
//		nBitOfs						偏移
//	返回参数
//		偏移字节
int CTSDBFileHeader::GetParamHeader(int nBitOfs)
{
	int nLen = sizeof( TSDBFILEHEADER ) + m_pHeader->m_cbFileNameLenCount;
	PBYTE pBuf = (PBYTE)m_pHeader;
	pBuf += nLen;
	register DWORD dwFlags = m_pHeader->m_dwFlags;
	for(int i=0; i<nBitOfs; i++)
	{
		if( dwFlags & 1 )
		{
			nLen += * ( (PWORD)pBuf );
			pBuf = nLen + ((PBYTE)m_pHeader);
		}
		dwFlags >>= 1;
	}
	ASSERT( nLen <= m_pHeader->m_cbSize );
	return nLen;
}
