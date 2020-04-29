///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-28
///
///=======================================================

// IPUnlockDrvMgr.cpp: implementation of the CIPUnlockDrvMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "iprecsvr.h"
#include "IPUnlockDrvMgr.h"
#include "IPEncryptDataStruct.h"
#include "CRC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPUnlockDrvMgr::CIPUnlockDrvMgr()
{

}

CIPUnlockDrvMgr::~CIPUnlockDrvMgr()
{
	POSITION pos = m_DrvBuf.GetStartPosition();
	while( pos )
	{
		int nKey;
		CIPUnlockDrvWrapper * pDrv = NULL;
		m_DrvBuf.GetNextAssoc( pos, nKey, pDrv );
		ASSERT( pDrv );
		if( pDrv )
			delete pDrv;
	}
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		获取解密驱动
/// 入口参数：
///		nDrvSN				驱动序号
/// 返回参数：
///		NULL				失败
///		其他				成功
CIPUnlockDrvWrapper * CIPUnlockDrvMgr::GetDrv(int nDrvSN)
{
	CIPUnlockDrvWrapper * pRetVal = NULL;
	if( m_DrvBuf.Lookup( nDrvSN, pRetVal ) )
	{
		ASSERT( pRetVal );
		return pRetVal;
	}

	if( FALSE == m_KeyMgr.m_bIsRcvIDReady )
		return NULL;						//	没有设置 RcvID，不能进行解密

	pRetVal = new CIPUnlockDrvWrapper;
	if( NULL == pRetVal )
		return NULL;
	if( FALSE == pRetVal->LoadDrv( nDrvSN ) )
	{
		delete pRetVal;
		return NULL;
	}

	if( FALSE == pRetVal->InitDrv( &m_KeyMgr ) )
	{
		delete pRetVal;
		return NULL;
	}

	CSingleLock	SynLock( &m_SynbObj, TRUE );	//	同步，以防止同时载入

	TRY
	{
		m_DrvBuf.SetAt( nDrvSN, pRetVal );
	}
	CATCH_ALL( e )
	{
		delete pRetVal;
		return NULL;
	}
	END_CATCH_ALL

	return pRetVal;
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		获取密码管理器
/// 入口参数：
///		无
/// 返回参数：
///		密码管理器
CIPEncryptKeyMgrImpl & CIPUnlockDrvMgr::GetKeyMgr()
{
	return m_KeyMgr;
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		解密数据
/// 入口参数：
///		pBuf				待解密的数据缓冲区
///		dwBufLen			缓冲区长度，
///							输入：缓冲区大小
///							输出：加密后有效数据的大小
/// 返回参数：
///		NULL				失败
///		其他，				有效数据的开始	
PBYTE CIPUnlockDrvMgr::UnlockData( PBYTE pBuf, DWORD & dwBufLen )
{	
	if( FALSE == IsDataTongShiModeLocked( pBuf, dwBufLen ) )
	{							//	没有加密
		ASSERT( FALSE );
		return pBuf;
	}

	PTS_IP_ENCRYPT_STRUCT pIPEncryptHeader = (PTS_IP_ENCRYPT_STRUCT)pBuf;
	pBuf += sizeof(TS_IP_ENCRYPT_STRUCT);
	ASSERT( dwBufLen > sizeof(TS_IP_ENCRYPT_STRUCT) );
	dwBufLen -= sizeof(TS_IP_ENCRYPT_STRUCT);
	ASSERT( dwBufLen == pIPEncryptHeader->m_wSrcDataLen );
	if( dwBufLen < pIPEncryptHeader->m_wSrcDataLen )
		return NULL;			//	长度错误

	int nDrvSN = pIPEncryptHeader->m_dwDrvSN;	

	CIPUnlockDrvWrapper * pDrv = GetDrv( nDrvSN );
	if( NULL == pDrv )
		return NULL;
	
	int nRetVal = pDrv->UnlockData( pBuf, dwBufLen, pIPEncryptHeader->m_dwSysCodeIndex );
	if( nRetVal <= 0 )
		return NULL;			//	失败了

	if( pIPEncryptHeader->m_dwSrcDataCRC32 != CCRC::GetCRC32( pIPEncryptHeader->m_wSrcDataLen, pBuf ) )
		return NULL;			//	数据错误

	dwBufLen = pIPEncryptHeader->m_wSrcDataLen;	
	return pBuf;
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		判断数据是否通视方式加密
/// 入口参数：
///		pBuf				数据缓冲区
///		dwBufLen			缓冲区大小
/// 返回参数：
///		TRUE				加密
///		FALSE				没有加密
BOOL CIPUnlockDrvMgr::IsDataTongShiModeLocked(PBYTE pBuf, DWORD dwBufLen)
{
	ASSERT( pBuf && dwBufLen );
	if( NULL == pBuf || dwBufLen < sizeof(TS_IP_ENCRYPT_STRUCT) )
		return FALSE;
	PTS_IP_ENCRYPT_STRUCT pIPEncryptHeader = (PTS_IP_ENCRYPT_STRUCT)pBuf;
	if( TS_IP_ENCRYPT_DATA_TAG != pIPEncryptHeader->m_dwTag )
		return FALSE;							//	标记错误
	if( pIPEncryptHeader->m_dwHeadCRC32 != \
		CCRC::GetCRC32( sizeof(TS_IP_ENCRYPT_STRUCT)-offsetof(TS_IP_ENCRYPT_STRUCT,m_wVersion),\
		PBYTE(&pIPEncryptHeader->m_wVersion) ) )
	{											//	数据头 CRC 错误
		return FALSE;
	}
	if( dwBufLen < pIPEncryptHeader->m_wSrcDataLen )
		return FALSE;							//	数据长度无效
	
	return TRUE;
}
