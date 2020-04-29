/*********************************************************************************
 *
 *	My RUDP Connection for client
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.29 @ Xi'an
 *
 *
 ********************************************************************************/

#ifndef __MY_RUDP_CLIENT_H_20150129__
#define __MY_RUDP_CLIENT_H_20150129__

#include <stdafx.h>
#include <mydatatype.h>
#include <MySyncObj.h>
#include <string>

#include <myframebufmgrhelper.h>

#include <pthread.h>
#include <list>
#include <semaphore.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// for testing, support Thread Pool
// __TEST_USING_THREAD_POOL__ will be defined in makefile
#ifdef __TEST_USING_THREAD_POOL__
	#include <mythreadpool.h>
#endif //__TEST_USING_THREAD_POOL__

#include "myrudp_packet_sender.h"
#include "myrudp_packet_mgr.h"
#include "myrudp_client_drv.h"

#ifdef __MYRUDP_USE_OPENSSL__
	#include "myrudp_openssl.h"
#endif // #ifdef __MYRUDP_USE_OPENSSL__

class CMyRUDP_EventObj;

#pragma pack( push, 8 )

class CMyRUDP_Client :
	public CMyRUDP_ClientDrv,
	protected CMyRUDP_Packet_Sender
{
public:
	CMyRUDP_Client();
	virtual ~CMyRUDP_Client();

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Open and connect to the server automatically, if disconnect from the server, it will re-connect again
	 *
	 * @param [in]	pszServerIP			server IP
	 * @param [in]	nPort				server port
	 * @param [in]	pEventObj			event object
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	virtual int Open( const char * pszServerIP, int nPort, CMyRUDP_EventObj * pEventObj );

	//--------------------------------------------------------------
	/** CYJ 2015-05-31
	 *
	 *	Reopen server IP and port
	 *
	 * @param [in]	pszServerIP			server IP
	 * @param [in]	nPort				server port
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	virtual int SetServerIPAndPort( const char * pszServerIP, int nPort );

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Disconnect from the server and free resources
	 */
	virtual void Close();

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Get working state, see also MYRUDP_CLIENT_STATE_xxx
	 *
	 * @return	working state
	 *
	 */
	virtual int GetState(){ return m_nState; }

	enum
	{
		THREAD_STATE_IDLE = 0,				// idle
		THREAD_STATE_RUNNING,				// running
		THREAD_STATE_REQ_EXIT,				// request to exit
	};

	//--------------------------------------------------------------
    /** CYJ 2015-02-12
     *
     *	Send data to peer
     *
     * @param [in]	pBuf				data buffer to be send
     * @param [in]	nLen				data length
     *
     * @return		0					succ
     *				other				error
     */
    virtual int SendRawDataToPeer( const uint8_t * pBuf, int nLen );

	//--------------------------------------------------------------
    /** CYJ 2015-04-13
     *
     *	Send data item to peer, need ACK
     *
     * @param [in]	pBuf				data buffer to be send
     * @param [in]	nLen				data length
     *
     * @return		0					succ
     *				other				error
     */
    virtual int SendDataPacketToPeer( const uint8_t * pBuf, int nLen );

    //--------------------------------------------------------------
    /** CYJ 2015-02-26
     *
     *	Register Fast timer callback function
     */
    virtual void RegisterFastTimer();

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
    /** CYJ 2015-02-12
     *
     *	Send data
     *
     * @param [in]	pBuf				data buffer
     * @param [in]	nLen				data length
     * @param [in]	pUserData			user data, used for notify data has been send succ.
     *									if NULL, do not send Event
     * @param [in]	nTimeOut			timeout seconds
     *									0  		if no enough buffer then return immediately
     *									-1		wait infinitely
     *									other	time out seconds
     * @return		0					succ
     *				other				error code
     */
    virtual int Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut );

    //--------------------------------------------------------------
    /** CYJ 2015-04-03
     *
     *	Set keep alive packet interval, in seconds
     *
     * @param [in]	nInterval			keep alive packet interval, in seconds
     *									default is 10 seconds
     *									[ 10 - 300 ]
     */
    virtual void SetKeepAliveInterval( int nInterval );

    //--------------------------------------------------------------
    /** CYJ 2015-04-03
     *
     *	Set network is avaiable or not
     *
     * @param [in]	bEnable				network is available or not
     *									true		available
     *									false		not available
     * @return		old value
     *
     * @note
     *	If network is not avaiable, then not try to connect to network, default is available
     */
    virtual bool SetNetowrkAvailable( bool bEnable );

    //--------------------------------------------------------------
    /** CYJ 2015-04-09
     *
     *	Check network station and sent keep alive packet, is disconnected, reconnect it
     *
     * @note
     *		For Android, sometimes the thread is wake up very long, so using this function to check the connection
     */
    virtual void CheckNetworkConnection();

#ifdef _DEBUG
	void DebugDump();
#endif //_DEBUG

protected:
	//--------------------------------------------------------------
    /** CYJ 2015-02-15
     *
     *	Notify one packet has been send succ
     *
     * @param [in]	pUserData			user data
     */
    virtual void OnPacketHasBeenSendSucc( void * pUserData );

	//--------------------------------------------------------------
    /** 2015-04-02
     *
     *	Notify one packet has been send failed
     *
     * @param [in]	wHeadSeqNo			HeadSeqNo
     * @param [in]	wTailSeqNo			TailSeqNo
     * @param [in]	pUserData			user data
     *
     * @note
     *		should do 2 step:
     *		(1)			send SKIP command to peer
     *		(2)			send send data failed event to APP
     */
    virtual void OnPacketSendFailed( uint16_t wHeadSeqNo, uint16_t wTailSeqNo, void * pUserData );

protected:
	int SendReqCmd();						// send REQ command to server
	void SendSyncCmd();						// send SYN command to server
	void OnRspCmd( unsigned char *pBuf, int nLen);		// get RSP command from server
	void OnSync2Cmd( unsigned char *pBuf, int nLen);	// Get ACK of Sync
	void 	Run_ReadData();					// read data thread run function
	static void *RunLink_ReadData( void * pThis );
	void 	Run_ProcessData();				// process data thread run function
	static void *RunLink_ProcessData( void * pThis );
	void DoReceiveData();
	void OnDataReceivedFromServer( unsigned char *pBuf, int nLen, const struct sockaddr_in & SrcAddr );
	void OnTimerOneSecond();
	void SendCloseCmdToServer();
	void SendKeepAliveToServer();
	void PrepareCommonHeader( uint8_t * pBuf, uint8_t byCmdVale );
	void CommitCommonHeader( uint8_t * pBuf );
	void UpdateCRC( uint8_t * pBuf );
	void OnCmdCloseFromServer( uint8_t * pBuf );
	bool VerifySessionAndTagID( uint8_t * pBuf );
	void OnDataCmdFromServer( unsigned char *pBuf, int nLen );
	void OnAckCmdFromServer( unsigned char *pBuf, int nLen );
	void OnResendCmdDataPacket( unsigned char *pBuf, int nLen );
	void OnSkipCmdFromServer( unsigned char *pBuf, int nLen );
	void SendACKToPeer( const uint16_t awACKSeqNo[], int nCount );
	void SendACKToPeer( uint16_t wHeadSeqNo, uint16_t wTailSeqNo );
	void RequestPeerToResendPacket( bool bForce );
	void NotifyHasDataToBeProcess();
	void OnDataReadyEvent();
	uint8_t * AllocateInputDataBuf();
	void GetIPFromDSN();
	void OnKeepAlivePacketRecieved( int nSecondsLastRecived );

#ifdef __MYRUDP_USE_OPENSSL__
	bool GetDataEncryptionAESKey( const uint8_t * pbyPeerPubKey, int nLen );
	std::vector<uint8_t> GetEncryptedPublicKey();
	void SetDataEncryptionAESKey( const uint8_t * pBuf, int nLen );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	enum
	{
		IN_DATA_BUFFER_SIZE = 4096,
		DEFAULT_KEEP_ALIVE_COUNTER = 50,	// send out keep alive command packet per 50 seconds
		SECONDS_RESEND_REQ_CMD = 6,			// if not get RSP command, then resend ReqCommand
		SECONDS_RESEND_SYN_CMD = 6,			// if not get Syn2 command, then resend Sync command
		MAX_SECONDS_SHOULD_GET_KAP = 10,	// when sending KeepAlive packet to server, the client should get the respone keep alive packet from the server
	};

	enum
	{
		READ_DATA_BUFFER_SIZE = 512L*1024,	// 512KB in data cache
		READ_DATA_PACKET_MAX_SIZE = 4096,		// 4KB
	};

	enum
	{
		EVENT_DATA_TYPE_RECEIVE_DATA = 1,
		EVENT_DATA_TYPE_SEND_OK = 2,
	};

	typedef struct tagEVENT_DATA_ITEM
	{
		int 	m_nEventType;
		union
		{
			void *	m_pEventData;
			int		m_nEventData;
		};
	}EVENT_DATA_ITEM,*PEVENT_DATA_ITEM;

protected:
	pthread_t		m_hThread_ReadData;
	pthread_t		m_hThread_ProcessData;
	volatile int	m_nThreadState;

	int				m_nRefCount;

	std::string		m_strServerIP;				// 2015.5.7 CYJ Add
	pthread_mutex_t m_SyncObj_SendOut;
	int 			m_hSocket;					// socket to receive and send data

	int 					m_nState;
	struct sockaddr_in 		m_ServerAddr;
	uint8_t					m_abyXorData[32];	// xor data for SessionTagID
	uint32_t				m_dwSessionID;		// session ID
	uint32_t				m_dwSessionTagID;	// session Tag ID
	uint16_t				m_wSeqNo;

	time_t 					m_tNow;
	int						m_nCounterToSendKeepAliveCmd;
	int						m_nSecondsFromLastReceiveFromServer;		// seconds from the last data recieved from server, if too long, should reconnect to server
	int						m_nSecondsToResendReqOrSyncCmd;

	uint8_t					m_byMTUIndex;
	uint16_t				m_wMTUSize;

	CMyRUDP_PacketMgr		m_InPacketMgr;								// in packet manager
	CMyRUDP_EventObj 	  * m_pEventObj;

	bool	volatile		m_bNeedFastTimer;
	int						m_nSendKeepAliveIntervalInSeconds;			// 2015.4.3 CYJ Add
	bool					m_bNetworkIsAvailable;						// 2015.4.3 CYJ Add
	int						m_nSecondsToReconnectToServer;				// 2 * m_nSendKeepAliveIntervalInSeconds

	CMyFrameRingBufMgr<sockaddr_in>	m_InCacheBuf;						// UDP input data buffer

#ifdef __MYRUDP_USE_OPENSSL__
	CMyRUDPCryption			m_OpenSSLDrv;
	uint8_t 				m_abyDataEncryptionAESKey[48];
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	// pipe[0] for read, 	pipe[1] for write
	int						m_aPipeToWakeupSelect[2];					// 2015.4.28 CYJ Add, to wake up select

protected:
#ifdef __TEST_USING_THREAD_POOL__
	class CReadDataTask : public CMyThreadPoolTask
	{
	public:
		CReadDataTask(){ m_pClient = NULL; }
		~CReadDataTask(){}
		virtual void RunTask();
		virtual int AddRef(){ return m_pClient->AddRef(); }
		virtual int Release(){ return m_pClient->Release(); }
		CMyRUDP_Client * m_pClient;
		CMutex				m_ReadDataTaskSyncObj;
	};
	class CProcessDataTask : public CMyThreadPoolTask
	{
	public:
		CProcessDataTask(){ m_pClient = NULL; }
		~CProcessDataTask(){}
		virtual void RunTask();
		virtual int AddRef(){ return m_pClient->AddRef(); }
		virtual int Release(){ return m_pClient->Release(); }
		CMyRUDP_Client * m_pClient;
		CMutex				m_ProcessDataTaskSyncObj;
	};

	CReadDataTask		m_ReadDataTask;
	CProcessDataTask	m_ProcessDataTask;
#endif // #ifdef __TEST_USING_THREAD_POOL__
};

#pragma pack( pop )

#endif // __MY_RUDP_CLIENT_H_20150129__

