/**************************************************************************************
 *
 *	My RUDP server thread pool
 *
 *
 *	Chen Yongjian @ zhoi
 *	2015.2.21 2 Xi'an
 *
 *	Server thread pool to process data received or data send event
 *
 *
 *************************************************************************************/

#ifndef __MY_RUDP_SVR__THREAD_POOL_H_20150221__
#define __MY_RUDP_SVR__THREAD_POOL_H_20150221__

#include <mythreadpool.h>
#include <assert.h>
#include "myrudp_event.h"

class CMyRUDP_OnePeerObject;

//////////////////////////////////////////////////
class CMyRUDPDataSendTask : public CMyThreadPoolTask
{
public:
	CMyRUDPDataSendTask( CMyRUDP_OnePeerObject * pObject )
	{
		m_pPeerObj = pObject;
	}
	virtual ~CMyRUDPDataSendTask(){}

	virtual void RunTask()
	{
		m_pPeerObj->ThreadTask_OnDataSend();
	}
	virtual int AddRef()
	{
		return m_pPeerObj->AddRef();
	}
    virtual int Release()
    {
    	return m_pPeerObj->Release();
    }
protected:
	CMyRUDP_OnePeerObject * m_pPeerObj;
};

//////////////////////////////////////////////////
class CMyRUDPDataReceiveTask : public CMyThreadPoolTask
{
public:
	CMyRUDPDataReceiveTask( CMyRUDP_OnePeerObject * pObject )
	{
		m_pPeerObj = pObject;
	}
	virtual ~CMyRUDPDataReceiveTask(){}

	virtual void RunTask()
	{
		m_pPeerObj->ThreadTask_OnDataReceived();
	}
	virtual int AddRef()
	{
		return m_pPeerObj->AddRef();
	}
    virtual int Release()
    {
    	return m_pPeerObj->Release();
    }
protected:
	CMyRUDP_OnePeerObject * m_pPeerObj;
};

////////////////////////////////////////////////////////////
class CMyRUDPNotifyDisconnectEventTask : public CMyThreadPoolTask
{
public:
	CMyRUDPNotifyDisconnectEventTask( CMyRUDP_OnePeerObject * pObject )
	{
		m_pPeerObj = pObject;
	}
	virtual ~CMyRUDPNotifyDisconnectEventTask(){}

	virtual void RunTask()
	{
		CMyRUDP_EventObj *pEventObj = m_pPeerObj->m_pEventObj;
		if( pEventObj )
			pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_DISCONNECTED );
	}
	virtual int AddRef()
	{
		return m_pPeerObj->AddRef();
	}
    virtual int Release()
    {
    	return m_pPeerObj->Release();
    }
protected:
	CMyRUDP_OnePeerObject * m_pPeerObj;
};


#endif // __MY_RUDP_SVR__THREAD_POOL_H_20150221__

