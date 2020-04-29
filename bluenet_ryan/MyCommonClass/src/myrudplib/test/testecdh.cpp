#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <memory.h>
#include <pthread.h>
#include <openssl/crypto.h>

#include "../src/myrudp_openssl.h"

const int TEST_COUNT = 1000*50;
const int THREAD_COUNT = 4;

CMyRUDPCryption g_ServerObj;

void * decrypt_thread( void * pArgs )
{
	CMyRUDPCryption & server = g_ServerObj;
	char cIndicator = (((uint64_t)(pArgs)) & 0x1F)+ '0';

    for(int i=0; i<TEST_COUNT; i++ )
	{
		if( (i%50) == 0 )
			fprintf( stderr, "%c", cIndicator );

		CMyRUDPCryption client;

		assert( 0 == client.Initialize() );

		const std::vector<uint8_t> & aClientPubKey = client.GetPubKey();
		assert( aClientPubKey.size() );

		const std::vector<uint8_t> & aServerPubKey = server.GetPubKey();
		assert( aServerPubKey.size() );

		std::vector<uint8_t> ClientAESKey = client.DecryptAESKey( aServerPubKey.data(), aServerPubKey.size() );
		assert( ClientAESKey.size() );
		std::vector<uint8_t> ServerAESKey = server.DecryptAESKey( aClientPubKey.data(), aClientPubKey.size() );
		assert( ServerAESKey.size() );

		assert( ServerAESKey.size() == ClientAESKey.size() );
		assert( 0 == memcmp( ServerAESKey.data(), ClientAESKey.data(), ClientAESKey.size() ) );
	}

	return NULL;
}

int main( int argc, char * argv[])
{
	srand( time(NULL) );
	MyRUDP_OpenSSL_Multithread_Init();

	assert( 0 == g_ServerObj.Initialize() );

	time_t tStart = time(NULL);

	pthread_t aTheadID[ THREAD_COUNT ];
	memset( aTheadID, 0, sizeof(aTheadID) );

	int i;
	for(i=0; i<THREAD_COUNT; i++ )
	{
		pthread_create( aTheadID+i, NULL, decrypt_thread, (void*)i );
	}
	for(i=0; i<THREAD_COUNT; i++ )
	{
		void * pRetVal;
		pthread_join( aTheadID[i], &pRetVal );
	}

	time_t tEnd = time(NULL);
	fprintf( stderr, "Average time: %.4f second\n", (double)(tEnd-tStart)/(2*THREAD_COUNT*TEST_COUNT) );

	return 0;
}
