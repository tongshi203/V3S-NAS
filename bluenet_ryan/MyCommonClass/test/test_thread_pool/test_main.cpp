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

#include <my_log_print.h>

extern void test_thread_pool();


int main( int argc, char * argv[] )
{
	MyLog_Init( "/tmp/shared" );

	test_thread_pool();

	MyLog_Exit();

	return 0;
}

