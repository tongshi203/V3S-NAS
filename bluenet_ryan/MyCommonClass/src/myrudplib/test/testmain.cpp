#include <stdafx.h>
#include <stdio.h>
#include <mydatatype.h>
#include <MySyncObj.h>

#include "myrudp_event.h"
#include "myrudp_dbgprint.h"
#include "myrudp_server_drv.h"
#include "myrudp_cmd_value.h"

#include <myframebufmgrhelper.h>
#include <mythreadpool.h>
#include <signal.h>

// #define __USE_LOCAL_SENDING__

////////////////////////////////////////////
CMyRUDP_ServerDrv *		g_pSocketDrv = NULL;
CMyRUDP_OnePeerObjectInterface *	g_pOnePeerObject = NULL;
uint8_t				g_abyBuf[ MYRUDP_MAX_DATA_LENGTH ];
uint32_t g_dwBps = 0;

////////////////////////////////////////////
class CMyRUDP_EventObjImpl : public CMyRUDP_EventObj, public CMyThreadPoolTask
{
public:
	CMyRUDP_EventObjImpl( CMyRUDP_OnePeerObjectInterface * pPeerObj )
	: m_Buf( 1024L*1024*64 )
	{
		m_nRefCount = 1;
		m_pPeerObj = pPeerObj;
		pthread_mutex_init( &m_SyncObj, NULL );
	}

	virtual ~CMyRUDP_EventObjImpl()
	{
		pthread_mutex_destroy( &m_SyncObj );
	}

	//--------------------------------------------------------------
    /** CYJ 2015-02-25
     *
     *	Allocate memory
     *
     * @param [in]	nBufSize			data buffer size
	 *
     * @return		NULL				succ
     *				other				buffer pointer
     */
    virtual unsigned char * OnDataReceived_Allocate( int nBufSize )
    {
		return m_Buf.Allocate( nBufSize );
    }

	//--------------------------------------------------------------
    /** CYJ 2015-02-14
     *
     *	On data received
     *
     * @param [in]	nLen			data length
     *
	 * @note	may be called in different thread
     */
    virtual void OnDataReceived_Commit( const unsigned char * pBuf, int nLen, int nBufSize )
    {
    //	MyRUDP_fprintf( "===== %d bytes received, 0x%02x\n", nLen, *pBuf );

	#ifdef __USE_LOCAL_SENDING__
		return;
	#else
		if( NULL == m_pPeerObj )
			return;
		uint8_t tDummyVal = 0;
		m_Buf.Submit( nLen, tDummyVal );

		g_pSocketDrv->GetThreadPoolObject()->AddTask( static_cast<CMyThreadPoolTask*>(this) );
	#endif
    }

    virtual void RunTask()
	{
		if( NULL == m_pPeerObj )
			return;

		pthread_mutex_lock( &m_SyncObj );

		PBYTE pBuf;
		DWORD nLen = 0;
		while( m_Buf.Peek( pBuf, nLen, 0 ) )
		{
			m_pPeerObj->Send( pBuf, nLen, NULL, -1 );
			m_Buf.Free();
		}

		pthread_mutex_unlock( &m_SyncObj );
	}

	virtual int AddRef()
	{
		return InterlockedIncrement( &m_nRefCount );
	}
    virtual int Release()
    {
		int nRetVal = InterlockedDecrement( &m_nRefCount );
		if( 0 == nRetVal )
		{
			delete this;
			return 0;
		}
		else
			return nRetVal;
    }

protected:
	CMyRUDP_OnePeerObjectInterface * m_pPeerObj;
	CMyFrameRingBufMgr<uint8_t>	m_Buf;
	pthread_mutex_t 	m_SyncObj;
	int m_nRefCount;
};

//////////////////////////////////////////////
class CMyRUDP_SocketEventImpl : public CMyRUDP_ServerEvent
{
public:
	CMyRUDP_SocketEventImpl(){}
	virtual ~CMyRUDP_SocketEventImpl(){}

	virtual CMyRUDP_EventObj * OnNewConnection( CMyRUDP_OnePeerObjectInterface * pPeerObj )
	{
		MyRUDP_fprintf( "================ OnNewConnection ( %p )\n", pPeerObj );
		CMyRUDP_EventObjImpl * pRetVal = new CMyRUDP_EventObjImpl( pPeerObj );
		if( NULL == pRetVal )
			return NULL;
		g_pOnePeerObject = pPeerObj;
		return static_cast< CMyRUDP_EventObj* >( pRetVal );
	}
};


////////////////////////////////////////////
CMyRUDP_SocketEventImpl	g_SocketEvent;

volatile bool 	g_bExit = false;

static void Ctrl_C_SigRroutine(int unused)
{
	fprintf( stderr, "Catch a signal SIGINT\n " );
	g_bExit = true;
}

////////////////////////////////////////////
int main( int argc, char *argv[])
{
	signal( SIGINT, Ctrl_C_SigRroutine );

	g_pSocketDrv = MyRUDP_CreateServerDrv();
	if( NULL == g_pSocketDrv )
	{
		fprintf( stderr, "Failed to create server driver.\n" );
		return -1;
	}
	int nRetVal = g_pSocketDrv->Open( 2, &g_SocketEvent, 5555 );

#ifdef __USE_LOCAL_SENDING__
	time_t tStart = time(NULL)-1;
	time_t tNow = time(NULL);
	int nDeltaTime = 1;
	uint32_t dwSeqNo = 0;
	int nByteSendLastSecond = 0;
	uint16_t awMask[]={ 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };
#endif // __USE_LOCAL_SENDING__

	g_bExit = false;
	while( false == g_bExit )
	{
#ifndef __USE_LOCAL_SENDING__
		sleep( 1 );
#else
		if( NULL == g_pOnePeerObject || g_pOnePeerObject->GetState() != CMyRUDP_OnePeerObjectInterface::PEER_OBJECT_STATE_CONNECTED )
		{
			sleep( 1 );
			continue;
		}

		int nLen = ( rand() * 0x100 + rand() );
		nLen = nLen & awMask[ nLen & 0x7 ];
		if( nLen < 16 )
			continue;
		memset( g_abyBuf, (uint8_t)dwSeqNo, nLen );
		*(uint32_t*)g_abyBuf=dwSeqNo;
		*(uint16_t*)(g_abyBuf+4)=(uint16_t)nLen;

		g_pOnePeerObject->Send( g_abyBuf, nLen, (void*)dwSeqNo, -1 );

		dwSeqNo ++;
		nByteSendLastSecond += nLen;
		tNow = time(NULL);
		if( tNow != tStart )
		{
			g_dwBps = ( 3*g_dwBps + nByteSendLastSecond ) / 4;
			tStart = tNow;
			nByteSendLastSecond = 0;

			int nTimeTmp = tNow % 3600;
			uint32_t nBytePerSecond = g_dwBps;
			MyRUDP_fprintf( "[%02d:%02d] send to server  %d,%03d,%03d Bps.\n",\
							nTimeTmp/60, nTimeTmp%60,\
							nBytePerSecond /1000000, (nBytePerSecond % 1000000)/1000, nBytePerSecond % 1000 );
		}
#endif // __USE_LOCAL_SENDING__
	}

	g_pSocketDrv->Close();
	g_pSocketDrv->Release();
	g_pSocketDrv = NULL;

	return 0;
}

