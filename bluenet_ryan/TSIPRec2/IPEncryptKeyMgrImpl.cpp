///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-27
///		用作实现密码管理
///
///=======================================================

// IPEncryptKeyMgrImpl.cpp: implementation of the CIPEncryptKeyMgrImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPRecSvr.h"
#include "IPEncryptKeyMgrImpl.h"

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

CIPEncryptKeyMgrImpl::CIPEncryptKeyMgrImpl()
{
	m_bIsRcvIDReady = FALSE;
}

CIPEncryptKeyMgrImpl::~CIPEncryptKeyMgrImpl()
{
	POSITION pos = m_KeyBuf.GetStartPosition();
	while( pos )
	{
		ULONGLONG nHashKey;
		COneIPEncryptKeyItem * pItem = NULL;
		m_KeyBuf.GetNextAssoc( pos, nHashKey, pItem );
		ASSERT( pItem && pItem->m_ulKeyID == nHashKey );
		delete pItem;
	}
	m_KeyBuf.RemoveAll();
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		分配一个数据项的内存
/// 入口参数：
///		Item			待分配的密码项
///		nBufLen			分配的内存大小
/// 返回参数：
///		TRUE			成功
///		FALSE			失败
BOOL	CIPEncryptKeyMgrImpl::AllocateXorBuf( IPENCRYPTKEYITEM & Item, int nBufLen )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	if( Item.m_pbyXorDataBuf )
	{
#ifdef _WIN32
		ASSERT( FALSE == ::IsBadWritePtr( Item.m_pbyXorDataBuf, 1 ) );
#endif //_WIN32        
		delete Item.m_pbyXorDataBuf;
	}
	Item.m_pbyXorDataBuf = new BYTE[nBufLen];			//	自动会被释放
	return ( NULL != Item.m_pbyXorDataBuf );
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		查找密码项
/// 入口参数：
///		nDrvSN			驱动序号
///		dwSysCodeIndex	系统密码索引
/// 返回参数：
///		NULL			失败
///		其他			密码项
PIPENCRYPTKEYITEM CIPEncryptKeyMgrImpl::FindItem( int nDrvSN, DWORD dwSysCodeIndex )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( nDrvSN && dwSysCodeIndex );
	if( 0 == nDrvSN || 0 == dwSysCodeIndex )
		return NULL;

	ULONGLONG nHashKey = MakeKeyItemKeyWord( nDrvSN, dwSysCodeIndex );
	COneIPEncryptKeyItem * pRetVal = NULL;
	if( FALSE == m_KeyBuf.Lookup( nHashKey, pRetVal ) )
		return NULL;

	return static_cast<PIPENCRYPTKEYITEM>( pRetVal );
}


///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		设置接收卡的ID
/// 入口参数：
///		RcvID			ID
/// 返回参数：
///		无
void CIPEncryptKeyMgrImpl::SetRcvID( DVB_USER_ID & RcvID )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_RcvIID = RcvID;
	m_bIsRcvIDReady = TRUE;
	m_Decoder.Init( RcvID );
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		获取接收ID
/// 入口参数：
///		无
/// 返回参数：
///		接收卡ID
///	注：
///		接收卡ID，仅仅指接收硬件的ID，但有时加密不用这个密码，因为可能用加密狗
void CIPEncryptKeyMgrImpl::GetRcvID( DVB_USER_ID & RcvID )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	RcvID = m_RcvIID;
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		添加密码项
/// 入口参数：
///		nDrvSN					驱动序号
///		dwSysCodeIndex			系统密码索引
///		abyUserCode				用户明码
///		nExpireMinute			超时的分钟数，缺省为 1440 分钟，即一天的时间
/// 返回参数：
///		TRUE					成功
///		FALSE					失败
BOOL	CIPEncryptKeyMgrImpl::InsertItem( int nDrvSN, DWORD dwSysCodeIndex, BYTE abyUserCode[16], int nExpireMinute )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( nDrvSN && dwSysCodeIndex && abyUserCode );
	if( 0 == nDrvSN || 0 == dwSysCodeIndex || NULL == abyUserCode )
		return FALSE;
	if( nExpireMinute <= 0 )
		nExpireMinute = 1440;
	nExpireMinute *= 60;		//	转换为妙数

	PIPENCRYPTKEYITEM pExistItem = FindItem( nDrvSN, dwSysCodeIndex );
	if( NULL == pExistItem )
	{							//	不存在，需要新申请
		COneIPEncryptKeyItem * pNewItem = new COneIPEncryptKeyItem;
		if( NULL == pNewItem )
			return FALSE;		//	申请内存失败
		ULONGLONG nHashKey = MakeKeyItemKeyWord( nDrvSN, dwSysCodeIndex );
		TRY
		{
			m_KeyBuf[ nHashKey ] = pNewItem;
		}
		CATCH_ALL( e )
		{
			delete pNewItem;
			return FALSE;
		}
		END_CATCH_ALL
		pExistItem = static_cast<PIPENCRYPTKEYITEM>( pNewItem );
	}
	pExistItem->m_nDrvSN = nDrvSN;
	pExistItem->m_dwSysCodeIndex = dwSysCodeIndex;
	pExistItem->m_bSysCodeCalculated = FALSE;			//	需要重新计算
#ifdef _DEBUG
  #ifdef _WIN32
	RtlZeroMemory( pExistItem->m_abySysCode, sizeof(pExistItem->m_abySysCode) );
  #else
	bzero( pExistItem->m_abySysCode, sizeof(pExistItem->m_abySysCode) );
  #endif //_WINew
#endif //_DEBUG
	memcpy( pExistItem->m_abyUserCode, abyUserCode, sizeof(pExistItem->m_abyUserCode) );
	pExistItem->m_ExpireTime = nExpireMinute + CTime::GetCurrentTime().GetTime();
	if( pExistItem->m_pbyXorDataBuf )
		delete pExistItem->m_pbyXorDataBuf;
	pExistItem->m_pbyXorDataBuf = NULL;

	return TRUE;
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		删除一个密码项
/// 入口参数：
///		nDrvSN				驱动序号
///		dwSysCodeIndex		系统密码索引
/// 返回参数：
///		TRUE				成功删除
///		FALSE				没有该密码项
BOOL	CIPEncryptKeyMgrImpl::DeleteItem( int nDrvSN, DWORD dwSysCodeIndex )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	ASSERT( nDrvSN && dwSysCodeIndex );
	if( 0 == nDrvSN || 0 == dwSysCodeIndex )
		return FALSE;

	ULONGLONG nHashKey = MakeKeyItemKeyWord( nDrvSN, dwSysCodeIndex );

	COneIPEncryptKeyItem * pRetVal = NULL;
	if( FALSE == m_KeyBuf.Lookup( nHashKey, pRetVal ) )
		return FALSE;

	ASSERT( pRetVal );
	if( pRetVal )
		delete pRetVal;

	m_KeyBuf.RemoveKey( nHashKey );

	return TRUE;
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		保存密码到文件中
/// 入口参数：
///		lpszFileName			文件名
/// 返回参数：
///		<0						失败
///		>=0						实际保存的密码项个数
int		CIPEncryptKeyMgrImpl::SaveToFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( lpszFileName );
	if( NULL == lpszFileName )
		return -1;
	CFile f;
	if( FALSE == f.Open( lpszFileName, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::typeBinary ) )
		return -2;
	ASSERT( sizeof(tagKEYITEMSAVE) == 44 );
	int nItemSaved = 0;
	WORD wFileHeadTag = ITEMSAVE_FILEHEAD_TAG;		//	标记
	WORD wFileVersion = KEYITEM_SAVED_VERSION;		//	版本，用于兼容考虑
	time_t CurrentTime = CTime::GetCurrentTime().GetTime();
	TRY
	{
		f.Write( &wFileHeadTag, sizeof(WORD) );
		f.Write( &wFileVersion, sizeof(WORD) );

		POSITION pos = m_KeyBuf.GetStartPosition();
		while( pos )
		{
			ULONGLONG nHashKey;
			COneIPEncryptKeyItem * pItem = NULL;
			m_KeyBuf.GetNextAssoc( pos, nHashKey, pItem );
			ASSERT( pItem && pItem->m_ulKeyID == nHashKey );
			ASSERT( pItem );
			if( NULL == pItem )
				continue;
			if( pItem->m_ExpireTime < CurrentTime )
				continue;					//	过期了
			struct tagKEYITEMSAVE ItemSave;
			ItemSave.m_ExpireTime = pItem->m_ExpireTime;
			ItemSave.m_dwDrvSN = pItem->m_nDrvSN;
			ItemSave.m_dwSysCodeIndex = pItem->m_dwSysCodeIndex;
			memcpy( ItemSave.m_abyUserCode, pItem->m_abyUserCode, sizeof(ItemSave.m_abyUserCode) );
			f.Write( &ItemSave, sizeof(ItemSave) );
			nItemSaved ++;
		}
		f.Close();
	}
	CATCH_ALL( e )
	{
		f.Abort();
		return -3;
	}
	END_CATCH_ALL

	return nItemSaved;
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		从文件读取密码项
/// 入口参数：
///		lpszFileName			文件名
/// 返回参数：
///		>=0						读取的密码项个数
///		<0						读取文件时发生错误，此时不读取任何密码
int		CIPEncryptKeyMgrImpl::LoadFromFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32    

	ASSERT( lpszFileName );
	if( NULL == lpszFileName )
		return -1;
	CFile f;
	if( FALSE == f.Open( lpszFileName, CFile::modeRead|CFile::shareDenyWrite|CFile::typeBinary ) )
		return -2;
	ASSERT( sizeof(tagKEYITEMSAVE) == 44 );
	int nItemLoad = 0;
	WORD wFileHeadTag;		//	标记
	WORD wFileVersion = KEYITEM_SAVED_VERSION;		//	版本，用于兼容考虑
	time_t CurrentTime = CTime::GetCurrentTime().GetTime();
	TRY
	{
		f.Read( &wFileHeadTag, sizeof(WORD) );
		f.Read( &wFileVersion, sizeof(WORD) );
		if( wFileHeadTag != ITEMSAVE_FILEHEAD_TAG )
			return -3;								//	错误的文件格式

		int nFileLen = f.GetLength();
		int nCount = (nFileLen-4)/sizeof(struct tagKEYITEMSAVE);		
		struct tagKEYITEMSAVE ItemSave;

		for(int i=0; i<nCount; i++ )
		{
			if( sizeof(ItemSave) != f.Read( &ItemSave, sizeof(ItemSave) ) )
			{											//	错误
				f.Close();
				return nItemLoad;
			}
			if( ItemSave.m_ExpireTime < CurrentTime )
				continue;								//	放弃，过期了
		}

		COneIPEncryptKeyItem * pItem = new COneIPEncryptKeyItem;
		if( NULL == pItem )
		{												//	时期·申请内存失败了
			f.Close();
			return nItemLoad;
		}		

		pItem->m_ExpireTime = ItemSave.m_ExpireTime;
		pItem->m_nDrvSN = ItemSave.m_dwDrvSN;
		pItem->m_dwSysCodeIndex = ItemSave.m_dwSysCodeIndex ;
		memcpy( pItem->m_abyUserCode, ItemSave.m_abyUserCode, sizeof(ItemSave.m_abyUserCode) );

		m_KeyBuf[ pItem->m_ulKeyID ] = pItem;
		
		nItemLoad ++;
	}
	CATCH_ALL( e )
	{
		f.Abort();
		return -3;
	}
	END_CATCH_ALL

	return nItemLoad;
}

///-------------------------------------------------------
/// 2002-11-27
/// 功能：
///		计算密码项的关键字
/// 入口参数：
///		nDrvSN					驱动
/// 返回参数：
///
ULONGLONG CIPEncryptKeyMgrImpl::MakeKeyItemKeyWord(int nDrvSN, DWORD dwSysCodeIndex)
{
#ifdef _WIN32
	ULARGE_INTEGER RetVal;
	RetVal.LowPart = dwSysCodeIndex;
	RetVal.HighPart = nDrvSN;
	return RetVal.QuadPart;
#else
	ULONGLONG ullRetVal = nDrvSN;
    ullRetVal <<= 32;
    ullRetVal |= dwSysCodeIndex;
    return ullRetVal;
#endif //_WIN32
}

///-------------------------------------------------------
/// 2002-11-29
/// 功能：
///		整理并删除过期的密码项
/// 入口参数：
///		无
/// 返回参数：
///		无
void CIPEncryptKeyMgrImpl::DeleteExpiredKey()
{

}

///-------------------------------------------------------
/// CYJ,2004-3-9
/// Function:
///		Process Licence data
/// Input parameter:
///		None
/// Output parameter:
///		None
void CIPEncryptKeyMgrImpl::ProcessLicData(PBYTE pBuf, DWORD dwLen)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( pBuf && dwLen );
	if(NULL == pBuf || 0 == dwLen )
		return;
	int nCount = dwLen / TSLOCKCODECLEN;
	DWORD adwLicData[4];
#ifdef _WIN32
	RtlZeroMemory( adwLicData, sizeof(adwLicData) );
#else
	bzero( adwLicData, sizeof(adwLicData) );
#endif //_WIN32
	for(int i=0; i<nCount; i++)
	{
		if( m_Decoder.Attach( pBuf ) )
		{
			if( m_Decoder.IsInRange() )
			{									//	卡号在里边
				DWORD dwSysCodeIndex = m_Decoder.GetSysCodeIndex();
				switch (m_Decoder.GetLicMethod() )
				{
				case LICOP_LIC:					//	授权
					{
						PDWORD pdwUserCode = m_Decoder.GetLicData();
						if( pdwUserCode )
						{
							adwLicData[0] = *pdwUserCode;
							OnLicDataReceived( dwSysCodeIndex, adwLicData, TRUE );
						}
					}
					break;

				case LICOP_DEL:					//	关机
					if( m_Decoder.IsToDelete() )
						OnLicDataReceived( dwSysCodeIndex, adwLicData, FALSE  );
					break;
				}
			}
		}
		pBuf += TSLOCKCODECLEN;
	}
}

///-------------------------------------------------------
/// CYJ,2004-3-11
/// Function:
///		register on receive licence data call back
/// Input parameter:
///		pfnCallBack			call back function
///		dwUserData			user data associated with the pfnCallback
///		bAdd				TRUE	add the call back function
///							FALSE	delete the call back function
/// Output parameter:
///		S_OK				succ
///	Note:
///		must unregister the call back fucntion before one application exit
HRESULT CIPEncryptKeyMgrImpl::RegisterOnLicReceived( PFN_ONRECEIVELIC pfnCallBack, DWORD dwUserData, BOOL bAdd)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( pfnCallBack );
	if( NULL == pfnCallBack )
		return 0x80004003;		//	E_POINTER;

	struct tagCALLBACKFUNCTION OneItem;
	OneItem.m_dwUserData = dwUserData;
	OneItem.m_pfnCallBack = pfnCallBack;	

	CSingleLock locker( &m_SyncObj_CallBack );
	if( FALSE == locker.Lock( 5000 ) )
		return 0x8001011F;		//	RPC_E_TIMEOUT;

	int nExistNo = -1;
	int nCount = m_aLicCallBackFunction.GetSize();
	for(int i=0; i<nCount; i++)
	{
		if( m_aLicCallBackFunction[i].m_pfnCallBack == pfnCallBack )
		{
			nExistNo = i;
			break;
		}
	}

	if( bAdd )
	{
		if( nExistNo >= 0 )
			return 0x8009000fL;	  	//NTE_EXISTS;
		TRY
		{
			m_aLicCallBackFunction.Add( OneItem );
		}
		CATCH_ALL( e )
		{
			return 0x8009000EL;		// NTE_NO_MEMORY;
		}
		END_CATCH_ALL
	}
	else if( nExistNo >= 0 )
		m_aLicCallBackFunction.RemoveAt( nExistNo );

	return 0;			//	S_OK;
}

///-------------------------------------------------------
/// CYJ,2004-3-11
/// Function:
///		On Lic data is received
/// Input parameter:
///		dwSysCode				syscodeindex
///		abyLic					licence data, may be NULL when bInsert = FALSE
///		bInsert					insert lic
/// Output parameter:
///		None
void CIPEncryptKeyMgrImpl::OnLicDataReceived(DWORD dwSysCodeIndex, DWORD adwLicData[], BOOL bInsert)
{
	CSingleLock locker( &m_SyncObj_CallBack );
	if( FALSE == locker.Lock( 5000 ) )
		return;
	int nCount = m_aLicCallBackFunction.GetSize();
	for(int i=0; i<nCount; i++)
	{
		struct tagCALLBACKFUNCTION & Item = m_aLicCallBackFunction[i];
		if( NULL == Item.m_pfnCallBack )
			continue;
		Item.m_pfnCallBack( dwSysCodeIndex, adwLicData, bInsert, Item.m_dwUserData );
	}
}
