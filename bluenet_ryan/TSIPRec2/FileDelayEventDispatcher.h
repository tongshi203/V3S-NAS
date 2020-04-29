// FileDelayEventDispatcher.h: interface for the CFileDelayEventDispatcher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEDELAYEVENTDISPATCHER_H__6BC00ABE_91AF_46C2_8B9A_2AD2C566D5D9__INCLUDED_)
#define AFX_FILEDELAYEVENTDISPATCHER_H__6BC00ABE_91AF_46C2_8B9A_2AD2C566D5D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPRecSvr.h"
#include "MyIUnknownImpl.h"
#include "FileObject.h"

#ifndef _WIN32
	#include <MyList.h>
#endif //_WIN32

#include <LookaheadPacketMgr.h>

class CDelayEvnet_FileObject : public CFileObject
{
public:
	CDelayEvnet_FileObject(){};
	void GetParamFromFileObj( CFileObject & FileObj );
	virtual DWORD GetBufSize();
	virtual DWORD GetDataLen();
	virtual BOOL	SetBufSize( DWORD dwNewValue );
	virtual PBYTE	GetBuffer();
	virtual void SafeDelete();

private:
	DWORD	m_dwFileLen;
	TSDBFILEATTRIBHEAD	m_AttribDataBuf;
};

class CDelayEventItem
{
public:
	CDelayEventItem();
	long AddRef();
	long Release();
	void Preset();

	enum{
		EVENT_TYPE_FILE_OK = 0,
		EVENT_TYPE_PROGRESS,
	};

public:
	int		m_nEventType;
	HDATAPORT	m_hDataPort;
	CDelayEvnet_FileObject	m_FileObject;	//	Progress 会借用 FileObj 的 m_strFileName

//	以下是Progress 事件
	float	m_fProgress;
	DWORD	m_dwBroLoopCount;
	int		m_dwFileCount;
	DWORD	m_dwTotalLen;
	DWORD	m_dwByteReceived;
	int		m_nCountReceived;

private:
	long	m_nRef;
};

class CFileDelayEventDispatcher : public CMyIUnknownImpl<IDVBReceiverEvent>
{
public:
	CFileDelayEventDispatcher();
	virtual ~CFileDelayEventDispatcher();

	virtual void OnFileOK( IFileObject * pObj, HDATAPORT hDataPort );
	virtual void OnSubFileOK( IFileObject * pObj, HDATAPORT hDataPort );
	virtual void OnProgress( HDATAPORT hDataPort, float fProgress, DWORD dwBroLoopCount, int dwFileCount, DWORD dwTotalLen, DWORD dwByteReceived, int nCountReceived, LPCSTR lpszFileName );

	void SetHandler( IDVBReceiverEvent * pHandler );
	void DispatchEvents();
	virtual void SafeDelete();
	void FlushAddCatch();

private:
	IDVBReceiverEvent *	m_pRealHandler;
	CLookaheadPacketMgr<CDelayEventItem> m_DelayEventQueue;
};

#endif // !defined(AFX_FILEDELAYEVENTDISPATCHER_H__6BC00ABE_91AF_46C2_8B9A_2AD2C566D5D9__INCLUDED_)
