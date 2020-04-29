// IPEncryptKeyMgrImpl.h: interface for the CIPEncryptKeyMgrImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPENCRYPTKEYMGRIMPL_H__6004D00A_1A48_4614_9049_0653D227628B__INCLUDED_)
#define AFX_IPENCRYPTKEYMGRIMPL_H__6004D00A_1A48_4614_9049_0653D227628B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CODUNLOCKDRVAPI.H"

#ifdef _WIN32
	#include <afxtempl.h>
    #include <afxmt.h>
#else
	#include <MyMap.h>    
#endif //_WIN32

#include "LicDecode.h"	// Added by ClassView


#define MYINLINE

class COneIPEncryptKeyItem : public IPENCRYPTKEYITEM
{
public:
	COneIPEncryptKeyItem()
	{
		m_dwSysCodeIndex = 0;
		m_nDrvSN = 0;
		m_ExpireTime = CTime::GetCurrentTime().GetTime();
		m_bSysCodeCalculated = FALSE;
#ifdef _WIN32
		RtlZeroMemory( m_abyUserCode, sizeof(m_abyUserCode) );
		RtlZeroMemory( m_abySysCode, sizeof(m_abySysCode) );
#else
		bzero( m_abyUserCode, sizeof(m_abyUserCode) );
		bzero( m_abySysCode, sizeof(m_abySysCode) );
#endif //_WIN32
		m_pbyXorDataBuf = NULL;
	}
	~COneIPEncryptKeyItem()
	{
		if( m_pbyXorDataBuf )
			delete m_pbyXorDataBuf;
		m_pbyXorDataBuf = NULL;
	}
};

class CIPEncryptKeyMgrImpl : public CIPEncryptKeyMgr2  
{
public:
	CIPEncryptKeyMgrImpl();
	virtual ~CIPEncryptKeyMgrImpl();

//	解密驱动调用
public:
	virtual BOOL	AllocateXorBuf( IPENCRYPTKEYITEM & Item, int nBufLen );	//	分配内存
	virtual PIPENCRYPTKEYITEM FindItem( int nDrvSN, DWORD dwSysCodeIndex );	//	查找对象

//	接收卡 ID
public:
	virtual void	SetRcvID( DVB_USER_ID & RcvID );
	virtual void	GetRcvID( DVB_USER_ID & RcvID );

//	公用函数接口
public:
	virtual BOOL	InsertItem( int nDrvSN, DWORD dwSysCodeIndex, BYTE abyUserCode[16], int nExpireMinute = 1440 );
	virtual BOOL	DeleteItem( int nDrvSN, DWORD dwSysCodeIndex );
	virtual int		SaveToFile( LPCSTR lpszFileName );
	virtual int		LoadFromFile( LPCSTR lpszFileName );

//	接口 2.0
public:
	virtual void ProcessLicData( PBYTE pBuf, DWORD dwLen );
	virtual HRESULT RegisterOnLicReceived( PFN_ONRECEIVELIC pfnCallBack, DWORD dwUserData, BOOL bAdd=TRUE);

public:	
	BOOL m_bIsRcvIDReady;

private:
	struct tagKEYITEMSAVE
	{
		DWORD	m_dwSysCodeIndex;
		DWORD	m_dwDrvSN;
		time_t	m_ExpireTime;			//	过期时间，过期后，将被删除
		BYTE	m_abyUserCode[32];		//	用户明码
	};
	enum{	KEYITEM_SAVED_VERSION = 1,	//	当前版本为 1
			ITEMSAVE_FILEHEAD_TAG = 'KI'
	};	
	struct tagCALLBACKFUNCTION
	{
		PFN_ONRECEIVELIC m_pfnCallBack;
		DWORD	m_dwUserData;
	};

private:
	DVB_USER_ID	m_RcvIID;
#ifdef _WIN32
	CMap<ULONGLONG,ULONGLONG&,COneIPEncryptKeyItem *, COneIPEncryptKeyItem *> m_KeyBuf;
	CArray<struct tagCALLBACKFUNCTION,struct tagCALLBACKFUNCTION&> m_aLicCallBackFunction;
#else
	CMyMap<ULONGLONG,ULONGLONG&,COneIPEncryptKeyItem *, COneIPEncryptKeyItem *> m_KeyBuf;
	CMyArray<struct tagCALLBACKFUNCTION> m_aLicCallBackFunction;
#endif //_WIN32
	CCriticalSection m_SyncObj_CallBack;				// Licence call back synchornize object

    CLicDecode m_Decoder;

private:
	void OnLicDataReceived( DWORD dwSysCodeIndex, DWORD adwLicData[4], BOOL bInsert );
	void DeleteExpiredKey();
	MYINLINE ULONGLONG MakeKeyItemKeyWord( int nDrvSN, DWORD dwSysCodeIndex );
};

#endif // !defined(AFX_IPENCRYPTKEYMGRIMPL_H__6004D00A_1A48_4614_9049_0653D227628B__INCLUDED_)
