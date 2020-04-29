///=======================================================
///
///     作者：陈永健
///     西安通视
///
///=======================================================

// DecoderThread.cpp: implementation of the CDecoderThread class.
//
//////////////////////////////////////////////////////////////////////
//	2002.12.11	Lookahead packet count = 8k，原来为 2048 不构用
//	2002.11.15	设置线程的级别

#include "stdafx.h"
#include "DecoderThread.h"

#ifdef _WIN32
    #ifdef _DEBUG
	    #undef THIS_FILE
 	   static char THIS_FILE[]=__FILE__;
  	  #define new DEBUG_NEW
    #endif
#else
	#include <unistd.h>    
	#include <pthread.h>
#endif //_WIN32    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDecoderThread::CDecoderThread() : CLookaheadPacketMgr< CIPData >( 10240, 512 )	// 10K
{

}

CDecoderThread::~CDecoderThread()
{

}

int CDecoderThread::ExitInstance()					//	返回参数
{
	return 0;
}

#ifdef _WIN32
BOOL CDecoderThread::InitInstance()					//	是否成功
#else
bool CDecoderThread::InitInstance()					//	是否成功
#endif //_WIN32
{
	return TRUE;
}

void CDecoderThread::Run()							//	运行主体
{
	m_bIsRequestQuit = FALSE;

#ifdef _WIN32
//	SetThreadPriority( m_hThread, THREAD_PRIORITY_ABOVE_NORMAL );	//	2002.11.15, 改变到较高级别
#else
	nice( -3 );			// I dont known how to change priority
   #ifdef _DEBUG
	TRACE("CDecoderThread run is called\n");
   #endif //_DEBUG
#endif //_WIN32    

	while( FALSE == m_bIsRequestQuit )
	{
#ifdef _DEBUG_
	printf("CDecoderThread::running, enter peek data\n");
#endif //_DEBUG

#ifdef _WIN32
		CIPData * pIPData = PeekData( 20 );
#else
		CIPData * pIPData = PeekData( 0 );
#endif	//_WIN32

#ifdef _DEBUG_
	printf("CDecoderThread::running, return from peek data\n");
#endif //_DEBUG

		if( NULL == pIPData )
		{
#ifdef __USE_GNU
			pthread_yield();
#endif //__USE_GNU
			Sleep( 20 );	// no data, sleep 20 ms
			continue;								//	暂时没有数据
		}

#ifdef __PRINT_DEBUG_MSG_RELEASE__
//		printf("CDecoderThread::Run.\n");
#endif // __PRINT_DEBUG_MSG_RELEASE__

		ASSERT( pIPData->m_pDataPortItem );
        if( pIPData->m_pDataPortItem )
        {
			pIPData->m_pDataPortItem->m_FileCombiner.DoInputOnePage( pIPData->m_pDataPortItem,\
				pIPData->GetBuffer(), pIPData->GetDataLen() );
        }

		DeAllocate( pIPData );
		pIPData->Release();
	}
}

void CDecoderThread::Delete()						//	删除自己
{
	// variable on data segment, need not delete
}
