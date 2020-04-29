///=======================================================
///
///     作者：陈永健
///     西安通视
///
///=======================================================

// DPRecThread.cpp: implementation of the CDPRecThread class.
//
//////////////////////////////////////////////////////////////////////
// 2003-3-5		修改DoReadSyncDataPort()，对于同步端口方式，每次读取 100 次，或没有数据为止
//	2002.12.30	添加 DoReadSyncDataPort，支持数据端口模式
//	2002.11.28	添加 g_UnlockDrvMgr，以及函数 GetIPEncryptKeyMgrObj
//				修改 OnDataOK，增加对数据解密的支持
//	2002.11.14  修改 OnDataOK，添加 AddPacket 失败的情况下的处理
//	2002.6.17	修改 Run 函数，优化算法，主要在释放同步对象的控制权上

#include "stdafx.h"
#include "resource.h"
#include "DPRecThread.h"
#include "IPUnlockDrvMgr.h"
#include "IPEncryptDataStruct.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//--------------------------------------------
//	解密驱动管理器
CIPUnlockDrvMgr	g_UnlockDrvMgr;

///-------------------------------------------------------
/// 2002-11-28
/// 功能：
///		获取密码项管理程序
/// 入口参数：
///		无
/// 返回参数：
///		密码项管理对象
extern "C" CIPEncryptKeyMgr2 * WINAPI GetIPEncryptKeyMgrObj()
{
	CIPEncryptKeyMgrImpl & KeyMgr = g_UnlockDrvMgr.GetKeyMgr();
	return static_cast<CIPEncryptKeyMgr2*>( &KeyMgr );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDPRecThread::CDPRecThread()
{
	m_pDecoderThread = NULL;
}

CDPRecThread::~CDPRecThread()
{

}

int CDPRecThread::ExitInstance()
{
	CleanUpIPData();
	return 0;
}

BOOL CDPRecThread::InitInstance()
{
	return TRUE;
}

//---------------------------------------
//	修改记录：
//	2002.12.30	添加 DoReadSyncDataPort
void CDPRecThread::Run()
{
	ASSERT( m_pDecoderThread && m_pDecoderThread->m_hThread );

	m_bIsRequestQuit = FALSE;
	SetThreadPriority( m_hThread, THREAD_PRIORITY_TIME_CRITICAL );	//	改变到最大级别

	HANDLE anEventObjs[ MAX_DATA_PORT_COUNT ];						//	不可能超过该事件对象
	COneDataPortItem * anDataPort[ MAX_DATA_PORT_COUNT ];			//	实际参与等待的对象
	while( FALSE == m_bIsRequestQuit )
	{	
		DoReadAsync();												//	读取数据
		DoReadSyncDataPort();										//	读取同步方式的数据端口7

		CSingleLock symobj( &m_SynObj, TRUE );				

		int nCount = m_anPorts.GetSize();		
		int nActualCount = 0;
		for(int i=0; i<nCount; i++)
		{															//	获取即将参与的端口同步对象
			ASSERT( m_anPorts[i]->m_pDataPort );
			SDP_HANDLE hAsync = m_anPorts[i]->GetHeadHandle();
			if( hAsync < 0 )
				continue;
			anEventObjs[nActualCount] = m_anPorts[i]->m_pDataPort->GetEventHandle( hAsync );
			anDataPort[nActualCount] = m_anPorts[i];
			nActualCount ++;
		}	
		if( 0 == nActualCount )				//	没有延迟操作
		{
			symobj.Unlock();				//	2002.6.17 添加，先释放，然后再休眠
			Sleep( 20 );					//	没有数据端口，休息 20 ms
			continue;
		}

		DWORD dwRetVal = ::WaitForMultipleObjects( nActualCount,anEventObjs,FALSE, 10 );	//	每 10 毫秒检测一次
		if( WAIT_TIMEOUT == dwRetVal )
		{															// 2002.6.17 添加修改，原来在 WaitForMultipleObjects 修改 20 ms，现改成 10ms，然后独立休眠10ms
			symobj.Unlock();										//	返回之前，先释放对象，然后再休眠
			Sleep( 5 );												//	暂时没有数据，休眠 5 ms，让其他线程工作	
			continue;												//	超时
		}
		
		if( dwRetVal >= WAIT_ABANDONED_0 && dwRetVal < WAIT_ABANDONED_0 + nActualCount )
		{															//	有对象被放弃/遗弃, ????
			TRACE("One event is abandoned.\n");
			int nNo = dwRetVal - WAIT_ABANDONED_0;
			ASSERT( FALSE == anDataPort[nNo]->m_AsyncHandle.IsEmpty() );
			ONEASYNCREAD oneread = anDataPort[nNo]->m_AsyncHandle.RemoveHead();

			anDataPort[nNo]->m_pDataPort->CancelAsynRead( oneread.m_hSDP );	//	是否还需要作其他事情 ???
				
			symobj.Unlock();

			m_pDecoderThread->DeAllocate( oneread.m_pIPData );		//	释放内存
			oneread.m_pIPData->Release();
			continue;
		}
		int nNo = dwRetVal - WAIT_OBJECT_0;							//	成功
		ASSERT( nNo >= 0 && nNo < nActualCount );

		COneDataPortItem * pPort = anDataPort[nNo];		
		ONEASYNCREAD & oneread = anDataPort[nNo]->m_AsyncHandle.GetHead();
		CIPData * pIPData = oneread.m_pIPData;
		DWORD dwByteRead = 0;										//	实际读取的字节数
		if( FALSE == pPort->m_pDataPort->GetOverlappedResult( oneread.m_hSDP, &dwByteRead, FALSE ) )
		{											//	失败，可能是还没有完成，按理说不太可能，因为我已经作等待
			if( WSA_IO_INCOMPLETE == pPort->m_pDataPort->m_nLastError )
			{
				TRACE("CDPRecThread, This condition can not be happen.\n");
				ASSERT( FALSE );
				continue;
			}

			pPort->m_pDataPort->CancelAsynRead( oneread.m_hSDP );	//	是否还需要作其他事情 ???
			anDataPort[nNo]->m_AsyncHandle.RemoveHead();

			symobj.Unlock();			

			m_pDecoderThread->DeAllocate( pIPData );	//	释放内存
			pIPData->Release();
			continue;
		}

//		TRACE("One Packet is received.\n");

		anDataPort[nNo]->m_AsyncHandle.RemoveHead();
		symobj.Unlock();

		OnDataOK( pPort, pIPData, dwByteRead );
		pIPData->Release();
	}
}

//////////////////////////////////////////////
//功能:
//		执行读取数据
//入口参数:
//		预先填写所有的异步读操作
//返回参数:
//		无
void CDPRecThread::DoReadAsync()
{
	CSingleLock symobj( &m_SynObj, TRUE );

	int nCount = m_anPorts.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pPort = m_anPorts[i];
		while( pPort->m_pDataPort->CanIDoReadAsync() )
		{								//	判断是否还有异步操作的记录空间
			CIPData * pIPData = m_pDecoderThread->AllocatePacket( 0 );
			if( NULL == pIPData )
				return;					//	没有了，只好放弃
			DWORD dwDataRead = 0;
			if( FALSE == pIPData->SetBufSize( pPort->m_wPacketBufSize ) )
			{
				symobj.Unlock();		//	申请内存失败，放弃
				m_pDecoderThread->DeAllocate( pIPData );
				pIPData->Release();
				return;
			}
			SDP_HANDLE hRead = pPort->m_pDataPort->ReadAsyn( pIPData->GetBuffer(), pPort->m_wPacketBufSize, &dwDataRead );
//			TRACE("........ To receive One packet. return value=%d\n",hRead);

			if( hRead > 0 )
			{						//	延迟操作
				ONEASYNCREAD oneread;
				oneread.m_hSDP = hRead;
				oneread.m_pIPData = pIPData;
				pPort->m_AsyncHandle.AddTail( oneread );
				continue;
			}
			symobj.Unlock();
			if( 0 == hRead )			//	成功读取
				OnDataOK( pPort, pIPData, dwDataRead );
			else							//	失败
				m_pDecoderThread->DeAllocate( pIPData );
			pIPData->Release();
			symobj.Lock();

			nCount = m_anPorts.GetSize();	//	可能被删除
			if( NULL== pPort->m_pDataPort )
				break;
		}
	}
}

//////////////////////////////////////////////
//功能:
//		接收到数据
//入口参数:
//		pDataPortItem			数据端口
//		pIPData					IP 数据包
//		dwByteCount				成功读取的字节数
//返回参数:
//		无
//修改记录：
//	2002.11.28	解密数据
//	2002.11.14	添加 AddPacket 失败的情况下的处理
void CDPRecThread::OnDataOK(COneDataPortItem *pDataPortItem, CIPData *pIPData, DWORD dwByteCount)
{
	ASSERT( pDataPortItem && dwByteCount );
	pIPData->m_pDataPortItem = pDataPortItem;
	pIPData->PutDataLen( dwByteCount );

//--------------- Begin 2002.11.28 Added for Unlock TongShi mode Data Encrypt -----------------
	if( g_UnlockDrvMgr.IsDataTongShiModeLocked( pIPData->GetBuffer(), dwByteCount ) )
	{							//	数据采用通视专用方式加密，需要进行解密
		PBYTE pRetVal = g_UnlockDrvMgr.UnlockData( pIPData->GetBuffer(), dwByteCount );
		if( NULL == pRetVal )
		{						//	解密失败
			TRACE0("Unlock Data Failed.\n");
			m_pDecoderThread->DeAllocate( pIPData );			//	2002.11.14 添加
			return;
		}
#ifdef _DEBUG
		ASSERT( int(pRetVal-pIPData->GetBuffer()) == int(pIPData->GetDataLen() - dwByteCount) );
		ASSERT( (pIPData->GetDataLen() - dwByteCount) == sizeof(TS_IP_ENCRYPT_STRUCT) );
#endif // _DEBUG

		pIPData->DeleteHeadData( sizeof(TS_IP_ENCRYPT_STRUCT) );

		ASSERT( pIPData->GetDataLen() == dwByteCount );
	}
//------------ End 2002.11.28 -----------------------------------------------

	if( FALSE == m_pDecoderThread->AddPacket( pIPData ) )	//	提交数据
		m_pDecoderThread->DeAllocate( pIPData );			//	2002.11.14 添加
}

//////////////////////////////////////////////
///功能:
///		安全地删除自己
///入口参数:
///		无
///返回参数:
///		无
void CDPRecThread::Delete()
{
	if( m_bAutoDelete )
		delete this;
}

//////////////////////////////////////////////
///功能:
///		创建接收线程
///入口参数:
///		pDecoderThread			解码线程
///返回参数:
///		TRUE					成功
///		FALSE					失败
///注：
///		pDecoderThread			解码线程必须先创建
BOOL CDPRecThread::Init(CDecoderThread *pDecoderThread)
{
	ASSERT( pDecoderThread && pDecoderThread->m_hThread );
	m_pDecoderThread = pDecoderThread;
	return CreateThread();
}

//////////////////////////////////////////////
///功能:
///		关闭线程
///入口参数:
///		无
///返回参数:
///		无
void CDPRecThread::Close()
{
	if( NULL != m_hThread )
		StopThread(-1);						//	停止运行
	else if( m_bAutoDelete )
		delete this;						//	还没有创建线程，所以只好自己删除对象
}

//////////////////////////////////////////////
///功能:
///		添加数据端口对象
///入口参数:
///		pDataPortItem			待添加地端口
///返回参数:
///		>0						成功添加，且返回当前所有地端口数目
///		<0						失败
int CDPRecThread::AddDataPort(COneDataPortItem *pDataPortItem)
{
	ASSERT( pDataPortItem && pDataPortItem->m_pDataPort && pDataPortItem->m_pFileObjMgr );
	if( NULL == pDataPortItem || NULL == pDataPortItem->m_pDataPort || NULL == pDataPortItem->m_pFileObjMgr )
		return -1;

	CSingleLock symobj( &m_SynObj, TRUE );

	int nNo = FindDataPortItem( pDataPortItem );
	if( nNo >= 0 )
	{								//	已经存在
		TRACE("CDPRecThread, this data port has been add already.\n");
		ASSERT( FALSE );
		return -1;					//	不能再次添加
	}
	
	if( m_anPorts.GetSize() >= MAX_DATA_PORT_COUNT )
	{
		TRACE("Too many data ports.\n");
		return -1;
	}

	m_anPorts.Add( pDataPortItem );

	return m_anPorts.GetSize();
}

//////////////////////////////////////////////
///功能:
///		删除一个数据端口
///入口参数:
///		pDataPortItem			待删除的端口，该数据必须是以前曾经添加过的端口
///返回参数:
///		无
void CDPRecThread::DeleteDataPort(COneDataPortItem *pDataPortItem)
{
	ASSERT( pDataPortItem );
	CSingleLock symobj( &m_SynObj, TRUE );

	int nNo = FindDataPortItem( pDataPortItem );
	if( nNo < 0 )
	{
		TRACE("CDPRecThread, this data port is not add ever.\n");
		return;
	}
	TRACE("CDPRecThread::DeleteDataPort, delete one data port=%08X\n",pDataPortItem );
	m_anPorts.RemoveAt( nNo );	

	symobj.Unlock();
	
	ASSERT( m_pDecoderThread );
	while( FALSE == pDataPortItem->m_AsyncHandle.IsEmpty() )
	{
		ONEASYNCREAD oneread = pDataPortItem->m_AsyncHandle.RemoveHead();
		m_pDecoderThread->DeAllocate( oneread.m_pIPData );	//	释放内存
		oneread.m_pIPData->Release();
	}
}

//////////////////////////////////////////////
///功能:
///		根据给定的指针查找对象
///入口参数:
///		pDataPortItem			待查找的数据端口
///返回参数:
///		<0						没有找到
///		>=0						序号
int CDPRecThread::FindDataPortItem(COneDataPortItem *pDataPortItem)
{
	int nCount = m_anPorts.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_anPorts[i];
		ASSERT( pItem );
		if( pItem == pDataPortItem )
			return i;
	}
	return -1;
}

//////////////////////////////////////////////
//功能:
//		释放在操作的所有 IPDATA 对象
//入口参数:
//		无
//返回参数:
//		无
void CDPRecThread::CleanUpIPData()
{
	CSingleLock symobj( &m_SynObj, TRUE );	
	int nCount = m_anPorts.GetSize();
	CArray< CIPData *, CIPData * > aIPDataToFree;
	TRY
	{
		for(int i=0; i<nCount; i++)
		{
			COneDataPortItem * pPort = m_anPorts[i];
			if( pPort->m_AsyncHandle.IsEmpty() )
				continue;

			ONEASYNCREAD oneread = pPort->m_AsyncHandle.RemoveHead();
			aIPDataToFree.Add( oneread.m_pIPData );		
		}
	}
	CATCH( CMemoryException, e )
	{
#ifdef _DEBUG
		e->ReportError();
#endif // _DEBUG
	}
	END_CATCH

	symobj.Unlock();

	nCount = aIPDataToFree.GetSize();
	for( int i=0; i<nCount; i++)
	{
		m_pDecoderThread->DeAllocate( aIPDataToFree[i] );
		aIPDataToFree[i]->Release();
	}
}

///-------------------------------------------------------------------
/// 2002-12-31
/// 功能：
///		读取同步数据端口
/// 入口参数：
///		无
/// 出口参数：
///		无
//	修改记录：
// 2003-3-5 对于同步端口方式，每次读取 100 次，或没有数据为止
void CDPRecThread::DoReadSyncDataPort()
{
	CSingleLock symobj( &m_SynObj, TRUE );

	int nCount = m_anPorts.GetSize();
	CIPData * pIPData = NULL;
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pPort = m_anPorts[i];
		ASSERT( pPort );
		if( pPort->m_pDataPort->GetItemCount() )
			continue;				//	为异步方式，不用该方式读取数据

		for(int j=0; j<100; j++)
		{
			if( NULL == pIPData )
			{							//	需要进行分配
				pIPData = m_pDecoderThread->AllocatePacket( 0 );
				if( NULL == pIPData )
					return;				//	没有了，只好放弃
				if( FALSE == pIPData->SetBufSize( pPort->m_wPacketBufSize ) )
				{
					symobj.Unlock();		//	申请内存失败，放弃
					m_pDecoderThread->DeAllocate( pIPData );
					pIPData->Release();
					return;
				}
			}
			DWORD dwByteRead = 0;
			if( FALSE == pPort->m_pDataPort->ReadSync( pIPData->GetBuffer(), pPort->m_wPacketBufSize, &dwByteRead ) ||\
				0 == dwByteRead ) 
			{
				break;			// 2003-3-5 改端口没有数据，轮到下个端口
			}

			symobj.Unlock();
			OnDataOK( pPort, pIPData, dwByteRead );
			pIPData->Release();
			pIPData = NULL;
			symobj.Lock();					
		}
	}

	symobj.Unlock();		//	申请内存失败，放弃
	if( pIPData )
	{
		m_pDecoderThread->DeAllocate( pIPData );
		pIPData->Release();
	}
}
