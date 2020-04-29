///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-28
///
///=======================================================

// IPUnlockDrvWrapper.cpp: implementation of the CIPUnlockDrvWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "iprecsvr.h"
#include "IPUnlockDrvWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPUnlockDrvWrapper::CIPUnlockDrvWrapper()
{
	Preset();
}

CIPUnlockDrvWrapper::~CIPUnlockDrvWrapper()
{
	if( m_hDll )
		::FreeLibrary( m_hDll );
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		获取驱动的 SN
/// 入口参数：
///		无
/// 返回参数：
///		驱动的SN
DWORD	CIPUnlockDrvWrapper::GetDrvSN()
{
	ASSERT( m_pfnGetDrvSN );
	if( NULL == m_pfnGetDrvSN )
		return 0;
	return m_pfnGetDrvSN();
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		初始化驱动
/// 入口参数：
///		pKeyMgr			密码管理
/// 返回参数：
///		TRUE			成功
///		FALSE			失败
BOOL	CIPUnlockDrvWrapper::InitDrv( CIPEncryptKeyMgr * pKeyMgr )
{
	ASSERT( pKeyMgr && m_pfnInitDrv );
	if( NULL == pKeyMgr || NULL == m_pfnInitDrv )
		return FALSE;
	return m_pfnInitDrv( pKeyMgr );
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		解密数据
/// 入口参数：
///		pBuf					缓冲区
///		dwBufLen				缓冲区大小
///		dwSysCodeIndex			系统密码索引
///		dwOfsInFile				数据在文件中的偏移，缺省为 0
/// 返回参数：
///		>0						成功执行解密，但数据不一定正确
///		=0						失败，没有解密密码
///		<0						其他未知错误
int		CIPUnlockDrvWrapper::UnlockData( PBYTE pBuf, DWORD dwBufLen, DWORD dwSysCodeIndex, DWORD dwOfsInFile )
{
	ASSERT( m_pfnUnlockData && pBuf && dwBufLen && dwSysCodeIndex );
	if( NULL == m_pfnUnlockData || NULL == pBuf || 0 == dwBufLen || 0 == dwSysCodeIndex )
		return -1;
	return m_pfnUnlockData( pBuf, dwBufLen, dwSysCodeIndex, dwOfsInFile );
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		加载驱动
/// 入口参数：
///		nDrvSN					驱动序号，驱动文件名为<WinSysDir>\CODUnlockDrvXXXXXX.DLL
/// 返回参数：
///		TRUE					成功
///		FALSE					失败
BOOL CIPUnlockDrvWrapper::LoadDrv(int nDrvSN)
{
	CString strFileName;
	char szSysDir[_MAX_PATH];
	::GetSystemDirectory( szSysDir, sizeof(szSysDir) );
	strFileName.Format("%s\\CODUnlockDrv%06X.DLL", szSysDir, nDrvSN );
	return LoadDrv( strFileName );
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		加载驱动
/// 入口参数：
///		lpszDrvFileName			文件名
/// 返回参数：
///		TRUE					成功
///		FALSE					失败
BOOL CIPUnlockDrvWrapper::LoadDrv(LPCSTR lpszDrvFileName)
{
	ASSERT( lpszDrvFileName );

	if( m_hDll )
		FreeLibrary( m_hDll );
	Preset();

	m_hDll = ::LoadLibrary( lpszDrvFileName );
	if( NULL == m_hDll )
		return FALSE;

	m_pfnGetDrvSN = (DWORD(WINAPI*)())::GetProcAddress( m_hDll, "GetDrvSN" );
	m_pfnInitDrv = (BOOL(WINAPI*)(CIPEncryptKeyMgr*))::GetProcAddress( m_hDll, "InitDrv" );
	m_pfnUnlockData = (int(WINAPI*)(PBYTE,DWORD,DWORD,DWORD))::GetProcAddress( m_hDll, "UnlockData" );
	return (m_pfnGetDrvSN && m_pfnInitDrv && m_pfnUnlockData);
}

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		预置参数
/// 入口参数：
///		无
/// 返回参数：
///		无
void CIPUnlockDrvWrapper::Preset()
{
	m_hDll = NULL;
	m_pfnGetDrvSN = NULL;
	m_pfnInitDrv = NULL;
	m_pfnUnlockData = NULL;
}
