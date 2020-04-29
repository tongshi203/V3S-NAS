/*************************************************************************************
 *
 *	My Thread Pool
 *
 *	Chen Yongjian @ Zhoi
 *	2014.10.22 @ Xi'an
 *
 ************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>

#include <my_log_print.h>
#include <my_pthread_mutex_help.h>
#include <MySyncObj.h>

#include "mythreadpool.h"

/////////////////////////////////////////////////////////////////
CMyThreadPoolTask::CMyThreadPoolTask()
{
	m_nDeputeCounter = 0;
	m_bEnableConcurrentRunning = false;
}

CMyThreadPoolTask::~CMyThreadPoolTask()
{

}

//--------------------------------------------------------------
/** CYJ 2015-04-05
 *
 *	委托任务
 *
 * @return	当前对象承载的委托任务个数		1			该任务对象仅包含一个任务，没有重复
 *										>1			该对象重复N个任务，每个任务指向同一个对象
 * @note
 *	  有时为了编程简单，多个任务指向同一个对象，且同一个任务不能并行执行，如此可能造成多个线程在等待同一个任务
 * 导致线程资源浪费。
 *	  如果任务允许并行执行，则一般不进行任务委托；反之，进行任务委托，委托完成后立即返回，
 * 被委托的第一个线程执行完后，继续执行委托的任务。
 */
int CMyThreadPoolTask::DuputeTask()
{
	return InterlockedIncrement( &m_nDeputeCounter );
}

//--------------------------------------------------------------
/** CYJ 2015-04-05
 *
 *	完成任务
 *
 * @return	还剩下的委托任务，如果为0，则表示全部任务已经完成，反之，继续执行剩余的任务
 */
int CMyThreadPoolTask::OnTaskDone()
{
#ifdef _DEBUG
	int nRetVal = InterlockedDecrement( &m_nDeputeCounter );
	assert( nRetVal >= 0 );
	return nRetVal;
#else
	return InterlockedDecrement( &m_nDeputeCounter );
#endif
}

/////////////////////////////////////////////////////////////////
CMyThreadPool::CMyThreadPool()
{
	m_nState = THREAD_STATE_IDLE;
	m_nRefCount = 1;

	pthread_mutex_init( &m_SyncObj, NULL);
	pthread_cond_init( &m_Event, NULL);
}

CMyThreadPool::~CMyThreadPool()
{
	Invalidate();

	pthread_cond_destroy( &m_Event );
	pthread_mutex_destroy( &m_SyncObj );
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Create the thread pool and start schedule the threads
 *
 * @param [in]	nThreadCount	the thread count to be created for this thread pool
 *
 * @return		0				succ
 *-				other			error code
 * @note
 *		Max thread count supported by linux can be consult in /proc/sys/kernel/thread-max
 */
int CMyThreadPool::Initialize( int nThreadCount )
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyThreadPool::%s(%d) called.\n", __FUNCTION__, nThreadCount );
#endif // _DEBUG

	Invalidate();

	if( nThreadCount <= 0 )
		nThreadCount = 4;			// default is 4 threads

	m_nState = THREAD_STATE_RUNNING;

	// 2015.4.21 CYJ Add, to support delay task
	pthread_t tid;
	int nRetVal = pthread_create( &tid, NULL, DelayTaskScheduleRunLink, (void*)this );
	if( nRetVal )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyThreadPool::%s Failed to create delay task thread, errno=%d.\n", __FUNCTION__, errno );
		return errno;
	}
	m_aThread.push_back( tid );

	 // TODO: COnsider lazy loading threads instead of creating all at once
	for( int i=0; i<nThreadCount; i++ )
	{
		int nRetVal = pthread_create( &tid, NULL, RunLink, (void*)this );
		if( nRetVal )
		{
			MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyThreadPool::%s Failed to create thread, errno=%d.\n", __FUNCTION__, errno );
			return errno;
		}

		m_aThread.push_back( tid );
	}

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyThreadPool::%s, %d threads created succ.\n", __FUNCTION__, nThreadCount );
#endif // _DEBUG

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Stop all threads and free them
 */
void CMyThreadPool::Invalidate()
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyThreadPool::%s called.\n", __FUNCTION__ );
#endif // _DEBUG

	m_nState = THREAD_STATE_REQ_EXIT;

	int nCount = m_aThread.size();
	for( int i=0; i<nCount; i++ )
	{
		pthread_cond_broadcast( &m_Event ); // try waking up a bunch of threads that are still waiting

		void* result;
		pthread_join( m_aThread[i], &result );
	}

	// 2015.4.4 CYJ add
	RemoveAllTask();

	m_aThread.clear();

	m_nState = THREAD_STATE_EXIT;

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyThreadPool::%s, Leave.\n", __FUNCTION__ );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-04-21
 *
 *	Delay task thread
 */
void * CMyThreadPool::DelayTaskScheduleRunLink( void * pThis )
{
	((CMyThreadPool*)pThis)->DelayTaskScheduleRun();
	return NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-04-21
 *
 *	Delay task thread
 */

void CMyThreadPool::DelayTaskScheduleRun()
{
	while( THREAD_STATE_RUNNING == m_nState )
	{
		// Try to pick a task
		pthread_mutex_lock( &m_SyncObj );
		int nRetVal = MoveReadyTimeoutTaskToTaskList();
		pthread_mutex_unlock( &m_SyncObj );

		if( nRetVal )
			pthread_cond_signal( &m_Event );

		sched_yield();
		usleep( 10*1000L );
	}
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Add one task to the thread pool and the task will be executed as soon as possible
 *
 * @param [in]	pTask			task to be add
 * @param [in]	bHighPriority	true 	insert the task to the front of the queue
 *								false 	append the task to the end of the queue
 * @return		0				succ
 *-				other			error code
 */
int CMyThreadPool::AddTask( CMyThreadPoolTask * pTask, bool bHighPriority )
{
#ifdef _DEBUG
	assert( pTask );
#endif // _DEBUG

	if( NULL == pTask )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyThreadPool::%s pTask is NULL.\n", __FUNCTION__ );
		return EINVAL;
	}

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	// 2015.4.4 CYJ Add, to avoid the task has been delete when the thread to execute it
	pTask->AddRef();

	// FIXME, should using try...cache to avoid no enough memory exception
	if( bHighPriority )
		m_aTasks.push_front( pTask );
	else
		m_aTasks.push_back( pTask );

	pthread_cond_signal( &m_Event );

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-21
 *
 *	Add delayed task
 *
 * @param [in]	pTask			task object
 * @param [in]	dwTimeOut		delay time, in MS
 *
 * @return		0				succ
 *				other			error code
 */
int CMyThreadPool::AddDelayTask( CMyThreadPoolTask * pTask, uint32_t dwTimeOut )
{
#ifdef _DEBUG
	assert( pTask );
#endif // _DEBUG

	if( NULL == pTask )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyThreadPool::%s pTask is NULL.\n", __FUNCTION__ );
		return EINVAL;
	}

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	// 2015.4.4 CYJ Add, to avoid the task has been delete when the thread to execute it
	pTask->AddRef();

	DELAY_TASK_ITEM NewItem;
	NewItem.m_llTimeOutMs = dwTimeOut + GetTickCount();
	NewItem.m_pTask = pTask;

	// 2015.4.21 CYJ add delayed task
	std::list<DELAY_TASK_ITEM>::reverse_iterator it_delay;
	for( it_delay=m_aDelayTasks.rbegin(); it_delay != m_aDelayTasks.rend(); ++it_delay )
	{
		DELAY_TASK_ITEM & Item = *it_delay;
		if( Item.m_llTimeOutMs > NewItem.m_llTimeOutMs )
			continue;
		break;
	}
	m_aDelayTasks.insert( it_delay.base(), NewItem );

#ifdef _DEBUG
	uint64_t llLastTick = 0;
	for( std::list<DELAY_TASK_ITEM>::iterator it=m_aDelayTasks.begin(); it!=m_aDelayTasks.end(); ++it )
	{
		assert( llLastTick <=  (*it).m_llTimeOutMs );
		llLastTick =  (*it).m_llTimeOutMs;
	}
#endif // _DEBUG

	MoveReadyTimeoutTaskToTaskList();

	pthread_cond_signal( &m_Event );
}

//--------------------------------------------------------------
/** CYJ 2015-04-21
 *
 *	Get Tick count
 *
 * @return	tick count, ms since 1970.1.1 00:00:00
 */
uint64_t CMyThreadPool::GetTickCount()
{
    struct timespec tv;
	// 2015.4.1 CYJ, porting to Android, but Android can not using CLOCK_MONOTONIC
#ifndef ANDROID
	clock_gettime( CLOCK_MONOTONIC, &tv );
#else
	clock_gettime( CLOCK_REALTIME, &tv );
#endif

	uint64_t llRetVal = tv.tv_sec;
	llRetVal = llRetVal * 1000 + ( tv.tv_nsec / 1000000L);

	return llRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-04-21
 *
 *	Move the delay task that has been timeout to m_aTask
 *
 * @return
 *		Task count move from delay list to task list
 * @note
 *	assume the caller has been synchronized
 */
int CMyThreadPool::MoveReadyTimeoutTaskToTaskList()
{
	int nRetVal = 0;
	uint64_t llCurTick = GetTickCount();

	while( false == m_aDelayTasks.empty() )
	{
		DELAY_TASK_ITEM & Item = m_aDelayTasks.front();
		if( Item.m_llTimeOutMs > llCurTick )
			break;
		m_aTasks.push_back( Item.m_pTask );
		m_aDelayTasks.erase( m_aDelayTasks.begin() );
		nRetVal ++;
	}
	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Remove the task from the queue if it has not been executed
 *
 * @param [in]	pTask			the task to be removed
 *
 * @note
 *		if there are more than one task in the queue, all task will be removed
 */
void CMyThreadPool::RemoveTask( CMyThreadPoolTask * pTask )
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

#ifdef _DEBUG
//	fprintf( stderr, "%ld items in the list before remove.\n", m_aTasks.size() );
#endif // _DEBUG

	// 2015.4.4 CYJ Modify
	std::list<CMyThreadPoolTask*>::iterator it;
	for( it=m_aTasks.begin(); it != m_aTasks.end(); )
	{
		CMyThreadPoolTask* pTmpItem = *it;
		if( pTask != pTmpItem )
		{
			++it;
			continue;
		}

	#ifdef _DEBUG
		assert( pTmpItem );
		assert( pTask->Release() >= 0 );
	#else
		pTask->Release();
	#endif // _DEBUG

		it = m_aTasks.erase( it );
	}

#ifdef _DEBUG
//	fprintf( stderr, "%ld items in the list after remove.\n", m_aTasks.size() );
	for( std::list<CMyThreadPoolTask*>::iterator it=m_aTasks.begin(); it!=m_aTasks.end(); ++it )
	{
		assert( *it != pTask );
	}
#endif // _DEBUG

	// 2015.4.21 CYJ add delayed task
	std::list<DELAY_TASK_ITEM>::iterator it_delay;
	for( it_delay=m_aDelayTasks.begin(); it_delay != m_aDelayTasks.end(); )
	{
		DELAY_TASK_ITEM & Item = *it_delay;
		if( Item.m_pTask != pTask )
		{
			++it_delay;
			continue;
		}

	#ifdef _DEBUG
		assert( Item.m_pTask );
		assert( pTask->Release() >= 0 );
	#else
		pTask->Release();
	#endif // _DEBUG

		it_delay = m_aDelayTasks.erase( it_delay );
	}

#ifdef _DEBUG
//	fprintf( stderr, "%ld items in the list after remove.\n", m_aTasks.size() );
	for( std::list<DELAY_TASK_ITEM>::iterator it=m_aDelayTasks.begin(); it!=m_aDelayTasks.end(); ++it )
	{
		assert( (*it).m_pTask != pTask );
	}
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	remove all tasks that has not been executed
 */
void CMyThreadPool::RemoveAllTask()
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	// 2015.4.4 CYJ Modify
	std::list<CMyThreadPoolTask*>::iterator it;
	for( it=m_aTasks.begin(); it != m_aTasks.end(); ++it )
	{
		CMyThreadPoolTask* pTask = *it;

	#ifdef _DEBUG
		assert( pTask );
		assert( pTask->Release() >= 0 );
	#else
		pTask->Release();
	#endif // _DEBUG

	}

	m_aTasks.clear();		// clear all

	// 2015.4.21 CYJ add delayed task
	std::list<DELAY_TASK_ITEM>::iterator it_delay;
	for( it_delay=m_aDelayTasks.begin(); it_delay != m_aDelayTasks.end(); ++it_delay )
	{
		DELAY_TASK_ITEM & Item = *it_delay;
		CMyThreadPoolTask* pTask = Item.m_pTask;

	#ifdef _DEBUG
		assert( pTask );
		assert( pTask->Release() >= 0 );
	#else
		pTask->Release();
	#endif // _DEBUG
	}

	m_aDelayTasks.clear();
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Thread run link
 *
 * @param [in]	pThis			class pointer
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void * CMyThreadPool::RunLink( void * pThis )
{
	( (CMyThreadPool*)pThis )->Run();
	return NULL;
}

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Threads run function
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void CMyThreadPool::Run()
{
	while( THREAD_STATE_RUNNING == m_nState )
	{
		// Try to pick a task
		pthread_mutex_lock( &m_SyncObj );

		// We need to put pthread_cond_wait in a loop for two reasons:
		// 1. There can be spurious wakeups (due to signal/ENITR)
		// 2. When mutex is released for waiting, another thread can be waken up
		// from a signal/broadcast and that thread can mess up the condition.
		// So when the current thread wakes up the condition may no longer be
		// actually true!
		while( THREAD_STATE_RUNNING == m_nState && m_aTasks.empty() )
		{
			// Wait until there is a task in the queue
			// Unlock mutex while wait, then lock it back when signaled

			// FIXME, should check the return value ?
			pthread_cond_wait( &m_Event, &m_SyncObj );
		}

		// If the thread was woken up to notify process shutdown, return from here
		if( THREAD_STATE_RUNNING != m_nState )
		{
			pthread_mutex_unlock( &m_SyncObj );
			return;		// exit the thread
		}

		CMyThreadPoolTask * pTask = m_aTasks.front();
#ifdef _DEBUG
		assert( pTask );
#endif //_DEBUG

		m_aTasks.pop_front();
		pthread_mutex_unlock( &m_SyncObj );

		// 2015.4.1 CYJ Modify, using sched_yield instead of pthread_yield
		sched_yield();

		if( pTask->IsEnableConcurrentRunning() )
			pTask->RunTask();
		else
		{
			// execute the task
			if( 1 == pTask->DuputeTask() )
			{	// if return > 1, then other thread is execute the task
				do
				{
					pTask->RunTask();
				}while( pTask->OnTaskDone() );
			}
		}

	#ifdef _DEBUG
		assert( pTask->Release() >= 0 );
	#else
		pTask->Release();
	#endif
	}
}

//--------------------------------------------------------------
// live reference counter
int CMyThreadPool::AddRef()
{
	return InterlockedIncrement( &m_nRefCount );
}

//--------------------------------------------------------------
// live reference counter

int CMyThreadPool::Release()
{
	int nRetVal = InterlockedDecrement( &m_nRefCount );
#ifdef _DEBUG
	assert( nRetVal >= 0 );
#endif // _DEBUG
	if( nRetVal )
		return nRetVal;
	RemoveAllTask();
	Invalidate();
	delete this;
	return 0;
}
