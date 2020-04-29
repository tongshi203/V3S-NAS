/******************************************************************************
 *
 *	My RUDP server driver interface
 *
 *	Chen Yongjian @ zhoi
 *	2015.3.2 @ Xi'an
 *
 *
 ******************************************************************************/


#ifndef __MY_RUDP_SERVER_INTERFACE_H_20150302__
#define __MY_RUDP_SERVER_INTERFACE_H_20150302__

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#pragma pack( push, 8 )

class CMyThreadPool;
class CMyThreadPoolTask;
class CMyRUDP_EventObj;
class CMyRUDP_OnePeerObjectInterface;

//////////////////////////////////////////////////////////////////////
class CMyRUDP_ServerEvent
{
public:
	CMyRUDP_ServerEvent(){}
	virtual ~CMyRUDP_ServerEvent(){}

	virtual CMyRUDP_EventObj * OnNewConnection( CMyRUDP_OnePeerObjectInterface * pPeerObj ) = 0;
};

//////////////////////////////////////////////////////////////////////
class CMyRUDP_ServerDrv
{
public:
	CMyRUDP_ServerDrv(){}
	virtual ~CMyRUDP_ServerDrv(){}

	//--------------------------------------------------------------
	/** CYJ 2015-01-12
	 *
	 *	Create One UDP socket and bind to the local IP, and preset data
	 *
	 * @param [in]	nThreadCount		Thread count
	 * @param [in]	pThreadPoolObj		thread pool object
	 * @param [in]	pEventObj			event object
	 * @param [in]	nPort				listen port
	 * @param [in]	pszLocalBindIP		local bind IP, if NULL, listen on all network interface
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	virtual int Open( int nThreadCount, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP = NULL ) = 0;
	virtual int Open( CMyThreadPool * pThreadPoolObj, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP = NULL ) = 0;

	//--------------------------------------------------------------
	/** CYJ 2015-01-12
	 *
	 *	Close the socket and exit the working thead
	 *
	 * send 2 MYRUDP_CLOSE data to all connections but not wait for ACK
	 */
	virtual void Close() = 0;

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
	virtual int SendTo( const unsigned char *pBuf, int nLen, const struct sockaddr * pDstAddr, socklen_t nAddrLen ) = 0;


	//--------------------------------------------------------------
	/** CYJ 2015-02-25
	 *
	 *	Get thread pool object
	 *
	 * @return	thread pool object
	 */
	virtual CMyThreadPool * GetThreadPoolObject() = 0;

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
    virtual int AddRef() = 0;
    virtual int Release() = 0;
};

///////////////////////////////////////////////////////////////////
class CMyRUDP_OnePeerObjectInterface
{
public:
	CMyRUDP_OnePeerObjectInterface(){}
	virtual ~CMyRUDP_OnePeerObjectInterface(){}

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Get working state, see also PEER_OBJECT_STATE_xxx
	 *
	 * @return	working state
	 */
	virtual int GetState()const = 0;

	//--------------------------------------------------------------
    /** CYJ 2015-02-12
     *
     *	Send data
     *
     * @param [in]	pBuf				data buffer
     * @param [in]	nLen				data length
     * @param [in]	pUserData			user data, used for notify data has been send succ.
     * @param [in]	nTimeOut			timeout seconds
     *									0  		if no enough buffer then return immediately
     *									-1		wait infinitely
     *									other	time out seconds
     * @return		0					succ
     *				other				error code
     */
    virtual int Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut ) = 0;

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
    virtual int AddRef() = 0;
    virtual int Release() = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-03-12
     *
     *	Get Associated Server Drv
     */
    virtual CMyRUDP_ServerDrv * GetServerDrv() = 0;

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
    virtual int SetForceSendingKeepAliveInterval( int nValue ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-04-23
     *
     *	Get Keep alive counter = seconds from last getting data from peer
     *
     * @return	seconds from last time getting data from peer
     *
     */
    virtual int GetKeepAliveCount() const = 0;

    enum
	{
		PEER_OBJECT_STATE_IDLE = 0,				// the session ID can be used
		PEER_OBJECT_STATE_DISCONNECTED,			// the sesstion is disconnect, if there are availabe sesstion, then keep this sesstion for re-connect fast
		PEER_OBJECT_STATE_CONNECTING = 0x10,	// peer is connecting to the server
		PEER_OBJECT_STATE_CONNECTED,			// the session is connected
	};
};

#pragma pack( pop )

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Create Server driver object
 *
 * @return	My RUDP server driver object
 */
CMyRUDP_ServerDrv * MyRUDP_CreateServerDrv();

#endif // __MY_RUDP_SERVER_INTERFACE_H_20150302__

