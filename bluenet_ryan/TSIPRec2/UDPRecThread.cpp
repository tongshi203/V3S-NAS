///---------------------------------------------------------
///
///      Chen Yongjian @ Xi'an Tongshi Technology Limited
///				2004.4.6
///      This file is implemented:
///				UDP Socket receive thread
///-----------------------------------------------------------
//---------------------------------------------------------------------------


#include "stdafx.h"

#include "UDPRecThread.h"
#include "DecoderThread.h"

#ifndef _WIN32
	#include <unistd.h>
#endif //_WIN32

extern CDecoderThread	g_DecoderThread;

//---------------------------------------------------------------------------

CUDPRecThread::CUDPRecThread()
{
	m_nPollFDCount = 0;
#ifdef _WIN32
	RtlZeroMemory( &m_SelectHandles, sizeof(m_SelectHandles) );
#else
	bzero( m_aPollFD, sizeof(m_aPollFD) );
    bzero( m_aDataPortItemAssociated, sizeof(m_aDataPortItemAssociated) );
#endif //_WIN32
}

CUDPRecThread::~CUDPRecThread()
{
	RemoveAll();
}


int CUDPRecThread::ExitInstance()
{
	RemoveAll();
	return 0;
}

#ifdef _WIN32
	BOOL CUDPRecThread::InitInstance()
#else
	bool CUDPRecThread::InitInstance()
#endif //_WIN32
{

#ifdef _WIN32
#ifndef _DEBUG
	SetThreadPriority( m_hThread, THREAD_PRIORITY_ABOVE_NORMAL );	//	2002.11.15, 改变到较高级别
#endif //_WIN32
#else
	nice( -5 );			// I dont known how to change priority
   #ifdef _DEBUG
	TRACE("CDecoderThread run is called\n");
   #endif //_DEBUG
#endif //_WIN32


	return true;
}

#ifdef _WIN32
	///-------------------------------------------------------
	/// CYJ,2004-5-16
	/// Function:
	///		Run function for Windows
	/// Input parameter:
	///		None
	/// Output parameter:
	///		None
	void CUDPRecThread::Run()
	{
		fd_set	 Handles;
		while( false == m_bIsRequestQuit )
		{
    		CSingleLock	syncobj( &m_SyncObj );
	   		if( FALSE == syncobj.Lock( 100 ) )
			{
				Sleep( 10 );
    			continue;
			}
			if( 0 == m_nPollFDCount )
			{
				syncobj.Unlock();
				Sleep( 100 );
        		continue;
			}
        	struct timeval TimeOutValue = {0, 100};
			RtlCopyMemory( &Handles, &m_SelectHandles, sizeof(m_SelectHandles) );
			if( select( 0, &Handles, NULL, NULL, &TimeOutValue ) <= 0 || 0 == Handles.fd_count )
			{
				syncobj.Unlock();
				Sleep( 10 );
				continue;
			}

			for( unsigned int i=0; i<Handles.fd_count; i++)
			{
				CIPData * pIPData = g_DecoderThread.AllocatePacket( 100 );
				if( NULL == pIPData )
            		break;				// no packet buffer
				if( FALSE == pIPData->SetBufSize( IP_MAX_PACKET_SIZE ) )
				{
					g_DecoderThread.DeAllocate( pIPData );
					pIPData->Release();
					continue;
				}
				int nRetVal = recv( Handles.fd_array[i], (char*)pIPData->GetBuffer(), IP_MAX_PACKET_SIZE, 0 );
				if( nRetVal <= 0 )
					g_DecoderThread.DeAllocate( pIPData );	// failed
				else
				{
					pIPData->PutDataLen( nRetVal );
					pIPData->m_pDataPortItem = m_mapDataPortItemAssociated[ int(Handles.fd_array[i]) ];
					g_DecoderThread.AddPacket( pIPData );
				}

				pIPData->Release();
			}
			g_DecoderThread.FlushAddCache( TRUE );
		}
	}

#else	// Linux
	///-------------------------------------------------------
	/// CYJ,2004-5-16
	/// Function:
	///		Run function for Linux
	/// Input parameter:
	///		None
	/// Output parameter:
	///		None
	void CUDPRecThread::Run()
	{
		int nContinuRunTimes = 0;	// 每运行10次，一定要主动放弃一次
						// 否则，其他线程无法快速获得同步权
		while( false == m_bIsRequestQuit )
		{
	   		CSingleLock	syncobj( &m_SyncObj );
	   		if( FALSE == syncobj.Lock( 0 ) )
			{
#ifdef __USE_GNU
				pthread_yield();
#endif //__USE_GNU
				Sleep( 50 );
	   			continue;
			}
			if( 0 == m_nPollFDCount )
			{
				syncobj.Unlock();
#ifdef __USE_GNU
				pthread_yield();
#endif //__USE_GNU
				Sleep( 50 );

	        	continue;
			}
			int i;
			for(i=0; i<m_nPollFDCount; i++)
			{
				m_aPollFD[i].revents = 0;
				m_aPollFD[i].events = POLLIN;
			}
			if( poll( m_aPollFD, m_nPollFDCount,100 ) <= 0 )
			{
				syncobj.Unlock();
#ifdef __USE_GNU
				pthread_yield();
#endif //__USE_GNU
				Sleep( 50 );

        		continue;
			}

			for( i=0; i<m_nPollFDCount; i++)
			{
        		if( 0 == (m_aPollFD[i].revents&POLLIN) )
            		continue;

				CIPData * pIPData = g_DecoderThread.AllocatePacket( 0 );
				if( NULL == pIPData )
            		break;				// no packet buffer
				if( FALSE == pIPData->SetBufSize( IP_MAX_PACKET_SIZE ) )
				{
					g_DecoderThread.DeAllocate( pIPData );
					pIPData->Release();
					continue;
				}
				int nRetVal = recv( m_aPollFD[i].fd, pIPData->GetBuffer(), IP_MAX_PACKET_SIZE, 0 );
				if( nRetVal <= 0 )
					g_DecoderThread.DeAllocate( pIPData );	// failed
				else
				{
					pIPData->PutDataLen( nRetVal );
					ASSERT( m_aDataPortItemAssociated[i] );
					pIPData->m_pDataPortItem = m_aDataPortItemAssociated[i];
					g_DecoderThread.AddPacket( pIPData );
				}

				pIPData->Release();
			}

			syncobj.Unlock();

			g_DecoderThread.FlushAddCache();
#ifdef __USE_GNU
			if( ++nContinuRunTimes > 10 )
			{
				nContinuRunTimes = 0;
				pthread_yield();
			}
#endif //__USE_GNU
		}
	}
#endif //_WIN32

void CUDPRecThread::Delete()
{
	// variable on data segment, need not delete
}

///------------------------------------------
/// Function:
///		Add one data port
/// Input Parameter:
///		lpszDstIP		destination multicast IP
///		nPort			Port
///		lpszLocalBindIP	local bind IP address
/// Output Parameter:
///		>0				succ, the data port in array
///		<0				failed
int CUDPRecThread::AddDataPort( LPCSTR lpszDstIP, int nPort, LPCSTR lpszLocalBindIP,COneDataPortItem * pDataPortItem )
{
    ASSERT( lpszDstIP && nPort>=1024 && pDataPortItem );
    if( NULL == lpszDstIP || nPort < 1024 || NULL == pDataPortItem )
    	return -1;

    int nNo = Find( lpszDstIP, nPort );
    if( nNo >= 0 )
    	return m_aDataPort.GetSize();

    CUDPDataPort * pDP = new CUDPDataPort;
    if( NULL == pDP )
    	return -1;

	if( false == pDP->Initialize( lpszDstIP, nPort, lpszLocalBindIP, pDataPortItem ) )
    {
    	delete pDP;
        return -1;
    }

    CSingleLock	syncobj( &m_SyncObj, TRUE );
    m_aDataPort.Add( pDP );
    RefleshPollFD();

    return m_aDataPort.GetSize();
}

///------------------------------------------
/// Function:
///		Find data port
/// Input Parameter:
///		lpszDstIP		dst ip
///		nPort			data port
///		bDoLock			lock, default is true
/// Output Parameter:
///		>=0				succ
///		<0				Not find
int CUDPRecThread::Find( LPCSTR lpszDstIP, int nPort, bool bDoLock )
{
    CSingleLock	syncobj( &m_SyncObj, bDoLock );

    struct sockaddr_in tmpIP;
    tmpIP.sin_port = htons( nPort );
    tmpIP.sin_addr.s_addr = inet_addr( lpszDstIP );

	int nCount = m_aDataPort.GetSize();
    for(int i=0; i<nCount; i++)
    {
    	CUDPDataPort * pDP = m_aDataPort[i];
#ifdef _DEBUG
		ASSERT( pDP );
#endif //_DEBUG
		if( NULL == pDP )
        	continue;

        if( tmpIP.sin_port != pDP->m_DstIP.sin_port )
        	continue;
        if( tmpIP.sin_addr.s_addr != pDP->m_DstIP.sin_addr.s_addr )
        	continue;

		return i;
    }

    return -1;
}

///------------------------------------------
/// Function:
///		delete one UDP data port
/// Input Parameter:
///		lpszDstIP		destination IP
///		nPort			the port to be delete
/// Output Parameter:
///		None
void CUDPRecThread::DeleteDataPort( LPCSTR lpszDstIP, int nPort )
{
	CSingleLock	syncobj( &m_SyncObj, TRUE );

	int nNo = Find( lpszDstIP, nPort, false );
        if( nNo < 0 )
       		return;

	WaitTillPacketBufIsEmpty();			// wait for in data list is empty

	CUDPDataPort * pDP = m_aDataPort[nNo];
	pDP->Invalid();
	delete pDP;
	m_aDataPort.RemoveAt( nNo, 1 );

	RefleshPollFD();
}

///------------------------------------------
/// Function:
///		the caller must lock the m_SyncObj
/// Input Parameter:
///		None
/// Output Parameter:
///		None
void CUDPRecThread::RefleshPollFD()
{
	int nCount = m_aDataPort.GetSize();
    if( nCount > MAX_DATAPORT_COUNT )
    	nCount = MAX_DATAPORT_COUNT;

	m_nPollFDCount = 0;
#ifdef _WIN32
	m_SelectHandles.fd_count = 0;
	m_mapDataPortItemAssociated.RemoveAll();
#endif //_WIN32

    for(int i=0; i<nCount; i++)
    {
    	CUDPDataPort * pDP = m_aDataPort[i];
#ifdef _DEBUG
		ASSERT( pDP && pDP->m_pDataPortItem  );
#endif //_DEBUG
		if( NULL == pDP )
        	continue;
#ifdef _WIN32
		int hSocket = *pDP;
		m_SelectHandles.fd_array[m_nPollFDCount] = hSocket;
		m_mapDataPortItemAssociated[ hSocket ] = pDP->m_pDataPortItem;
#else
		m_aPollFD[m_nPollFDCount].fd = *pDP;
		m_aDataPortItemAssociated[m_nPollFDCount] = pDP->m_pDataPortItem;
#endif //_WIN32

        m_nPollFDCount ++;

#ifdef _WIN32
		m_SelectHandles.fd_count ++;
		ASSERT( m_SelectHandles.fd_count == (unsigned int)m_nPollFDCount );
#endif //_WIN32
    }
}


///------------------------------------------
/// Function:
///		Remove all data port
/// Input Parameter:
///		None
/// Output Parameter:
///		None
void CUDPRecThread::RemoveAll()
{
	CSingleLock	syncobj( &m_SyncObj );
    if( FALSE == syncobj.Lock() )
    	return;				// timeout

	WaitTillPacketBufIsEmpty();			// wait for in data list is empty

    int nCount = m_aDataPort.GetSize();
    for(int i=0; i<nCount; i++)
    {
    	CUDPDataPort * pDP = m_aDataPort[i];
#ifdef _DEBUG
		ASSERT( pDP );
#endif //_DEBUG
		if( NULL == pDP )
        	continue;
        pDP->Invalid();
        delete pDP;
    }
}

///-------------------------------------------------------
/// CYJ,2004-5-25
/// Function:
///		wait till the look ahead packet buffer is empty
/// Input parameter:
///		nTimeOut			time out
/// Output parameter:
///		None
void CUDPRecThread::WaitTillPacketBufIsEmpty(int nTimeOut)
{
	time_t tNow = time(NULL);
	while( time(NULL) - tNow < nTimeOut )
	{
		if( 0 == g_DecoderThread.GetInDataItemCount( FALSE ) )
			return;
#ifdef _WIN32
		Sleep( 100 );
#else
		Sleep( 50 );
#endif //_WIN32
	}
}
