// LicDecode.h: interface for the CLicDecode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_)
#define AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_

#include "CodecFormat.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CODUNLOCKDRVAPI.H"

class CLicDecode  
{
public:
	WORD GetVersion();
	BOOL IsToDelete();
	void Init( DVB_USER_ID & UserID);
	LICOP_METHOD GetLicMethod();
	void Detach();
	DWORD GetSysCodeIndex();
	PDWORD GetLicData();
	BOOL IsInRange();
	BOOL Attach( PBYTE pLicBuf );
	CLicDecode();
	virtual ~CLicDecode();

//	测试用
#ifdef __CYJ_TEST_LICCODE__
	int ListIDCode( PDWORD pBuf );
#endif // #ifdef __CYJ_TEST_LICCODE__

private:
	DWORD UnlockIDCode( DWORD dwIDCode );
	void LockIDCode( );

	DVB_USER_ID m_UserID;					//  2004-3-11 用户数据
	DWORD m_dwIDCodeReal;					//	实际的卡号
	PBYTE m_pDataBuf;						//	数据缓冲区
	PBLKHEADER m_pHeader;					//	数据头
	DWORD m_dwIDCode;						//	加绕后的卡号,即实际编码的卡号
};

#endif // !defined(AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_)
