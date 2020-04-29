/******************************************************************
 *
 *	My RUDP Sync Debug fprintf( stderr, .... )
 *
 *	Chen Yongjian @ Zhoi
 *	2015.2.16 @ Xi'an
 *
 ****************************************************************/


#ifndef __MY_RUDP_DEBUG_PRINTF_H_20150216__
#define __MY_RUDP_DEBUG_PRINTF_H_20150216__

void MyRUDP_fprintf( const char *pszFormat, ... );
void MyRUDP_fprintf_lock();
void MyRUDP_fprintf_unlock();


class CMyRUDP_fprintf_SyncHelper
{
public:
	CMyRUDP_fprintf_SyncHelper()
	{
		MyRUDP_fprintf_lock();
	}

	virtual ~CMyRUDP_fprintf_SyncHelper()
	{
		MyRUDP_fprintf_unlock();
	}
};

#endif // __MY_RUDP_DEBUG_PRINTF_H_20150216__
