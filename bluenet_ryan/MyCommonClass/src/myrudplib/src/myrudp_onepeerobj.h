/*******************************************************************************
 *
 *	My RUDP One Peer Object
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.12 @ Xi'an
 *
 *
 ******************************************************************************/

#ifndef __MY_RUDP_ONE_PEER_OBJECT_H_20150112__
#define __MY_RUDP_ONE_PEER_OBJECT_H_20150112__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <list>

#include "myrudp_packet_mgr.h"
#include "myrudp_packet_sender.h"
#include "myrudp_server_drv.h"


#pragma pack( push, 8 )

class CMyRUDP_Socket;
class CMyRUDP_HeaderBuilder;
class CMyRUDP_Connection_Svr_Helper;
class CMyRUDP_EventObj;
class CMyRUDPDataSendTask;
class CMyRUDPDataReceiveTask;
class CMyRUDPNotifyDisconnectEventTask;
class CMyThreadPool;

class CMyRUDP_OnePeerObject :
	public CMyRUDP_OnePeerObjectInterface,
	public CMyRUDP_Packet_Sender
{
	friend class CMyRUDP_HeaderBuilder;
	friend class CMyRUDPDataSendTask;
	friend class CMyRUDPDataReceiveTask;
	friend class CMyRUDPNotifyDisconnectEventTask;
	friend class CMyRUDP_Connection_Svr_Helper;
public:
	CMyRUDP_OnePeerObject( CMyRUDP_Socket *	pSessionMgr );
	virtual ~CMyRUDP_OnePeerObject();

	//--------------------------------------------------------------
	/** CYJ 2015-01-16
	 *
	 *	Initialize One peer object
	 *
	 * @param [in]	PeerAddr		peer object to be connected
	 * @param [in]	nSessionID		sesstion ID associated with the session
	 *
	 * @return		0				succ
	 *				other			error code
	 */
	int Initialize( const struct sockaddr_in & PeerAddr, int nSessionID );

	//--------------------------------------------------------------
	/** CYJ 2015-01-16
	 *
	 *	Invalidate and free resource
	 */
	void Invalidate();

	//--------------------------------------------------------------
	/** CYJ 2015-01-21
	 *
	 *	Increase or Decrease reference
	 *
	 * @return	Increase or Decreased reference count
	 *
	 */
	virtual int AddRef();
	virtual int Release();

	//--------------------------------------------------------------
	/** CYJ 2015-01-21
	 *
	 *	Get Session TagID
	 *
	 * @param [in]
	 * @param [in]
	 * @param [in]
	 *
	 * @return
	 *
	 */
	uint32_t GetSesstionTagID() const { return m_dwSesstionTagID; }
	int GetSessionID()const { return m_nSesstionID; }

	//--------------------------------------------------------------
	/** CYJ 2015-01-16
	 *
	 *	On packet received
	 *
	 * @param [in]	pBuf			data buffer
	 * @param [in]	nLen			data size
	 * @param [in]	PeerAddr		peer socket addr
	 */
	void OnDataPacket( const unsigned char * pBuf, int nLen, const struct sockaddr_in & PeerAddr );
	int OnConnectDataPacket( const unsigned char * pBuf, int nLen );

	//--------------------------------------------------------------
	/** CYJ 2015-01-16
	 *
	 *	On timer per 1000 ms
	 *
	 * @param [in]	tNow			current time tick in UTC
	 */
	void OnTimer( time_t tNow );

	//--------------------------------------------------------------
	/** CYJ 2015-01-17
	 *
	 *	Check session
	 *
	 * @param [in]	pBuf			command data buffer
	 * @param [in]	dwSessionTagID	session tag ID
	 *
	 * @return		true			session is OK
	 *				false
	 */
	bool CheckSessionValid( const unsigned char *pBuf, uint32_t dwSessionTagID );

	virtual int GetState()const { return m_nState; }
	int GetMTUSizeIndex()const	{ return m_byMTUSizeIndex; }

	//--------------------------------------------------------------
	/** CYJ 2015-01-28
	 *
	 *	Send raw data to peer
	 *
	 * @param [in]	pBuf			data buffer to be sent
	 * @param [in]	nLen			data length
	 *
	 * @return		0				succ
	 *				other			error code
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
    /** CYJ 2015-02-12
     *
     *	Send data
     *
     * @param [in]	pBuf				data buffer
     * @param [in]	nLen				data length
     * @param [in]	pUserData			user data, used for notify data has been send succ.
     *									if user data = NULL, not send event
     * @param [in]	nTimeOut			timeout seconds
     *									0  		if no enough buffer then return immediately
     *									-1		wait infinitely
     *									other	time out seconds
     * @return		0					succ
     *				other				error code
     */
    virtual int Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut );

    //--------------------------------------------------------------
    /** CYJ 2015-03-12
     *
     *	Get Associated Server Drv
     */
    virtual CMyRUDP_ServerDrv * GetServerDrv();

protected:
	//--------------------------------------------------------------
	/** CYJ 2015-02-21
	 *
	 *	On data has been send succ task
	 */
	void ThreadTask_OnDataSend();

	//--------------------------------------------------------------
	/** CYJ 2015-02-21
	 *
	 *	On data has been recieved task
	 */
	void ThreadTask_OnDataReceived();

	//--------------------------------------------------------------
    /** CYJ 2015-02-15
     *
     *	Notify one packet has been send succ
     *
     * @param [in]	pUserData			user data
     */
    virtual void OnPacketHasBeenSendSucc( void * pUserData );

	//--------------------------------------------------------------
    /** CYJ 2015-04-02
     *
     *	Notify one packet has been send succ
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

    //--------------------------------------------------------------
    /** CYJ 2015-04-12
     *
     *	set force send out keep alive interval seconds
     *
     * @param [in]	nValue				interval in seconds
     *
     * @return		old value
     *
     * @note
     *		default interval is 270 seconds
     */
    virtual int SetForceSendingKeepAliveInterval( int nValue );

    //--------------------------------------------------------------
    /** CYJ 2015-04-18
     *
     *	Get Thread pool object
     */
    CMyThreadPool * GetThreadPoolObj(){ return m_pThreadPoolObj; }

    //--------------------------------------------------------------
    /** CYJ 2015-04-23
     *
     *	Get Keep alive counter = seconds from last getting data from peer
     *
     * @return	seconds from last time getting data from peer
     *
     */
    virtual int GetKeepAliveCount()const { return m_nKeepAliveCounter; }

protected:
	enum
	{
		KEEP_ALIVE_COUNTER_MAX_VALUE = 60*20,			// if no any data incoming in 20 minutes, then indicate the client is off line
		DEFAULT_SECONDS_TO_SEND_NOP_CMD_TO_PEER = 50,	// if no data send to peer, then force to send keep alive ( NOP ) command to peer when the peer send NOP cmd to me
		SECONS_SHOULD_GET_ACK_FROM_PEER = 30,			// assume should get ACK from peer after send out data packet
	};

private:
	bool CheckKeepAliveTimer();
	uint8_t * GetXorData(){ return m_abyXorData; }
	void OnSyncDataPacket( const unsigned char * pBuf, int nLen );
	void OnDATACmdDataPacket( const unsigned char * pBuf, int nLen );
	void SendACKToPeer( const uint16_t awACKSeqNo[], int nCount );
	void SendACKToPeer( uint16_t wHeadSeqNo, uint16_t wTailSeqNo );
	void OnACKCmdDataPacket( const unsigned char * pBuf, int nLen );
	void SendKeepAliveCmdToPeer();
	void SetStateToDisconnected();
	void OnResendCmdDataPacket( const unsigned char * pBuf, int nLen );
	void OnCloseCmdDataPacket( const unsigned char * pBuf, int nLen );
	void RequestPeerToResendPacket( bool bForce );
	void OnSkipCmdFromServer( const unsigned char *pBuf, int nLen );
	void OnKeepAliveCmdRecieved( int nSecondsFromLastReceiveData );
#ifdef __MYRUDP_USE_OPENSSL__
	void SetDataEncryptionAESKey( const uint8_t * pBuf, int nLen );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

protected:
	int					m_nRefCount;
	int 				m_nSesstionID;
	unsigned int 		m_dwSesstionTagID;
	int					m_nState;
	pthread_mutex_t 	m_SyncObj;
	struct sockaddr_in	m_PeerSockAddr;			// keep the peer address recently for send data to peer proactively, but the peer's IP may changed

	int					m_nKeepAliveCounter;	// keep alive counter, when reach KEEP_ALIVE_COUNTER_MAX_VALUE, then mean the client is off line, should disconnect itself
	CMyRUDP_Socket	*	m_pSessionMgr;
	unsigned char 		m_abyXorData[32];		// Xor Data
	uint8_t				m_byMTUSizeIndex;
	uint8_t				m_byTimeToSendNopCmdToPeer;	// send NOP command to peer to keep alive


	CMyRUDP_Connection_Svr_Helper * m_pConnectionHelper;

	CMyRUDP_PacketMgr	m_DataPacketMgr;
	CMyRUDP_EventObj  *	m_pEventObj;

	CMyRUDPDataSendTask *	m_pSendTask;
	CMyRUDPDataReceiveTask *m_pReceiveTask;
	CMyRUDPNotifyDisconnectEventTask * m_pDelayNotifyDisconnectTask;

	pthread_mutex_t 	m_SyncObj_SendTask;
	std::list<void*>	m_listDataHasBeenSendSucc;
	std::list<void*>	m_listDataHasBeenSendFailed;

	pthread_mutex_t 	m_SyncObj_ReceiveTask;
	CMyThreadPool *		m_pThreadPoolObj;
	int					m_nSecondsForceToSendKeepAlive;		// 2015.4.12 CYJ add
	bool				m_bSend2ndKeepAlivePacket;			// 2015.4.12 CYJ add, send 2nd Keep alive packet

	int					m_nSecondsShouldGetACK;				// 2015.4.13 CYJ add, help to check the peer is on line or not
	bool				m_bHasGetDataFromPeer;				// 2015.5.7 CYJ Add, to check the peer is alive or not

#ifdef __MYRUDP_USE_OPENSSL__
	uint8_t 			m_aDataEncryptionAESKey[48];
#endif //#ifdef __MYRUDP_USE_OPENSSL__
};

//////////////////////////////////////////////////////////////////
class CMyRUDP_OnePeerObject_AutoLock
{
public:
	CMyRUDP_OnePeerObject_AutoLock( CMyRUDP_OnePeerObject * pObj )
	{
	#ifdef _DEBUG
		assert( pObj );
	#endif // _DEBUG
		m_pObj = pObj;
		m_pObj->AddRef();
	}
	virtual ~CMyRUDP_OnePeerObject_AutoLock()
	{
		m_pObj->Release();
	}
	bool IsValid() const { return NULL != m_pObj; }
protected:
	CMyRUDP_OnePeerObject * m_pObj;
};

#pragma pack( pop )

#endif // __MY_RUDP_ONE_PEER_OBJECT_H_20150112__

