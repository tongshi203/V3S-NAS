///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-4-4
///
///		用途：
///			位图管理辅助对象
///=======================================================

// IPFileMendHelper.cpp : Implementation of CIPFileMendHelper
#include "stdafx.h"
#include "IPRecSvr.h"
#include "IPFileMendHelper.h"

// 比特对应的 1 的个数
static BYTE g_BIT_1_COUNT_TBL[16]={ 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };

/////////////////////////////////////////////////////////////////////////////
// CIPFileMendHelper

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		获取已经成功接收的子文件个数
/// 入口参数：
///		pVal				输出个数
/// 返回参数：
///
long CIPFileMendHelper::GetSubFileHasReceived()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		获取子文件总数
/// 入口参数：
///
/// 返回参数：
///
long CIPFileMendHelper::GetTotalSubFileCount()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_nTotalBitCount;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		判断一个子文件是否成功接收，即 1 ＝ 成功接收，0 ＝ 失败或未曾接收
/// 入口参数：
///
/// 返回参数：
///
BOOL CIPFileMendHelper::GetIsSubFileOK( long nIndex )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nByteOfs = nIndex / 8;
	int nBitOfs = nIndex % 8;

	ASSERT( nByteOfs < m_BitmapArray.GetSize() );
	if( nByteOfs >= m_BitmapArray.GetSize() )
		return FALSE;

	BYTE byMask = 1 << nBitOfs;
	return ( m_BitmapArray[nByteOfs] & byMask );
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		获取DataBuffer for VC Only
/// 入口参数：
///
/// 返回参数：
///
PBYTE CIPFileMendHelper::GetDataBufferVC()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_BitmapArray.GetData();
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		设置子文件总数
/// 入口参数：
///		nNewValue			新的文件总数
/// 返回参数：
///
BOOL CIPFileMendHelper::SetTotalSubFileCount(long nNewValue)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nTotalBitCount = nNewValue;
	nNewValue = ( nNewValue + 7 ) / 8;
	TRY
	{
		if( nNewValue != m_BitmapArray.GetSize() )
			m_BitmapArray.SetSize( nNewValue, 4096 );		//	增长 4096 字节
	}
	CATCH_ALL( e )
	{
		m_BitmapArray.RemoveAll();
		m_nTotalBitCount = 0;
		return  FALSE;
	}
	END_CATCH_ALL

	return TRUE;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		设置一个数值
/// 入口参数：
///		nIndex					数组序号
///		nBitValue				新的值
///		pRetVal					修改后的已经成功接收的子文件个数
/// 返回参数：
///		<0						是不
long CIPFileMendHelper::SetBitValue( int nIndex,int nBitValue )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( nIndex >= m_nTotalBitCount )
	{								//	太大了，重新分配内存
		if( FALSE == SetTotalSubFileCount( nIndex+1 ) )
			return -1;
	}

	int nByteOfs = nIndex / 8;
	int nBitOfs = nIndex % 8;
	ASSERT( nByteOfs < m_BitmapArray.GetSize() );
	BYTE byMask = ( 1 << nBitOfs );

	BYTE & byValue = m_BitmapArray[nByteOfs];

	if( nBitValue )
	{
		if( 0 == (byValue&byMask) )
			m_nBit_1_Count ++;			//	新增加的数据
		ASSERT( m_nBit_1_Count <= m_nTotalBitCount );
		byValue |= byMask;
	}
	else
	{
		if( byValue&byMask )
			m_nBit_1_Count --;			//	新增加的数据
		ASSERT( m_nBit_1_Count >= 0 );
		byValue &= (~byMask);
	}

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		为 GetNextFile 作准备
/// 入口参数：
///
/// 返回参数：
///
void CIPFileMendHelper::Prepare()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nNextFileIDPtr = 0;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		获取下个满足条件的数值
/// 入口参数：
///		nBitValue			比特数值
///		pRetVal				输出数组序号
/// 返回参数：
///
long CIPFileMendHelper::GetNextFileID( int nBitValue )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	while( m_nNextFileIDPtr < m_nTotalBitCount )
	{
		int nByteOfs = m_nNextFileIDPtr / 8;
		int nBitOfs = m_nNextFileIDPtr % 8;
		m_nNextFileIDPtr ++;

		BYTE byMask = (1 << nBitOfs );
		BYTE byResult = m_BitmapArray[nByteOfs] & byMask;
		if( (nBitValue&&byResult) || (0==nBitValue && 0 ==byResult) )
			return m_nNextFileIDPtr-1;
	}

	return -1;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		重新统计已经成功接收的子文件个数
/// 入口参数：
///		pRetVal				输出已经成功接收的文件个数
/// 返回参数：
///
long CIPFileMendHelper::ReStat()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nBit_1_Count = 0;

	int nCount = m_BitmapArray.GetSize();
	PBYTE pBuf = m_BitmapArray.GetData();
	for(int i=0; i<nCount; i++)
	{
		m_nBit_1_Count += g_BIT_1_COUNT_TBL[ (*pBuf) & 0xF ];
		m_nBit_1_Count += g_BIT_1_COUNT_TBL[ (*pBuf) >> 4 ];
		pBuf ++;
	}

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		合并
/// 入口参数：
///		pSrcObj				合并两个位图，采用 OR 方法
///		pRetVal				输出已经成功接收的个数
/// 返回参数：
///		<0					failed
long CIPFileMendHelper::Combine( IIPFileMendHelper *pSrcObj )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	PBYTE pSrcBuf = NULL;
	long nSrcCount = 0;
	TRY
	{
		pSrcBuf = pSrcObj->GetDataBufferVC();
		if( NULL == pSrcBuf )
			return -1;
		nSrcCount = pSrcObj->GetTotalSubFileCount();
		if( nSrcCount < 0 )
			return -1;
		int nCount = m_nTotalBitCount;
		if( nCount > nSrcCount )
			nCount = nSrcCount;
		int nByteCount = (nCount+7) / 8;
		PBYTE pDstBuf = m_BitmapArray.GetData();
		for(int i=0; i<nByteCount; i++)
		{
			*pDstBuf |= *pSrcBuf;
			pDstBuf ++;
			pSrcBuf ++;
		}
		return ReStat( );
	}
	CATCH_ALL( e )
	{
		return -1;
	}
	END_CATCH_ALL

	return -1;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///		从文件读取数据
/// 入口参数：
///		bstrFileName		文件名
///		pRetVal				返回已经成功读取的子文件个数
/// 返回参数：
///		<0					failed
BOOL CIPFileMendHelper::LoadFromFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nTotalBitCount = 0;
	m_nBit_1_Count = 0;
	m_BitmapArray.RemoveAll();

	CString strFileName = lpszFileName;
	CFile f;
	if( FALSE == f.Open( strFileName, CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite ) )
		return FALSE;

	TRY
	{
		int nLen = f.GetLength();
		if( nLen < 4 )					//	文件长度不对
			return FALSE;

		if( sizeof(int) != f.Read( &m_nTotalBitCount, sizeof(int) ) || m_nTotalBitCount <= 0 )
		{
			m_nTotalBitCount = 0;
			return FALSE;				//	错误
		}

		int nByteCount = (m_nTotalBitCount + 7 )/8;
		m_BitmapArray.SetSize( nByteCount, 4096 );
		if( nByteCount != (int) f.Read( m_BitmapArray.GetData(), nByteCount ) )
		{
			m_nTotalBitCount = 0;
			m_BitmapArray.RemoveAll();
			return FALSE;
		}

		long nTotalCount = ReStat();						//	重新统计
		ASSERT( nTotalCount == m_nBit_1_Count );
		return TRUE;
	}
	CATCH_ALL( e )
	{
		m_nTotalBitCount = 0;
		m_BitmapArray.RemoveAll();
	}
	END_CATCH_ALL

	return FALSE;
}

///-------------------------------------------------------
/// 2003-4-4
/// 功能：
///
/// 入口参数：
///
/// 返回参数：
///
BOOL CIPFileMendHelper::SaveToFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CString strFileName = lpszFileName;
	CFile f;
	if( FALSE == f.Open( strFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite ) )
		return FALSE;

	TRY
	{
		f.Write( &m_nTotalBitCount, sizeof(int) );
		if( m_nTotalBitCount )
		{
			ASSERT( m_BitmapArray.GetSize() == ((m_nTotalBitCount+7)/8) );
			f.Write( m_BitmapArray.GetData(), m_BitmapArray.GetSize() );
		}
		f.Close();
	}
	CATCH_ALL( e )
	{
	}
	END_CATCH_ALL

	return TRUE;
}

///-------------------------------------------------------
/// 2003-4-5
/// 功能：
///		复制一个新的对象
/// 入口参数：
///
/// 返回参数：
///
IIPFileMendHelper * CIPFileMendHelper::Clone()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CIPFileMendHelper * pInstance = new CIPFileMendHelper;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();

	pInstance->m_BitmapArray.RemoveAll();
	pInstance->m_BitmapArray.Copy( m_BitmapArray );
	pInstance->m_nBit_1_Count = m_nBit_1_Count;
	pInstance->m_nTotalBitCount = m_nTotalBitCount;

	return static_cast<IIPFileMendHelper*>(pInstance);

}
