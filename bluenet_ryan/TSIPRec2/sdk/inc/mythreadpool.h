/*************************************************************************************
 *
 *	My Thread Pool
 *
 *	Chen Yongjian @ Zhoi
 *	2014.10.22 @ Xi'an
 *
 ************************************************************************************/


#ifndef __MY_THREAD_POOL_H_20141022__
#define __MY_THREAD_POOL_H_20141022__

#include <pthread.h>
#include <vector>
#include <list>
#include <stdint.h>

#pragma pack( push, 8 )

/**
 * @name MyThreadPool
 * @{
 */

/////////////////////////////////////////////////////////////////
class CMyThreadPoolTask
{
public:
	CMyThreadPoolTask();
	virtual ~CMyThreadPoolTask();

	//--------------------------------------------------------------
	/** CYJ 2014-10-22
	 *
	 *	Run task in the thread
	 */
	virtual void RunTask() = 0;

	//--------------------------------------------------------------
	// live reference counter
	virtual int AddRef() = 0;
    virtual int Release() = 0;

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
    int DuputeTask();

    //--------------------------------------------------------------
    /** CYJ 2015-04-05
     *
     *	完成任务
     *
     * @return	还剩下的委托任务，如果为0，则表示全部任务已经完成，反之，继续执行剩余的任务
     */
    int OnTaskDone();

    //--------------------------------------------------------------
    /** CYJ 2015-04-05
     *
     *	设置是否允许并行计算
     *
     * @param [in]	bEnable				true			运行并行计算
     *									false			禁止并行
     */
    void EnableConcurrentRunning( bool bEnable ){ m_bEnableConcurrentRunning = bEnable; }

    //--------------------------------------------------------------
    /** CYJ 2015-04-05
     *
     *	查询是否允许并行计算
     *
     * @return 		true			运行并行计算
     *				false			禁止并行
     */
	bool IsEnableConcurrentRunning()const { return m_bEnableConcurrentRunning; }
protected:
	int 	m_nDeputeCounter;				// default value = 0
	bool	m_bEnableConcurrentRunning;		// default value = false
};

/////////////////////////////////////////////////////////////////
class CMyThreadPool
{
public:
	CMyThreadPool();
	virtual ~CMyThreadPool();

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
	int Initialize( int nThreadCount = 4 );

	//--------------------------------------------------------------
	/** CYJ 2014-10-22
	 *
	 *	Stop all threads and free them
	 */
	void Invalidate();

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
	 * @note
	 *		The caller has duty to free the pTask
	 */
	int AddTask( CMyThreadPoolTask * pTask, bool bHighPriority = false );

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
	int AddDelayTask( CMyThreadPoolTask * pTask, uint32_t dwTimeOut );

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
	void RemoveTask( CMyThreadPoolTask * pTask );

	//--------------------------------------------------------------
	/** CYJ 2014-10-22
	 *
	 *	remove all tasks that has not been executed
	 */
	void RemoveAllTask();

	//--------------------------------------------------------------
	// live reference counter
	virtual int AddRef();
    virtual int Release();

protected:
	static void * RunLink( void * pThis );
	void Run();
	static uint64_t GetTickCount();
	int MoveReadyTimeoutTaskToTaskList();
	static void * DelayTaskScheduleRunLink( void * pThis );
	void DelayTaskScheduleRun();

	enum
	{
		THREAD_STATE_IDLE = 0,
		THREAD_STATE_RUNNING,
		THREAD_STATE_REQ_EXIT,
		THREAD_STATE_EXIT,
	};

	typedef struct tagDELAY_TASK_ITEM
	{
		uint64_t			m_llTimeOutMs;
		CMyThreadPoolTask*	m_pTask;
	}DELAY_TASK_ITEM;

protected:
	std::vector<pthread_t> 			m_aThread;
	std::list<CMyThreadPoolTask*> 	m_aTasks;
	std::list<DELAY_TASK_ITEM> 		m_aDelayTasks;		// 2015.4.21 CYJ Add
	volatile int 					m_nState;
	pthread_mutex_t 				m_SyncObj;
	pthread_cond_t					m_Event;
	int								m_nRefCount;
};

#pragma pack( pop )

/**
 * @}
 */

#endif  // __MY_THREAD_POOL_H_20141022__

