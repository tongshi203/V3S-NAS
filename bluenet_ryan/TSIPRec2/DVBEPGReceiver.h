// DVBEPGReceiver.h: interface for the CDVBEPGReceiver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBEPGRECEIVER_H__9263BD1E_ACA8_4DBD_848B_3A5766CEE44C__INCLUDED_)
#define AFX_DVBEPGRECEIVER_H__9263BD1E_ACA8_4DBD_848B_3A5766CEE44C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DVBFileReceiver.h"
#ifdef _WIN32
	#include <afxtempl.h>
#else
	#include <MyMap.h>
#endif //_WIN32

#pragma pack(push,8)

class CDVBEPGReceiver : public CDVBFileReceiver
{
public:
	CDVBEPGReceiver();
	virtual ~CDVBEPGReceiver();

	virtual bool GetEnableOldEPG();
	virtual void SetEnableOldEPG( bool bNewValue );

//	IUnknwon
	virtual DWORD QueryInterface( REFIID iid,void ** ppvObject);
	virtual BOOL Init(bool bSaveFileInBackground=false,int varMaxPacketSize=2048);

protected:
	virtual void Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort  );
	virtual void Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort  );

private:
	bool	m_bEnableOldEPG;
#ifdef _WIN32
	CMap< DWORD,DWORD,time_t, time_t > m_mapFileEPG;
#else
	CMyMap< DWORD,DWORD,time_t, time_t > m_mapFileEPG;
#endif //_WIN32

private:
	bool TransferToXMLFormat( IFileObject * pObject );

};

#pragma pack(pop)

#endif // !defined(AFX_DVBEPGRECEIVER_H__9263BD1E_ACA8_4DBD_848B_3A5766CEE44C__INCLUDED_)
