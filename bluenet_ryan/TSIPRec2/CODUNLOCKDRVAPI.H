///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-26
///
///=======================================================

#ifndef __COD_UNLOCKDRV_INCLUDE_20021125__
#define __COD_UNLOCKDRV_INCLUDE_20021125__

#include "time.h"

#ifndef WINAPI
	#define WINAPI	__stdcall
#endif //_WINAPI

#pragma pack(push,1)
typedef struct tagIPENCRYPTKEYITEM
{
	union
	{
		struct
		{
			DWORD		m_dwSysCodeIndex;		//	系统密码索引
			int			m_nDrvSN;				//	解密驱动序号			
		};
		ULONGLONG	m_ulKeyID;			//	密码总的ID
	};
	time_t		m_ExpireTime;			//	过期时间，过期后，将被删除
	BOOL		m_bSysCodeCalculated;	//	系统密码已经经过计算
	BYTE		m_abyUserCode[16];		//	用户明码
	BYTE		m_abySysCode[16];		//	系统密码
	PBYTE		m_pbyXorDataBuf;		//	由管理程序申请的计算XOR需要的内存
}IPENCRYPTKEYITEM, *PIPENCRYPTKEYITEM;

typedef struct tagDVB_USER_ID
{
	union
	{
		struct
		{
			BYTE	m_abyRcvID[6];					//	接收卡ID		
			BYTE	m_abyManufacture[2];			//	硬件厂商ID
		}m_DVB;
		BYTE	m_abyDAMB[8];

		DWORD	m_adwID[2];							//	m_adwID[0]将用于接收授权V1.00和V1.01授权，其他版本可能使用8字节的授权信息
		ULONGLONG m_ullID;							//	用ULONGLONG形式表示
	}m_RcvID;

	BYTE	m_abyLockerID[8];						//	加密狗ID，若没有使用加密狗信息，则将该字段清 0
}DVB_USER_ID,*PDVB_USER_ID;

#pragma pack(pop)

///-------------------------------------------------------
/// Function:
///		On receive one licence data
/// Input parameter:
///		dwSysCodeIndex			syscodeindex
///		abyLicData				user licence data
///		bInsert					TRUE	insert one item
///								FALSE	remove the licence mapped by dwSysCodeIndex, abyLicData has no mean
///		dwUserData				the input parameter called by RegisterOnLicReceived.
/// Output parameter:
///		None
typedef void (WINAPI *PFN_ONRECEIVELIC)(DWORD dwSysCodeIndex,DWORD adwLicData[4],BOOL bInsert,DWORD dwUserData);

//-----------------------------------------------------
//	解密管理器
class CIPEncryptKeyMgr
{
//	解密驱动调用
public:
	virtual BOOL	AllocateXorBuf( IPENCRYPTKEYITEM & Item, int nBufLen ) = 0;	//	分配内存
	virtual PIPENCRYPTKEYITEM FindItem( int nDrvSN, DWORD dwSysCodeIndex ) = 0;	//	查找对象

//	接收卡 ID
public:
	virtual void	SetRcvID( DVB_USER_ID & RcvID ) = 0;
	virtual void	GetRcvID( DVB_USER_ID & RcvID ) = 0;

//	公用函数接口
public:
	virtual BOOL	InsertItem( int nDrvSN, DWORD dwSysCodeIndex, BYTE abyUserCode[16], int nExpireMinute = 1440 ) = 0;
	virtual BOOL	DeleteItem( int nDrvSN, DWORD dwSysCodeIndex ) = 0;
	virtual int		SaveToFile( LPCSTR lpszFileName ) = 0;
	virtual int		LoadFromFile( LPCSTR lpszFileName ) = 0;
};

class CIPEncryptKeyMgr2 : public CIPEncryptKeyMgr
{
public:
	virtual void ProcessLicData( PBYTE pBuf, DWORD dwLen ) = 0;
	virtual HRESULT RegisterOnLicReceived( PFN_ONRECEIVELIC pfnCallBack, DWORD dwUserData, BOOL bAdd=TRUE)=0;
};

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

CIPEncryptKeyMgr2 * WINAPI GetIPEncryptKeyMgrObj();

DWORD	WINAPI GetDrvSN();
BOOL	WINAPI InitDrv( CIPEncryptKeyMgr * pKeyMgr );
int		WINAPI UnlockData( PBYTE pBuf, DWORD dwBufLen,	DWORD dwSysCodeIndex, DWORD dwOfsInFile = 0 );

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // __COD_UNLOCKDRV_INCLUDE_20021125__