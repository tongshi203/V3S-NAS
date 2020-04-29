/******************************************************************
 *
 *	My RUDP Sync Debug fprintf( stderr, .... )
 *
 *	Chen Yongjian @ Zhoi
 *	2015.2.16 @ Xi'an
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <unistd.h>

#include <my_pthread_mutex_help.h>

#ifdef ANDROID
	#include <android/log.h>
#else
	#include <my_log_print.h>
#endif // ANDROID


#include "myrudp_dbgprint.h"


//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	fprintf with sync locked
 */
void MyRUDP_fprintf( const char *pszFormat, ... )
{
	va_list fmt_args;
    va_start( fmt_args, pszFormat );     /* Initialize variable arguments. */

#ifdef ANDROID
	__android_log_vprint( ANDROID_LOG_DEBUG, "MYRUDP", pszFormat, fmt_args );
#else
	MyRUDP_fprintf_lock();

    vfprintf( stderr, pszFormat, fmt_args );

	MyRUDP_fprintf_unlock();
#endif //#ifdef ANDROID

	va_end( fmt_args );
}

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Lock frintf
 */
void MyRUDP_fprintf_lock()
{
#ifndef ANDROID
	MyLog_Lock();
#endif // #ifndef ANDROID
}

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Unlock fprintf
 */
void MyRUDP_fprintf_unlock()
{
#ifndef ANDROID
	MyLog_Unlock();
#endif //#ifndef ANDROID
}
