/***********************************************************************
 *
 *	My RUDP Socket functions, server side
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.12 @ Xi'an
 *
 *
 * FIXME: only support IPv4 now, later should support IPv6
 *
 ***********************************************************************/

#ifndef __MY_RUDP_SOCKET_H_20150112__
#define __MY_RUDP_SOCKET_H_20150112__

/////////////////////////////////////////////////////////////////
#include <mythreadpool.h>

#include <stdafx.h>
#include <mydatatype.h>
#include <MySyncObj.h>

#include <pthread.h>
#include <list>
#include <semaphore.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <myframebufmgrhelper.h>


#include "myrudp_onepeerobj.h"
#include "myrudp_server_drv.h"

#define MY_RUDP_MAX_SESSION_NUMBER		10240

#pragma pack( push, 8 )

class CMyRUDP_EventObj;


/////////////////////////////////////////////////////////////////
class CMyRUDP_Socket : public CMyRUDP_ServerDrv
{
	friend class CMyRUDP_OnePeerObject;
public:
	CMyRUDP_Socket();
	virtual ~CMyRUDP_Socket();

	//--------------------------------------------------------------
	/** CYJ 2015-01-12
	 *
	 *	Create One UDP socket and bind to the local IP, and preset data
	 *
	 * @param [in]	nThreadCount		Thread count
	 * @param [in]	pThreadPoolObj		thread pool object
	 * @param [in]	pEventObj			event object
	 * @param [in]	LocalBind			local bind IP
	 * @param [in]	nPort				listen port
	 * @param [in]	pszLocalBindIP		local bind IP, if NULL, listen on all network interface
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	virtual int Open( int nThreadCount, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP = NULL );
	virtual int Open( CMyThreadPool * pThreadPoolObj, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP = NULL );

	//--------------------------------------------------------------
	/** CYJ 2015-01-12
	 *
	 *	Close the socket and exit the working thead
	 *
	 * send 2 MYRUDP_CLOSE data to all connections but not wait for ACK
	 */
	virtual void Close();

	//--------------------------------------------------------------
	/** CYJ 2015-01-12
	 *
	 *	Send data to the peer, just call sento with sync
	 *
	 * @param [in] 	pBuf				data to sent
	 * @param [in]	nLen				data length
	 * @param [in]	pDstAddr			peer socket addr
	 * @param [in]	nAddrLen			peer socket addr length
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	virtual int SendTo( const unsigned char *pBuf, int nLen, const struct sockaddr * pDstAddr, socklen_t nAddrLen );

	//--------------------------------------------------------------
	/** CYJ 2015-02-21
	 *
	 *	Create Event object, which will associated with the peer object
	 *
	 * @param [in]	pPeerObject			peer object
	 *
	 * @return		NULL				failed
	 *				other				event responser
	 */
	CMyRUDP_EventObj * CreateEventObject( CMyRUDP_OnePeerObject * pPeerObject );

	//--------------------------------------------------------------
	/** CYJ 2015-02-25
	 *
	 *	Get thread pool object
	 *
	 * @return	thread pool object
	 */
	virtual CMyThreadPool * GetThreadPoolObject()  { return m_pThreadPoolObj; }

	//--------------------------------------------------------------
	/** CYJ 2015-02-26
	 *
	 *	Register fast timer
	 *
	 * @param [in]	nSessionID
	 */
	void AddFastTimer( int nSessionID );

	//--------------------------------------------------------------
	/** CYJ 2015-02-26
	 *
	 *	Remove fast timer
	 *
	 * @note
	 *		if the FastTimer has been scheduled, then can not remove it
	 */
	void RemoveFastTimer( int nSessionID );

	    //--------------------------------------------------------------
    /** CYJ 2015-03-02
     *
     *	Add or Release refernce count
     *
     * @return	updated reference count
     *
     * @note
     *		when reference count = 0, delete itself
     */
    virtual int AddRef();
    virtual int Release();

    //--------------------------------------------------------------
    /** CYJ 2015-04-21
     *
     *	Check socket is closed
     *
     * @return	true					closed
     *			false					socket is openned
     */
    bool IsClosed(){ return( m_hSocket < 0); }

protected:
	//--------------------------------------------------------------
	/** CYJ 2015-01-23
	 *
	 *	Disconnect the session, only called by CMyRUDP_OnePeerObject
	 *
	 * @param [in]	nSessionID			session ID to be disconnected
	 */
	void DisconnectSession( int nSessionID );

	enum
	{
		READ_DATA_BUFFER_SIZE = 48L*1024*1024,	// 48 MB in data cache
		READ_DATA_PACKET_MAX_SIZE = 0x10000,	// 64KB
	};

	enum
	{
		THREAD_STATE_IDLE = 0,				// idle
		THREAD_STATE_RUNNING,				// running
		THREAD_STATE_REQ_EXIT,				// request to exit
	};

private:
	void 	Run_ReadData();					// read data thread run function
	void 	Run_ProcessData();				// process data thread run function
	void 	Run_Timer();					// process data thread run function
	static void *RunLink_ReadData( void * pThis );
	static void *RunLink_ProcessData( void * pThis );
	static void *RunLink_Timer( void * pThis );
	void 	OnDataReceived( const unsigned char *pBuf, int nLen, const struct sockaddr_in * pSrcAddr );
	void 	OnPeerReqConnecting( const unsigned char *pBuf, int nLen, const struct sockaddr_in * pSrcAddr );
	void 	IncreateReadThreadPriority();
	int		WaitForInDataReady();
	unsigned char * AllocateInputDataBuf();
	int 	IsSessionValid( const unsigned char *pBuf, int dwSessionID, uint32_t dwSessionTagID );
	int		AllocateOneSession( const struct sockaddr_in & PeerAddr );
	void 	DeallocateOneSession( int nSessionID );
	bool	IsValidSessionID( unsigned char * pBuf );
	void	RunTimer();
	void	RunFastTimer();
	void 	DoDelayRemoveSession();

protected:
	pthread_t	m_hThread_ReadData;
	pthread_t	m_hThread_ProcessData;
	pthread_t	m_hThread_Timer;
	volatile int	m_nThreadState;
	int				m_nRefCount;

	int 	m_hSocket;							// socket to receive and send data
	struct sockaddr_in m_LocalBindIP;
	pthread_mutex_t m_SyncObj_Socket;			// socket sync object for sending data

	CMyFrameRingBufMgr<sockaddr_in>	m_InDataBuf;	// UDP input data buffer

	// m_apPeerObjects will only be modify in the process_data thread
	// but may be used in other thread, so add a read/write sync object
	CReadWriteLock	m_PeerObject_SyncObj;
	CMyRUDP_OnePeerObject * m_apPeerObjects[ MY_RUDP_MAX_SESSION_NUMBER ];
	std::list<int> 	m_listUsedSession;			// used session ID
	std::list<int> 	m_listFreeSession;			// free session ID
	CMutex			m_SyncObj_DelayRemoveSession;
	std::list<int> 	m_listDelayRemoveSession;	// free session ID

	pthread_mutex_t 		m_SyncObj_FastTimer;
	pthread_cond_t			m_Event_FastTimer;
	pthread_condattr_t		m_Event_FastTimer_Attr;
	bool m_anFastTimerSessionID[ MY_RUDP_MAX_SESSION_NUMBER ];
	volatile bool m_bHasFastTimer;

	time_t	m_tNow;								// current time tick for timer
	CMyRUDP_ServerEvent * 	m_pEventObj;
	CMyThreadPool  		*	m_pThreadPoolObj;
};

#pragma pack( pop )

#endif // __MY_RUDP_SOCKET_H_20150112__
