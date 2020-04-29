/*********************************
 *
 *
 *********************************/
#include <stdafx.h>

#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <sys/select.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <list>

#include "myrudp_client_drv.h"
#include "myrudp_event.h"
#include "myrudp_cmd_value.h"
#include "myrudp_dbgprint.h"
#include <MySyncObj.h>


// #define __TEST_LOCAL_SENDING__

//#define __TEST_USING_THREAD_POOL__

#ifdef __TEST_USING_THREAD_POOL__
	#include <mythreadpool.h>
	extern void MyRUDPClientSetThreadPool( CMyThreadPool * pThreadPool );
	CMyThreadPool	s_ThreadPool;
#endif // #ifdef __TEST_USING_THREAD_POOL__

#ifdef _DEBUG
extern void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen );
#endif // #ifdef _DEBUG

////////////////////////////////////////////
CMyRUDP_ClientDrv	*	g_pRUDP_Client = NULL;
uint8_t					g_abyBuf[ MYRUDP_MAX_DATA_LENGTH ];

CMutex					g_SkipDataListSyncObj;
std::list<uint32_t>		g_listSkipData;


////////////////////////////////////////////
class CMyRUDP_EventObjImpl : public CMyRUDP_EventObj
{
public:
	CMyRUDP_EventObjImpl()
	{
		m_dwLastReceived = 1;
	}

	virtual ~CMyRUDP_EventObjImpl()
	{
	}

	virtual void OnSendDataFailed( void * pUserData )
	{
		CSingleLock Sync( &g_SkipDataListSyncObj, true );

		g_listSkipData.push_back( (uint32_t)(long long)pUserData );
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
    	MyRUDP_fprintf( "=====*** %d bytes received 0x%02x <==> 0x%08x\n", nLen, pBuf[0], m_dwLastReceived );
    	assert( nLen >= 6 );

//    	DbgDumpData( "InData:", pBuf, nLen );

    	uint32_t dwSeqNo = *(uint32_t*)pBuf;
    	uint16_t wDataLen = *(uint16_t*)(pBuf+4);

    	CSingleLock Sync( &g_SkipDataListSyncObj, true );

    	int nTryTimes = g_listSkipData.size();
    	while( m_dwLastReceived != dwSeqNo )
		{
			bool bFound = false;
			std::list<uint32_t>::iterator it;
			for( it=g_listSkipData.begin(); it!=g_listSkipData.end(); ++it )
			{
				if( *it != m_dwLastReceived )
					continue;
				g_listSkipData.erase( it );
				m_dwLastReceived ++;
				bFound = true;
				break;
			}
			assert( bFound );
			nTryTimes --;
			assert( nTryTimes >= 0 );
		}

    	assert( wDataLen == nLen );

    	for(int i=6; i<nLen; i++ )
		{
			assert( pBuf[i] == pBuf[0] );
		}

		if( m_dwLastReceived != dwSeqNo )
		{
			MyRUDP_fprintf( "===== error data, 0x%08x != 0x%08x\n", m_dwLastReceived, dwSeqNo );
			m_dwLastReceived = dwSeqNo;
		}

		m_dwLastReceived ++;
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
    	return m_abyDataBuf;
    }

    virtual int AddRef()
	{
		return 1;
	}
    virtual int Release()
    {
		return 0;
    }
protected:
	uint32_t	m_dwLastReceived;
	uint8_t		m_abyDataBuf[ 0x10000 ];
};

////////////////////////////////////////////////////////////
CMyRUDP_EventObjImpl	g_EventObject;
uint32_t g_dwBps = 0;

volatile bool 	g_bExit = false;

static void Ctrl_C_SigRroutine(int unused)
{
	fprintf( stderr, "Catch a signal SIGINT\n " );
	g_bExit = true;
}

int main( int agc, char * argv[] )
{
	signal( SIGINT, Ctrl_C_SigRroutine );

#ifdef __TEST_USING_THREAD_POOL__
	s_ThreadPool.Initialize( 4 );
	MyRUDPClientSetThreadPool( &s_ThreadPool );
#endif // #ifdef __TEST_USING_THREAD_POOL__

	g_pRUDP_Client = MyRUDP_CreateClientDrv();
	if( NULL == g_pRUDP_Client )
	{
		fprintf( stderr, "Failed to create my rudp client driver\n" );
		return 2;
	}

	const char * lpszServerIP = "127.0.0.1";
	if( g_pRUDP_Client->Open( lpszServerIP, 5555, &g_EventObject ) )
	{
		fprintf( stderr, "Open and connect to server failed.\n" );
		return 1;
	}

	time_t tStart = time(NULL)-1;
	time_t tNow = time(NULL);
	uint32_t dwSeqNo = 1;
	int nByteSendLastSecond = 0;
	uint16_t awMask[]={ 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };

	g_pRUDP_Client->SetKeepAliveInterval( 300 );

	g_bExit = false;
	while( false == g_bExit )
	{
#ifdef __TEST_LOCAL_SENDING__
		for(int i=0; i<10; i++ )
		{
			int nTimeTmp = time( NULL ) % 3600;
			fprintf( stderr, "[%02d:%02d] \n", nTimeTmp / 60, nTimeTmp % 60 );
			sleep( 1 );
		}
#endif // __TEST_LOCAL_SENDING__

		if( g_pRUDP_Client->GetState() != CMyRUDP_ClientDrv::MYRUDP_CLIENT_STATE_CONNECTED )
			continue;

		int nLen = ( rand() * 0x100 + rand() );
		nLen = nLen & awMask[ nLen & 0x7 ];
		if( nLen < 16 )
			continue;
		memset( g_abyBuf, (uint8_t)dwSeqNo, nLen );
		*(uint32_t*)g_abyBuf=dwSeqNo;
		*(uint16_t*)(g_abyBuf+4)=(uint16_t)nLen;

		int nBytePerSecond = g_dwBps;
		if( g_pRUDP_Client->Send( g_abyBuf, nLen, (void*)dwSeqNo, 5000 ) )
			continue;

		dwSeqNo ++;
		nByteSendLastSecond += nLen;
		tNow = time(NULL);
		if( tNow != tStart )
		{
			g_dwBps = ( 3*g_dwBps + nByteSendLastSecond ) / 4;
			tStart = tNow;
			nByteSendLastSecond = 0;
			int nTimeTmp = tNow % 3600;
			fprintf( stderr, "[%02d:%02d] send to server  %d,%03d,%03d Bps.\n",\
					nTimeTmp / 60, nTimeTmp % 60, nBytePerSecond /1000000, (nBytePerSecond % 1000000)/1000, nBytePerSecond % 1000 );
		}
	}

	fprintf( stderr, "\n----------------- Close Client.\n" );

#ifdef __TEST_USING_THREAD_POOL__
	s_ThreadPool.Invalidate();
#endif // #ifdef __TEST_USING_THREAD_POOL__

	g_pRUDP_Client->Close();
	g_pRUDP_Client->Release();
	g_pRUDP_Client = NULL;

	return 0;
}

