/******************************************************************
 *
 *	My RUDP Client interface
 *
 *	Chen Yongjian @ Zhoi
 *	2015.3.2 @ Xi'an
 *
 *******************************************************************/

#ifndef __MY_RUDP_CLIENT_INTERFACE_H_20150302__
#define __MY_RUDP_CLIENT_INTERFACE_H_20150302__

class CMyRUDP_EventObj;

#pragma pack( push, 8 )

class CMyRUDP_ClientDrv
{
public:
	CMyRUDP_ClientDrv(){}
	virtual ~CMyRUDP_ClientDrv(){}

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
	virtual int Open( const char * pszServerIP, int nPort, CMyRUDP_EventObj * pEventObj ) = 0;

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
	virtual int SetServerIPAndPort( const char * pszServerIP, int nPort ) = 0;

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Disconnect from the server and free resources
	 */
	virtual void Close() = 0;

	//--------------------------------------------------------------
	/** CYJ 2015-01-29
	 *
	 *	Get working state, see also MYRUDP_CLIENT_STATE_xxx
	 *
	 * @return	working state
	 */
	virtual int GetState() = 0;

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
     *									-2		if no enough buffer, return without waiting, otherwise send with MUST SUCC
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
    /** CYJ 2015-04-03
     *
     *	Set keep alive packet interval, in seconds
     *
     * @param [in]	nInterval			keep alive packet interval, in seconds
     *									default is 10 seconds
     *									[ 10 - 300 ]
     */
    virtual void SetKeepAliveInterval( int nInterval ) = 0;

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
    virtual bool SetNetowrkAvailable( bool bEnable ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-04-09
     *
     *	Check network station and sent keep alive packet, is disconnected, reconnect it
     *
     * @note
     *		For Android, sometimes the thread is wake up very long, so using this function to check the connection
     */
    virtual void CheckNetworkConnection() = 0;

	enum
	{
		MYRUDP_CLIENT_STATE_DISCONNECTED = 0,	// idle or disconnected
		MYRUDP_CLIENT_STATE_WAIT_RSP,			// has sent REQUEST, wait Rsp
		MYRUDP_CLIENT_STATE_WAIT_SYNC_2,		// has sent RESPONSE, wait for SYN2, 2015.5.4 CYJ Add
		MYRUDP_CLIENT_STATE_CONNECTED,			// connected
	};
};

#pragma pack( pop )

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Create Client driver
 *
 * @note
 *		call Release function to release the object instance
 */
CMyRUDP_ClientDrv * MyRUDP_CreateClientDrv();

#endif // __MY_RUDP_CLIENT_INTERFACE_H_20150302__

