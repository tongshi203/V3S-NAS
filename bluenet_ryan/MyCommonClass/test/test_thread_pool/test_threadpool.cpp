/********************************************************************
 *
 *	Test Thread Pool
 *
 *
 *	Chen Yongjian @ zhoi
 *	2014.10.22 @ Xi'an
 *
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <mythreadpool.h>
#include <MySyncObj.h>

#define THREAD_COUNT 	100

int g_nTaskCount = 0;

class CMyTestTask : public CMyThreadPoolTask
{
public:
	CMyTestTask( int nID ){ m_nID = nID; m_nRefCount = 0; }
	virtual ~CMyTestTask(){}

	virtual void RunTask()
	{
		fprintf( stderr, " [ %5d ] ", m_nID );
		g_nTaskCount ++;
		if( ( g_nTaskCount & 15 ) == 15 )
			fprintf( stderr, "\n" );
		int nTimeLen = (rand() & 0xF ) + 1;
		usleep( 1000 * nTimeLen );
	}

	virtual int AddRef(){ return InterlockedIncrement( &m_nRefCount ); }
	virtual int Release()
	{
		int nRetVal = InterlockedDecrement( &m_nRefCount );
		assert( nRetVal >= 0 );
		if( 0 == nRetVal )
		{
			assert( 0 == m_nDeputeCounter );
			delete this;
		}
		return nRetVal;
	}

	int m_nID;
	int m_nRefCount;
};

class CMyTestDelayTask : public CMyTestTask
{
public:
	CMyTestDelayTask( int nID ) : CMyTestTask( nID )
	{
		fprintf( stderr, "\nAdd Delay Task now: %ld.\n", time(NULL) );
	}
	virtual void RunTask()
	{
		fprintf( stderr, "\nDelay Task ( %d ) MS, now: %ld.\n", m_nID, time(NULL) );
	}
};

//--------------------------------------------------------------
/** CYJ 2014-10-22
 *
 *	Test Thread Pool
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void test_thread_pool()
{
	CMyThreadPool thread_pool;
	int i;
	CMyTestTask *pTask;

	fprintf( stderr, "------------- %s : %d, init thread pool ------\n", __FUNCTION__, __LINE__ );
	if( thread_pool.Initialize( THREAD_COUNT ) )
	{
		fprintf( stderr, "Failed to init thread pool\n" );
		return;
	}

	fprintf( stderr, "------------- %s : %d, Test Delay Task ------\n", __FUNCTION__, __LINE__ );
	for( i=0; i<1000; i++ )
	{
		pTask = new CMyTestTask( i );
		thread_pool.AddTask( pTask );
	}

	pTask = new CMyTestDelayTask( 1000 );
	thread_pool.AddDelayTask( pTask, 1000 );
	pTask = new CMyTestDelayTask( 10000 );
	thread_pool.AddDelayTask( pTask, 10000 );

	fprintf( stderr, "------------- %s : %d, add 2 delay tasks (1 and 10 seconds)------\n", __FUNCTION__, __LINE__ );
	fprintf( stderr, "------------- %s : %d, Wait 20 s------\n", __FUNCTION__, __LINE__ );
	sleep( 20 );

	fprintf( stderr, "------------- %s : %d, add %d tasks ------\n", __FUNCTION__, __LINE__, THREAD_COUNT*200 );
	for( i=0; i<THREAD_COUNT*200; i++ )
	{
		pTask = new CMyTestTask( i );
		thread_pool.AddTask( pTask );
	}

	fprintf( stderr, "------------- %s : %d, add some tasks and remove it ------\n", __FUNCTION__, __LINE__ );
	pTask = new CMyTestTask( 9999 );
	thread_pool.AddTask( pTask );
	thread_pool.AddTask( pTask );
	thread_pool.AddTask( pTask );
	thread_pool.RemoveTask( pTask );

	fprintf( stderr, "------------- %s : %d, remove all tasks ------\n", __FUNCTION__, __LINE__ );
	thread_pool.RemoveAllTask();

	fprintf( stderr, "------------- %s : %d, add %d tasks again ------\n", __FUNCTION__, __LINE__ ,THREAD_COUNT*2000);
	for( i=0; i<THREAD_COUNT*2000; i++ )
	{
		pTask = new CMyTestTask( i+1000 );
		thread_pool.AddTask( pTask, i&1 );
	}

	fprintf( stderr, "------------- %s : %d, wait 10 seconds ------\n", __FUNCTION__, __LINE__ );
	sleep( 10 );

	fprintf( stderr, "------------- %s : %d, Invalidate all tasks ------\n", __FUNCTION__, __LINE__ );
	thread_pool.Invalidate();
}
