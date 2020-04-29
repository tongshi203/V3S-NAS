// UnCompressObj.cpp: implementation of the CUnCompressObj class.
//
//////////////////////////////////////////////////////////////////////
//	2001.8.15	添加纯虚函数 FreeMemory，用于释放内存

#include "stdafx.h"
#include "UnCompressObj.h"

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

CUnCompressObj::CUnCompressObj()
{
	m_pTSDBCmpHead = NULL;
	m_pDstDataBuf = NULL;								//	目标数据
	m_nOutDataLen = 0;									//	输出缓冲区大小
}

CUnCompressObj::~CUnCompressObj()
{
	if( m_pDstDataBuf )
		delete m_pDstDataBuf ;
	m_pDstDataBuf = NULL;
}

//	解压其中一个文件
//	入口参数
//		nFileNo						文件子序号, 不应超过 GetFileNum() 的值
//		outfStatus					输出文件参数
//		pDstBuf						用户指定的输出缓冲区, 若 pDstBuf = NULL, 则自动分配缓冲区, 调用者必须删除该内存
//									缓冲区大小必须根据 GetFileInfo 获取的长度进行分配
//	返回参数
//		NULL						失败
//		若 pDstBuf != NULL, 则返回 pDstBuf
PBYTE CUnCompressObj::DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf)
{
	return NULL;
}

//	获取文件内容
//	入口参数
//		nFileNo						文件序号
//		outfStatus					输出文件状态
//	返回参数
//		文件长度
int CUnCompressObj::GetFileInfo(int nFileNo,CFileStatus & outfStatus)
{
	return 0;
}

//	获取压缩文件中的文件个数
//	返回参数
//		文件个数
int CUnCompressObj::GetFileNum()
{
	return 0;
}

//	将指定长度及缓冲区的数据附着到对象中
//	入口参数
//		nFileLen					文件长度
//		pBuf						数据缓冲区
//	返回参数
//		文件个数
int CUnCompressObj::Attach(int nFileLen,PBYTE pBuf)
{
	ASSERT( nFileLen && pBuf );
	m_pTSDBCmpHead = (PTSDBCOMPRESSHEAD) pBuf;
	m_pSrcDataBuf = GetDataBuf();
	m_nSrcDataLen = m_pTSDBCmpHead->m_dwFileLen;
	m_nDataRead = 0;
	return 1;
}

//	取压缩方法
DWORD CUnCompressObj::GetCompressMethod()
{
	return 0;
}

//	获取解压软件的版本号
int CUnCompressObj::GetDecoderVersion()
{
	return 0;
}

//	清除数据
void CUnCompressObj::Detach()
{
	m_pTSDBCmpHead = NULL;
}

//	获取压缩数据头
PTSDBCOMPRESSHEAD CUnCompressObj::GetHeader()
{
	ASSERT( m_pTSDBCmpHead );
	return m_pTSDBCmpHead ;
}

//	获取数据缓冲区
PBYTE CUnCompressObj::GetDataBuf()
{
	ASSERT( m_pTSDBCmpHead  );
	PBYTE pRet = (PBYTE)m_pTSDBCmpHead;
	pRet += m_pTSDBCmpHead->m_cbSize;
	return pRet;
}

//	设置输出缓冲区
//	入口参数
//		nDstBufLen						缓冲区大小
//		pBuffer							输出缓冲区
//	返回参数
//		无
//	注:
//		若 pBuffer = NULL, 则自动分配输出数据, 但调用者必须管理,释放该内存
void CUnCompressObj::SetDstBuffer(int nDstBufLen,PBYTE pBuffer)
{
	ASSERT( nDstBufLen );						//	除数缓冲区大小
	if( pBuffer == NULL )
		m_pDstDataBuf = new BYTE [ nDstBufLen ];
	else
		m_pDstDataBuf = pBuffer;				//	目标数据

	m_pOutDataBufPtr = m_pDstDataBuf;			//	输出的数据缓冲区指针
	m_nOutDataLen = 0;							//	实际输出缓冲区大小
	m_nDstBufSize = nDstBufLen;					//	输出缓冲区大小
}

//	获取输出缓冲区
//	入口参数
//		outfLen							输出输出文件长度
//	返回参数
//		输出缓冲区
PBYTE CUnCompressObj::ReleaseDstBuffer(long &outfLen)
{
	outfLen = m_nOutDataLen;	
	PBYTE pBuffer = m_pDstDataBuf;
	m_pDstDataBuf = NULL;
	return pBuffer;
}

//	读取 1 字节
//	返回参数
//		EOF								文件结束
//		其他							正常数据
int CUnCompressObj::ReadOneByte()
{
	ASSERT( m_pSrcDataBuf );
	if( m_nDataRead >= m_nSrcDataLen )
		return EOF;
	m_nDataRead ++;
	return *m_pSrcDataBuf++;
}

//	输出 1 字节
//	入口参数
//		byData							数据
void CUnCompressObj::OutputOneByte(BYTE byData)
{
	ASSERT( m_pOutDataBufPtr );
	if( m_nOutDataLen < m_nDstBufSize && m_pOutDataBufPtr )
	{
		*m_pOutDataBufPtr ++ = byData;
		m_nOutDataLen ++;
	}
}

//	数据指定字节数据
//	入口参数
//		pBuf							数据缓冲区
//		nCount							字节数
void CUnCompressObj::Write(PBYTE pBuf, int nCount)
{
	int nLen = m_nDstBufSize - m_nOutDataLen;
	ASSERT( nLen >= 0 );
	if( nLen == 0 || m_pOutDataBufPtr == NULL )
		return;
	if( nLen > nCount )
		nLen = nCount;
	memcpy( m_pOutDataBufPtr, pBuf, nLen );
	m_pOutDataBufPtr += nLen;
	m_nOutDataLen += nLen;
}

//	判断是否压缩
//	返回参数
//		TRUE							压缩
//		FALSE							不压缩
BOOL CUnCompressObj::IsCompress(int nLen, PBYTE pBuffer)
{
	ASSERT( nLen && pBuffer );
	PTSDBCOMPRESSHEAD pHead = (PTSDBCOMPRESSHEAD)pBuffer;
	if( pBuffer == NULL || nLen<sizeof(TSDBCOMPRESSHEAD) || nLen < pHead->m_cbSize )
		return FALSE;
	if( pHead->m_CLSID != CLSID_TSDBCOMPRESSHEADER )
		return FALSE;
	if( pHead->m_dwHeaderCRC32 != \
		CCRC::GetCRC32( pHead->m_cbSize - (sizeof(TSDBCOMPRESSHEAD)-offsetof(TSDBCOMPRESSHEAD,m_dwMethod)), (PBYTE)&pHead->m_dwMethod ) )
	{
		return FALSE;
	}
	return TRUE;
}
