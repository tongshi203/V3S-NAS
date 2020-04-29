// UDPDataPort.h: interface for the CUDPDataPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_)
#define AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SrcDataPort.H"
#include <afxmt.h>
#include "MyWS2_32.h"	// Added by ClassView
#include <afxtempl.h>
#include <WS2TCPIP.H>

typedef struct tagUDPOVERLAP
{
	WSAEVENT		m_hEvent;					//	同步对象
	BOOL			m_bIsPending;				//	是否延迟操作
	WSABUF			m_wsaDataBuf;				//	Socket 用的缓冲区
	WSAOVERLAPPED	m_overlapped;				//	重叠对象
	DWORD			m_dwByteRead;				//	成功读取的字节数
	DWORD			m_dwFlags;					//	标志
}UDPOVERLAP,*PUDPOVERLAP;

class CUDPDataPort : public CSrcDataPort  
{
public:
	CUDPDataPort();
	virtual ~CUDPDataPort();

public:	
	virtual BOOL	Initialize( LPCSTR lpszIP, UINT nPort, LPCSTR lpszLocalBind = NULL, int nCount = -1 );
	virtual void	Invalidate();
	virtual HANDLE 	GetEventHandle(SDP_HANDLE hNo);
	virtual SDP_HANDLE	ReadAsyn(	PBYTE pBuf, DWORD dwBufSize,PDWORD pdwByteRead );
	virtual BOOL	ReadSync( PBYTE pBuf, DWORD dwBufSize, PDWORD pdwByteRead );
	virtual BOOL	GetOverlappedResult( SDP_HANDLE hReadNo, PDWORD pdwByteRead, BOOL bWait );
	virtual void	CancelAsynRead(SDP_HANDLE hReadNo);
	virtual int		GetItemCount();
	virtual void	SafeDelete();
	virtual	BOOL	CanIDoReadAsync();

	enum { DEFAULT_ASYN_COUNT = 128 };

private:
	int m_nItemCount;			//	异步操作的个数
	int	m_nCurReadItemCount;	//	当前读取的记录数

	SOCKET m_hSocket;
	struct ip_mreq m_mrMReq;			// Contains IP and interface of the host group

private:
	BOOL m_bNeedToCallCleanUp;
	CArray<UDPOVERLAP,UDPOVERLAP&> m_asynobjs;
	CMyWS2_32 m_drv;
};

#endif // !defined(AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_)
