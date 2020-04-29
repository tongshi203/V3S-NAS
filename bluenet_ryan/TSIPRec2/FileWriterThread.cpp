// FileWriterThread.cpp: implementation of the CFileWriterThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileWriterThread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileWriterThread::CFileWriterThread()
{

}

CFileWriterThread::~CFileWriterThread()
{

}

int CFileWriterThread::ExitInstance()
{
	return 0;
}

#ifdef _WIN32
	BOOL CFileWriterThread::InitInstance()
	{
		return TRUE;
	}

#else
	bool CFileWriterThread::InitInstance()
	{
		return true;
	}
#endif //_WIN32

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		运行
/// Input parameter:
///		None
/// Output parameter:
///		None
void CFileWriterThread::Run()
{
	while( false == m_bIsRequestQuit )
	{
    	CSingleLock	syncobj( &m_SyncObj );
		if( FALSE == syncobj.Lock( 0 ) )
		{
	#ifdef __USE_GNU
			pthread_yield();
	#endif //__USE_GNU
			Sleep( 10 );
			continue;
		}

		int nCount = m_apReceiverObjs.GetSize();
		for(int i=0; i<nCount; i++)
		{
			m_apReceiverObjs[i]->WriteThread_DoCheckAndSaveFiles();
		}

			syncobj.Unlock();
#ifdef __USE_GNU
			pthread_yield();
#endif //__USE_GNU
#ifdef _WIN32
			Sleep( 100 );
#else	
			Sleep( 50 );
#endif //_WIN32
	}
}

void CFileWriterThread::Delete()
{
//	静态函数，不用删除
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		添加一个接收文件对象
/// Input parameter:
///		None
/// Output parameter:
///		None
void CFileWriterThread::Add( CDVBFileReceiver * pReceiver )
{
	CSingleLock	syncobj( &m_SyncObj );
	if( FALSE == syncobj.Lock() )
		return;
	if( Find( pReceiver ) >= 0 )
		return;
	m_apReceiverObjs.Add( pReceiver );
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		删除一个文件对象
/// Input parameter:
///		None
/// Output parameter:
///		None
void CFileWriterThread::Remove( CDVBFileReceiver * pReceiver )
{
	CSingleLock	syncobj( &m_SyncObj );
	if( FALSE == syncobj.Lock() )
		return;

	int nNo = Find( pReceiver );
	if( nNo < 0 )
		return;						//	不存在
	m_apReceiverObjs.RemoveAt( nNo );
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		查找一个文件接收对象
/// Input parameter:
///		None
/// Output parameter:
///		<0					失败
///		>=0					数组序号
///	Note:
///		调用者必须获取 m_apRec的控制权
int CFileWriterThread::Find( CDVBFileReceiver * pReceiver )
{
	int nCount = m_apReceiverObjs.GetSize();
	for(int i=0; i<nCount; i++)
	{
		if( m_apReceiverObjs[i] == pReceiver )
			return i;
	}

	return -1;
}
