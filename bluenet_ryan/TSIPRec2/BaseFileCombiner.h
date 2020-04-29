// BaseFileCombiner.h: interface for the CBaseFileCombiner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEFILECOMBINER_H__0FA1FFFE_3D8F_46B2_886F_17D834FD0824__INCLUDED_)
#define AFX_BASEFILECOMBINER_H__0FA1FFFE_3D8F_46B2_886F_17D834FD0824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
	#include <MyList.h>
    #include <MyMap.h>
#endif //_WIN32

#include "MB_OneFile.h"
#include "LookaheadPacketMgr.h"
#include "TSDVBBROPROTOCOL.H"
#include "TSDBFileSystem.h"

class COneDataPortItem;

class CBaseFileCombiner : public CLookaheadPacketMgr<CMB_OneFile>
{
public:
	DWORD GetIPBPS();
	void DoInputOnePage( COneDataPortItem * pDPItem, PBYTE pBuf, DWORD dwLen );
	CBaseFileCombiner();
	virtual ~CBaseFileCombiner();

	enum{	
		Y2KSECOND = 946656000L,					//	2000.1.1 的秒数
	};


private:
	DWORD m_dwByteReceived;						//	总共接收到的字节数
	DWORD m_dwLastTickCount;					//	上次计算的时间
	DWORD m_dwLastBPS;							//	上次计算的 BPS
#ifdef _WIN32
	CMap< DWORD,DWORD,CMB_OneFile*,CMB_OneFile*>m_OneFileMgr;
#else
	CMyMap< DWORD,DWORD,CMB_OneFile*,CMB_OneFile*>m_OneFileMgr;
#endif //_WIN32

private:
	void OnOneSubFileOK( COneDataPortItem *pDPItem, PBYTE pBuf, DWORD dwLen );
	void FreeActiveOne_FileObject();
	void OnFileOK( COneDataPortItem *pDPItem, CMB_OneFile * pOneFile );
	CMB_OneFile * AllocOneFile(PTSDVBMULTICASTPACKET0 pHeader);
	static time_t RestoreBroTime(time_t BroTime);
};

#endif // !defined(AFX_BASEFILECOMBINER_H__0FA1FFFE_3D8F_46B2_886F_17D834FD0824__INCLUDED_)
