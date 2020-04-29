/************************************************************************************************
 *
 *	My RUDP OpenSSL server side helper
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.18 @ Xi'an
 *
 ************************************************************************************************/

#include <stdafx.h>
#include <errno.h>
#include <openssl/aes.h>
#include <openssl/md5.h>

#include "myrudp_dbgprint.h"

#include "myrudp_openssl.h"
#include "myrudp_openssl_svr_helper.h"

#ifdef _DEBUG
extern void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen );
#endif //#ifdef _DEBUG

CMyRUDPOpenSSLServerHelper::CMyRUDPOpenSSLServerHelper()
{
	memset( m_apOpenSSLObj, 0, sizeof(m_apOpenSSLObj) );
}

CMyRUDPOpenSSLServerHelper::~CMyRUDPOpenSSLServerHelper()
{
	Invalidate();
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Create server side public key object
 *
 * @return	0					succ
 *			other				error code
 */
int CMyRUDPOpenSSLServerHelper::Initialize()
{
	Invalidate();

	int nRetVal = MyRUDP_OpenSSL_Multithread_Init();
	if( nRetVal )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "MyRUDP_OpenSSL_Multithread_Init()=%d, failed\n", nRetVal );
	#endif // _DEBUG
		return nRetVal;
	}


	for(int i=0; i<MAX_OPENSSL_OBJECT_NUMBER; i++ )
	{
		m_apOpenSSLObj[i] = new CMyRUDPCryption;
		if( NULL == m_apOpenSSLObj[i] )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "Failed to allocate memory for CMyRUDPCryption\n" );
		#endif // _DEBUG

			return ENOMEM;
		}

		nRetVal = m_apOpenSSLObj[i]->Initialize( (uint8_t)(i+1) );
		if( nRetVal )
		{
			delete m_apOpenSSLObj[i];
			m_apOpenSSLObj[i] = NULL;
		#ifdef _DEBUG
			MyRUDP_fprintf( "Failed to Init CMyRUDPCryption(%d)\n", i+1 );
		#endif // _DEBUG
			return EIO;
		}
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Free resource
 */
void CMyRUDPOpenSSLServerHelper::Invalidate()
{
	for(int i=0; i<MAX_OPENSSL_OBJECT_NUMBER; i++ )
	{
		if( m_apOpenSSLObj[i] )
			delete m_apOpenSSLObj[i];
	}
	memset( m_apOpenSSLObj, 0, sizeof(m_apOpenSSLObj) );

	MyRUDP_OpenSSL_Multithread_Cleanup();
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	On date chaned, should renew server side public key
 *
 * @param [in]	tNow			current time
 */
void CMyRUDPOpenSSLServerHelper::OnDateChanged( time_t tNow )
{
    CSingleLock_ReadWrite SyncObj( &m_SyncObj, true, true );

    for(int i=0; i<MAX_OPENSSL_OBJECT_NUMBER; i++ )
	{
		if( m_apOpenSSLObj[i] )
			m_apOpenSSLObj[i]->Initialize( (uint8_t)(i+1) );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Get encrypted servier side public key
 *
 * @param [in]	byType			ECC type
 * @param [in]	pbyXorData		Xor data to encrypted the server side public key
 * @param [in]	nSessionID		session ID, used as AES IV
 *
 * @return		encrypted server side public key
 *
 * @note
 *		output data length format:
 *	<public data > < PadLen 1 Byte>
 */
std::vector<uint8_t> CMyRUDPOpenSSLServerHelper::GetEncryptedPubKey( uint8_t byType, const uint8_t pbyXorData[], int nSessionID )
{
	std::vector<uint8_t> aRetVal;

	byType --;
	if( byType >= MAX_OPENSSL_OBJECT_NUMBER )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%s] Error peer data type: %d \n", __FUNCTION__, byType + 1 );
	#endif // _DEBUG
		return aRetVal;
	}

	CSingleLock_ReadWrite SyncObj( &m_SyncObj, false, true );

	CMyRUDPCryption *pObj = m_apOpenSSLObj[byType];
#ifdef _DEBUG
	assert( pObj );
#endif // _DEBUG
	if( NULL == pObj )
		return aRetVal;

	const std::vector<uint8_t> aPubKey = pObj->GetPubKey();
#ifdef _DEBUG
	assert( aPubKey.size() );
//	DbgDumpData( "ORIGIANL SVR PUBLIC KEY:", aPubKey.data(), aPubKey.size() );
#endif // _DEBUG

	int nEncryptedDataLen = ( aPubKey.size() + 15 ) & (~15);
	uint8_t byPaddingLen = (uint8_t)( nEncryptedDataLen - aPubKey.size() );
	uint32_t nLenNeed = nEncryptedDataLen + 1;	// last byte is the padding length

	aRetVal.resize( nLenNeed );
	if( aRetVal.size() != nLenNeed )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%s] Failed to allocate memory for Output Public Key\n", __FUNCTION__ );
	#endif // _DEBUG
		aRetVal.clear();
		return aRetVal;
	}

	unsigned char abyAESKey[32];
	unsigned char abyIV[16];

	byPaddingLen |= (abyAESKey[0] & 0xF0);		// set high 4 bit to a random data

	memcpy( abyAESKey, pbyXorData, 32 );
	abyAESKey[25] ^= 0xff;
	abyAESKey[15] ^= 0x78;
	abyAESKey[5] &= 0xA5;
	memset( abyIV, pbyXorData[0], 16 );
	abyIV[0] = (uint8_t)(nSessionID>>8);
	abyIV[1] = (uint8_t)(nSessionID);
	abyIV[7] = (uint8_t)(nSessionID);
	abyIV[15] = pbyXorData[9];

#ifdef _DEBUG
//	DbgDumpData( "AES - 256 Key:", abyAESKey, sizeof(abyAESKey) );
//	DbgDumpData( "AES - 256 IV:", abyIV, sizeof(abyIV) );
#endif //_DEBUG


	AES_KEY aes;
	AES_set_encrypt_key( abyAESKey, 256, &aes);
	AES_cbc_encrypt( aPubKey.data(), aRetVal.data(), nEncryptedDataLen, &aes, abyIV, 1 );	// do encrypt

	aRetVal[ nEncryptedDataLen ] = byPaddingLen;

#ifdef _DEBUG
//	DbgDumpData( "ENCRYPTED SVR PUBLIC KEY:", aRetVal.data(), aRetVal.size() );
#endif // _DEBUG

	return aRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Get AES Key
 *
 * @param [in]	pbyPeerPubKey		peer public key, first by is type
 * @param [in]	nLen				peer public key length
 * @param [in]	pbyXorData			Xor data
 * @param [out]	byECCType			output ECC type
 *
 * @return	AES Key
 *
 * @note
 *		pbyPeerPubKey	is encrypted AES-128, using XorData
 */
std::vector<uint8_t> CMyRUDPOpenSSLServerHelper::GetAESKey( const uint8_t * pbyPeerPubKey, int nLen, const uint8_t pbyXorData[], uint8_t & byECCType  )
{
	std::vector<uint8_t> aRetAESKey;

#ifdef _DEBUG
	assert( nLen > 17 );
#endif // _DEBUG

	if( nLen < 17 || ( (nLen&0xF) != 1 ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%s] error encrypted peer key length (%d bytes)\n", __FUNCTION__, nLen );
	#endif // _DEBUG
		return aRetAESKey;
	}

	nLen --;
    uint8_t byPaddingLen = pbyPeerPubKey[nLen] & 0xF;
#ifdef _DEBUG
	assert( (nLen & 0xF) == 0 );
#endif // _DEBUG

	// decrypt peer public key data
	std::vector<uint8_t> aPeerPubKey;
    aPeerPubKey.resize( nLen );
    if( aPeerPubKey.size() != (uint32_t)nLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%s] Failed to allocate memory for aPeerPubKey\n", __FUNCTION__ );
	#endif // _DEBUG
		return aRetAESKey;
	}

	unsigned char abyAESKey[16];
	unsigned char abyIV[16];
	memcpy( abyAESKey, pbyXorData, 16 );
	memcpy( abyIV, pbyXorData+16, 16 );

	abyAESKey[3] = 0x67;
	abyAESKey[7] ^= 0x83;
	abyAESKey[12] &= 0x51;
	abyIV[8] = 0x22;
	abyIV[5] ^= 0xFF;
	abyIV[11] ^= 0x88;

#ifdef _DEBUG
//	DbgDumpData( "AES 128 KEY:", abyAESKey, sizeof(abyAESKey) );
//	DbgDumpData( "AES 128 IV:", abyIV, sizeof(abyIV) );
//	DbgDumpData( "ENCRYPTED CLIENT PUBLIC KEY:", pbyPeerPubKey, nLen );
#endif // _DEBUG

	AES_KEY aes;
	AES_set_decrypt_key( abyAESKey, 128, &aes);
	AES_cbc_encrypt( pbyPeerPubKey, aPeerPubKey.data(), nLen, &aes, abyIV, 0 );	// decrypt

    nLen -= byPaddingLen;
    aPeerPubKey.resize( nLen );

#ifdef _DEBUG
//	DbgDumpData( "DECRYPTED CLIENT PUBLIC KEY:", aPeerPubKey.data(), aPeerPubKey.size() );
#endif //_DEBUG

    pbyPeerPubKey = aPeerPubKey.data();
    uint8_t byType = pbyPeerPubKey[0];
    byECCType = byType;
    byType --;

	if( byType >= MAX_OPENSSL_OBJECT_NUMBER )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%s] Error peer data type \n", __FUNCTION__, byType + 1 );
	#endif // _DEBUG
		return aRetAESKey;
	}

    CSingleLock_ReadWrite SyncObj( &m_SyncObj, false, true );

    CMyRUDPCryption *pObj = m_apOpenSSLObj[byType];
#ifdef _DEBUG
	assert( pObj );
#endif // _DEBUG
	if( NULL == pObj )
		return aRetAESKey;

#ifdef _DEBUG
	aRetAESKey = pObj->DecryptAESKey( pbyPeerPubKey, nLen );
//	DbgDumpData( "AES KEY:", aRetAESKey.data(), aRetAESKey.size() );
	return aRetAESKey;
#else
	return pObj->DecryptAESKey( pbyPeerPubKey, nLen );
#endif	// _DEBUG
}
