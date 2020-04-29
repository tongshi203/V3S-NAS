// FileWriterThread.h: interface for the CFileWriterThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEWRITERTHREAD_H__229AAF11_F72A_41B0_B32A_DBF10FC75E40__INCLUDED_)
#define AFX_FILEWRITERTHREAD_H__229AAF11_F72A_41B0_B32A_DBF10FC75E40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyThread.h>
#include <MyArray.h>

#ifndef _WIN32
	#include <MySyncObj.h>    
#endif //_WIN32

#include "DVBFileReceiver.h"

class CFileWriterThread : public CMyThread  
{
public:
	CFileWriterThread();
	virtual ~CFileWriterThread();

	void Add( CDVBFileReceiver * pReceiver );
	void Remove( CDVBFileReceiver * pReceiver );

	virtual int ExitInstance();					//	返回参数
#ifdef _WIN32
	virtual BOOL InitInstance();				//	是否成功
#else
	virtual bool InitInstance();				//	是否成功
#endif //_WIN32
	virtual void Run();							//	运行主体
	virtual void Delete();

private:
	CCriticalSection   m_SyncObj;			// synchornization object for m_aDataPort and m_aPollFD

	CMyArray< CDVBFileReceiver * >	m_apReceiverObjs;

private:
	int Find( CDVBFileReceiver * pReceiver );	
};

#endif // !defined(AFX_FILEWRITERTHREAD_H__229AAF11_F72A_41B0_B32A_DBF10FC75E40__INCLUDED_)
