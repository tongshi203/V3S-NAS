// IPUnlockDrvWrapper.h: interface for the CIPUnlockDrvWrapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPUNLOCKDRVWRAPPER_H__4CC0BB45_CE7F_44E3_8662_23F81DDE8CE5__INCLUDED_)
#define AFX_IPUNLOCKDRVWRAPPER_H__4CC0BB45_CE7F_44E3_8662_23F81DDE8CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CODUNLOCKDRVAPI.H"

class CIPUnlockDrvWrapper  
{
public:
	BOOL LoadDrv( LPCSTR lpszDrvFileName );
	BOOL LoadDrv( int nDrvSN );
	CIPUnlockDrvWrapper();
	virtual ~CIPUnlockDrvWrapper();

	DWORD	GetDrvSN();
	BOOL	InitDrv( CIPEncryptKeyMgr * pKeyMgr );
	int		UnlockData( PBYTE pBuf, DWORD dwBufLen,	DWORD dwSysCodeIndex, DWORD dwOfsInFile = 0 );

private:
	void Preset();
	DWORD	(WINAPI	* m_pfnGetDrvSN)();
	BOOL	(WINAPI	* m_pfnInitDrv)( CIPEncryptKeyMgr * pKeyMgr );
	int		(WINAPI	* m_pfnUnlockData)( PBYTE pBuf, DWORD dwBufLen,	DWORD dwSysCodeIndex,DWORD dwOfsInFile );

	HMODULE m_hDll;
};

#endif // !defined(AFX_IPUNLOCKDRVWRAPPER_H__4CC0BB45_CE7F_44E3_8662_23F81DDE8CE5__INCLUDED_)
