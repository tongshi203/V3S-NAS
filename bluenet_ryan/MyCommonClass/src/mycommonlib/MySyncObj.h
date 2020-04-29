/////////////////////////////////////////////////////
// My Synchronize Object For Linux
//
//      Chen Yongjian @ Xi'an Tongshi technology Limited.
//      2003.8.4
//----------------------------------------------------

#ifndef __MY_SYNCHRONIZE_OBJ_H_20030804__
#define __MY_SYNCHRONIZE_OBJ_H_20030804__

#pragma pack(push,4)

#include <semaphore.h>

#ifndef __USE_GNU
  #define __USE_GNU
#endif //__USE_GNU

#ifndef __USE_XOPEN2K
	#define __USE_XOPEN2K
	#include <pthread.h>
	#undef __USE_XOPEN2K
#else
	#include <pthread.h>
#endif // __USE_XOPEN2K

#define InterlockedIncrement( pValue )			__sync_add_and_fetch( pValue, 1 )
#define InterlockedDecrement( pValue )			__sync_sub_and_fetch( pValue, 1 )
#define InterlockedExchange( pTarget, nValue )	__sync_lock_test_and_set( pTarget, nValue )
#define InterlockedExchangeAdd(pAddend,nValue)	__sync_fetch_and_add( pAddend, nValue )

//-------------------------------------------------
// Base SyncObj, define common method
class CMySyncObjBase
{
    friend class CSingleLock;
public:
    CMySyncObjBase();
    ~CMySyncObjBase();

    virtual bool Lock( long nTimeOut = -1 ) = 0;
    virtual bool Unlock() = 0;
    bool IsValid(){ return m_bIsInitOK; }

protected:
    bool m_bIsInitOK;       // Is Initialized success
};

//---------------------------------------------------
class CSemaphore : public CMySyncObjBase
{
public:
    virtual bool Lock( long nTimeOut = -1 );
    virtual bool Unlock();

    CSemaphore( int nInitialCount = 1 );
    ~CSemaphore();

protected:
    sem_t   m_SyncData;         // data for sync
};

//--------------------------------------------------
class CMutex : public CMySyncObjBase
{
public:
    virtual bool Lock( long nTimeOut = -1 );
    virtual bool Unlock();

    CMutex( bool bInitiallyOwn = false );
    ~CMutex();

    operator pthread_mutex_t* () { return &m_SyncData; }

protected:
    pthread_mutex_t m_SyncData;
};
typedef CMutex CCriticalSection;

//-------------------------------------------
class CEvent : public CMySyncObjBase
{
public:
	CEvent();
	virtual ~CEvent();

	virtual bool Lock( long nTimeOut = -1 );
    virtual bool Unlock(){ return false; }		// not used
    void SetEvent();
	void SetEventToAll();

protected:
	pthread_mutex_t 	m_SyncData;
	pthread_cond_t		m_EventData;
};

//--------------------------------------------
// Read/Write Lock
class CReadWriteLock : public CMySyncObjBase
{
public:
	CReadWriteLock();
	virtual ~CReadWriteLock();
	virtual bool Lock( long nTimeOut = -1 ){ return ReadLock( nTimeOut ); }
    virtual bool Unlock();
    bool ReadLock( long nTimeOut = -1 );
    bool WriteLock( long nTimeOut = -1 );
protected:
	pthread_rwlock_t  m_SyncData;
};

//------------------------------------------
class CSingleLock
{
public:
    CSingleLock( CMySyncObjBase * pObj, bool bInitialLock = false );
    ~CSingleLock();

    virtual bool Lock( long nTimeOut = -1 );
    bool Unlock();
    bool IsLocked(){ return m_bAcquired; }

protected:
    CMySyncObjBase * m_pSyncObj;    // Synchronize object
    bool m_bAcquired;               // Is Locked
};

//----------------------------------------------------
class CSingleLock_ReadWrite : public CSingleLock
{
public:
	CSingleLock_ReadWrite( CReadWriteLock *pSyncObj, bool bWriteMode, bool bInitialLock = false );
	virtual ~CSingleLock_ReadWrite();
	virtual bool Lock( long nTimeOut = -1 );
protected:
	bool m_bMode;
};

#pragma pack(pop)

#endif // __MY_SYNCHRONIZE_OBJ_H_20030804__
