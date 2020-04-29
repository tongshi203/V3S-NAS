///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-14
///
///=======================================================

// MB_OneFile.cpp: implementation of the CMB_OneFile class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14	添加 SetMulticastParameter，添加成员变量 m_strMC_DstIP 和 m_wMC_Port，
//				用来表示与之相关的多播参数
//  2002.4.30	修改 CollectDataUseXorChksum，当错误页数超过范围时，放弃纠错

#ifndef _WIN32
#include <stdio.h>
#endif //_WWIN32

#include "stdafx.h"
#include "MB_OneFile.h"
#include "TSDVBBROPROTOCOL.H"
#include <stdio.h>
#include "IPData.h"

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

CMB_OneFile::CMB_OneFile() :
	CBufPacket4C<IBufPacket>( 0, 4096 )			//	以 4K 为单位进行分配
{
	m_nXorChkSumDataLen = 0;					//	没有校验数据
#ifdef _WIN32
	RtlZeroMemory( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	RtlZeroMemory( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#else
	bzero( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	bzero( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#endif //_WIN32
	m_wMC_Port = 0;						//	2002.11.14 添加，多播端口
}

CMB_OneFile::~CMB_OneFile()
{
}

void CMB_OneFile::SafeDelete()
{
	delete this;
}

//	初始化
//	入口参数
//		chFile				通道号
//		lpszFileName		文件名
//		dwLen				长度
//		FileTime			文件时间
//	返回参数
//		TRUE				成功
//		FALSE				失败
BOOL CMB_OneFile::Initialize(TSDBCHANNEL chFile, LPCSTR lpszFileName, DWORD dwLen, time_t FileTime)
{
	ASSERT( lpszFileName && dwLen );
	ASSERT( dwLen > 0 && dwLen < 1024*512 );		//	< 512K

	m_dwFileLen = 0;
	m_dwByteRead = 0;
	m_nXorChkSumDataLen = 0;

#ifdef _WIN32
	RtlZeroMemory( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	RtlZeroMemory( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#else
	bzero( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	bzero( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#endif //_WIN32
	if( FALSE == SetBufSize( dwLen ) )
		return FALSE;
	PutDataLen( dwLen );

	m_dwFileLen = dwLen;
	m_Time = FileTime;	

	m_chFile.m_dwData = chFile.m_dwData;
	if( lpszFileName[0] )
		strncpy( m_szFileName, lpszFileName, 13 );
	else
		m_szFileName[0] = 0;

	m_strMC_DstIP = "";					//	2002.11.14 添加，多播IP地址
	m_wMC_Port = 0;						//	2002.11.14 添加，多播端口

	return TRUE;
}

//	判断文件是否改变
BOOL CMB_OneFile::IsFileChanged(time_t t,DWORD dwFileLen)
{
	ASSERT( GetBuffer() );
	if( dwFileLen != m_dwFileLen || m_Time != t )
		return TRUE;
	return FALSE;
}

//	添加一页
//	入口参数
//		pBuf				缓冲区地址，参见 PTSDVBMULTICASTPACKET0
//		dwLen				缓冲区长度
//	返回参数
//		MBROF_DATA_ERR		数据有错误
//		MBROF_FILE_CHANGED	文件改变
//		MBROF_DATAOK_FILENOTOK	成功，但文件没有收齐
//		MBROF_FILE_OK		整个文件接收 OK
int CMB_OneFile::AddOnePage(PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pBuf && dwLen && GetBuffer() );
	PTSDVBMULTICASTPACKET0 pHeader = (PTSDVBMULTICASTPACKET0)pBuf;
#ifdef _DEBUG
	ASSERT( pHeader->m_cbSize < dwLen );
	if( CCRC::GetCRC32(pHeader->m_cbSize-offsetof(TSDVBMULTICASTPACKET0,m_PacketTime),\
		(PBYTE)&pHeader->m_PacketTime ) != pHeader->m_dwHeaderCRC32 )
	{
		ASSERT( FALSE );
		return MBROF_DATA_ERR;
	}
	if( IsFileChanged( pHeader->m_PacketTime, pHeader->m_dwFileLen ) )
	{
		ASSERT( FALSE );
		return MBROF_FILE_CHANGED;
	}
#endif // _DEBUG	

	ASSERT( pHeader->m_dwFileLen == m_dwFileLen );

	PBYTE pPageBuf = pBuf + pHeader->m_cbSize;
	BOOL bIsPageOK = CCRC::GetCRC32( pHeader->m_wPageLen, pPageBuf ) == pHeader->m_dwPageCRC32;
	
	DWORD dwOfsInFile = pHeader->GetPageOfsInFile();
	if( dwOfsInFile >= pHeader->GetFileLen() )
	{											//	XOR 纵向校验
		ASSERT( 0 == m_nXorChkSumDataLen );
		m_nXorChkSumDataLen = pHeader->GetPageLen();
		if( m_nXorChkSumDataLen > XORCHKSUMBUFLEN )
			m_nXorChkSumDataLen = 0;			//	太大，放弃
		else
			memcpy( m_abyXorChkSum, pPageBuf, m_nXorChkSumDataLen );
		ASSERT( 0 == m_dwByteRead );			//	一般第一页即校验数据, ????
		return MBROF_DATAOK_FILENOTOK;
	}

	memcpy( GetBuffer() + dwOfsInFile, pPageBuf, pHeader->m_wPageLen );
	if( bIsPageOK )
		m_dwByteRead += pHeader->m_wPageLen;
#ifdef _DEBUG
	else
		TRACE("Receive one page error \n");
#endif // _DEBUG
	ASSERT( m_dwByteRead <= m_dwFileLen );

	m_dwResultFlags.m_bIsReceived = TRUE;
	m_dwResultFlags.m_bIsFileErr |= bIsPageOK;
	WORD wPageLen = m_nXorChkSumDataLen;		//	若非0，则一定是一页的长度
	if( 0 == wPageLen )
		wPageLen = pHeader->GetPageLen();
	ASSERT( wPageLen );
	SetPageReceived( dwOfsInFile/wPageLen, bIsPageOK );
	
	if( m_dwByteRead >= m_dwFileLen )
		return MBROF_FILE_OK;
	else
		return MBROF_DATAOK_FILENOTOK;			//	还没有完成全部接收
}

void CMB_OneFile::SetPageReceived(int nPageNo, BOOL bIsErr)
{
	if( nPageNo >= PRS_MAX_PAGENUM )
		return;
	int nOfs = nPageNo >> 5;				//	32
	int dwMask = 1L << ( nPageNo&0x1f );	//	%32
	BOOL bRetVal = m_adwPageRecFlags[nOfs] & dwMask;		//	获取原来的状态

	m_adwPageRecFlags[nOfs] &= ~dwMask;						//	清除状态
	m_adwPageErrFlags[nOfs] &= ~dwMask;

	m_adwPageRecFlags[nOfs] |= dwMask;						//	设置接收标记
	if( bIsErr )
		m_adwPageErrFlags[nOfs] |= dwMask;					//	设置错误标志
}

//	2001.10.12	添加
//	用 XOR 校验纠错
//	返回参数
//		TRUE				成功纠错
//		FALSE				没有纠错
BOOL CMB_OneFile::CollectDataUseXorChksum()
{
	ASSERT( m_dwByteRead < m_dwFileLen && GetBuffer() );
	TRACE("One File is changed, %d/%d=   ",m_dwByteRead,m_dwFileLen  );
	if( (m_dwFileLen-m_dwByteRead) > (DWORD)m_nXorChkSumDataLen )	//	太多错误，或没有校验和
	{
		TRACE("Too many data lost %d/%d, can not collect ????\n",m_dwFileLen-m_dwByteRead, m_nXorChkSumDataLen );
		return FALSE;	
	}
	TRACE("Collect succ. +++++++++\n");

	ASSERT( m_nXorChkSumDataLen );							//	正好是一页的大小
	int nTotalPacket = (m_dwFileLen+m_nXorChkSumDataLen-1) / m_nXorChkSumDataLen;
	if( nTotalPacket > PRS_MAX_PAGENUM )
		nTotalPacket = PRS_MAX_PAGENUM;						//	最大记录的页数
	int nPTmp = (nTotalPacket+7)/8;
	PBYTE pRecFlag = (PBYTE)m_adwPageRecFlags;				//	判断错误的位置
	PBYTE pbyDstBuf = NULL;
	int nErrPacketNo;
    int i;
	for(i=0; i<nPTmp; i++, pRecFlag++ )
	{
		BYTE byFlag = *pRecFlag;
		if( 0xFF == byFlag )					
			continue;				//	正确
		BYTE byMask = 1;
        int j;
		for(j=0; j<8; j++)
		{
			if( 0 == (byMask&byFlag) )
				break;
			byMask <<= 1;
		}
		ASSERT( j < 8 );
		nErrPacketNo = i*8+j;
		if( nErrPacketNo >= nTotalPacket )
			break;							//	2002.4.30添加，超过错误范围
		ASSERT( nErrPacketNo*m_nXorChkSumDataLen < (int)m_dwFileLen );
		pbyDstBuf = GetBuffer() + nErrPacketNo*m_nXorChkSumDataLen;
		break;
	}
	if( NULL == pbyDstBuf )
		return FALSE;	

	for(i=0; i<nTotalPacket; i++)
	{
		if( i == nErrPacketNo )				//	已经错误，无需参与运算
			continue;
		PDWORD pdwSrc = (PDWORD)( GetBuffer() + i*m_nXorChkSumDataLen );
		PDWORD pdwDst = (PDWORD)m_abyXorChkSum;
		int nByteToXor;
		if( i==(nTotalPacket-1) ) 
		{
			nByteToXor = m_dwFileLen%m_nXorChkSumDataLen;
			if( 0 == nByteToXor )
				nByteToXor = m_nXorChkSumDataLen;
		}
		else
			nByteToXor = m_nXorChkSumDataLen;

		int nXorCount = nByteToXor >> 2;
        int j;
		for(j=0; j<nXorCount; j++)
		{
			*pdwDst ^= *pdwSrc;
			pdwDst ++;
			pdwSrc ++;
		}
		nXorCount = nByteToXor & 3;
		PBYTE pbySrc = (PBYTE)pdwSrc;
		PBYTE pbyDst = (PBYTE)pdwDst;
		for(j=0; j<nXorCount; j++)
		{
			*pbyDst ^= *pbySrc;
			pbyDst ++;
			pbySrc ++;
		}
	}

	int nByteToCopy = (nErrPacketNo==(nTotalPacket-1)) ? m_dwFileLen%m_nXorChkSumDataLen : m_nXorChkSumDataLen;
	if( 0 == nByteToCopy )
		nByteToCopy = m_nXorChkSumDataLen;
	memcpy( pbyDstBuf, m_abyXorChkSum, nByteToCopy );

	m_dwByteRead += nByteToCopy;	
	ASSERT( m_dwByteRead == m_dwFileLen );
	m_dwResultFlags.m_bIsReceived = TRUE;
	m_dwResultFlags.m_bIsFileErr = FALSE;			//	已纠正纠正
	SetPageReceived( nErrPacketNo, TRUE );
	return TRUE;
}

///-------------------------------------------------------
/// 2002-11-14
/// 功能：
///		设置与之相关的IP地址和端口
/// 入口参数：
///		lpszIP				lpszIP 地址
///		wPort				端口
///		pDataPortItem		data port item
/// 返回参数：
///		无
void CMB_OneFile::SetMulticastParameter(LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem )
{
	ASSERT( lpszIP && wPort && pDataPortItem );
	if( lpszIP )
		m_strMC_DstIP = lpszIP;
	else
		m_strMC_DstIP = "";
	m_wMC_Port = wPort;
	m_pDataPortItem = pDataPortItem;
}
