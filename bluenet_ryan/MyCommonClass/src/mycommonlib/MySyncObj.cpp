/////////////////////////////////////////////////////
// My Synchronize Object For Linux
//
//      Chen Yongjian @ Xi'an Tongshi technology Limited.
//      2003.8.4
//----------------------------------------------------

#ifndef __USE_XOPEN2K
	#define __USE_XOPEN2K
#endif // __USE_XOPEN2K

#include "MySyncObj.h"
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sched.h>

//---------------------------------------
//	MySleep, unit ms
static void MySleepMs( int nMs )
{
	usleep( nMs*1000 );
}

//-------------------------------------------------
// Base SyncObj, define common method
CMySyncObjBase::CMySyncObjBase()
{
    m_bIsInitOK = false;
}

CMySyncObjBase::~CMySyncObjBase()
{
}

//---------------------------------------------------
//  CSemaphore Initializer
//  Input parameter
//      nInitialCount       default is 1
CSemaphore::CSemaphore( int nInitialCount /* = 1 */ )
{
#ifdef _DEBUG
    assert( nInitialCount > 0 );
#endif //_DEBUG
    if( nInitialCount <= 0 )
        nInitialCount = 1;
    m_bIsInitOK = ( sem_init( &m_SyncData, 0, nInitialCount ) == 0 );
}

CSemaphore::~CSemaphore()
{
	#ifdef _DEBUG
		assert( m_bIsInitOK );
	#endif //_DEBUG

    if( m_bIsInitOK )
        sem_destroy( &m_SyncData );
}

//-------------------------------------------------
//  Lock Samaphore
//  Input parameter
//      nTimeOut        time out value, in MS
//  Return parameter
//      true            succ
//      false           time out or do create synchronize object
bool CSemaphore::Lock( long nTimeOut /* = -1*/ )
{
	#ifdef _DEBUG
		assert( m_bIsInitOK );
	#endif //_DEBUG

    if( !m_bIsInitOK )
        return false;

    if( nTimeOut < 0 )        // wait until acquire
         return sem_wait( &m_SyncData ) == 0;

    for(int i=0; i<=nTimeOut; i++)
    {
        if( sem_trywait( &m_SyncData ) == 0 )
            return true;
        MySleepMs( 1 );        // sleep 1 ms
    }
    return false;
}

//------------------------------------------------
//  Unlock
bool CSemaphore::Unlock()
{
	#ifdef _DEBUG
		assert( m_bIsInitOK );
	#endif //_DEBUG

    if( !m_bIsInitOK )
        return false;

    return sem_post( &m_SyncData ) == 0;
}

//----------------------------------------------
//  Input parameter
//      bInitiallyOwn       true call Lock
CMutex::CMutex( bool bInitiallyOwn /*= false */ )
{
     pthread_mutexattr_t attr;
     pthread_mutexattr_init( &attr );
	 pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );

     pthread_mutex_init( &m_SyncData, &attr );

     m_bIsInitOK = true;

     if( bInitiallyOwn )
         Lock();
}

CMutex::~CMutex()
{
	pthread_mutex_destroy( &m_SyncData );
}

//-------------------------------------------------
//  Lock Mutex
//  Input parameter
//      nTimeOut        time out value, in MS
//  Return parameter
//      true            succ
//      false           time out or do create synchronize object
bool CMutex::Lock( long nTimeOut /*= -1*/ )
{
	return ( 0 == pthread_mutex_lock( &m_SyncData ) );
}

//-------------------------------------------------
bool CMutex::Unlock()
{
	pthread_mutex_unlock( &m_SyncData );
	// 2015.4.1 CYJ Modify, replace pthread_yield with sched_yield
	sched_yield();	// 2011.6.4 CYJ Add, to let other thread has chance
	return true;
}

//------------------------------------------
CSingleLock::CSingleLock( CMySyncObjBase * pObj, bool bInitialLock /*= false*/ )
{
#ifdef _DEBUG
    assert( pObj );
#endif //_DEBUG
    m_bAcquired = false;
    m_pSyncObj = pObj;
    if( bInitialLock )
        Lock();
}

CSingleLock::~CSingleLock()
{
    if( m_bAcquired )
        Unlock();
}

//-------------------------------------------------
//  Lock Synchronize object
//  Input parameter
//      nTimeOut        time out value, in MS
//  Return parameter
//      true            succ
//      false           time out or do create synchronize object
bool CSingleLock::Lock( long nTimeOut /*= -1*/ )
{
#ifdef _DEBUG
    assert( false == m_bAcquired );
    assert( m_pSyncObj );
#endif //_DEBUG
    if( NULL == m_pSyncObj )
        return false;
    m_bAcquired = m_pSyncObj->Lock( nTimeOut );
    return m_bAcquired;
}

//----------------------------------------
//  unlock
//  return parameter
//      true            succ
//      false           failure
bool CSingleLock::Unlock()
{
#ifdef _DEBUG
    assert( m_bAcquired && m_pSyncObj );
#endif //_DEBUG
    if( NULL == m_pSyncObj )
        return false;
    if( m_bAcquired )
    {
        m_bAcquired = false;
        return m_pSyncObj->Unlock();
    }
    return true;
}

/////////////////////////////////////////////////////
// 2015.3.2 CYJ Add
CEvent::CEvent()
{
	pthread_mutex_init( &m_SyncData, NULL );

// 2015.4.1 CYJ, porting to Android
#ifndef ANDROID
	pthread_condattr_t	EventAttrData;
	pthread_condattr_init( &EventAttrData );
	pthread_condattr_setclock( &EventAttrData, CLOCK_MONOTONIC );
	pthread_cond_init( &m_EventData, &EventAttrData );
	pthread_condattr_destroy( &EventAttrData );
#endif //ANDROID

	m_bIsInitOK = true;
}

CEvent::~CEvent()
{
	pthread_cond_destroy( &m_EventData );
	pthread_mutex_destroy( &m_SyncData );
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	wait event
 *
 * @param [in]	nTimeout		-1			wait infinitely
 *								other		timeout ms
 * @return		true			succ
 *				false			failed
 */
bool CEvent::Lock( long nTimeOut )
{
	pthread_mutex_lock( &m_SyncData );

	int nRetVal;
	if( nTimeOut < 0 )
		nRetVal = pthread_cond_wait( &m_EventData, &m_SyncData );
	else
	{
		struct timespec tv;
		// 2015.4.1 CYJ, porting to Android, but Android can not using CLOCK_MONOTONIC
	#ifndef ANDROID
		clock_gettime( CLOCK_MONOTONIC, &tv );
	#else
		clock_gettime( CLOCK_REALTIME, &tv );
	#endif
		nTimeOut += ( tv.tv_nsec / 1000000L );
		tv.tv_sec += ( nTimeOut / 1000L );
		tv.tv_nsec = ( nTimeOut%1000 ) * 1000000L;
		nRetVal = pthread_cond_timedwait( &m_EventData, &m_SyncData, &tv );
	}

	pthread_mutex_unlock( &m_SyncData );

	return ( 0 == nRetVal );
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Signal event
 */
void CEvent::SetEvent()
{
	pthread_mutex_lock( &m_SyncData );
	pthread_cond_signal( &m_EventData );
	pthread_mutex_unlock( &m_SyncData );
	// 2015.4.1 CYJ Modify, replace pthread_yield with sched_yield
	sched_yield();
}

//--------------------------------------------
void CEvent::SetEventToAll()
{
	pthread_cond_broadcast( &m_EventData );
}

//--------------------------------------------
// Read/Write Lock
CReadWriteLock::CReadWriteLock()
{
	pthread_rwlock_init( &m_SyncData, NULL );
	m_bIsInitOK = true;
}

CReadWriteLock::~CReadWriteLock()
{
	pthread_rwlock_destroy( &m_SyncData );
}

//--------------------------------------------------------------
/** CYJ 2015-03-09
 *
 *	Unlock the lock
 *
 * @return	true				succ
 *			false				failed
 */
bool CReadWriteLock::Unlock()
{
	pthread_rwlock_unlock( &m_SyncData );
	// 2015.4.1 CYJ Modify, replace pthread_yield with sched_yield
	sched_yield();
}

//--------------------------------------------------------------
/** CYJ 2015-03-09
 *
 *	Read Lock
 *
 * @param [in]	nTimeOut		timeout in MS
 *
 * @return		true			succ
 *				false			failed
 */
bool CReadWriteLock::ReadLock( long nTimeOut )
{
	int nRetVal;

	if( nTimeOut < 0 )
		nRetVal = pthread_rwlock_rdlock( &m_SyncData );
	else
	{
		struct timespec tv;
		clock_gettime( CLOCK_REALTIME, &tv );
		nTimeOut += ( tv.tv_nsec / 1000000L );
		tv.tv_sec += ( nTimeOut / 1000L );
		tv.tv_nsec = ( nTimeOut%1000 ) * 1000000L;

		nRetVal = pthread_rwlock_timedrdlock( &m_SyncData, &tv );
	}

	return 0 == nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-03-09
 *
 *	Write Lock
 *
 * @param [in]	nTimeOut		timeout in MS
 *
 * @return		true			succ
 *				false			failed
 */
bool CReadWriteLock::WriteLock( long nTimeOut )
{
	int nRetVal;

	if( nTimeOut < 0 )
		nRetVal = pthread_rwlock_wrlock( &m_SyncData );
	else
	{
		struct timespec tv;
		clock_gettime( CLOCK_REALTIME, &tv );
		nTimeOut += ( tv.tv_nsec / 1000000L );
		tv.tv_sec += ( nTimeOut / 1000L );
		tv.tv_nsec = ( nTimeOut%1000 ) * 1000000L;

		nRetVal = pthread_rwlock_timedwrlock( &m_SyncData, &tv );
	}

	return 0 == nRetVal;
}

//----------------------------------------------------
CSingleLock_ReadWrite::CSingleLock_ReadWrite( CReadWriteLock *pSyncObj, bool bWriteMode, bool bInitialLock )
	: m_bMode( bWriteMode ),
	CSingleLock( pSyncObj, false )
{
	if( bInitialLock )
		Lock();
}

CSingleLock_ReadWrite::~CSingleLock_ReadWrite()
{
}

//--------------------------------------------------------------
/** CYJ 2015-03-09
 *
 *	Lock
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
bool CSingleLock_ReadWrite::Lock( long nTimeOut )
{
	if( NULL == m_pSyncObj )
        return false;

	if( m_bMode )
		m_bAcquired = ((CReadWriteLock *)m_pSyncObj)->WriteLock( nTimeOut );
	else
		m_bAcquired = ((CReadWriteLock *)m_pSyncObj)->ReadLock( nTimeOut );

	return m_bAcquired;
}
