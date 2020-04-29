/************************************************************************
 *
 *	Test main
 *
 * Chen Yongjian @ zhoi
 * 2014.10.22 @ xi'an
 *
 ***********************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <my_log_print.h>
#include <MySyncObj.h>


CReadWriteLock	s_ReadWriteSyncObj;
int				s_nDataVal_1 = 1;
int				s_nDataVal_2 = 2;


void * thread_work_read( void *  )
{
//	CSingleLock SyncObj( &s_ReadWriteSyncObj, true );
	int nID = (int)pthread_self();
	while( 1 )
	{
		CSingleLock_ReadWrite SyncObj( &s_ReadWriteSyncObj, false, true );

		fprintf( stderr, "[%d] s_nDataVal = %d : %d\n", nID, s_nDataVal_1, s_nDataVal_2 );

		SyncObj.Unlock();
		usleep( (rand() & 0xFF) * 10000 );
	}

	return NULL;
}

void * thread_work_write( void * )
{
	while( 1 )
	{
		CSingleLock_ReadWrite SyncObj( &s_ReadWriteSyncObj, true, true );

		s_nDataVal_1 = rand() &0xFF;
		s_nDataVal_2 = rand() &0xFF;
		fprintf( stderr, "set s_nDataVal = %d : %d\n", s_nDataVal_1, s_nDataVal_2 );

		sleep( 2 );

		SyncObj.Unlock();

		sleep( rand() & 7 );
	}
	return NULL;
}

int main( int argc, char * argv[] )
{
	MyLog_Init( "/tmp/shared" );

	pthread_t write_id;
	pthread_create( &write_id, NULL, thread_work_write, NULL );

	pthread_t read_id;
	for(int i=0; i<10; i++ )
	{
		pthread_create( &read_id, NULL, thread_work_read, NULL );
	}

	while( 1 )
	{
		CSingleLock_ReadWrite SyncObj( &s_ReadWriteSyncObj, true, true );

		s_nDataVal_1 = rand() &0xFF;
		s_nDataVal_2 = rand() &0xFF;
		fprintf( stderr, "Main set s_nDataVal = %d : %d\n", s_nDataVal_1, s_nDataVal_2 );

		sleep( 2 );

		SyncObj.Unlock();

		sleep( rand() & 7 );
	}


	MyLog_Exit();

	return 0;
}

