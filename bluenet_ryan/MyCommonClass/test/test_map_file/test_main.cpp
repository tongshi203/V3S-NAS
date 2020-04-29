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
#include <assert.h>

#include <my_map_file.h>
#include <string.h>
#include "my_memory_block_mgr.h"

/////////////////////////////////////////////////
CMyMemoryBlockManager g_MMBM;
const int64_t TEST_FILE_SIZE = 4L*1024L*1024*1024;		// 1G

void * TestMMBMMultiThread( void * pArg)
{
	CMyMemoryBlockManager & mmbm = g_MMBM;
	int nBlockCount = mmbm.GetTotalItemCount();
	int nThreadIndex = (int)(int64_t)pArg;
	char cIndicator = '1' + nThreadIndex;

	std::list<uint32_t>	aListAllocate;
	for(int i=0; i<10*1000*1000; i++ )
	{
		if( (i % 1000) == 0 )
			fprintf( stderr, "%c", cIndicator );
		uint32_t dwBlockID;
		bool bDoAllocate = (rand()&1) ? true : false;

		if( bDoAllocate )
		{
			dwBlockID = mmbm.Allocate();
			if( dwBlockID )
				aListAllocate.push_back( dwBlockID );
		}
		else if( false == aListAllocate.empty() )
		{
			dwBlockID = aListAllocate.front();
			aListAllocate.pop_front();
			mmbm.Free( dwBlockID );
		}
	}

	// save used item block ID
	std::list<uint32_t>::iterator it;
	for( it=aListAllocate.begin(); it!=aListAllocate.end(); it++)
	{
		uint32_t dwBlockID = *it;
		mmbm.Free( dwBlockID );
	}
}


int main( int argc, char * argv[] )
{
	int64_t llSize = TEST_FILE_SIZE;

	srand( time(NULL) );

#if 0
	fprintf( stderr, "Open file ...." );

	CMyMappedFile mapfile;

	int nRetVal = mapfile.Open( "a.dat", llSize, false );
	if( nRetVal )
	{
		fprintf( stderr, "Map file failed.\n" );
		return 1;
	}

	uint8_t *pBuf = mapfile.GetBuffer( 0 );
	fprintf( stderr, "Mapped buffer = %p\n", pBuf );

	memset( pBuf, 0x55, 100 );
	pBuf = mapfile.GetBuffer( llSize-100 );
	memset( pBuf, 0xAA, 100 );

	mapfile.SyncToFile( 0, 100 );


	mapfile.SyncToFile( 0, 100 );

#endif // 0


	int64_t llBlockSize = 1024;
	int64_t llHeadSize = 40*1024*1024;


	uint8_t *pBuf = new uint8_t[ llSize ];	// 1 GB
	int nBlockNumber = (TEST_FILE_SIZE-llHeadSize) / llBlockSize;
	assert( nBlockNumber * 4 < llHeadSize );
	assert( 0 == g_MMBM.Initialize( pBuf, TEST_FILE_SIZE, llHeadSize, llBlockSize ) );

	pthread_t aThreadID[8];
	for(int i=0; i<sizeof(aThreadID)/sizeof(aThreadID[0]); i++ )
	{
		pthread_create( aThreadID+i, NULL, TestMMBMMultiThread, (void*)i );
	}
	for(int i=0; i<sizeof(aThreadID)/sizeof(aThreadID[0]); i++ )
	{
		void * pRetVal = NULL;
		pthread_join( aThreadID[i], &pRetVal );
	}

	g_MMBM.DebugCheckUsedFreeItems();

	g_MMBM.Invalidate();

	delete pBuf;
	return 0;
}

