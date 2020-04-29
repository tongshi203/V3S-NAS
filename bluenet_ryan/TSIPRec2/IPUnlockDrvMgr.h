// IPUnlockDrvMgr.h: interface for the CIPUnlockDrvMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPUNLOCKDRVMGR_H__4CAAA9C3_D289_4E13_AFB4_58E4049AF9F1__INCLUDED_)
#define AFX_IPUNLOCKDRVMGR_H__4CAAA9C3_D289_4E13_AFB4_58E4049AF9F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPUnlockDrvWrapper.h"
#include <afxtempl.h>
#include "IPEncryptKeyMgrImpl.h"
#include <afxmt.h>

class CIPUnlockDrvMgr  
{
public:
	static BOOL IsDataTongShiModeLocked( PBYTE pBuf, DWORD dwBufLen );
	PBYTE UnlockData( PBYTE pBuf, DWORD & dwBufLen );
	CIPEncryptKeyMgrImpl & GetKeyMgr();
	CIPUnlockDrvWrapper * GetDrv( int nDrvSN );
	CIPUnlockDrvMgr();
	virtual ~CIPUnlockDrvMgr();

private:
	CMap< int,int,CIPUnlockDrvWrapper*,CIPUnlockDrvWrapper*> m_DrvBuf;
	CIPEncryptKeyMgrImpl	m_KeyMgr;
	CCriticalSection		m_SynbObj;				//	m_DrvBuf 通过对象
};

#endif // !defined(AFX_IPUNLOCKDRVMGR_H__4CAAA9C3_D289_4E13_AFB4_58E4049AF9F1__INCLUDED_)
