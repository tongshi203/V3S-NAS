/*************************************************
 *
 * pthread mutex helper class
 *
 * Chen Yongjan @ Tongshi
 * 2011.11.25 @ Xi'an
 *
 ****************************************************/

#ifndef __MY_PTHREAD_MUTEX_HELPER_H_20111125__
#define __MY_PTHREAD_MUTEX_HELPER_H_20111125__

#include <pthread.h>
#include <sched.h>		// 2015.4.1 CYJ Add

// 2016.12.17 CYJ Add
#pragma pack(push,8)

class CMy_pthread_Mutex_Locker
{
public:
	CMy_pthread_Mutex_Locker( pthread_mutex_t * pMutex, bool bDoLock = true )
	{
		m_pMutex = pMutex;
		m_bIsLocked = false;
		if( bDoLock && pMutex )
			Lock();
	}

	virtual ~CMy_pthread_Mutex_Locker()
	{
		Unlock();
	}

	bool TryLock()
	{
		if( NULL == m_pMutex || m_bIsLocked )
			return m_bIsLocked;
		if( 0 == pthread_mutex_trylock( m_pMutex ) )
			m_bIsLocked = true;
		return m_bIsLocked;
	}

	void Lock()
	{
		if( NULL == m_pMutex || m_bIsLocked )
			return;
		if( 0 == pthread_mutex_lock( m_pMutex ) )
			m_bIsLocked = true;
	}

	void Unlock()
	{
		if( NULL == m_pMutex || false == m_bIsLocked )
			return;
		pthread_mutex_unlock( m_pMutex );
		m_bIsLocked = false;
//		sched_yield();	// 2015.4.1 CYJ Modify, using sched_yield instead of pthread_yield();
	}

	bool IsLocked()
	{
		return m_bIsLocked;
	}
protected:
	pthread_mutex_t * m_pMutex;
	bool m_bIsLocked;
};

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // __MY_PTHREAD_MUTEX_HELPER_H_20111125__


