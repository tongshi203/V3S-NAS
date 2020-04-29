/***************************************************************************************
 *
 *	RelaxTalk Open SSL ( ECDH )
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.13 @ Xi'an
 *
 **************************************************************************************/

#include <stdafx.h>

#include <openssl/aes.h>
#include <openssl/ecdh.h>
#include <openssl/err.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>
#include <openssl/crypto.h>
#include <openssl/obj_mac.h>

#include "myrudp_openssl.h"
#include "myrudp_dbgprint.h"


/////////////////////////////////////////////////////////////////////////////
static pthread_rwlock_t	* 	s_apMyRUDP_OpenSSL_SyncObj = NULL;
static int					s_nMyRUDP_OpenSSL_SyncObjCount = 0;

//--------------------------------------------------------------
static void MyRUDP_locking_callback(int mode, int type, const char *file, int line )
{
#ifdef _DEBUG
	assert( s_apMyRUDP_OpenSSL_SyncObj );
	assert( type < s_nMyRUDP_OpenSSL_SyncObjCount );
#endif // _DEBUG

	if( mode & CRYPTO_LOCK )
	{
        if( mode & CRYPTO_READ )
			pthread_rwlock_rdlock( s_apMyRUDP_OpenSSL_SyncObj + type );
		else
			pthread_rwlock_wrlock( s_apMyRUDP_OpenSSL_SyncObj + type );
    }
    else
		pthread_rwlock_unlock( s_apMyRUDP_OpenSSL_SyncObj + type );
}

//--------------------------------------------------------------
static unsigned long MyRUDP_thread_id(void)
{
    return (unsigned long)pthread_self();
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Initialize OpenSSL multi-thread support
 *
 * @return	0				succ
 *			other			error code
 */
int MyRUDP_OpenSSL_Multithread_Init()
{
	MyRUDP_OpenSSL_Multithread_Cleanup();

	int nCount = CRYPTO_num_locks();
	if( nCount <= 0 )
		return 0;
	s_apMyRUDP_OpenSSL_SyncObj = new pthread_rwlock_t[ nCount ];
	if( NULL == s_apMyRUDP_OpenSSL_SyncObj )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, allocate memory for pthread_rwlock_t[%d] failed.\n", __FUNCTION__, nCount );
	#endif // _DEBUG
		return ENOMEM;
	}

	for(int i=0; i<nCount; i++ )
	{
		pthread_rwlock_init( s_apMyRUDP_OpenSSL_SyncObj + i, NULL );
	}

	s_nMyRUDP_OpenSSL_SyncObjCount = nCount;

	CRYPTO_set_id_callback( MyRUDP_thread_id );
    CRYPTO_set_locking_callback( MyRUDP_locking_callback );

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Cleanup OpenSSL multi-thread support
 */
void MyRUDP_OpenSSL_Multithread_Cleanup()
{
	if( s_nMyRUDP_OpenSSL_SyncObjCount && s_apMyRUDP_OpenSSL_SyncObj )
	{
		for( int i=0; i<s_nMyRUDP_OpenSSL_SyncObjCount; i++ )
		{
			pthread_rwlock_destroy( s_apMyRUDP_OpenSSL_SyncObj + i );
		}

		delete []s_apMyRUDP_OpenSSL_SyncObj;
	}

	s_apMyRUDP_OpenSSL_SyncObj = NULL;
	s_nMyRUDP_OpenSSL_SyncObjCount = 0;
}


/////////////////////////////////////////////////////////////////////////////
CMyRUDPCryption::CMyRUDPCryption()
{
	m_pKey = NULL;
	m_pPoint = NULL;
	m_pGroup = NULL;
	m_byKeyType = MYRUDP_ECDH_TYPE_INVALID;
}

CMyRUDPCryption::~CMyRUDPCryption()
{
	Invalidate();
}

//--------------------------------------------------------------
/** CYJ 2015-04-13
 *
 *	Initialize ECDH key objct
 *
 * @return		0				succ
 *				other			error code
 */
int CMyRUDPCryption::Initialize( uint8_t byType )
{
	Invalidate();

	int nNID;						// default, for server side, should prepare more type
	if( MYRUDP_ECDH_TYPE_521 == byType )
	{
		nNID = NID_secp521r1;
		m_byKeyType = MYRUDP_ECDH_TYPE_521;
	}
	else
	{
		nNID = NID_secp384r1;		// default, for server side, should prepare more type
		m_byKeyType = MYRUDP_ECDH_TYPE_384;
	}

	//Generate Public
    m_pKey = EC_KEY_new_by_curve_name( nNID );
    if( NULL == m_pKey )
    {
    #ifdef _DEBUG
		MyRUDP_fprintf( "EC_Key_New failed.\n" );
	#endif // _DEBUG
		return EIO;
    }

	bool bGenerateKeySucc = false;
	for(int i=0; i<1000; i++ )
	{
		if( 0 == EC_KEY_generate_key( m_pKey ) )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "%s, EC_KEY_generate_key failed (%d), try again\n", __FUNCTION__, ERR_get_error() );
		#endif //_DEBUG
			continue;
		}
		bGenerateKeySucc = true;
		break;
	}
    if( false == bGenerateKeySucc )
    {
	#ifdef _DEBUG
		MyRUDP_fprintf( "EC_KEY_generate_key failed.\n" );
	#endif // _DEBUG
		Invalidate();
		return 2;
    }

    // build public key
	int nRetVal = BuildPublicKey();
	if( nRetVal )
	{
	#ifdef _DEBUG
		assert( false );
	#endif // _DEBUG

		Invalidate();
		return nRetVal;
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Get Public Key
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
int CMyRUDPCryption::BuildPublicKey()
{
    // ECC key generate succ, now get public key, peer pointer, group
    m_pPoint = EC_KEY_get0_public_key( m_pKey );
    if( NULL == m_pPoint )
    {
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, EC_KEY_get0_public_key failed, err code: %d\n", __FUNCTION__, ERR_get_error() );
	#endif //_DEBUG
		return 3;
    }

    // set group
    m_pGroup = EC_KEY_get0_group( m_pKey );
    if( NULL == m_pGroup )
    {
    #ifdef _DEBUG
		MyRUDP_fprintf( "%s, EC_KEY_get0_group failed, err code: %d\n", __FUNCTION__, ERR_get_error() );
	#endif //_DEBUG
		return 4;
    }

    // get public key
    int nTmpECDHPubKeyLen = EC_POINT_point2oct( m_pGroup, m_pPoint, POINT_CONVERSION_COMPRESSED, NULL, 0, NULL );
    if( 0 == nTmpECDHPubKeyLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, EC_POINT_point2oct failed, err code: %d\n", __FUNCTION__, ERR_get_error() );
	#endif //_DEBUG
		return 5;
	}

	uint32_t nBufSizeNeed = nTmpECDHPubKeyLen + 1;
    m_aPubKey.resize( nBufSizeNeed );
    uint8_t * pOutKeyBuf = &m_aPubKey.front();
    if( m_aPubKey.size() != nBufSizeNeed || NULL == pOutKeyBuf )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, resize public key array failed\n", __FUNCTION__ );
	#endif //_DEBUG
		return 6;
	}

	// first byte is key type
	*pOutKeyBuf = m_byKeyType;
	pOutKeyBuf ++;

	nTmpECDHPubKeyLen = EC_POINT_point2oct( m_pGroup, m_pPoint, POINT_CONVERSION_COMPRESSED, pOutKeyBuf, nTmpECDHPubKeyLen, NULL );
	if( 0 == nTmpECDHPubKeyLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, EC_POINT_point2oct failed, err code: %d\n", __FUNCTION__, ERR_get_error() );
	#endif //_DEBUG
		return 7;
	}

    return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-13
 *
 *	Invalidate and free resource
 */
void CMyRUDPCryption::Invalidate()
{
	if( m_pKey )
	{
		EC_KEY_free( m_pKey );
		m_pKey = NULL;
	}
	m_pPoint = NULL;
	m_pGroup = NULL;
	m_byKeyType = MYRUDP_ECDH_TYPE_INVALID;

	m_aPubKey.clear();
}


//--------------------------------------------------------------
/** CYJ 2015-04-13
 *
 *	Decrypt AES Key
 *
 * @param [in]	pPeerPubKey		Peer Public Key
 * @param [in]	nLen			Peer Public Key Length
 *
 * @return	AES Key
 *
 * @note
 *		First byte is ECDH key type, see also MYRUDP_ECDH_TYPE_xxx
 */
std::vector<uint8_t> CMyRUDPCryption::DecryptAESKey( const uint8_t * pPeerPubKey, int nLen )
{
#ifdef _DEBUG
	assert( pPeerPubKey && nLen > 1 );
#endif // _DEBUG

	std::vector<uint8_t> aRetAESKey;

	if( NULL== pPeerPubKey || nLen < 2 )
		return aRetAESKey;

#ifdef _DEBUG
	assert( *pPeerPubKey == m_byKeyType );
#endif //_DEBUG
	if( *pPeerPubKey != m_byKeyType )
		return aRetAESKey;

	// skip key type.
	pPeerPubKey ++;
	nLen --;

	//ComputeKey
	EC_POINT * point_peer = EC_POINT_new( m_pGroup );
	if( NULL == point_peer )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "Failed to EC_POINT_new, err code: %d\n", ERR_get_error() );
	#endif //_DEBUG
		return aRetAESKey;
	}

	if( 0 == EC_POINT_oct2point( m_pGroup, point_peer, pPeerPubKey, nLen, NULL ) )
	{
		EC_POINT_free( point_peer );
	#ifdef _DEBUG
		MyRUDP_fprintf( "Failed to EC_POINT_new, err code: %d\n", ERR_get_error() );
	#endif //_DEBUG
		return aRetAESKey;
	}

	unsigned char abySharedKey[ 256 ];
	uint32_t nShareKeyLen = ECDH_compute_key( abySharedKey, sizeof(abySharedKey), point_peer, m_pKey, NULL );
	EC_POINT_free( point_peer );
	if( nShareKeyLen <= 0 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "Failed to ECDH_compute_key, err code: %d\n", ERR_get_error() );
	#endif //_DEBUG
		return aRetAESKey;
	}

	aRetAESKey.resize( nShareKeyLen );
	if( aRetAESKey.size() == nShareKeyLen)
		memcpy( &aRetAESKey.front(), abySharedKey, nShareKeyLen );
	else
		aRetAESKey.clear();

	return aRetAESKey;
}
